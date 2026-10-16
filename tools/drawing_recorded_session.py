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


def admit(root,catalog,register):
 from rendering_coverage import evidence_record
 evidence={e['id']:e for e in register['evidence']};builds={b['id']:b for b in register['builds']}
 if catalog['schema']!=1 or type(catalog['schema']) is not int or catalog['source_sha256']!=BUILD or catalog['boundary']!='observed_egl_session':raise ValueError('Recorded session schema/build/boundary')
 retained=evidence_record(root,evidence[RETENTION],builds)
 if not same_json(retained,catalog):raise ValueError('Recorded session differs from immutable retention')
 ev=evidence[catalog['evidence']];binding=evidence[catalog['asset_binding_evidence']]
 if ev['kind'] not in ('isolated_original','live_observation') or binding['kind']!='native_integration' or ev['sha256']!=catalog['original_report_sha256'] or binding['sha256']!=catalog['binding_report_sha256']:raise ValueError('Recorded session needs independent original/binding provenance')
 original=evidence_record(root,ev,builds);bound=evidence_record(root,binding,builds)
 if (original['source_sha256']!=BUILD or any(original.get(k) is not True for k in ('success','normal_original_quit_verified','original_sections_unchanged','original_drawing_active','original_manifest_verified_before_after')) or
     original['original_pixels_used_as_native_inputs'] is not False or bound['original_report']['sha256']!=ev['sha256'] or bound['success'] is not True or bound['original_pixels_used_as_native_inputs'] is not False or not bound['all_observed_presentations_compared']):raise ValueError('Recorded original interval/provenance incomplete')
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
 for kept,observed in zip(catalog['frames'],original['egl_physical_observation']['frames']):
  if kept['ordinal']!=observed['ordinal'] or kept['oracle']['raw_sha256']!=observed['sha256'] or len(artifact(root,kept['oracle']))!=800*600*4:raise ValueError('Recorded original whole plane changed')
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


def check(*,root=ROOT,catalog_path=CATALOG,native,presenter,headless=False):
 root=Path(root).resolve();path=local(root,catalog_path);catalog_bytes=path.read_bytes();catalog=json.loads(catalog_bytes);reg_path=root/'research/runtime/coverage/register.json';reg_bytes=reg_path.read_bytes();register=json.loads(reg_bytes)
 raw,records,frames,assets=admit(root,catalog,register);comparator=load_tool('compare-drawing-quick-battle')
 parent=root/'working/tests/recorded-drawing-session';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(run/'.retain').write_text('Portable observed-session source replay/GPU/physical comparisons.\n');owned=run/'owned';owned.mkdir()
 last=max(f['sequence'] for f in frames if f['sequence'] is not None);end=64
 for _ in range(last):end+=struct.unpack_from('<I',raw,end)[0]
 source=raster_inputs(raw[:end]);(owned/'producers.bin').write_bytes(source)
 asset_hashes={}
 for name,normalized,data in assets:
  target=owned/normalized.casefold();target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data);asset_hashes[name]=sha(data)
 unique={(f['sequence'],f['canvas']):f for f in frames if f['fill'] is None and not f.get('background')}
 for f in frames:
  bg=f.get('background',{})
  if 'sequence' in bg:unique[bg['sequence'],bg['canvas']]=bg
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
  if item['index']!=index+1 or (item['width'],item['height'])!=(800,600) or item['path']!=f'frame-{index:06}.rgba' or item['native_path']!=f'frame-{index:06}.565':raise ValueError('Recorded GPU frame identity changed')
  original=artifact(root,frame['oracle']);words=(run/f'gpu/frame-{index:06}.565').read_bytes();physical=(run/f'gpu/frame-{index:06}.rgba').read_bytes()
  if sha(words)!=item['native_sha256'] or sha(physical)!=item['sha256'] or len(physical)!=len(original):raise ValueError('Recorded native whole plane changed')
  delta=np.max(np.abs(np.frombuffer(physical,dtype=np.uint8).reshape(-1,4)[:,:3].astype(np.int16)-np.frombuffer(original,dtype=np.uint8).reshape(-1,4)[:,:3].astype(np.int16)),axis=1)
  comparisons.append(dict(ordinal=index+1,gpu=comparator.compare_plane(words,original),physical=dict(pixels=800*600,mismatching_pixels=int(np.count_nonzero(delta>1)),max_channel_difference=int(delta.max()),native_sha256=sha(physical),original_sha256=sha(original))))
 relevant={catalog['evidence'],catalog['asset_binding_evidence'],RETENTION}
 current=json.loads(reg_path.read_bytes())
 if (path.read_bytes()!=catalog_bytes or any(sha(Path(p).read_bytes())!=h for p,h in binaries.items()) or
     not same_json([e for e in register['evidence'] if e['id'] in relevant],[e for e in current['evidence'] if e['id'] in relevant]) or
     not same_json(register['builds'],current['builds'])):raise ValueError('Recorded catalog/evidence/build/binaries changed')
 admit(root,catalog,register)
 mismatches=sum(c['gpu']['mismatching_pixels']+c['physical']['mismatching_pixels'] for c in comparisons)
 return dict(schema=1,success=not mismatches,outcome='matching' if not mismatches else 'mismatch',boundary='observed_egl_session',frames=len(comparisons),pixels_compared=2*len(comparisons)*800*600,mismatches=mismatches,comparisons=comparisons,channel_tolerance=1,alpha_compared=False,original_reference_admitted=True,original_pixels_used_as_native_inputs=False,original_oracles_read_by_native=False,new_original_execution=False,complete_presented_journey=False,draw02_complete=False,original_evidence=catalog['evidence'],binaries=binaries,artifacts=str(run.relative_to(root)),scope=catalog['scope'])
