#!/usr/bin/env python3
"""Rebuild independent original history and recompare immutable captured swaps."""
import argparse,hashlib,importlib.util,json,os,subprocess,sys
from pathlib import Path
import numpy as np
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract
from rendering_pilot import acquisition,compare,inflate,input_acknowledgements
from rendering_normal_acceptance import fullscreen
ROOT=Path(__file__).resolve().parents[1]
def sha(b):return hashlib.sha256(b).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in ['acquisition','previous-comparison','report']:p.add_argument('--'+n,required=True)
 for n in ['assets','build','output','executable']:p.add_argument('--'+n,type=Path,required=True)
 a=p.parse_args();a.build=a.build.resolve();a.output=a.output.resolve();a.assets=a.assets.resolve()
 if (ROOT/a.report).exists() or a.output.exists() or not a.output.is_relative_to(ROOT/'working'):p.error('New owned report/output required')
 a.output.mkdir(parents=True)
 names=['tools/recompare-rendering-divergence.py','tools/diagnose-rendering-divergence.py','tools/rendering_pilot.py','tools/rendering_normal_acceptance.py','tools/coverage_claims.py',
        'tests/acceptance-world-reference.cpp','tests/acceptance-world-history.cpp','tests/world-raster-reference.hpp','tests/sprite-binary-reference.cpp','tests/clipped-word-grayscale-cases.hpp','compat/legacy/world-rolling/CMakeLists.txt']
 result=dict(schema=1,success=False,sources={n:sha((ROOT/n).read_bytes()) for n in names},inputs={},artifacts=[],
             fresh_original_execution=False,fresh_native_game_execution=False,original_pixels_used_as_native_inputs=False,player_accepted=False,
             scope='Fresh isolated original history from the complete owned producer prefix, compared to immutable captured Qt swaps. Original key/fullscreen proofs are revalidated from those same captures. Native engine/capture sources must match the retained acquisition. No fresh native process or nominal acceptance claim.')
 before=False
 def run(label,command):
  with (a.output/(label+'.log')).open('xb') as out:subprocess.run(list(map(str,command)),cwd=ROOT,stdout=out,stderr=subprocess.STDOUT,check=True,timeout=300)
 try:
  run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
  with Reader(ROOT) as reader:
   def read(n):result['inputs'][n]=reader.digest(n);return reader.json(n)
   acquired=read(a.acquisition);previous=read(a.previous_comparison)
   if not acquired['success'] or not acquired['private_wine_stopped'] or acquired.get('retirement',{}).get('state')!='completed':raise ValueError('Stopped successful acquisition required')
   if previous['acquisition']['sha256']!=reader.digest(a.acquisition):raise ValueError('Previous comparison belongs to another acquisition')
   captured=dict(acquired['sources']);captured.update(acquired['native_admission']['native_only']['sources'])
   drift=[n for n,h in captured.items() if sha((ROOT/n).read_bytes())!=h]
   if drift:raise ValueError('Native engine/capture drift: '+str(drift))
   result['captured_native_sources']=captured;result['sources'].update(captured)
   def payload(artifact):
    data=reader.read_bytes(artifact['record'])
    if sha(data)!=artifact['sha256']:raise ValueError('Acquired payload identity: '+artifact['path'])
    result['inputs'][artifact['record']]=artifact['sha256'];return data
   candidates=[r for r in acquired['artifacts'] if r['path'].endswith('/'+Path(acquired['temporary_root']).name+'/worlds.bin')]
   if len(candidates)!=1:raise ValueError('Unique complete source profile required')
   profile=a.output/'worlds.bin';profile.write_bytes(payload(candidates[0]))
   streams=[payload(r) for r in acquired['artifacts'] if '/capture/' in r['path'] and r['path'].endswith('.bin')]
   observed=acquisition(streams,acquired['session_identity']);del streams
   result['observations']=observed['observations']
   ack=next(r for r in acquired['artifacts'] if r['path'].endswith('/render-input.bin.acceptance'))
   result['input_acknowledgements']=input_acknowledgements(payload(ack),acquired['flow'],observed)
   result['fullscreen']=fullscreen(acquired['flow'],observed)
  assets=previous['asset_inputs']
  def assets_stable():return all(sha((a.assets/n).read_bytes())==h for n,h in assets.items())
  if not assets_stable():raise ValueError('Reference asset inputs changed')
  result['asset_inputs']=assets
  result['source_executable_sha256']=sha(a.executable.read_bytes())
  if result['source_executable_sha256']!='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168':raise ValueError('Unsupported original executable')
  run('configure',['cmake','-S',ROOT/'compat/legacy/world-rolling','-B',a.build,'-DCMAKE_BUILD_TYPE=Release','-DBUILD_TESTING=ON'])
  run('history-build',['cmake','--build',a.build,'--target','rendering-divergence-history','--parallel','4'])
  spec=importlib.util.spec_from_file_location('divergence_sources',ROOT/'tools/diagnose-rendering-divergence.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
  for n in module.compiled_sources(a.build,'rendering-divergence-history'):result['sources'][n]=sha((ROOT/n).read_bytes())
  original=a.output/'original-reference'
  run('original-build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-fno-pie','-no-pie',ROOT/'tests/acceptance-world-reference.cpp','-o',original])
  history=a.build/'rendering-divergence-history';result['binary_sha256']={str(p.name):sha(p.read_bytes()) for p in [original,history]}
  reference=a.output/'reference';run('original-history',[history,profile,a.assets,original,a.executable,reference])
  index=json.loads((reference/'worlds.json').read_text());worlds={}
  if not index['success'] or index['native_player_pixels_read'] or index['original_destination_capture_read']:raise ValueError('Original history independence')
  for row in index['worlds']:
   g=int(row['generation'])
   if g!=len(worlds)+1 or Path(row['path']).name!=row['path']:raise ValueError('Original World cohort identity')
   raw=inflate((reference/row['path']).read_bytes())
   if len(raw)!=row['width']*row['height']*2:raise ValueError('Original plane extent')
   worlds[g]=np.frombuffer(raw,dtype='<u2').astype('uint32').reshape(row['height'],row['width'])
  result['original_reference']=dict(worlds=len(worlds),draws=index['draws'],source_chunks=index['source_chunks'],native_player_pixels_read=False,original_destination_capture_read=False)
  result['fresh_original_execution']=True;result['comparison']=compare(observed,worlds)
  result['previous_result']=dict(success=previous['success'],mismatching_pixels=previous['comparison']['mismatching_pixels'])
  result['sources_stable']=all(sha((ROOT/n).read_bytes())==h for n,h in result['sources'].items())
  result['asset_inputs_stable']=assets_stable();result['executable_stable']=sha(a.executable.read_bytes())==result['source_executable_sha256']
  result['success']=result['comparison']['success'] and result['sources_stable'] and result['asset_inputs_stable'] and result['executable_stable']
 except Exception as e:result.update(success=False,error=str(e))
 finally:
  if before:
   try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);result['original_manifest_verified_before_after']=True
   except Exception as e:result.update(success=False,original_manifest_error=str(e))
  for f in sorted(a.output.rglob('*')):
   if f.is_file():
    n=str(Path(a.report).with_suffix(''))+'-'+sha(str(f.relative_to(a.output)).encode())[:20]+'.bin';data=f.read_bytes();result['artifacts'].append(dict(path=str(f.relative_to(a.output)),record=n,sha256=write_report(ROOT,n,data),bytes=len(data)))
  write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
 print(json.dumps({k:result.get(k) for k in ['success','error','previous_result','original_reference','observations']}|dict(comparison={k:v for k,v in result.get('comparison',{}).items() if k in ('success','pixels_compared','mismatching_pixels','world_presentations')})),flush=True)
 return 0 if result['success'] else 1
if __name__=='__main__':raise SystemExit(main())
