#!/usr/bin/env python3
"""Retain an entire independently recorded EGL interval, preserving its limits."""
import argparse,copy,gzip,json,shutil,subprocess,tempfile
from pathlib import Path
from drawing_corpus import ROOT,BUILD,local,sha
from drawing_recorded_session import admit,RETENTION,STORAGE_RETENTION,presentation_profile,witness_files

def retain(evidence_id,binding_id,destination,retention_id=None):
 from rendering_coverage import evidence_record
 destination=destination.resolve()
 if destination.exists() or not destination.is_relative_to(ROOT/'tests/fixtures'):raise ValueError('Choose a new contained fixture directory')
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());evidence={e['id']:e for e in register['evidence']};builds={b['id']:b for b in register['builds']}
 ev=evidence[evidence_id];binding=evidence[binding_id];original=evidence_record(ROOT,ev,builds);bound=evidence_record(ROOT,binding,builds)
 if retention_id is not None and (not retention_id.strip() or retention_id in evidence):raise ValueError('Choose a new immutable retention evidence ID')
 if original.get('egl_correct_stale_upload_policy') and retention_id is None:raise ValueError('Corrected reference requires a new explicit retention evidence ID')
 if retention_id is not None and not original.get('staging',{}).get('campaign_storage'):raise ValueError('Explicit profile retention requires action-bound storage capture')
 if ev['kind'] not in ('isolated_original','live_observation') or binding['kind']!='native_integration' or bound['original_report']['sha256']!=ev['sha256'] or (bound['success'] is not True and not (original.get('staging',{}).get('campaign_storage') and bound.get('all_observed_presentations_compared') is True and bound.get('native_exit_code')==0 and bound.get('native_gpu_session',{}).get('report',{}).get('success') is True)):raise ValueError('Completed original and native binding records required')
 capture=Path(original['experiment'])/'capture';run=ROOT/bound['artifacts'];native=json.loads((run/'owned-inputs/drawing-session.json').read_text());game=Path(original['experiment'])/'game';existing={p.relative_to(game).as_posix().casefold():p for p in game.rglob('*') if p.is_file()};prepared={}
 def pack(data,historical):
  packed=gzip.compress(data,mtime=0);path=destination/(sha(data)+'.gz');prepared[path]=packed
  return dict(path=path.relative_to(ROOT).as_posix(),encoding='gzip',raw_sha256=sha(data),sha256=sha(packed),historical_path=historical)
 def original_read(path,digest):
  data=path.read_bytes()
  if sha(data)!=digest:raise ValueError('Historical artifact changed: '+str(path))
  return data
 raw=original_read(capture/'canvas-producers.bin',original['producer_observation']['producer_journal_sha256']);chunk=32*1024*1024
 catalog=dict(schema=1,boundary='observed_egl_session',source_sha256=BUILD,evidence=evidence_id,original_report_sha256=ev['sha256'],asset_binding_evidence=binding_id,binding_report_sha256=binding['sha256'],
  scope='Complete recorded Main/setup/loading/Quick Battle/Results/returned-Main EGL interval. All323observed full800x600frames, duplicates included. Native GPU rectangle/fill/history composition and post-swap X11 output; RGB tolerance1, alpha excluded. Original global presentation-route completeness, full action semantics, fourteen journeys/twenty-one operations and hardware scanout remain unproved.',
  input_chunks=[pack(raw[i:i+chunk],str((capture/'canvas-producers.bin').relative_to(ROOT))) for i in range(0,len(raw),chunk)],traces={},assets=[],frames=[],
  retention_sources={n:sha((ROOT/n).read_bytes()) for n in ['tools/retain-recorded-drawing-session.py','tools/drawing_recorded_session.py','tools/drawing_corpus.py']})
 trace_names={'points':('api-producer-points.bin',original['producer_observation']['point_journal_sha256']),'journal':('session.bin',original['observation']['journal_sha256']),'intervals':('api-call-intervals.bin',original['call_intervals']['interval_sha256']),'closure':('canvas-session-0000.done',original['producer_observation']['session_producer_marker_sha256'])}
 for key,(name,digest) in trace_names.items():catalog['traces'][key]=pack(original_read(capture/name,digest),str((capture/name).relative_to(ROOT)))
 from drawing_scene_corpus import asset_name
 for name,digest in sorted(native['assets'].items()):
  path=existing[asset_name(name).casefold()];historical=str(path.relative_to(ROOT))
  if bound['inputs'].get(historical)!=digest:raise ValueError('Native asset pin differs')
  entry=pack(original_read(path,digest),historical);entry['name']=name;catalog['assets'].append(entry)
 for frame in original['egl_physical_observation']['frames']:
  path=capture/frame['path'];catalog['frames'].append(dict(ordinal=frame['ordinal'],oracle=pack(original_read(path,frame['sha256']),str(path.relative_to(ROOT)))))
 if original.get('staging',{}).get('campaign_storage'):
  from drawing_action_session import bind
  if bind(capture,original['egl_physical_observation'])!=original['action_bound_presentations']:raise ValueError('Original action binding changed')
  catalog.update(schema=2,boundary='observed_egl_action_session',scope='Complete observed original Apprentice storage/rebuild/resume EGL interval with exact callback epoch brackets and saved-byte checks; native source/GPU/X11 replay at actual extents. Qt menu policy, global routes, generic fourteen-journey/twenty-one-operation acceptance and hardware remain separate.')
  action_bytes=b''.join(original_read(capture/f'egl-physical-actions-{frame["ordinal"]:06d}.bin',original['action_bound_presentations']['frames'][frame['ordinal']-1]['sha256']) for frame in original['egl_physical_observation']['frames'])
  catalog['action_frames']=pack(action_bytes,'Original action/frame sidecars concatenated in ordinal order')
  flow=original['storage_flow'];save=game/'Save'/(flow['saved_name']+'.sav');catalog['save']=pack(original_read(save,flow['saved_sha256']),str(save.relative_to(ROOT)))
 if retention_id is not None:
  catalog.update(schema=3,retention_evidence=retention_id,reference_profile=presentation_profile(original),witnesses=[])
  catalog['scope']+=' Explicit presentation policy profile and all source/default-framebuffer/XCB witnesses retained; corrected software uploads are a driver-policy reference, not unmodified-driver equivalence.'
  for row in original['egl_swap_completion_observation']['frames']:
   files={}
   for name,digest in witness_files(row).items():
    if Path(name).name!=name:raise ValueError('Unsafe original witness name')
    files[name]=pack(original_read(capture/name,digest),str((capture/name).relative_to(ROOT)))
   catalog['witnesses'].append(dict(ordinal=row['ordinal'],files=files))
 chosen_id=retention_id or (STORAGE_RETENTION if catalog['schema']==2 else RETENTION)
 if chosen_id in evidence:raise ValueError('Retention evidence ID already registered')
 data=(json.dumps(catalog,indent=2)+'\n').encode();retention=dict(id=chosen_id,kind='static',record=str((destination/'session.json').relative_to(ROOT)),sha256=sha(data),scope='Lossless portable retention of an independently acquired observed-EGL interval; no new original execution or global presentation admission.',assertions=[dict(pointer='/boundary',equals=catalog['boundary'])])
 with tempfile.TemporaryDirectory(prefix='recorded-retention-',dir=ROOT/'working/tests') as temporary:
  mirror=Path(temporary)
  for path,packed in prepared.items():target=mirror/path.relative_to(ROOT);target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(packed)
  fake=copy.deepcopy(register);fake['evidence'].append(retention)
  for key in [evidence_id,binding_id]:
   e=evidence[key];target=mirror/e['record'];target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/e['record'],target)
  target=mirror/retention['record'];target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data);admit(mirror,catalog,fake)
 destination.mkdir()
 for path,packed in prepared.items():
  with path.open('xb') as out:out.write(packed)
 with (destination/'session.json').open('xb') as out:out.write(data)
 return catalog,retention

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--evidence',required=True);p.add_argument('--bindings',required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--retention-id',help='New immutable evidence ID; retains an explicit presentation profile and independent witnesses');a=p.parse_args()
 subprocess.run(['./tools/original-manifest.sh','verify'],cwd=ROOT,check=True)
 try:catalog,entry=retain(a.evidence,a.bindings,a.output,a.retention_id)
 finally:subprocess.run(['./tools/original-manifest.sh','verify'],cwd=ROOT,check=True)
 print(json.dumps(dict(frames=len(catalog['frames']),assets=len(catalog['assets']),chunks=len(catalog['input_chunks']),retention=entry)))
if __name__=='__main__':main()
