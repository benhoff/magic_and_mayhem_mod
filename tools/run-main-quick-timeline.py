#!/usr/bin/env python3
"""One bounded Main-to-Quick diagnostic, retaining the durable timeline on failure."""
import argparse,hashlib,importlib.util,json,os,signal,sqlite3,subprocess,sys,time
from pathlib import Path
from coverage_evidence import write_report
from menu_transition_timeline import Recorder,now
from rendering_stall_sampler import OwnedRing
ROOT=Path(__file__).resolve().parents[1];PREFIX='research/runtime/main-quick-timeline-results-20261101/'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def module(name,path):
 s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--control',action='store_true');a=p.parse_args()
 if not a.report.startswith(PREFIX) or (ROOT/a.report).exists():p.error('New immutable owned report required')
 out=ROOT/'working/main-quick-timeline'/Path(a.report).stem;out.mkdir(parents=True,exist_ok=False);(out/'.retain').write_text('Bounded Main/Quick observations, including failed attempts; preserve exact bytes.\n')
 result=dict(schema=1,kind='main_quick_timeline_acquisition',success=False,native_journey_success=False,control=a.control,nominal_acceptance=False,original_behavior_recovered=False,wire_observer_writes=False,producer_deadlines_changed=False,scope='First Main/Quick transition only. Host timestamped observations, read-only wire visibility and untimestamped original callback receipts. Up to one controlled read-suppression timeout and two unforced full-intro launches in this worktree; no rendering equivalence or whole-player acceptance.')
 db=None;child=server=lease=None;session=None;observer=None;before=set();verified=False;env=None;owner=None;resolver=None
 def run(label,args,timeout=300):
  with (out/(label+'.log')).open('xb') as log:subprocess.run([str(x) for x in args],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=timeout)
 try:
  build=(ROOT/'working/build/qt-shell').resolve();tmp=build.parent/'tmp';tmp.mkdir(exist_ok=True);os.environ['TMPDIR']=str(tmp)
  run('build',['cmake','--build',build,'--target','mnm-qt-shell','--parallel','4'])
  campaign=module('timeline_campaign','tools/test-native-campaign.py');launcher=module('timeline_launcher','tools/run-native-owned.py')
  compiled=set(campaign.linked_compiler_sources(build,ROOT));names=compiled|{'tools/run-main-quick-timeline.py','tools/menu_transition_timeline.py','tools/rendering_stall_sampler.py','tools/run-native-owned.py','tools/run-native-game.py','tools/run-qt-shell.sh','tools/native_only_contract.py','tools/native_prefix_template.py','tools/test-native-campaign.py','runtime/menu/observer.c','runtime/menu/channel.h','protocols/include/mnm/menu_v1.h','protocols/include/mnm/menu_v5.h'}
  sources={n:sha(ROOT/n) for n in sorted(names)};result['sources']=sources
  binary=build/'mnm-qt-shell';name=str(Path(a.report).with_suffix(''))+'-player.bin';result['player']=dict(record=name,sha256=write_report(ROOT,name,binary.read_bytes()),compiled_sources=sorted(compiled))
  result['fixture_executable_sha256']=sha(ROOT/'working/game-nocd/Chaos.exe');assert result['fixture_executable_sha256']=='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
  run('original-before',[ROOT/'tools/original-manifest.sh','verify']);verified=True
  env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE')) and k not in ['DISPLAY','XAUTHORITY','LIBGL_ALWAYS_SOFTWARE','GALLIUM_DRIVER','MESA_LOADER_DRIVER_OVERRIDE','QT_XCB_GL_INTEGRATION','__GLX_VENDOR_LIBRARY_NAME','__EGL_VENDOR_LIBRARY_FILENAMES']}
  env.update(QT_QPA_PLATFORM='xcb',LP_NUM_THREADS='4',QT_SCALE_FACTOR='1',WINEDEBUG='-all',TMPDIR=str(tmp),MNM_QT_BUILD_TYPE='Release',MNM_QT_BUILD_TESTS='OFF',MNM_MENU_GAME_SOURCE=str(ROOT/'working/game-nocd'),MNM_LIVE_MENU_TEST_QT_JOURNEY='1',MNM_QUICK_BATTLE_MAP_ROW='0',MNM_ACCEPTANCE_PILOT='1',MNM_ACCEPTANCE_NORMAL='1',MNM_ACCEPTANCE_DIAGNOSTIC='1',MNM_ACCEPTANCE_SKIP_INTROS=str(int(a.control)),MNM_MENU_TIMELINE='1',MNM_MENU_TIMELINE_CONTROL=str(int(a.control)),MNM_TERRAIN_LIGHTING='native',MNM_NO_ROUTE_SCREENSHOTS='1',XDG_CONFIG_HOME=str(out/'config'))
  host=out/'host';host.mkdir();server,display,_=launcher.start_display(host,allow_glx=True);env.update(DISPLAY=display['display'],XAUTHORITY=display['authority'])
  from native_prefix_template import borrow
  lease,result['prefix_pool']=borrow(ROOT/'working/fixtures/prefix-seed',out/'wineprefix',env)
  before=set((ROOT/'working/tests/native-output-owner').glob('run-*'));command=[str(ROOT/'run-native.sh'),'--prefix',str(out/'wineprefix'),'--','--live-menu-test',str(out/'flow.json')]
  if a.control:command.append('--skip-movies')
  result['command']=command
  # A launch is charged before Popen; an interrupted/incomplete attempt still
  # consumes its slot. This database is append-only and never reset by the tool.
  budget=sqlite3.connect(ROOT/'working/main-quick-timeline/attempt-budget.sqlite3');budget.execute('CREATE TABLE IF NOT EXISTS attempts(id INTEGER PRIMARY KEY,report TEXT UNIQUE,control INTEGER,started_ns INTEGER)');budget.execute('BEGIN IMMEDIATE');used=budget.execute('SELECT COUNT(*) FROM attempts WHERE control=?',(int(a.control),)).fetchone()[0]
  if used>=(1 if a.control else 2):raise ValueError('Bounded native launch budget exhausted')
  budget.execute('INSERT INTO attempts(report,control,started_ns) VALUES(?,?,?)',(a.report,int(a.control),now()));budget.commit();budget.close();result['attempt_ordinal_in_mode']=used+1
  observer=Recorder(out/'timeline.sqlite3');deadline=time.monotonic()+390
  with (out/'public-launch.log').open('xb') as log:
   child=subprocess.Popen(command,cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
   while child.poll() is None:
    if time.monotonic()>deadline:raise TimeoutError('Bounded native journey collector deadline')
    owners=set((ROOT/'working/tests/native-output-owner').glob('run-*'))-before
    if len(owners)>1:raise ValueError('Multiple public owners in owned checkout')
    if owners and resolver is None:
     owner=next(iter(owners))/'owner.json'
     if owner.is_file():resolver=OwnedRing(ROOT,owner)
    if resolver is not None:
     found=resolver.read()
     if found is not None:session=resolver.path.parent
    if session is not None:observer.observe(session)
    time.sleep(.01)
  result['exit_code']=child.returncode
  subprocess.run(['wineserver','-w'],env=dict(env,WINEPREFIX=str(out/'wineprefix')),check=True,timeout=15);result['private_wine_stopped']=True
  if session is None:raise ValueError('No admitted observation generation')
  observer.observe(session,final=True);result['timeline']=observer.summary();result['session']=str(session);result['owner']=str(owner)
  flow=json.loads((out/'flow.json').read_bytes());result['flow']=flow
  result['native_journey_success']=child.returncode==0 and flow.get('success') is True
  if result['native_journey_success']:
   from native_only_contract import outcome
   result['retirement']=outcome(session);assert result['retirement']['state']=='completed'
  if a.control:
   t=result['timeline'];result['control_verified']=bool(child.returncode and t['control'] and t['fallback'] and not t['destination_ready'] and t['original_callback_pair_retained'] and t['ack_observed_before_client_retire'] and 'Menu adapter timed out' in flow.get('error',''))
   result['success']=result['control_verified']
  else:
   result['unforced_main_failure_reproduced']=result['timeline']['outcome']=='timeout_or_fallback' and 'Menu adapter timed out' in flow.get('error','')
   result['success']=result['native_journey_success'] or result['unforced_main_failure_reproduced']
  assert result['timeline']['header_closed']
 except Exception as error:result.update(success=False,error=repr(error))
 finally:
  if child is not None and child.poll() is None:
   try:os.killpg(child.pid,signal.SIGTERM);child.wait(timeout=15)
   except (ProcessLookupError,subprocess.TimeoutExpired):
    try:os.killpg(child.pid,signal.SIGKILL);child.wait(timeout=10)
    except ProcessLookupError:pass
  if env is not None and child is not None:
   try:subprocess.run(['wineserver','-k'],env=dict(env,WINEPREFIX=str(out/'wineprefix')),timeout=10,check=False);subprocess.run(['wineserver','-w'],env=dict(env,WINEPREFIX=str(out/'wineprefix')),timeout=15,check=True);result['private_wine_stopped']=True
   except Exception as e:result['wine_stop_error']=repr(e)
  if observer is not None:
   try:
    observer.close()
    with sqlite3.connect((out/'timeline.sqlite3').as_uri()+'?mode=ro',uri=True) as check:
     assert check.execute('PRAGMA integrity_check').fetchone()[0]=='ok';result['persisted_host_rows']=check.execute('SELECT COUNT(*) FROM host_events').fetchone()[0]
   except Exception as e:result.update(success=False,timeline_close_error=repr(e))
  if server is not None:
   try:server.terminate();server.wait(timeout=10)
   except subprocess.TimeoutExpired:server.kill();server.wait(timeout=5);result['display_forced_stop']=True
  if lease is not None:
   try:lease.close();result['prefix_returned']=True;result['prefix_pool']['returned']=True;result['prefix_pool']['receipt']=str(lease.receipt)
   except Exception as e:result.update(success=False,prefix_return_error=repr(e))
  if verified:
   try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);result['original_manifest_verified_before_after']=True
   except Exception as e:result.update(success=False,original_verification_error=repr(e))
  if 'sources' in result:
   result['sources_stable']=all(sha(ROOT/n)==h for n,h in result['sources'].items());result['player_binary_stable']=sha(binary)==result.get('player',{}).get('sha256');result['compiler_closure_stable']=compiled==set(campaign.linked_compiler_sources(build,ROOT));result['success'] &= result['sources_stable'] and result['player_binary_stable'] and result['compiler_closure_stable']
  result['artifacts']=[];files=[f for f in out.iterdir() if f.is_file() and f.name!='Xauthority']
  if session is not None:
   files += [session/n for n in ['menu-host-timeline.bin','events.bin','channel.bin','native-media.bin','native-media-report.json','native-only-session.json','native-output-admission.json','original-process-result.json','render-input.bin.acceptance','manifest.json'] if (session/n).is_file()]
  for f in sorted(set(files)):
   if f.stat().st_size>64*1024**2:raise ValueError('Unexpected unbounded diagnostic artifact')
   raw=f.read_bytes();n=str(Path(a.report).with_suffix(''))+'-'+hashlib.sha256(str(f.relative_to(ROOT)).encode()).hexdigest()[:20]+'.bin';result['artifacts'].append(dict(path=str(f.relative_to(ROOT)),record=n,sha256=write_report(ROOT,n,raw),bytes=len(raw)))
  write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps({k:result.get(k) for k in ['success','control','control_verified','native_journey_success','unforced_main_failure_reproduced','error','timeline']}),flush=True)
  if result.get('private_wine_stopped') and result.get('prefix_returned') and session is not None:
   from evidence_workspace import register_completed
   register_completed(ROOT,[out,session],dict(record=a.report,sha256=sha_report(ROOT/a.report)))
 return 0 if result['success'] else 1

def sha_report(path):
 from coverage_evidence import read_bytes
 return hashlib.sha256(read_bytes(path)).hexdigest()
if __name__=='__main__':raise SystemExit(main())
