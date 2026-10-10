#!/usr/bin/env python3
"""Frozen original producer/native continuous World validation and bounded profiling."""
import argparse,fcntl,hashlib,json,os,shutil,signal,statistics,subprocess,tempfile,time
from pathlib import Path
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1]
BEHAVIORS=['NR.world-continuous-source','RS.world-continuous-producer']
SCENARIO='native-world-continuous-source-20261010'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def summary(values):
 v=sorted(values);return dict(samples=len(v),median=statistics.median(v),p95=v[min(len(v)-1,int(len(v)*.95))],maximum=max(v)) if v else None
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--queues',type=int,default=4096);p.add_argument('--mode',choices=['complete','consumer-death','producer-death'],default='complete');p.add_argument('--prepared',type=Path);a=p.parse_args();assert 1<=a.queues<=4096
 parent=ROOT/'working/tests/native-world-continuous';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
 r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());bs=[next(b for b in r['behaviors'] if b['id']==bid) for bid in BEHAVIORS];s=next(s for s in r['scenarios'] if s['id']==SCENARIO);sources={n:sha(ROOT/n) for n in sorted(set().union(*(required_sources(b) for b in bs)))};claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{x['id']:x for x in r['builds']}),scenarios={SCENARIO:scenario_contract(s)}) for b in bs];declaration=out/'claims.json';declaration.write_text(json.dumps(dict(sources=sources,claims=claims),indent=2)+'\n')
 report=dict(success=False,mode=a.mode,requested_queues=a.queues,sources=sources,claims=claims,original_pixels_used_as_native_inputs=False,independent_original_equivalence=False)
 env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='8');process=None;experiment=None;before=False
 def run(name,cmd,**kw):
  with (out/(name+'.log')).open('x') as log:return subprocess.run([str(x) for x in cmd],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=600,**kw)
 try:
  run('original-before',[ROOT/'tools/original-manifest.sh','verify'],check=True);before=True
  frozen=out/'source';configured={n for n in subprocess.check_output(['git','ls-files','-z'],cwd=ROOT).decode().split('\0') if n and (Path(n).suffix in ('.cpp','.hpp','.c','.h','.S','.cmake') or Path(n).name=='CMakeLists.txt')}
  for n in sorted(configured|set(sources)):
   target=frozen/n;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/n,target)
  report['frozen_sources']={n:sha(frozen/n) for n in configured|set(sources)};build=out/'build';dependency_root=frozen
  if a.prepared:
   previous=json.loads((a.prepared/'report.json').read_text());assert previous['success'] and previous['sources_stable'] and previous['frozen_sources_stable'];build=a.prepared/'build';dependency_root=a.prepared/'source'
   assert sha(build/'mnm-world-continuous')==previous['native_binary_sha256']
   for n,h in previous['compiled_dependency_review']['sources'].items():assert sha(ROOT/n)==sha(dependency_root/n)==h,n
   report['prepared_report_sha256']=sha(a.prepared/'report.json')
  else:
   run('configure',['cmake','-S',frozen/'compat/legacy/world-rolling','-B',build,'-DCMAKE_BUILD_TYPE=RelWithDebInfo','-DBUILD_TESTING=ON'],check=True)
   run('build',['cmake','--build',build,'--target','mnm-world-continuous','world-continuous-channel-test','-j4'],check=True)
  compiled=set()
  for dep in build.rglob('*.o.d'):
   for token in dep.read_text().replace('\\\n',' ').split():
    path=Path(token).resolve()
    if path.is_file() and path.is_relative_to(dependency_root):compiled.add(path.relative_to(dependency_root).as_posix())
  missing=sorted(compiled-set(sources));report['compiled_dependency_review']=dict(sources={n:sha(dependency_root/n) for n in sorted(compiled)},undeclared_dependencies=missing);assert not missing,missing
  run('channel-fixtures',[build/'world-continuous-channel-test'],check=True);report['channel_fixtures_passed']=True;report['native_binary_sha256']=sha(build/'mnm-world-continuous');report['channel_fixture_sha256']=sha(build/'world-continuous-channel-test')
  command=['xvfb-run','-a','-s','-screen 0 1280x1024x24','python3',ROOT/'tools/capture-scene-game.py','--canvas-producers','--world-producer-handoff','--samples','32','--world-raster-prefix','32','--world-raster-batch','--skip-window-screenshot','--map','2','--magic-items','0','--producer-live',build/'mnm-world-continuous','--continuous-queues',str(a.queues),'--claims',declaration]
  injected=False;fault_generation=None
  with (ROOT/'working/rolling-live-staging.lock').open('a') as gate,(out/'capture.log').open('x') as log:
   fcntl.flock(gate,fcntl.LOCK_EX);process=subprocess.Popen([str(x) for x in command],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True);deadline=time.monotonic()+900
   while process.poll() is None:
    roots=[Path(x) for x in (out/'capture.log').read_text().splitlines() if x.startswith(str(ROOT/'working/experiments/scene-observer/run-')) and Path(x).is_dir()]
    if roots:experiment=roots[-1]
    if a.mode!='complete' and experiment and not injected:
     channel=experiment/'world-continuous.bin';ready=experiment/'native-producers/ready'
     if channel.exists() and ready.exists():
      import struct
      with channel.open('rb') as f:
       f.seek(128);one=f.read(64);f.seek(128+(128+16*1024*1024+197184+960064));two=f.read(64)
      active=[struct.unpack_from('<Q',x,8)[0] for x in (one,two) if len(x)==64 and struct.unpack_from('<I',x)[0] in (2,3)]
      if active and max(active)>=34:
       fault_generation=max(active)
       if a.mode=='consumer-death':os.kill(json.loads(ready.read_text())['pid'],signal.SIGKILL)
       else:subprocess.run(['wineserver','-k'],env=dict(env,WINEPREFIX=str(experiment/'wineprefix')),check=True,timeout=10)
       injected=True
    if time.monotonic()>deadline:raise RuntimeError('Continuous live deadline')
    time.sleep(.005 if a.mode!='complete' else .1)
  report.update(exit_code=process.returncode,experiment=str(experiment),fault_injected=injected,fault_generation=fault_generation)
  if a.mode!='complete':
   assert injected and process.returncode!=0
   text=(out/'capture.log').read_text();expected='Native continuous viewer failed' if a.mode=='consumer-death' else 'Original game exited during continuous source delivery';assert expected in text,'Expected peer-death refusal absent'
   native=experiment/'native-producers/live-report.json';assert not native.exists() or not json.loads(native.read_text())['success']
   subprocess.run(['wineserver','-w'],env=dict(env,WINEPREFIX=str(experiment/'wineprefix')),check=True,timeout=10)
   report.update(expected_terminal_failure=True,private_wine_stopped=True,completed_success_claimed=False,success=True,capture_log_sha256=sha(out/'capture.log'))
  else:
   assert process.returncode==0,'Original/native continuous capture failed; inspect '+str(out/'capture.log')
   capture=json.loads((experiment/'report.json').read_text());native=capture['native_continuous'];assert capture['success'] and native['success'] and int(native['original_returns'])==int(native['native_completions'])==a.queues and native['remaining_surfaces']==native['image_uploads']==native['diagnostic_files']==0
   assert native['channel_bytes']==35869312 and native['peak_source_bytes']<=16*1024*1024 and len(native['samples'])<=1024
   report.update(completed=capture,profile={k:summary([row[k] for row in native['samples'][1:] if k in row]) for k in ['producer_capture_ms','producer_wait_writeback_ms','producer_whole_world_ms','source_acquire_ms','prepare_ms','native_consume_ms','request_to_reply_ms','qt_paint_ms']},success=True)
  if experiment:
   metadata=json.loads((experiment/'manifest.json').read_text());report['source_executable_sha256']=metadata['source_sha256'];report['staged_binaries']=metadata;report['inputs']={str(experiment/'game'/n):sha(experiment/'game'/n) for n in ['Chaos.exe','MnmScene.dll','MnmMenu.dll']};report['inputs_stable']=all(sha(Path(n))==h for n,h in report['inputs'].items())
 except Exception as e:report.update(success=False,error=str(e))
 finally:
  if process and process.poll() is None:
   os.killpg(process.pid,signal.SIGINT)
   try:process.wait(timeout=20)
   except subprocess.TimeoutExpired:os.killpg(process.pid,signal.SIGKILL);process.wait(timeout=5)
  if before:run('original-after',[ROOT/'tools/original-manifest.sh','verify'],check=True)
  report.update(original_manifest_verified_before_after=before,sources_stable=all(sha(ROOT/n)==h for n,h in sources.items()),frozen_sources_stable=all(sha(out/'source'/n)==h for n,h in report.get('frozen_sources',{}).items()));report['success']=report['success'] and report['sources_stable'] and report['frozen_sources_stable'] and report.get('inputs_stable',False);(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
 if not report['success']:raise SystemExit(1)
if __name__=='__main__':main()
