#!/usr/bin/env python3
"""Measure exact ordered native World inputs and baseline-restored redraws on TITAN."""
import argparse,json,os,shutil,subprocess,tempfile
from pathlib import Path
from profile_native_resident_helpers import sha,distribution
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('--input-record',type=Path,default=ROOT/'research/runtime/resident-profile-offline-o2-20261011.json')
 p.add_argument('--worlds',type=int,default=128)
 p.add_argument('--behavior',action='append')
 p.add_argument('--scenario',default='native-draw-cost-offline-20261012')
 p.add_argument('--prepared',type=Path,help='Reuse a compiled source closure verified before replay, including an earlier refused preflight')
 p.add_argument('--baseline',type=Path,help='Previous successful source-bound draw-cost run; replay its frozen binary again')
 a=p.parse_args();assert 64<=a.worlds<=8192
 parent=ROOT/'working/tests/native-draw-costs';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
 subprocess.run(['python3',str(ROOT/'tools/draft-coverage-claims.py'),*[arg for b in (a.behavior or ['NR.native-draw-cost-profiling']) for arg in ['--behavior',b]],'--scenario',a.scenario,'--output',str(out/'claims.json')],cwd=ROOT,check=True,stdout=subprocess.DEVNULL)
 d=json.load(open(out/'claims.json'));report=dict(success=False,sources=d['sources'],claims=d['claims'],scope=__doc__,worlds=a.worlds,extra_replays_per_world=2,live_gameplay_executed=False,original_pixels_used_as_native_inputs=False)
 env={k:v for k,v in os.environ.items() if k not in ('DISPLAY','WAYLAND_DISPLAY','LIBGL_ALWAYS_SOFTWARE','__GLX_VENDOR_LIBRARY_NAME','MESA_LOADER_DRIVER_OVERRIDE','GALLIUM_DRIVER','QT_XCB_GL_INTEGRATION','QT_QPA_PLATFORM_PLUGIN_PATH','MNM_PROFILE_CHECKPOINTS')}
 env.update(QT_QPA_PLATFORM='mnm-headless-egl',MNM_PROFILE_EGL_VENDOR='NVIDIA',__EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/10_nvidia.json')
 inputs={};verified=False
 def run(label,cmd,environment=env):
  with (out/(label+'.log')).open('x') as log:subprocess.run([str(x) for x in cmd],cwd=ROOT,env=environment,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=900)
 try:
  run('original-before',[ROOT/'tools/original-manifest.sh','verify']);verified=True
  historical=json.load(open(a.input_record));assert historical['success'] and historical['inputs_stable']
  norm=next(Path(n) for n in historical['inputs'] if Path(n).name=='resident-source-v1.bin');assert sha(norm)==historical['inputs'][str(norm)]
  game_files={n:h for n,h in historical['inputs'].items() if '/game/' in n};assert game_files
  assets=Path(next(iter(game_files)).split('/game/')[0])/'game'
  for n,h in game_files.items():assert sha(Path(n))==h,n
  inputs.update(game_files);inputs[str(norm)]=sha(norm);inputs[str(a.input_record.resolve())]=sha(a.input_record)
  report['input_record_sha256']=sha(a.input_record);report['normalized_corpus']=str(norm);report['assets']=str(assets)
  # Only source/build inputs are frozen. Historical evidence and original media
  # are referenced by hash rather than copied into another installation.
  frozen=out/'source';names=subprocess.check_output(['git','ls-files','-z'],cwd=ROOT).decode().split('\0')
  configured={n for n in names if n and n.split('/')[0] not in ('working','original','research') and (Path(n).suffix in ('.cpp','.hpp','.c','.h','.S','.cmake','.json') or Path(n).name=='CMakeLists.txt')}
  for n in configured|set(d['sources']):
   dst=frozen/n;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/n,dst)
  build=out/'build'
  if a.prepared:
   old=json.load(open(a.prepared/'report.json'));assert old['compiled_sources'] and old['sources_stable']
   for n,h in old['compiled_sources'].items():assert sha(a.prepared/'source'/n)==h==d['sources'].get(n),n
   for n,h in old['sources'].items():
    if Path(n).name=='CMakeLists.txt' or Path(n).suffix=='.cmake':assert sha(a.prepared/'source'/n)==h==d['sources'].get(n),n
   build=a.prepared/'build';report['prepared_build']=dict(path=str(build),reviewed_source_closure=old['compiled_sources'],prior_report_sha256=sha(a.prepared/'report.json'))
  else:run('configure',['cmake','-S',frozen/'tools/native-render-profile','-B',build,'-DCMAKE_BUILD_TYPE=Release','-DCMAKE_CXX_FLAGS_RELEASE=-O2 -DNDEBUG -g'])
  if not a.prepared:run('build',['cmake','--build',build,'--target','mnm-resident-profile','mnm-gpu-timing-fixture','mnm_headless_egl','-j4'])
  compiled=set()
  for dep in build.rglob('*.o.d'):
   for token in dep.read_text().replace('\\\n',' ').split():
    path=Path(token).resolve()
    dependency_root=(a.prepared/'source') if a.prepared else frozen
    if path.is_file() and path.is_relative_to(dependency_root.resolve()):compiled.add(path.relative_to(dependency_root.resolve()).as_posix())
  report['compiled_sources']={n:sha(frozen/n) for n in sorted(compiled)};report['undeclared_compiler_inputs']=sorted(compiled-set(d['sources']));assert not report['undeclared_compiler_inputs'],report['undeclared_compiler_inputs']
  env['QT_QPA_PLATFORM_PLUGIN_PATH']=str(build/'platforms');run('gpu-fixture',[build/'mnm-gpu-timing-fixture']);report['gpu_fixture']=json.loads((out/'gpu-fixture.log').read_text().splitlines()[-1])
  # Retain the actual build location; a reused compiled closure stays immutable.
  report['build_path']=str(build.resolve())
  binary=build/'mnm-resident-profile';report['binary_sha256']=sha(binary);report['egl_plugin_sha256']=sha(build/'platforms/libmnm_headless_egl.so')
  targets=[('current',binary,build)]
  if a.baseline:
   previous=json.load(open(a.baseline/'report.json'));assert previous['success'] and previous['worlds']==a.worlds and previous['normalized_corpus']==str(norm) and previous['inputs']==inputs
   oldbuild=Path(previous.get('build_path',str(a.baseline/'build')));oldbinary=oldbuild/'mnm-resident-profile';assert sha(oldbinary)==previous['binary_sha256'];assert sha(oldbuild/'platforms/libmnm_headless_egl.so')==previous['egl_plugin_sha256']
   for n,h in previous['sources'].items():assert sha(a.baseline/'source'/n)==h,n
   report['baseline']=dict(path=str(a.baseline),report_sha256=sha(a.baseline/'report.json'),sources=previous['sources']);targets.insert(0,('baseline',oldbinary,oldbuild))
  native={};summaries={};pixel_rows={}
  for label,exe,plugin in targets:
   native[label]={};summaries[label]={};e={**env,'QT_QPA_PLATFORM_PLUGIN_PATH':str(plugin/'platforms')}
   for mode in ('timestamps','throughput','checkpoints'):
    dest=out/(label+'-'+mode+'.json');run(label+'-'+mode,[exe,norm,assets,'timestamps' if mode=='timestamps' else 'throughput',2 if mode=='throughput' else 1,dest,a.worlds,2],{**e,**({'MNM_PROFILE_CHECKPOINTS':'1'} if mode=='checkpoints' else {})})
    result=json.load(open(dest));assert result['success'] and 'TITAN' in result['renderer'];native[label][mode]=result
    for trial in result['runs']:assert trial['worlds']==a.worlds and not trial['query_drops'] and not trial['remaining_surfaces'] and not trial['remaining_queries']
    warm=[row for trial in result['runs'] for row in trial['samples'] if int(row['generation'])>=64]
    extras=[x for row in warm for x in row['extra_replays']]
    summaries[label][mode]={'primary_cpu_ms':distribution([r['native_consume_ms'] for r in warm]),'primary_gpu_timeline_ms':distribution([r['gpu_timeline_ms'] for r in warm if 'gpu_timeline_ms' in r]),'extra_cpu_ms':distribution([r['cpu_ms'] for r in extras]),'extra_gpu_timeline_ms':distribution([r['gpu_timeline_ms'] for r in extras if 'gpu_timeline_ms' in r]),'primary_counts':{k:distribution([r['costs'][k] for r in warm]) for k in warm[0]['costs']},'extra_counts':{k:distribution([r[k] for r in extras]) for k in warm[0]['costs']},'completion_worlds_per_second':distribution([r['completion_worlds_per_second'] for r in result['runs']])}
    if mode=='checkpoints':pixel_rows[label]=[r['native_words_sha256'] for r in result['runs'][0]['samples']]
   hashes={trial['final_native_words_sha256'] for result in native[label].values() for trial in result['runs']};assert len(hashes)==1
  if a.baseline:assert pixel_rows['baseline']==pixel_rows['current'],'Paired primary native pixel mismatch'
  report.update(native=native,summary=summaries,checkpoint_worlds=a.worlds,checkpoint_extra_replays=a.worlds*2,paired_pixel_equality=not a.baseline or pixel_rows['baseline']==pixel_rows['current'],inputs=inputs,success=True)
 except (OSError,ValueError,KeyError,AssertionError,subprocess.SubprocessError) as error:report['error']=str(error)
 finally:
  if verified:
   try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
   except (OSError,subprocess.SubprocessError) as error:report.update(success=False,manifest_error=str(error))
  report['sources_stable']=all(sha(ROOT/n)==h for n,h in d['sources'].items());report['inputs_stable']=all(sha(Path(n))==h for n,h in inputs.items());report['success']=report['success'] and report['sources_stable'] and report['inputs_stable'] and report.get('original_manifest_verified_before_after',False)
  (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
 print('Draw-cost report:',out/'report.json',flush=True);return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
