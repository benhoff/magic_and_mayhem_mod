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
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--software',action='store_true');p.add_argument('--skip-intros',action='store_true');a=p.parse_args()
 if not a.report.startswith(PREFIX) or (ROOT/a.report).exists():p.error('New owned immutable report required')
 token=Path(a.report).stem;out=ROOT/'working/pilot'/token;out.mkdir(parents=True,exist_ok=False);(out/'.retain').write_text('Actual public pilot and all negative attempts. Retain before RAM teardown.\n')
 launcher=module('run-native-owned.py');campaign=module('test-native-campaign.py')
 names=set(campaign.linked_compiler_sources(ROOT/'working/build/qt-shell',ROOT))
 names.update(['run-native.sh','tools/run-native-game.py','tools/run-native-owned.py','tools/run-qt-shell.sh','tools/native_only_contract.py','tools/native_prefix_template.py','tools/directplay_dx6_launch.py','tools/working-retention.py','tools/run-rendering-acceptance-pilot.py'])
 sources={n:sha(ROOT/n) for n in sorted(names)}
 report=dict(schema=1,kind='public_native_pilot_acquisition',success=False,player_accepted=False,
  sources=sources,source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),session_identity=secrets.token_hex(16),
  configuration=dict(id='software-probe' if a.software else 'default-pilot',software_flag=a.software,skip_intros=a.skip_intros,terrain_lighting='original',native_lighting_capture=False,viewport_scaling='integer-sharp'),
  original_pixels_used_as_native_inputs=False,original_reference_executed=False,scope=__doc__)
 env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE')) and k not in ['DISPLAY','XAUTHORITY','LIBGL_ALWAYS_SOFTWARE','GALLIUM_DRIVER','MESA_LOADER_DRIVER_OVERRIDE','QT_XCB_GL_INTEGRATION','__GLX_VENDOR_LIBRARY_NAME','__EGL_VENDOR_LIBRARY_FILENAMES']}
 env.update(QT_QPA_PLATFORM='xcb',LP_NUM_THREADS='4',QT_SCALE_FACTOR='1',WINEDEBUG='-all',TMPDIR=str(out),
  MNM_QT_BUILD_TYPE='Release',MNM_QT_BUILD_TESTS='OFF',MNM_MENU_GAME_SOURCE=str(ROOT/'working/game-nocd'),
  MNM_LIVE_MENU_TEST_QT_JOURNEY='1',MNM_QUICK_BATTLE_MAP_ROW='0',MNM_ACCEPTANCE_PILOT='1',MNM_ACCEPTANCE_SKIP_INTROS=str(int(a.skip_intros)),
  MNM_TERRAIN_LIGHTING='original',MNM_NO_ROUTE_SCREENSHOTS='1',MNM_RESIDENT_STAGE_PROFILE='1',MNM_RESIDENT_PROFILE_CAPTURE=str(out/'worlds.bin'),
  MNM_ACCEPTANCE_CAPTURE_DIRECTORY=str(out/'capture'),MNM_ACCEPTANCE_CAPTURE_ID=report['session_identity'])
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
   child.wait(timeout=390)
  report['exit_code']=child.returncode
  subprocess.run(['wineserver','-w'],env=dict(env,WINEPREFIX=str(out/'wineprefix')),check=True,timeout=10)
  report['private_wine_stopped']=True
  if child.returncode:raise ValueError('Public launcher failed; retained its actual output')
  flow=json.loads((out/'flow.json').read_text());report['flow']=flow
  if flow.get('success') is not True or flow.get('world_visits')!=2 or flow.get('input_via_qt_viewport') is not True or flow.get('native_fallback') is not False:raise ValueError('Actual Quick Battle action route incomplete')
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
  # Physical files here are bounded execution interchange. Citable bytes go
  # directly to SQLite before this RAM namespace may retire.
  report['artifacts']=[]
  candidates=[f for f in out.rglob('*') if f.is_file() and 'wineprefix' not in f.relative_to(out).parts and f.name!='Xauthority']
  for owner in set((ROOT/'working/tests/native-output-owner').glob('run-*'))-before:
   candidates += [f for f in owner.rglob('*') if f.is_file() and f.name!='Xauthority']
  experiments=set((ROOT/'working/experiments/menu-observer').glob('run-*'))-before_experiments
  report['experiments']=[str(v) for v in sorted(experiments)]
  for session in sorted(experiments):
   candidates += [f for f in session.rglob('*') if f.is_file() and ('game' not in f.relative_to(session).parts or f.suffix.lower() in ('.dll','.cfg','.sav','.ini'))]
  for f in sorted(set(candidates)):
   if f.is_symlink() or f.stat().st_size>512*1024**2:continue
   relative=str(f.relative_to(ROOT));name=str(Path(a.report).with_suffix(''))+'-raw-'+hashlib.sha256(relative.encode()).hexdigest()[:20]+'.bin'
   raw=f.read_bytes();digest=write_report(ROOT,name,raw);report['artifacts'].append(dict(path=relative,record=name,sha256=digest,bytes=len(raw)))
  report['artifact_count']=len(report['artifacts']);report['temporary_root']=str(out)
  digest=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps({k:report.get(k) for k in ['success','error','exit_code','artifact_count']}|{'record':a.report,'sha256':digest}),flush=True)
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
