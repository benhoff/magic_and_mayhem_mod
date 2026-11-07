#!/usr/bin/env python3
"""Two bounded public Quit Battle/Results/Main focus cases on the fixed player."""
import argparse,hashlib,importlib.util,json,os,signal,sqlite3,struct,subprocess,sys,time
from pathlib import Path
from coverage_evidence import Reader,write_report
from native_only_contract import configure,outcome
from native_prefix_template import borrow
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-functional-ten-results-20261107/'
PLAN=PREFIX+'input-plan-03.json'

def module(name):
 s=importlib.util.spec_from_file_location('return_'+name.replace('-','_'),ROOT/'tools'/(name+'.py'));m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
base=module('run-rendering-menu-acceptance')
sha=base.sha

class Journey(base.Journey):
 def click_id(self,name):
  values=[v for _,v in self.access.nodes() if v.get('accessible_id','').endswith('.'+name)]
  require(len(values)==1 and values[0]['enabled'],'Missing physical control '+name)
  self.actions.append(dict(action='click',object=name,host_ns=time.monotonic_ns(),physical=self.input.click(values[0]['rect'])))
 def run(self):
  self.input.activate();self.click('Launch game')
  # No AT-SPI calls while synchronous staging owns the GUI thread.
  self.wait(3,0,timeout=90);self.capture('main-initial','Quick Battle')
  self.click('Preferences');preferences=self.wait(10,1)['preferences'];require(preferences[2]==0,'Expected original High mode')
  self.key('preferences-cancel',0xff1b);self.wait(3,2)
  self.click('Quick Battle');self.wait(22,3);self.click('Create Single Player Game');self.wait(14,4)
  self.click('Map');self.wait(25,5)
  lists=[(n,v) for n,v in self.access.nodes() if v['role']=='list'];require(len(lists)==1,'Ambiguous map list')
  self.input.click(lists[0][1]['rect']);self.key('map-first',0xff50)
  selected=lists[0][0].get_selection_iface().get_selected_child(0);require(selected and selected.get_name()==self.last['maps'][0],'Wrong map selection')
  self.key('map-confirm',0xff0d);setup=self.wait(14,6)['setup'];require(setup['map']==1,'Wrong Quick map')
  self.click_id('singlePlayerStart');self.wait(7,7,timeout=60);self.click_id('spellboxOK')
  # The live report is published at retirement; a completed World handoff also
  # leaves the original input sampler's return ordinal. Read it, never write it.
  deadline=time.monotonic()+45;last=None
  while time.monotonic()<deadline:
   require(self.child.poll() is None,'Host exited before World input')
   p=self.session/'input-admission.bin'
   if p.exists():
    raw=p.read_bytes()
    if len(raw)>=160 and raw[:8]==b'MNMCAI01' and (len(raw)-16)%144==0:
     fields=struct.unpack_from('<36I',raw,len(raw)-144)
     if fields[32]>=10:last=dict(original_row=fields[0],thread=fields[2],world_returns=fields[32]);break
   time.sleep(.05)
  require(last is not None,'Ten returned Worlds not observed')
  self.capture('world-before-exit')
  code=self.input.x.XKeysymToKeycode(self.input.display,0xff1b)
  for down,delay in ((1,.6),(0,0)):
   require(self.input.xt.XTestFakeKeyEvent(self.input.display,code,down,0),'Escape physical delivery failed');self.input.x.XFlush(self.input.display);time.sleep(delay)
  self.actions.append(dict(action='world-open-mini',host_ns=time.monotonic_ns(),physical=dict(symbol=0xff1b,down_up=True,held_ms=600,host_focus=self.input.focus())))
  deadline=time.monotonic()+30;value=None
  while time.monotonic()<deadline:
   value=self.read()
   if value and value[0]['ready'] and value[0]['screen'] in (17,26):break
   require(self.child.poll() is None,'Host exited before Results');time.sleep(.05)
  require(value and value[0]['ready'] and value[0]['screen'] in (17,26),'No actual Mini/Results after Escape')
  via_mini=value[0]['screen']==17
  if via_mini:self.wait(17);self.capture('mini-ready');self.click_id('miniMenuButton7')
  result=self.wait(26,timeout=30);self.capture('results-ready');self.click_id('quickResultAction2')
  returned=self.wait(22);self.capture('quick-returned');self.key('quick-return-main',0xff1b)
  main=self.wait(3);shot=self.capture('main-returned','Quick Battle')
  # Prove that focus and input are useful after Results, then normal original Quit.
  self.key('main-focused-space',0x20);self.wait(22);self.key('quick-cancel-again',0xff1b);self.wait(3)
  self.capture('main-before-quit','Quick Battle');self.click('Quit')
  return dict(preferences=preferences,setup=setup,world_ready=last,result=result,returned_setup=returned,returned_main=main,focused_main_sha256=shot['sha256'],entry='public_quit_battle' if via_mini else 'public_escape_results',natural_victory_defeat_proved=False,independent_world_pixel_comparison=False)

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--software',action='store_true');a=p.parse_args()
 require(a.report.startswith(PREFIX) and '..' not in Path(a.report).parts and not (ROOT/a.report).exists(),'Fresh owned report required')
 out=ROOT/'working/results-return'/Path(a.report).stem;out.mkdir(parents=True,exist_ok=False);(out/'.retain').touch()
 result=dict(schema=1,kind='public_results_return_acquisition',success=False,configuration=dict(software_flag=a.software,actual_size=[800,600],skip_intros=True,terrain_lighting='native',compiled_test_driver_invoked=False),original_game_runs=0,scope=__doc__)
 hs=ls=child=journey=lease=None;before=False;env={}
 with Reader(ROOT) as r:
  plan=r.json(PLAN);plan_digest=r.digest(PLAN);require(plan['success'] and all(r.digest(n)==h for n,h in plan['compiler_sources'].items()),'Fixed-player compiler drift')
  binary=ROOT/plan['player_binary']['path'];require(sha(binary)==plan['player_binary']['sha256'],'Fixed player changed')
 names=set(plan['compiler_sources'])|{'tools/run-rendering-results-return.py','tools/run-rendering-menu-acceptance.py','tools/rendering_menu_accessibility.py','tools/native_only_contract.py','tools/native_prefix_template.py','tools/run-native-owned.py','tools/test-menu-observer.py'}
 sources={n:sha(ROOT/n) for n in sorted(names)};result.update(sources=sources,compiler_sources=plan['compiler_sources'],player_binary=plan['player_binary'],input_plan=dict(record=PLAN,sha256=plan_digest))
 try:
  subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True);before=True
  base.module('run-opengl-game.py').patch_staged_preferences(ROOT/'working/game-nocd','VIDEO',{'IsHighRes':'TRUE'})
  result['installation_inputs']={str(f.relative_to(ROOT/'working/game-nocd')):dict(sha256=sha(f),bytes=f.stat().st_size) for f in (ROOT/'working/game-nocd').rglob('*') if f.is_file()}
  from rendering_progression import prepare_launch_manifest
  raw=prepare_launch_manifest(ROOT/'working/game-nocd',ROOT/'working/manifests/game-nocd.tsv');n=a.report.removesuffix('.json')+'-launch-manifest.bin';result['launch_manifest']=dict(record=n,sha256=write_report(ROOT,n,raw))
  host=out/'host';owner=out/'owner';host.mkdir();owner.mkdir();launcher=base.module('run-native-owned.py')
  hs,hr,_=launcher.start_display(host,allow_glx=True);ls,lr,cookie=launcher.start_display(owner)
  runtime=out/'runtime';runtime.mkdir(mode=0o700)
  env=configure({k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))},(['--software-rendering'] if a.software else [])+['--skip-movies'])
  env.update(DISPLAY=hr['display'],XAUTHORITY=hr['authority'],MNM_NATIVE_OUTPUT_OWNER=str(owner/'owner.json'),MNM_LEGACY_DISPLAY=lr['display'],MNM_LEGACY_XCOOKIE=cookie.hex(),MNM_MENU_GAME_SOURCE=str(ROOT/'working/game-nocd'),MNM_MENU_DIRECTPLAY_DX6='1',MNM_CACHE_ADAPTERS='1',MNM_VERIFY_ORIGINALS='0',MNM_VERIFY_ASSETS='0',MNM_TERRAIN_LIGHTING='native',QT_QPA_PLATFORM='xcb',QT_LINUX_ACCESSIBILITY_ALWAYS_ON='1',QT_ACCESSIBILITY='1',LP_NUM_THREADS='4',QT_SCALE_FACTOR='1',WINEDEBUG='-all',XDG_CONFIG_HOME=str(out/'config'),XDG_CACHE_HOME=str(out/'cache'),LIBGL_ALWAYS_SOFTWARE='1',__GLX_VENDOR_LIBRARY_NAME='mesa',QT_XCB_GL_INTEGRATION='xcb_glx',MNM_ACCEPTANCE_GAMEPLAY='1',MNM_RESIDENT_STAGE_PROFILE='1',XDG_RUNTIME_DIR=str(runtime),__EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json',GALLIUM_DRIVER='llvmpipe')
  lease,result['prefix_pool']=borrow(ROOT/'working/fixtures/prefix-seed',out/'wineprefix',env);env['MNM_OWNED_WINE_PREFIX']=str(out/'wineprefix')
  with sqlite3.connect(ROOT/'working/results-return-attempts.sqlite3') as db:
   db.execute('CREATE TABLE IF NOT EXISTS attempts(report TEXT PRIMARY KEY,software INTEGER)')
   previous=list(db.execute('SELECT report,software FROM attempts'))
   with Reader(ROOT) as r:
    repair=r.json(plan['preparation_repair']['record']);direct=r.json(plan['direct_results_repair']['record']);excluded={repair['parent']['record']:repair['parent']['sha256'],direct['parent']['record']:direct['parent']['sha256']};positive=[]
    for n,s in previous:
     row=r.json(n)
     if n in excluded:require(not row['success'] and r.digest(n)==excluded[n] and row['original_game_runs']==int(n==direct['parent']['record']),'Changed exact repair exclusion')
     else:require(row['success'],'Stop on first further Results failure');positive.append((n,s))
    require(len(previous)<4 and len(positive)<2 and all(s!=int(a.software) for _,s in positive),'Results attempt/game budget exhausted')
   db.execute('INSERT INTO attempts VALUES(?,?)',(a.report,int(a.software)))
  command=[str(binary),'--repo',str(ROOT),'--renderer','opengl','--live-menus','--native-commands','--resident-world','--native-only','--skip-movies']
  if a.software:command.append('--software-rendering')
  result['command']=command;result['game_launch_requested']=False
  with (out/'qt.log').open('xb') as log:
   child=subprocess.Popen(command,cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
   os.environ.update(DISPLAY=env['DISPLAY'],XAUTHORITY=env['XAUTHORITY']);journey=Journey(child,out,env)
   deadline=time.monotonic()+20
   while time.monotonic()<deadline:
    require(child.poll() is None,'Host failed to open')
    if journey.input.find_window() and any(v['name']=='Launch game' for _,v in journey.access.nodes()):break
    time.sleep(.1)
   result['game_launch_requested']=True;result['proof']=journey.run()
   deadline=time.monotonic()+30
   while time.monotonic()<deadline:
    try:result['retirement']=outcome(journey.session);break
    except (ValueError,FileNotFoundError):time.sleep(.1)
   require(result.get('retirement',{}).get('state')=='completed','Normal retirement incomplete')
   result['resident']=json.loads((journey.session/'resident-world-report.json').read_text());child.wait(timeout=10);result['exit_code']=child.returncode
   require(child.returncode==0,'Qt host failed normal exit')
  subprocess.run(['wineserver','-w'],env=dict(env,WINEPREFIX=str(out/'wineprefix')),timeout=10,check=True);result['private_wine_stopped']=True;result['success']=True
 except Exception as e:
  result['error']=str(e)
  if journey:
   try:journey.capture('failure-last')
   except Exception as e:result['failure_capture_error']=str(e)
 finally:
  if journey:result.update(states=journey.states,actions=journey.actions,screenshots=journey.screenshots,session=str(journey.session) if journey.session else None);journey.input.close()
  if child and child.poll() is None:
   os.killpg(child.pid,signal.SIGTERM)
   try:child.wait(timeout=12)
   except subprocess.TimeoutExpired:os.killpg(child.pid,signal.SIGKILL);child.wait(timeout=10)
  if lease and not result['success']:
   subprocess.run(['wineserver','-k'],env=dict(env,WINEPREFIX=str(out/'wineprefix')),timeout=10);subprocess.run(['wineserver','-w'],env=dict(env,WINEPREFIX=str(out/'wineprefix')),timeout=10);result['forced_private_prefix_stop']=True
  for server in (hs,ls):
   if server and server.poll() is None:server.terminate();server.wait(timeout=10)
  if lease:
   try:
    from experiment_environment import _idle
    deadline=time.monotonic()+20
    while True:
     try:_idle(out/'wineprefix','wineprefix');break
     except (ValueError,subprocess.TimeoutExpired):
      if time.monotonic()>=deadline:raise
      time.sleep(.2)
    lease.close();result['prefix_pool']['returned']=True
   except Exception as e:result.update(success=False,prefix_return_error=str(e))
  if before:
   try:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True);result['original_manifest_verified_before_after']=True
   except Exception as e:result.update(success=False,original_verification_error=str(e))
  result['original_game_runs']=int(bool(journey and journey.states));result['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());result['success']&=result['sources_stable']
  files=[f for f in out.rglob('*') if f.is_file() and 'wineprefix' not in f.relative_to(out).parts and f.name!='Xauthority']
  if journey and journey.session:files += [f for f in journey.session.rglob('*') if f.is_file() and (f.parent==journey.session or f.parent==journey.session/'game' and (f.suffix.lower()=='.dll' or f.name=='Chaos.exe' or f.name.startswith('native-')) or f.relative_to(journey.session).as_posix() in ('game/CFG/chaos.cfg','game/CFG/prefs.cfg','game/CFG/Encrypted/chaos.cfg','game/CFG/Encrypted/prefs.cfg','native-music/manifest.json'))]
  result['artifacts']=[]
  for f in sorted(set(files)):
   if f.is_symlink() or f.stat().st_size>512*1024**2:continue
   n=a.report.removesuffix('.json')+'-raw-'+hashlib.sha256(str(f.relative_to(ROOT)).encode()).hexdigest()[:20]+'.bin';raw=f.read_bytes();result['artifacts'].append(dict(path=str(f.relative_to(ROOT)),record=n,sha256=write_report(ROOT,n,raw),bytes=len(raw)))
  h=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=result['success'],record=a.report,sha256=h,error=result.get('error'),original_game_runs=result['original_game_runs'])),flush=True)
 return 0 if result['success'] else 1
if __name__=='__main__':raise SystemExit(main())
