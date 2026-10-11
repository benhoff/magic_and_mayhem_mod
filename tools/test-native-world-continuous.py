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
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--native-draw',action='store_true',help='Native graphics services with real ddraw disabled; V4 resident delivery');p.add_argument('--profile-producer',action='store_true');p.add_argument('--profile-waits',action='store_true');p.add_argument('--profile',action='store_true',help='Separate CPU sampling probe; timings remain charged to the full-path budget');p.add_argument('--hardware',action='store_true',help='Private no-monitor NVIDIA seat; refuse software fallback');p.add_argument('--hardware-driver',choices=['zink','nvidia'],default='zink');p.add_argument('--impossible-budget',action='store_true',help='Negative control: require warm median <=0.001ms');p.add_argument('--resident',action='store_true',help='V4 GPU delivery without pixel replies; retains normal performance gates');p.add_argument('--normal',action='store_true',help='Measured normal path without CPU reference composition');p.add_argument('--queues',type=int,default=4096);p.add_argument('--mode',choices=['complete','consumer-death','producer-death'],default='complete');p.add_argument('--prepared',type=Path);a=p.parse_args();assert 1<=a.queues<=4096
 if a.native_draw:a.resident=True
 if a.resident:a.normal=True
 if a.profile_producer and a.profile_waits:p.error('Choose one producer profiler')
 if a.impossible_budget and not a.normal:p.error('--impossible-budget requires --normal')
 if a.hardware_driver!='zink' and not a.hardware:p.error('--hardware-driver requires --hardware')
 parent=ROOT/'working/tests/native-world-continuous';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
 behaviors=['NR.world-gpu-ack-delivery','NR.directdraw-retirement'] if a.native_draw else ['NR.world-gpu-ack-delivery'] if a.resident else ['NR.world-continuous-normal-performance'] if a.normal else BEHAVIORS;scenario=('native-world-native-draw-peer-failure-20261011' if a.mode!='complete' else 'native-world-native-draw-live-20261010') if a.native_draw else 'native-world-gpu-ack-live-20261010' if a.resident else 'native-world-continuous-normal-20261010' if a.normal else SCENARIO
 r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());bs=[next(b for b in r['behaviors'] if b['id']==bid) for bid in behaviors];s=next(s for s in r['scenarios'] if s['id']==scenario);sources={n:sha(ROOT/n) for n in sorted(set().union(*(required_sources(b) for b in bs)))};claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{x['id']:x for x in r['builds']}),scenarios={scenario:scenario_contract(s)}) for b in bs];assert set(s['tests'])<=set(sources),'Undeclared scenario launch/staging inputs: '+repr(sorted(set(s['tests'])-set(sources)));declaration=out/'claims.json';declaration.write_text(json.dumps(dict(sources=sources,claims=claims),indent=2)+'\n')
 report=dict(success=False,mode=a.mode,normal=a.normal,resident=a.resident,requested_queues=a.queues,native_draw=a.native_draw,sources=sources,claims=claims,original_pixels_used_as_native_inputs=False,independent_original_equivalence=False)
 env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='8');process=None;experiment=None;before=False;gpu_server=None
 def run(name,cmd,**kw):
  with (out/(name+'.log')).open('x') as log:return subprocess.run([str(x) for x in cmd],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=kw.pop('timeout',600),**kw)
 try:
  run('original-before',[ROOT/'tools/original-manifest.sh','verify'],check=True);before=True
  frozen=out/'source';configured={n for n in subprocess.check_output(['git','ls-files','-z'],cwd=ROOT).decode().split('\0') if n and (Path(n).suffix in ('.cpp','.hpp','.c','.h','.S','.cmake') or Path(n).name=='CMakeLists.txt')}
  for n in sorted(configured|set(sources)):
   target=frozen/n;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/n,target)
  report['frozen_sources']={n:sha(frozen/n) for n in configured|set(sources)};build=out/'build';dependency_root=frozen
  if a.prepared:
   previous=json.loads((a.prepared/'report.json').read_text());assert previous.get('memory_fixtures_passed') and previous.get('channel_fixtures_passed') and previous.get('hash_fixtures_passed') and previous.get('normal_fixtures_passed') and previous['sources_stable'] and previous['frozen_sources_stable'];build=a.prepared/'build';dependency_root=a.prepared/'source'
   assert sha(build/'mnm-world-continuous')==previous['native_binary_sha256']
   for n,h in previous['compiled_dependency_review']['sources'].items():assert sha(ROOT/n)==sha(dependency_root/n)==h,n
   report['prepared_report_sha256']=sha(a.prepared/'report.json')
   for n in ['compat/legacy/world-rolling/CMakeLists.txt','renderer/CMakeLists.txt','renderer/scenes/CMakeLists.txt','renderer/resources/CMakeLists.txt','assets/CMakeLists.txt','assets/resources/CMakeLists.txt','compat/legacy/canvas-producers/CMakeLists.txt']:
    assert sha(ROOT/n)==sha(dependency_root/n),n
  else:
   run('configure',['cmake','-S',frozen/'compat/legacy/world-rolling','-B',build,'-DCMAKE_BUILD_TYPE=Release','-DCMAKE_CXX_FLAGS_RELEASE=-O3 -DNDEBUG -g','-DBUILD_TESTING=ON'],check=True)
   run('build',['cmake','--build',build,'--target','world-continuous-memory-test','mnm-world-continuous','world-continuous-channel-test','world-continuous-hash-test','world-continuous-normal-test','world-gpu-ack-test','-j4'],check=True)
  cache=(build/'CMakeCache.txt').read_text().splitlines()
  assert 'CMAKE_BUILD_TYPE:STRING=Release' in cache and 'CMAKE_CXX_FLAGS_RELEASE:STRING=-O3 -DNDEBUG -g' in cache,'Prepared build differs from production compiler policy'
  report['compiler_policy']=dict(build_type='Release',flags='-O3 -DNDEBUG -g',cache_sha256=sha(build/'CMakeCache.txt'))
  compiled=set()
  for dep in build.rglob('*.o.d'):
   for token in dep.read_text().replace('\\\n',' ').split():
    path=Path(token).resolve()
    if path.is_file() and path.is_relative_to(dependency_root):compiled.add(path.relative_to(dependency_root).as_posix())
  missing=sorted(compiled-set(sources));report['compiled_dependency_review']=dict(sources={n:sha(dependency_root/n) for n in sorted(compiled)},undeclared_dependencies=missing);assert not missing,missing
  if a.resident:run('ack-fixtures',[build/'world-gpu-ack-test'],check=True);report['ack_fixtures_passed']=True
  run('memory-fixtures',[build/'world-continuous-memory-test'],check=True);report['memory_fixtures_passed']=True
  run('channel-fixtures',[build/'world-continuous-channel-test'],check=True);report['channel_fixtures_passed']=True;run('hash-fixtures',[build/'world-continuous-hash-test'],check=True);report['hash_fixtures_passed']=True;report['native_binary_sha256']=sha(build/'mnm-world-continuous');report['channel_fixture_sha256']=sha(build/'world-continuous-channel-test')
  run('normal-fixtures',['xvfb-run','-a','-s','-screen 0 1280x1024x24',build/'world-continuous-normal-test'],check=True);report['normal_fixtures_passed']=True
  if a.hardware:
   import re
   gpu=subprocess.check_output(['nvidia-smi','--query-gpu=pci.bus_id','--format=csv,noheader'],text=True).strip().splitlines()[0]
   match=re.fullmatch(r'[0-9A-Fa-f]{8}:([0-9A-Fa-f]{2}):([0-9A-Fa-f]{2})\.([0-7])',gpu);assert match,'Unsupported GPU bus identity'
   bus='PCI:'+':'.join(str(int(x,16)) for x in match.groups())
   display=next(n for n in range(170,190) if not Path(f'/tmp/.X11-unix/X{n}').exists() and not Path(f'/tmp/.X{n}-lock').exists())
   configuration=out/'private-gpu.conf';configuration.write_text('Section "ServerFlags"\n Option "AutoAddDevices" "False"\n Option "AutoEnableDevices" "False"\n Option "DontVTSwitch" "True"\nEndSection\nSection "Device"\n Identifier "native-test-gpu"\n Driver "nvidia"\n BusID "'+bus+'"\n Option "UseDisplayDevice" "none"\n Option "AllowEmptyInitialConfiguration" "True"\n Option "ProbeAllGpus" "False"\nEndSection\nSection "Screen"\n Identifier "native-test-screen"\n Device "native-test-gpu"\n DefaultDepth 24\n SubSection "Display"\n  Depth 24\n  Virtual 1280 1024\n EndSubSection\nEndSection\nSection "ServerLayout"\n Identifier "native-test-layout"\n Screen "native-test-screen"\nEndSection\n')
   gpu_server=subprocess.Popen(['/usr/lib/Xorg',f':{display}','-seat','native-render-test','-config',str(configuration),'-logfile',str(out/'private-gpu-xorg.log'),'-nolisten','tcp','-noreset','-novtswitch','-sharevts'],stdout=(out/'private-gpu-start.log').open('x'),stderr=subprocess.STDOUT,start_new_session=True)
   until=time.monotonic()+30
   while not Path(f'/tmp/.X11-unix/X{display}').exists():
    assert gpu_server.poll() is None and time.monotonic()<until,'Private GPU X server failed'
    time.sleep(.1)
   for key in ['LIBGL_ALWAYS_SOFTWARE','__GLX_VENDOR_LIBRARY_NAME','__EGL_VENDOR_LIBRARY_FILENAMES','WAYLAND_DISPLAY']:env.pop(key,None)
   env['DISPLAY']=f':{display}'
   for key in ['MESA_LOADER_DRIVER_OVERRIDE','GALLIUM_DRIVER','QT_XCB_GL_INTEGRATION']:env.pop(key,None)
   if a.hardware_driver=='zink':env.update(MESA_LOADER_DRIVER_OVERRIDE='zink',GALLIUM_DRIVER='zink')
   else:env.update(__GLX_VENDOR_LIBRARY_NAME='nvidia',__EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/10_nvidia.json',QT_XCB_GL_INTEGRATION='xcb_glx')
   run('hardware-driver',['glxinfo','-B'],check=True,timeout=20);driver=(out/'hardware-driver.log').read_text();assert 'NVIDIA' in driver and 'Accelerated: yes' in driver and 'llvmpipe' not in driver and 'lavapipe' not in driver,'Hardware fallback refused'
   report['hardware_driver']=a.hardware_driver;report['hardware_environment']={k:env[k] for k in ['DISPLAY','MESA_LOADER_DRIVER_OVERRIDE','GALLIUM_DRIVER','__GLX_VENDOR_LIBRARY_NAME','__EGL_VENDOR_LIBRARY_FILENAMES','QT_XCB_GL_INTEGRATION'] if k in env};report['hardware_driver_log_sha256']=sha(out/'hardware-driver.log')
  command=['xvfb-run' ,'-a','-s','-screen 0 1280x1024x24','python3',ROOT/'tools/capture-scene-game.py','--canvas-producers','--world-producer-handoff','--samples','32','--world-raster-prefix','32','--world-raster-batch','--skip-window-screenshot','--map','2','--magic-items','0','--producer-live',build/'mnm-world-continuous','--continuous-queues',str(a.queues),'--claims',declaration,*([] if a.normal else ['--continuous-verify'])]
  if a.native_draw:command.append('--native-draw')
  if a.resident:command.append('--continuous-resident')
  if a.profile_producer:command.append('--continuous-producer-profile')
  if a.profile_waits:command.append('--continuous-producer-waits')
  if a.profile:
   wrapper=out/'profile-native';wrapper.write_text('#!/usr/bin/env python3\nimport os,sys\nos.execvp("perf",["perf","record","-F","99","-g","--call-graph","dwarf","-o",'+repr(str(out/'native-perf.data'))+',"--",'+repr(str(build/'mnm-world-continuous'))+',*sys.argv[1:]])\n');wrapper.chmod(0o700)
   command[command.index('--producer-live')+1]=wrapper
  if a.hardware:command=command[4:];command.append('--continuous-hardware')
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
   if a.native_draw:
    metadata=json.loads((experiment/'manifest.json').read_text())
    assert metadata['native_draw'] is True
    assert sha(experiment/'game/MnmDraw.dll')==metadata['native_draw_sha256'],'Native graphics DLL changed during fault control'
    raw=(experiment/'native-draw-status.bin').read_bytes()
    assert len(raw)==96 and raw[:8]==b'MNMDRW01','Native graphics status missing or refusal record'
    status=struct.unpack('<24I',raw)
    assert status[2:5]==(2,96,0) and status[19]==status[23]==0,'Real ddraw loaded or malformed graphics status'
    assert status[9]>0 and status[10]>0 and status[20]>=status[9] and status[22]==0,'Native graphics startup incomplete'
    assert status[5]<=64 and status[6]<=512 and status[7]<=128 and status[8]<=64
    assert status[15]<=512 and status[16]<=256*1024*1024 and status[17]<=128 and status[18]<=64
    report['native_draw_fault']=dict(ddraw_disabled=True,ddraw_module_absent=True,status_sha256=sha(experiment/'native-draw-status.bin'),native_dll_sha256=metadata['native_draw_sha256'],factory_creations=status[9],surface_creations=status[10],peak_surfaces=status[15],peak_bytes=status[16],scope='Terminal peer-death control after native graphics startup and at least33 completed queues, with real ddraw disabled/absent and unchanged DLL. Forced private process stop is not original semantic COM teardown.')

  else:
   assert process.returncode==0,'Original/native continuous capture failed; inspect '+str(out/'capture.log')
   capture=json.loads((experiment/'report.json').read_text());native=capture['native_continuous'];assert capture['success'] and native['success'] and int(native['original_returns'])==int(native['native_completions'])==a.queues and native['remaining_surfaces']==native['image_uploads']==native['diagnostic_files']==0
   if a.native_draw:
    assert capture['native_draw']['ddraw_disabled'] and capture['native_draw']['ddraw_module_absent']
    report['selected_session_directdraw_backend_retired']=True
   if a.resident:
    assert native['resident'] and native['world_readbacks']==0 and int(native['pixel_replies'])==int(native['original_world_writebacks'])==0 and int(native['checked_resident_returns'])==a.queues
   assert native['verify']==(not a.normal) and int(native['cpu_reference_queues'])==(0 if a.normal else a.queues)
   assert native['channel_bytes']==35869312 and native['peak_source_bytes']<=16*1024*1024 and len(native['samples'])<=1024
   report.update(completed=capture,profile={k:summary([row[k] for row in native['samples'][1:] if int(row['generation'])>=64 and k in row]) for k in ['producer_region_queries','engine_between_world_ms','original_frame_interval_ms','engine_pacer_ms','engine_screen_copy_ms','engine_screen_copy_calls','producer_capture_ms','producer_wait_writeback_ms','producer_whole_world_ms','source_acquire_ms','prepare_ms','native_consume_ms','request_to_reply_ms','qt_paint_ms']},success=True)
   if a.normal:
    hist=native['all_warm_world_ms'];counts=[int(x) for x in hist['histogram']];count=int(hist['count']);assert len(counts)==151 and sum(counts)==count==a.queues-63 and hist['maximum']<=0xffffffff
    def rank(n):
     seen=0
     for ms,number in enumerate(counts):
      seen+=number
      if seen>n:return ms
     raise ValueError('Histogram rank outside samples')
    world=dict(samples=count,median=(rank((count-1)//2)+rank(count//2))/2,p95=rank(min(count-1,int(count*.95))),maximum=hist['maximum'],scope='All completed original queues from generation64; last1024 rows only for stage detail')
    report['profile']['producer_whole_world_ms']=world;growth=native['rss_maximum_bytes']-native['rss_warm_bytes'];limit=.001 if a.impossible_budget else 50
    checks=dict(warm_median=world is not None and world['median']<=limit,warm_p95=world is not None and world['p95']<=75,warm_maximum=world is not None and world['maximum']<=150,cold=native['cold_delivery_ms']<=500,original_fps=native['warm_original_fps']>=19,qt_paint_fps=native['warm_qt_paint_fps']>=19,rss=native['rss_warm_bytes']>0 and growth<=32*1024*1024,caches=native['producer_cache_maximum_bytes']<=64*1024*1024 and native['gpu_cache_maximum_frames']<=512,no_coalescing=int(native['coalesced_presentations'])==0,all_presentations=int(native['qt_paints'])==a.queues)
    report['performance_budget']=dict(passed=all(checks.values()),checks=checks,warm_world=world,median_limit_ms=limit,p95_limit_ms=75,maximum_limit_ms=150,cold_limit_ms=500,fps_minimum=19,rss_growth_bytes=growth,rss_limit_bytes=32*1024*1024,impossible_negative_control=a.impossible_budget)
    if not all(checks.values()):report.update(success=False,error='Full-path performance budget exceeded')
   if a.hardware:
    assert 'NVIDIA' in native['driver']['renderer'] and 'llvmpipe' not in native['driver']['renderer'],'Native hardware renderer fallback'
  if experiment:
   metadata=json.loads((experiment/'manifest.json').read_text());report['source_executable_sha256']=metadata['source_sha256'];report['staged_binaries']=metadata;report['inputs']={str(experiment/'game'/n):h for n,h in json.loads((experiment/'input-fingerprints.json').read_text()).items()};report['inputs_stable']=all(sha(Path(n))==h for n,h in report['inputs'].items())
 except Exception as e:report.update(success=False,error=str(e))
 finally:
  if process and process.poll() is None:
   os.killpg(process.pid,signal.SIGINT)
   try:process.wait(timeout=20)
   except subprocess.TimeoutExpired:os.killpg(process.pid,signal.SIGKILL);process.wait(timeout=5)
  if gpu_server and gpu_server.poll() is None:
   os.killpg(gpu_server.pid,signal.SIGTERM);gpu_server.wait(timeout=10)
  if before:run('original-after',[ROOT/'tools/original-manifest.sh','verify'],check=True)
  report.update(original_manifest_verified_before_after=before,sources_stable=all(sha(ROOT/n)==h for n,h in sources.items()),frozen_sources_stable=all(sha(out/'source'/n)==h for n,h in report.get('frozen_sources',{}).items()));report['success']=report['success'] and report['sources_stable'] and report['frozen_sources_stable'] and report.get('inputs_stable',False);(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
 if not report['success']:raise SystemExit(1)
if __name__=='__main__':main()
