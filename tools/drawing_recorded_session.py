"""Portable complete observed-EGL interval, without full-journey promotion."""
import json,os,struct,subprocess,tempfile
from pathlib import Path
import numpy as np
from drawing_corpus import ROOT,BUILD,artifact,local,sha,load_tool
from drawing_scene_corpus import raster_inputs,asset_name
from drawing_producer_points import decode_points
from drawing_session import same_json
from recorded_drawing_plan import prepare
CATALOG='tests/fixtures/drawing-recorded-session/session.json'
RETENTION='DRAW02.recorded-session.retention-20261016'
STORAGE_RETENTION='DRAW02.storage-session.retention-20261016'
STORAGE_CATALOG='tests/fixtures/drawing-storage-session/session.json'
def retention_id(catalog):return STORAGE_RETENTION if catalog.get('schema')==2 else RETENTION


def admit(root,catalog,register):
 from rendering_coverage import evidence_record
 evidence={e['id']:e for e in register['evidence']};builds={b['id']:b for b in register['builds']}
 if catalog['schema'] not in (1,2) or type(catalog['schema']) is not int or catalog['source_sha256']!=BUILD or catalog['boundary']!=('observed_egl_action_session' if catalog['schema']==2 else 'observed_egl_session'):raise ValueError('Recorded session schema/build/boundary')
 retained=evidence_record(root,evidence[retention_id(catalog)],builds)
 if not same_json(retained,catalog):raise ValueError('Recorded session differs from immutable retention')
 ev=evidence[catalog['evidence']];binding=evidence[catalog['asset_binding_evidence']]
 if ev['kind'] not in ('isolated_original','live_observation') or binding['kind']!='native_integration' or ev['sha256']!=catalog['original_report_sha256'] or binding['sha256']!=catalog['binding_report_sha256']:raise ValueError('Recorded session needs independent original/binding provenance')
 original=evidence_record(root,ev,builds);bound=evidence_record(root,binding,builds)
 if (original['source_sha256']!=BUILD or any(original.get(k) is not True for k in ('success','normal_original_quit_verified','original_sections_unchanged','original_drawing_active','original_manifest_verified_before_after')) or
     original['original_pixels_used_as_native_inputs'] is not False or bound['original_report']['sha256']!=ev['sha256'] or (bound['success'] is not True and catalog['schema']==1) or bound['original_pixels_used_as_native_inputs'] is not False or not bound['all_observed_presentations_compared']):raise ValueError('Recorded original interval/provenance incomplete')
 if catalog['schema']==2 and (bound.get('native_exit_code')!=0 or bound.get('compared_presentations')!=len(original['egl_physical_observation']['frames']) or bound.get('native_gpu_session',{}).get('report',{}).get('success') is not True):raise ValueError('Retained native comparison did not complete')
 chunks=catalog['input_chunks']
 if not 1<=len(chunks)<=16:raise ValueError('Recorded producer chunk budget')
 raw=b''.join(artifact(root,c) for c in chunks)
 if len(raw)>512*1024*1024 or sha(raw)!=original['producer_observation']['producer_journal_sha256']:raise ValueError('Recorded original producer changed')
 trace={name:artifact(root,entry) for name,entry in catalog['traces'].items()}
 if set(trace)!={'points','journal','intervals','closure'}:raise ValueError('Recorded source trace set')
 if sha(trace['journal'])!=original['observation']['journal_sha256'] or sha(trace['intervals'])!=original['call_intervals']['interval_sha256']:raise ValueError('Recorded original API trace changed')
 parser=load_tool('inspect-canvas-producers');_,records=parser.decode_session(raw,trace['closure'])
 observation=decode_points(trace['points'],trace['journal'],trace['intervals'],records)
 observation.update(producer_journal_sha256=sha(raw),session_producer_marker_sha256=sha(trace['closure']))
 if not same_json(observation,original['producer_observation']):raise ValueError('Recorded source-point acquisition differs')
 rows=list(struct.iter_unpack('<48I',trace['points'][64:-192]));points={r[2]:r[32:48] for r in rows if r[1]==1};destinations={r[2]:r[16:32] for r in rows if r[1]==1};returns={r[2]:r[16:32] for r in rows if r[1]==3}
 comparator=load_tool('compare-drawing-quick-battle');frames,refusals=comparator.frame_bindings(original,points,len(records),destinations,records,returns);comparator.background_bindings(original,points,frames)
 if refusals or len(frames)!=len(original['egl_physical_observation']['frames']) or [f['ordinal'] for f in frames]!=list(range(1,len(frames)+1)):raise ValueError('Recorded presentation history has unbound/missing frames')
 if len(catalog['frames'])!=len(frames) or not 1<=len(frames)<=8192:raise ValueError('Recorded original frame count changed')
 if catalog['schema']==2:
  admit_actions(root,catalog,original,trace['journal'])
  comparator.display_initialization(frames,original['action_bound_presentations'])
 for kept,observed in zip(catalog['frames'],original['egl_physical_observation']['frames']):
  if kept['ordinal']!=observed['ordinal'] or kept['oracle']['raw_sha256']!=observed['sha256'] or len(artifact(root,kept['oracle']))!=observed['width']*observed['height']*4:raise ValueError('Recorded original whole plane changed')
 assets=[];names={};logical=set()
 if not 1<=len(catalog['assets'])<=512:raise ValueError('Recorded source asset budget')
 for entry in catalog['assets']:
  name=entry['name'];normalized=asset_name(name);data=artifact(root,entry)
  if name in logical or (normalized.casefold() in names and names[normalized.casefold()]!=sha(data)) or bound['inputs'].get(entry['historical_path'])!=sha(data):raise ValueError('Recorded asset binding differs')
  logical.add(name);names[normalized.casefold()]=sha(data);assets.append((name,normalized,data))
 last=max(f['sequence'] for f in frames if f['sequence'] is not None)
 requested={payload[:-1].decode('ascii') for r,payload in records[:last] if r[2] in (7,20)}
 if not {asset_name(n).casefold() for n in requested}<=set(names):raise ValueError('Missing requested encoded source asset')
 return raw,records,frames,assets


def admit_actions(root,catalog,original,journal):
 from drawing_action_session import bind
 flow=original.get('storage_flow',{})
 if (flow.get('success') is not True or flow.get('display_rebuilds')!=[1,0] or
     [c.get('phase') for c in flow.get('checkpoints',[])]!=['before-save','after-save','after-load','after-recreate-load'] or
     any(c['world_returns']<c['source_base']+3 for c in flow['checkpoints'])):raise ValueError('Retained storage flow incomplete')
 saved=artifact(root,catalog['save'])
 if sha(saved)!=flow['saved_sha256'] or len(saved)!=flow['save_bytes']:raise ValueError('Retained original save changed')
 actions=artifact(root,catalog['action_frames']);frames=original['egl_physical_observation']['frames']
 if len(actions)!=128*len(frames):raise ValueError('Retained action/frame cohort incomplete')
 with tempfile.TemporaryDirectory(prefix='retained-action-bind-') as temporary:
  p=Path(temporary);(p/'session.bin').write_bytes(journal)
  for index in range(len(frames)):(p/f'egl-physical-actions-{index+1:06d}.bin').write_bytes(actions[index*128:(index+1)*128])
  observed=bind(p,original['egl_physical_observation'])
  if not same_json(observed,original['action_bound_presentations']):raise ValueError('Retained action epoch binding changed')
  complete=[(e['screen'],e['action']) for e in observed['actions'] if e['phase']==2 and e['status']==1]
  required=[(3,22),(18,23),(18,25),(17,29),(13,31),(17,28),(12,31),(17,15),(6,26),(3,18),(10,19),(3,18),(10,19),(3,27),(12,31),(17,15),(6,26)]
  if complete!=required or len(observed['worker_handoffs'])!=2:raise ValueError('Retained storage callback sequence incomplete')


def check(*,root=ROOT,catalog_path=CATALOG,native,presenter,headless=False,output_parent=None):
 root=Path(root).resolve();path=local(root,catalog_path);catalog_bytes=path.read_bytes();catalog=json.loads(catalog_bytes);reg_path=root/'research/runtime/coverage/register.json';reg_bytes=reg_path.read_bytes();register=json.loads(reg_bytes)
 raw,records,frames,assets=admit(root,catalog,register);comparator=load_tool('compare-drawing-quick-battle')
 parent=Path(os.path.abspath(output_parent if output_parent is not None else root/'working/tests/recorded-drawing-session'))
 if not parent.is_relative_to(root/'working'):raise ValueError('Recorded output parent must be under working')
 parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(run/'.retain').write_text('Portable observed-session source replay/GPU/physical comparisons.\n');owned=run/'owned';owned.mkdir()
 asset_hashes={}
 for name,normalized,data in assets:
  target=owned/normalized.casefold();target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data);asset_hashes[name]=sha(data)
 reads=[f for f in frames if f['fill'] is None and not f.get('background')]
 for f in frames:
  bg=f.get('background',{})
  if 'sequence' in bg:reads.append(bg)
 from drawing_copy_order import normalize
 original_record=next(e['record'] for e in register['evidence'] if e['id']==catalog['evidence'])
 normalized,copy_order=normalize(raw,records,artifact(root,catalog['traces']['points']),json.loads((root/original_record).read_text()),reads)
 unique={(f['sequence'],f['canvas']):f for f in reads}
 end=64
 for _ in range(max(s for s,c in unique)):end+=struct.unpack_from('<I',normalized,end)[0]
 source=raster_inputs(normalized[:end]);(owned/'producers.bin').write_bytes(source)
 bounds=[];queues=0;cursor=0
 for (seq,canvas),f in sorted(unique.items()):
  for record,_ in records[cursor:seq]:
   if record[2]==12:queues=record[14]
  cursor=seq;bounds.append(dict(sequence=seq,queues=queues,canvas=canvas,width=f['width'],height=f['height']))
 (owned/'drawing-session.json').write_text(json.dumps(dict(schema=1,session='1',assets=asset_hashes,segments=[dict(segment=1,stage='scene',producer=dict(path='producers.bin',sha256=sha(source)),frames=bounds)]))+'\n')
 binaries={str(Path(p).resolve()):sha(Path(p).read_bytes()) for p in (native,presenter)};env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='8',QT_SCALE_FACTOR='1')
 def execute(label,command):
  if headless:command=['xvfb-run','-a','-s','-screen 0 1280x1024x24',*command]
  result=subprocess.run(command,env=env,capture_output=True,timeout=600);(run/(label+'.log')).write_bytes(result.stdout+result.stderr)
  if result.returncode:raise ValueError('Recorded native '+label+' failed; inspect '+str(run/(label+'.log')))
 execute('sources',[str(Path(native).resolve()),str(owned),str(run/'native'),'--source-snapshots']);snapshots=json.loads((run/'native/session-report.json').read_text())
 if not snapshots['success'] or snapshots['remaining_surfaces'] or snapshots['original_oracles_read'] or snapshots['original_pixels_used_as_native_inputs'] or len(snapshots['frames'])!=len(bounds):raise ValueError('Recorded native source cohort/refusal')
 for expected,observed in zip(bounds,snapshots['frames']):
  if any(expected[k]!=observed[k] for k in ('sequence','canvas','width','height')):raise ValueError('Recorded native source order changed')
 prepare(run/'gpu-owned',run/'native',snapshots['frames'],frames)
 execute('gpu',[str(Path(presenter).resolve()),str(run/'gpu-owned'),str(run/'gpu'),'--recorded-composition']);gpu=json.loads((run/'gpu/presentation-report.json').read_text())
 if not gpu['success'] or gpu['remaining_surfaces'] or gpu['qimage_uploads'] or not gpu['gpu_composition'] or not gpu['physical_presentation'] or gpu['original_oracles_read'] or len(gpu['frames'])!=len(frames) or gpu['plan_sha256']!=sha((run/'gpu-owned/presentation.json').read_bytes()):raise ValueError('Recorded GPU frame cohort/provenance')
 comparisons=[]
 for index,(frame,item) in enumerate(zip(catalog['frames'],gpu['frames'])):
  if item['index']!=index+1 or (item['width'],item['height'])!=(frames[index].get('output_width',800),frames[index].get('output_height',600)) or item['path']!=f'frame-{index:06}.rgba' or item['native_path']!=f'frame-{index:06}.565':raise ValueError('Recorded GPU frame identity changed')
  original=artifact(root,frame['oracle']);words=(run/f'gpu/frame-{index:06}.565').read_bytes();physical=(run/f'gpu/frame-{index:06}.rgba').read_bytes()
  if sha(words)!=item['native_sha256'] or sha(physical)!=item['sha256'] or len(physical)!=len(original):raise ValueError('Recorded native whole plane changed')
  delta=np.max(np.abs(np.frombuffer(physical,dtype=np.uint8).reshape(-1,4)[:,:3].astype(np.int16)-np.frombuffer(original,dtype=np.uint8).reshape(-1,4)[:,:3].astype(np.int16)),axis=1)
  comparisons.append(dict(ordinal=index+1,gpu=comparator.compare_plane(words,original,item['width'],item['height']),physical=dict(pixels=item['width']*item['height'],mismatching_pixels=int(np.count_nonzero(delta>1)),max_channel_difference=int(delta.max()),native_sha256=sha(physical),original_sha256=sha(original))))
 relevant={catalog['evidence'],catalog['asset_binding_evidence'],retention_id(catalog)}
 current=json.loads(reg_path.read_bytes())
 if (path.read_bytes()!=catalog_bytes or any(sha(Path(p).read_bytes())!=h for p,h in binaries.items()) or
     not same_json([e for e in register['evidence'] if e['id'] in relevant],[e for e in current['evidence'] if e['id'] in relevant]) or
     not same_json(register['builds'],current['builds'])):raise ValueError('Recorded catalog/evidence/build/binaries changed')
 admit(root,catalog,register)
 mismatches=sum(c['gpu']['mismatching_pixels']+c['physical']['mismatching_pixels'] for c in comparisons)
 return dict(schema=1,copy_order=copy_order,success=not mismatches,outcome='matching' if not mismatches else 'mismatch',boundary=catalog['boundary'],actions_bound=catalog['schema']==2,frames=len(comparisons),pixels_compared=2*sum(c['gpu']['pixels'] for c in comparisons),mismatches=mismatches,comparisons=comparisons,channel_tolerance=1,alpha_compared=False,original_reference_admitted=True,original_pixels_used_as_native_inputs=False,original_oracles_read_by_native=False,new_original_execution=False,complete_presented_journey=False,draw02_complete=False,original_evidence=catalog['evidence'],binaries=binaries,artifacts=str(run.relative_to(root)),scope=catalog['scope'])
