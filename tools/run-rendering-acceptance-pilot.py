#!/usr/bin/env python3
"""Capture a real public-player pilot; retain all results without acceptance promotion."""
import argparse,hashlib,importlib.util,json,os,secrets,signal,subprocess,sys,time
from pathlib import Path
from coverage_evidence import write_report
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-acceptance-pilot-results-20261031/'
def sha(path):
 with Path(path).open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def module(name):
 spec=importlib.util.spec_from_file_location(name,ROOT/'tools'/name);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--software',action='store_true');p.add_argument('--skip-intros',action='store_true');p.add_argument('--trace-timing',action='store_true');p.add_argument('--diagnostic-only',action='store_true');p.add_argument('--stall-intro-host',action='store_true');p.add_argument('--stall-world-host',action='store_true');p.add_argument('--storage-policy',type=Path);p.add_argument('--scenario',choices=['pilot','normal-quick','normal-storage'],default='pilot');a=p.parse_args()
 if a.stall_intro_host and a.stall_world_host:p.error('Choose one diagnostic host fault')
 if a.stall_world_host:
  if a.scenario!='normal-quick':p.error('World host fault requires normal-quick')
  a.diagnostic_only=True
 if a.stall_intro_host:
  if a.skip_intros or a.scenario!='normal-quick':p.error('Intro host fault requires normal-quick with enabled intros')
  a.diagnostic_only=True
 normal=a.scenario!='pilot';prefix='research/runtime/rendering-normal-results-20261031/' if normal else PREFIX
 if not a.report.startswith(prefix) or (ROOT/a.report).exists():p.error('New owned immutable report required')
 token=Path(a.report).stem;out=ROOT/'working/pilot'/token;out.mkdir(parents=True,exist_ok=False);(out/'.retain').write_text('Actual public pilot and all negative attempts. Retain before RAM teardown.\n')
 launcher=module('run-native-owned.py');campaign=module('test-native-campaign.py')
 # The public launcher can rebuild. Freeze dependencies only after that build,
 # then require the same executable and dependency graph after execution.
 build=(ROOT/'working/build/qt-shell').resolve()
 build_tmp=build.parent/'tmp';build_tmp.mkdir(parents=True,exist_ok=True)
 build_commands=[['cmake','-S',str(ROOT/'apps/qt-shell'),'-B',str(build),'-DCMAKE_BUILD_TYPE=Release','-DBUILD_TESTING=OFF'],['cmake','--build',str(build),'--target','mnm-qt-shell','--parallel','4']]
 build_error=None
 try:
  with (out/'player-build.log').open('xb') as log:
   for command in build_commands:subprocess.run(command,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=300,env=dict(os.environ,TMPDIR=str(build_tmp)))
 except Exception as e:build_error=str(e)
 build_record=str(Path(a.report).with_suffix(''))+'-player-build.bin'
 build_sha=write_report(ROOT,build_record,(out/'player-build.log').read_bytes())
 if build_error:
  result=dict(schema=1,kind='native_player_build_preparation_failure',success=False,error=build_error,commands=build_commands,log=dict(record=build_record,sha256=build_sha),original_artifacts_consumed=False)
  digest=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=False,error=build_error,record=a.report,sha256=digest)),flush=True);return 1
 compiled=set(campaign.linked_compiler_sources(build,ROOT))
 binary=build/'mnm-qt-shell';binary_record=str(Path(a.report).with_suffix(''))+'-player-binary.bin'
 binary_sha=write_report(ROOT,binary_record,binary.read_bytes())
 names=set(compiled)
 names.update(['run-native.sh','tools/run-native-game.py','tools/run-native-owned.py','tools/run-qt-shell.sh','tools/native_only_contract.py','tools/native_prefix_template.py','tools/directplay_dx6_launch.py','tools/working-retention.py','tools/run-rendering-acceptance-pilot.py','tools/test-native-campaign.py'])
 if normal:names.update(['tools/menu-storage-input.py','tools/campaign_storage.py'])
 sources={n:sha(ROOT/n) for n in sorted(names)}
 report=dict(schema=1,kind='public_native_watchdog_diagnostic_acquisition' if a.diagnostic_only else 'public_native_normal_acquisition' if normal else 'public_native_pilot_acquisition',success=False,player_accepted=False,scenario=a.scenario,
  sources=sources,source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),session_identity=secrets.token_hex(16),
  configuration=dict(id='software-probe' if a.software else 'default-pilot',software_flag=a.software,skip_intros=a.skip_intros,terrain_lighting='original',native_lighting_capture=False,viewport_scaling='integer-sharp'),
  original_pixels_used_as_native_inputs=False,original_reference_executed=False,scope=__doc__)
 report['player_build']=dict(commands=build_commands,log=dict(record=build_record,sha256=build_sha),binary=dict(path='working/build/qt-shell/mnm-qt-shell',record=binary_record,sha256=binary_sha),compiler_sources={n:sources[n] for n in sorted(compiled)})
 report['configuration']['diagnostic_only']=a.diagnostic_only
 env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE')) and k not in ['DISPLAY','XAUTHORITY','LIBGL_ALWAYS_SOFTWARE','GALLIUM_DRIVER','MESA_LOADER_DRIVER_OVERRIDE','QT_XCB_GL_INTEGRATION','__GLX_VENDOR_LIBRARY_NAME','__EGL_VENDOR_LIBRARY_FILENAMES']}
 env.update(QT_QPA_PLATFORM='xcb',LP_NUM_THREADS='4',QT_SCALE_FACTOR='1',WINEDEBUG='-all',TMPDIR=str(out),
  MNM_QT_BUILD_TYPE='Release',MNM_QT_BUILD_TESTS='OFF',MNM_MENU_GAME_SOURCE=str(ROOT/'working/game-nocd'),
  MNM_LIVE_MENU_TEST_QT_JOURNEY='1',MNM_QUICK_BATTLE_MAP_ROW='0',MNM_ACCEPTANCE_PILOT='1',MNM_ACCEPTANCE_SKIP_INTROS=str(int(a.skip_intros)),
  MNM_TERRAIN_LIGHTING='original',MNM_NO_ROUTE_SCREENSHOTS='1',MNM_RESIDENT_STAGE_PROFILE='1',MNM_RESIDENT_PROFILE_CAPTURE=str(out/'worlds.bin'),
  MNM_ACCEPTANCE_CAPTURE_DIRECTORY=str(out/'capture'),MNM_ACCEPTANCE_CAPTURE_ID=report['session_identity'])
 if normal:
  env.update(MNM_ACCEPTANCE_NORMAL='1',MNM_TERRAIN_LIGHTING='native',MNM_RESIDENT_CAPTURE_SOURCE_CHUNKS='1',XDG_CONFIG_HOME=str(out/'config'))
  report['configuration']['terrain_lighting']='native'
  report['configuration']['source_chunks']=True
  report['configuration'].update(viewport_capture_stored_limit_bytes=1024**3,world_capture_retained_limit_bytes=2*1024**3)
  report['configuration']['configuration_home']=str(out/'config')
 if a.trace_timing:
  env['MNM_RESIDENT_POLL_TIMING']='1'
  report['configuration']['bounded_host_timing_trace']=True
 if a.stall_intro_host:
  env['MNM_ACCEPTANCE_STALL_INTRO_HOST']='1'
  report['configuration']['injected_fault']=dict(kind='gui-stall-after-first-movie-frame',planned_ms=6200)
 if a.stall_world_host:
  env['MNM_ACCEPTANCE_STALL_WORLD_HOST']='1'
  report['configuration']['injected_fault']=dict(kind='gui-stall-after-first-world-paint',planned_ms=6200)
 if a.storage_policy:
  policy=a.storage_policy.resolve(strict=True)
  env['MNM_WORKING_STORAGE_POLICY']=str(policy)
  name=str(Path(a.report).with_suffix(''))+'-storage-policy.bin'
  report['storage_policy']=dict(path=str(policy),record=name,sha256=write_report(ROOT,name,policy.read_bytes()))
 if a.scenario=='normal-storage':
  deadline_ms=350000+(0 if a.skip_intros else 180000)
  report['configuration'].update(deadline_ms=deadline_ms,movie_budget_ms=0 if a.skip_intros else 180000,viewport_capture_stored_limit_bytes=1024**3,world_capture_retained_limit_bytes=2*1024**3)
  env.pop('MNM_LIVE_MENU_TEST_QT_JOURNEY')
  env.update(MNM_CAMPAIGN_SMOKE_TEST='1',MNM_CAMPAIGN_STORAGE='1',MNM_CAMPAIGN_DIFFICULTY='1',MNM_CAMPAIGN_RECREATE_WORLD='1',MNM_MENU_DISPLAY_OBSERVE='1',MNM_CAMPAIGN_STORAGE_NAME='native-render-'+secrets.token_hex(6),MNM_CAMPAIGN_TIMEOUT_MS=str(deadline_ms))
  report['configuration']['difficulty']=1
 report['installation_inputs']={str(f.relative_to(ROOT/'working/game-nocd')):dict(sha256=sha(f),bytes=f.stat().st_size) for f in sorted((ROOT/'working/game-nocd').rglob('*')) if f.is_file()}
 before_experiments=set((ROOT/'working/experiments/menu-observer').glob('run-*'))
 server=child=None;verified=False;before=set((ROOT/'working/tests/native-output-owner').glob('run-*'))
 def execute_log(label,command):
  with (out/(label+'.log')).open('xb') as log:subprocess.run([str(v) for v in command],stdout=log,stderr=subprocess.STDOUT,check=True,timeout=180)
 try:
  execute_log('original-before',[ROOT/'tools/original-manifest.sh','verify']);verified=True
  host=out/'host';host.mkdir();server,owner,_=launcher.start_display(host,allow_glx=True);env.update(DISPLAY=owner['display'],XAUTHORITY=owner['authority'])
  command=[str(ROOT/'run-native.sh'),'--prefix',str(out/'wineprefix'),'--prefix-template',str(ROOT/'working/fixtures/prefix-seed'),'--','--live-menu-test',str(out/'flow.json')]
  if a.software:command.append('--software-rendering')
  if a.skip_intros:command.append('--skip-movies')
  report['command']=command
  with (out/'public-launch.log').open('xb') as log:
   child=subprocess.Popen(command,cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
   child.wait(timeout=deadline_ms/1000+40 if a.scenario=='normal-storage' else 390)
  report['exit_code']=child.returncode
  subprocess.run(['wineserver','-w'],env=dict(env,WINEPREFIX=str(out/'wineprefix')),check=True,timeout=10)
  report['private_wine_stopped']=True
  if child.returncode:raise ValueError('Public launcher failed; retained its actual output')
  flow=json.loads((out/'flow.json').read_text());report['flow']=flow
  if a.scenario=='normal-storage':
   from campaign_storage import validate_storage_flow
   validate_storage_flow(flow)
  elif flow.get('success') is not True or flow.get('world_visits')!=2 or flow.get('input_via_qt_viewport') is not True or flow.get('native_fallback') is not False:raise ValueError('Actual Quick Battle action route incomplete')
  owners=set((ROOT/'working/tests/native-output-owner').glob('run-*'))-before
  if len(owners)!=1:raise ValueError('Ambiguous public launch owner')
  owner=owners.pop();report['owner']=str(owner);sessions=sorted((owner/'sessions').glob('*.json'))
  if len(sessions)!=1:raise ValueError('Pilot requires one normal player generation')
  session=Path(json.loads(sessions[0].read_text())['session']);report['session']=str(session)
  from native_only_contract import outcome
  report['retirement']=outcome(session)
  if report['retirement']['state']!='completed':raise ValueError('Normal original Quit/resources missing')
  report['native_admission']=json.loads((session/'native-output-admission.json').read_text())
  report['resident']=json.loads((session/'resident-world-report.json').read_text())
  report['success']=True
 except Exception as e:report['error']=str(e)
 finally:
  if child is not None and child.poll() is None:
   try:os.killpg(child.pid,signal.SIGTERM);child.wait(timeout=15)
   except (ProcessLookupError,subprocess.TimeoutExpired):
    try:os.killpg(child.pid,signal.SIGKILL);child.wait(timeout=10)
    except ProcessLookupError:pass
  if server is not None:server.terminate();server.wait(timeout=10)
  if verified:
   try:execute_log('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
   except Exception as e:report.update(success=False,original_verification_error=str(e))
  report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable']
  try:
   report['compiler_closure_stable']=set(campaign.linked_compiler_sources(build,ROOT))==compiled
   report['player_binary_stable']=sha(binary)==binary_sha
  except Exception as e:
   report.update(compiler_closure_stable=False,player_binary_stable=False,build_binding_error=str(e))
  report['success'] &= report['compiler_closure_stable'] and report['player_binary_stable']
  if a.storage_policy:
   report['storage_policy_stable']=sha(policy)==report['storage_policy']['sha256'];report['success'] &= report['storage_policy_stable']
  if a.stall_intro_host or a.stall_world_host:
   try:
    rows=[json.loads(line[len('NATIVE_HOST_FAULT '):]) for line in (out/'public-launch.log').read_text(errors='replace').splitlines() if line.startswith('NATIVE_HOST_FAULT ')]
    report['host_fault']=rows
    report['host_fault_recorded']=len(rows)==2 and [r['phase'] for r in rows]==['begin','end'] and all(r['planned_ms']==6200 and r['target']==('intro' if a.stall_intro_host else 'world') and len(bytes.fromhex(r['snapshot_hex']))==(2048 if a.stall_intro_host else 64) for r in rows) and int(rows[1]['observed_host_us'])-int(rows[0]['begin_host_us'])>=6200000
   except Exception as e:report.update(host_fault_recorded=False,host_fault_error=str(e))
   report['success'] &= report['host_fault_recorded']
  # Physical files here are bounded execution interchange. Citable bytes go
  # directly to SQLite before this RAM namespace may retire.
  report['artifacts']=[]
  candidates=[f for f in out.rglob('*') if f.is_file() and 'wineprefix' not in f.relative_to(out).parts and f.name!='Xauthority']
  for owner in set((ROOT/'working/tests/native-output-owner').glob('run-*'))-before:
   candidates += [f for f in owner.rglob('*') if f.is_file() and f.name!='Xauthority']
  experiments=set((ROOT/'working/experiments/menu-observer').glob('run-*'))-before_experiments
  report['experiments']=[str(v) for v in sorted(experiments)]
  for session in sorted(experiments):
   candidates += [f for f in session.rglob('*') if f.is_file() and ('game' not in f.relative_to(session).parts or f.suffix.lower() in ('.dll','.cfg','.sav','.ini') or (f.name.startswith('native-') and f.suffix=='.bin'))]
  for f in sorted(set(candidates)):
   limit=2*1024**3 if f==out/'worlds.bin' else 1024**3 if f.parent==out/'capture' and normal else 512*1024**2
   if f.is_symlink() or f.stat().st_size>limit:continue
   relative=str(f.relative_to(ROOT));name=str(Path(a.report).with_suffix(''))+'-raw-'+hashlib.sha256(relative.encode()).hexdigest()[:20]+'.bin'
   raw=f.read_bytes();digest=write_report(ROOT,name,raw);report['artifacts'].append(dict(path=relative,record=name,sha256=digest,bytes=len(raw)))
  report['artifact_count']=len(report['artifacts']);report['temporary_root']=str(out)
  digest=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps({k:report.get(k) for k in ['success','error','exit_code','artifact_count']}|{'record':a.report,'sha256':digest}),flush=True)
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
