#!/usr/bin/env python3
"""Exercise normal staged terrain lighting, campaign reload, fallback and clean Quit.

Uses the public prepare/run-menu pipeline with original graphics. The native Qt
presentation and arbitrary-map/default rollout remain separate milestones.
"""
import argparse,ctypes as c,fcntl,hashlib,re,importlib.util,json,mmap,os,signal,sqlite3,struct,subprocess,time,shlex
from pathlib import Path
from coverage_evidence import write_report,read_json
from coverage_claims import behavior_contract,required_sources,scenario_contract
import terrain_light_session as lighting_session
ROOT=Path(__file__).resolve().parents[1]
BUILD=lighting_session.HASH
MODELS=['terrain_lighting.cpp','terrain_creature_lighting.cpp','terrain_static_lighting.cpp','terrain_lighting_cycle.cpp']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def module(name,path):
 spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--main-root',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--report',type=Path,required=True);p.add_argument('--mode',choices=['observe','native'],required=True);p.add_argument('--validated-observation',type=Path);p.add_argument('--journey',choices=['campaign','campaign-reload'],default='campaign');p.add_argument('--worker-loss',action='store_true');p.add_argument('--prefix',type=Path,required=True);a=p.parse_args()
 out=a.output.resolve();target=a.report.absolute();main_root=a.main_root.resolve();prefix=a.prefix.resolve()
 if not os.environ.get('DISPLAY') or out.exists() or target.exists() or not out.is_relative_to(ROOT/'working') or not prefix.is_relative_to(ROOT/'working/terrain-light-session') or not target.is_relative_to(ROOT/'research/runtime'):p.error('Require private display and new owned output/report/prefix')
 if (a.mode=='native')!=(a.validated_observation is not None):p.error('Native mode requires matching current-source observation')
 out.mkdir(parents=True);(out/'.retain').write_text('Normal-session lighting evidence; retain raw sources, captures and diagnostics.\n');(out/'tmp').mkdir()
 env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(TMPDIR=os.environ.get('TMPDIR',str(out/'tmp')),WINEPREFIX=str(prefix),WINEDEBUG='-all',WINEDLLOVERRIDES='mscoree,mshtml=;winemenubuilder.exe=d',MNM_TERRAIN_LIGHTING=a.mode,MNM_TERRAIN_LIGHT_CAPTURE='reload32' if a.journey=='campaign-reload' else 'first32',MNM_MENU_DIRECTPLAY_DX6='1',MNM_GLYPH_BACKEND='off',MNM_VERIFY_ORIGINALS='0')
 report=dict(schema=1,success=False,mode=a.mode,journey=a.journey,worker_loss=a.worker_loss,original_sha256=BUILD,results=[],commands=[],normal_launcher=True,native_graphics=False,live_replacement=a.mode=='native')
 reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());behaviors=[b for b in reg['behaviors'] if b['id'] in ['TL.normal-session-lighting','TL.session-light-policy']];builds={b['id']:b for b in reg['builds']};sids={'TLSN25.'+a.mode}|({'TLSN25.reload'} if a.journey=='campaign-reload' else set())|({'TLSN25.worker-loss'} if a.worker_loss else set());scenarios=[s for s in reg['scenarios'] if s['id'] in sids]
 paths=set().union(*(required_sources(b)|set(b['documents']) for b in behaviors));paths.update(['tools/test-live-qt-campaign-quit.py','tools/test-menu-observer.py','tools/capture-fade-writer-game.py','tools/fade_live_capture.py'])
 for unit,compiler,flags in [(ROOT/'runtime/menu/observer.c','clang',['--target=i686-pc-windows-msvc','-ffreestanding']), (ROOT/'runtime/lighting/session_observer.c','clang',['--target=i686-pc-windows-msvc','-ffreestanding']),(ROOT/'tools/terrain-light-replay.cpp','g++',['-std=c++17','-DORIGINAL_REFERENCE','-m32']),*[(ROOT/n,'g++',['-std=c++17']) for n in ['tools/terrain-light-session-worker.cpp',*[str(Path('reconstruction/rendering')/n) for n in MODELS]]]]:
  listing=subprocess.check_output([compiler,*flags,'-MM',str(unit)],cwd=ROOT,text=True).replace('\\\n',' ')
  for name in shlex.split(listing.split(':',1)[1]):
   dep=Path(name).resolve()
   if dep.is_relative_to(ROOT):paths.add(dep.relative_to(ROOT).as_posix())
 sources={n:sha(ROOT/n) for n in sorted(paths)};report.update(sources=sources,claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,builds),scenarios={s['id']:scenario_contract(s) for s in scenarios if b['id'] in s['behaviors']}) for b in behaviors])
 process=None;channel=None;lighting=None;log=None;experiment=None;lease=None
 pending_evidence={}
 def retain(name,data):
  path='research/runtime/terrain-light-session-results-20261025/'+out.name+'-'+name;pending_evidence[path]=data;return dict(path=path,sha256=hashlib.sha256(data).hexdigest())
 def run(label,cmd,timeout=180):
  r=subprocess.run(list(map(str,cmd)),cwd=ROOT,env=env,capture_output=True,text=True,timeout=timeout);(out/(label+'.log')).write_text(r.stdout+r.stderr);report['commands'].append(dict(label=label,exit_code=r.returncode,log=retain(label+'.log',(r.stdout+r.stderr).encode())));r.check_returncode();return r.stdout
 try:
  run('original-before',[main_root/'tools/original-manifest.sh','verify']);pe=main_root/'original/Arcane_Nocd/Chaos.exe';assert sha(pe)==BUILD
  if a.validated_observation:
   prior=read_json(a.validated_observation.absolute(),root=ROOT);assert prior['success'] and prior['mode']=='observe' and prior['native_matches']==32 and all(sources.get(n)==h for n,h in prior['sources'].items()),'Observation does not bind current sources';report['validated_observation']=str(a.validated_observation)
  lease=(prefix.parent/'prefix.lock').open('a+b');fcntl.flock(lease,fcntl.LOCK_EX|fcntl.LOCK_NB)
  if prefix.exists():
   run('prefix-idle',['wineserver','-w'],timeout=5);report['prefix_registry_before']={n:retain('prefix-'+n+'.bin',(prefix/n).read_bytes()) for n in ['system.reg','user.reg','userdef.reg']}
  previous=dict(os.environ)
  try:
   os.environ.clear();os.environ.update(env);experiment=module('normal_light_prepare','tools/prepare-menu-observer.py').prepare(actions=True,source_game=ROOT/'working/game-nocd')
  finally:os.environ.clear();os.environ.update(previous)
  experiment=Path(experiment);(experiment/'.retain').write_text('Normal-session lighting staged installation and evidence; preserve.\n');report['experiment']=str(experiment);report['staged_preferences']=module('normal_light_prefs','tools/capture-fade-writer-game.py').stage_preferences(experiment/'game');(experiment/'game/Save').mkdir(exist_ok=True)
  encoder=module('normal_light_encoder','tools/encode-cfg.py');decoder=encoder.load_decoder(ROOT);report['high_res_preferences']={}
  for name,packed in [('CFG/prefs.cfg',False),('CFG/Encrypted/prefs.cfg',True)]:
   path=experiment/'game'/name
   if not path.is_file():continue
   before=path.read_bytes();text=decoder.decode(before)[1] if packed else before;text=re.sub(rb'(?mi)^IsHighRes\s*=.*(?:\r?\n|$)',b'',text);text=text.replace(b'[VIDEO]',b'[VIDEO]\r\nIsHighRes=TRUE',1);after=encoder.encode(text,decoder)[0] if packed else text;path.write_bytes(after);report['high_res_preferences'][name]=dict(before=hashlib.sha256(before).hexdigest(),after=hashlib.sha256(after).hexdigest())
  metadata=json.loads((experiment/'manifest.json').read_text());report['manifest_before']=retain('manifest-before.bin',(experiment/'manifest.json').read_bytes())
  original=pe.read_bytes();staged=(experiment/'game/Chaos.exe').read_bytes();head=struct.unpack_from('<I',original,60)[0];table=head+24+struct.unpack_from('<H',original,head+20)[0]
  for i in range(struct.unpack_from('<H',original,head+6)[0]):
   size,offset=struct.unpack_from('<II',original,table+i*40+16);assert original[offset:offset+size]==staged[offset:offset+size],'Original section bytes changed'
  report['original_section_bytes_unchanged']=True
  assert all(sources.get(n)==h for n,h in json.loads((experiment/'lighting-build/manifest.json').read_text())['sources'].items()),'Missing lighting compiler provenance'
  models=[ROOT/'reconstruction/rendering'/n for n in MODELS];original_binary=out/'original-light-replay';sanitized_binary=out/'sanitized-light-replay'
  run('original-build',['g++','-std=c++17','-O2','-m32','-fno-pie','-no-pie','-DORIGINAL_REFERENCE',ROOT/'tools/terrain-light-replay.cpp',*models,'-o',original_binary]);run('sanitized-build',['g++','-std=c++17','-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie',ROOT/'tools/terrain-light-replay.cpp',*models,'-o',sanitized_binary]);env.update(ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
  raw=bytearray(131072);raw[:16]=b'MNMMCM13'+struct.pack('<II',13,len(raw));struct.pack_into('<III',raw,16,2,1,1);(experiment/'channel.bin').write_bytes(raw)
  with (experiment/'channel.bin').open('r+b') as f:channel=mmap.mmap(f.fileno(),0)
  with (experiment/'terrain-light-session/channel.bin').open('r+b') as f:lighting=mmap.mmap(f.fileno(),0)
  heartbeat=1;request=0;seq=2
  def state():
   before=struct.unpack_from('<I',channel,128)[0];row=struct.unpack_from('<7I',channel,132);payload=bytes(channel[160:16924]);return (row,payload) if not before&1 and before==struct.unpack_from('<I',channel,128)[0] else None
  def beat(action=0,generation=0,argument=0,rules=None,name=None):
   nonlocal heartbeat,request,seq
   heartbeat+=1;seq+=2
   if action:request+=1
   struct.pack_into('<I',channel,16,seq-1)
   if action:struct.pack_into('<6I',channel,20,1,heartbeat,request,action,generation,argument)
   else:struct.pack_into('<I',channel,24,heartbeat)
   if rules is not None:struct.pack_into('<17I',channel,44,*rules)
   if name is not None:
    text=name.encode('ascii');assert 0<len(text)<128;channel[106496:106624]=text+b'\0'*(128-len(text))
   struct.pack_into('<I',channel,16,seq)
  def pump():pass
  def stats():return dict(zip(lighting_session.STATS,struct.unpack_from('<17I',lighting,56)))
  def wait_lighting(reload=False):
   deadline=time.monotonic()+120
   while time.monotonic()<deadline:
    beat();v=stats()
    if v['disabled'] or v['mismatches'] or v['error']:raise RuntimeError('Lighting disabled: '+repr(v))
    if v['processed']>=96 and (not reload and a.journey=='campaign-reload' or v['records']==32):
     if a.mode=='native' and v['applied']<96:raise RuntimeError('Native updates did not apply')
     report['statistics_after_reload' if reload else 'statistics_before_quit']=v;return
    if process.poll() is not None:raise RuntimeError('Original process exited during lighting')
    time.sleep(.02)
   raise RuntimeError('Continuous lighting target not reached: '+repr(stats()))
  def lose_worker():
   with sqlite3.connect(experiment/'terrain-light-session/outcome.sqlite3') as db:pid,ticks=db.execute("SELECT pid,start_ticks FROM lifecycle WHERE phase='started'").fetchone()
   assert int(Path('/proc',str(pid),'stat').read_text().rsplit(') ',1)[1].split()[19])==ticks
   before=stats();os.kill(pid,signal.SIGKILL);deadline=time.monotonic()+15
   while time.monotonic()<deadline:
    beat();after=stats()
    if after['disabled'] and after['fallback']>=before['fallback']+16:break
    if process.poll() is not None:raise RuntimeError('Original exited after worker loss')
    time.sleep(.02)
   else:raise RuntimeError('Worker loss fallback did not continue')
   applied=after['applied'];struct.pack_into('<I',lighting,124,1);deadline=time.monotonic()+5
   while struct.unpack_from('<I',lighting,124)[0]==1 and time.monotonic()<deadline:beat();time.sleep(.02)
   assert struct.unpack_from('<I',lighting,124)[0]==2,'No original fallback sample'
   ni,no=struct.unpack_from('<I',lighting,20)[0],struct.unpack_from('<I',lighting,28)[0];assert 300<=ni<=lighting_session.CAPACITY and 48<=no<=lighting_session.CAPACITY
   data=bytes(lighting[128:128+ni]);at=128+2*lighting_session.CAPACITY;result=bytes(lighting[at:at+no]);compare_frame('fallback',data,b'',result)
   hold(1);final=stats();assert final['applied']==applied and final['fallback']>after['fallback'];report['worker_loss_control']=dict(before=before,after=final,worker_pid=pid,original_fallback_verified=True)
  def compare_frame(name,data,reply,result):
   pre=out/(name+'.in');pre.write_bytes(data);original=out/(name+'.original');sanitized=out/(name+'.sanitized');run(name+'-original',[original_binary,pre,original,pe],timeout=20);run(name+'-sanitized',[sanitized_binary,pre,sanitized],timeout=20)
   assert original.read_bytes()[32:]==result and sanitized.read_bytes()[32:]==result and (not reply or reply[32:]==result),'Lighting differs from independent original/sanitized replay'
   return dict(input=retain(name+'-input.bin',data),result=retain(name+'-result.bin',result),caller=struct.unpack_from('<I',data,108)[0],clock=struct.unpack_from('<I',data,32)[0],independent_original_match=True,sanitized_match=True)
  def wait(screen=None,ack=None,timeout=90):
   until=time.monotonic()+timeout
   while time.monotonic()<until:
    pump();s=state()
    if s and (screen is None or s[0][1]==screen and s[0][2]) and (ack is None or s[0][3]==ack):return s
    beat()
    if process.poll() is not None:raise RuntimeError('Original game exited during startup')
    time.sleep(.02)
   raise RuntimeError('Menu startup timed out')
  def send(screen,action,argument=0,rules=None,name=None):
   s=wait(screen);beat(action,s[0][0],argument,rules,name);reply=wait(ack=request)
   report.setdefault('menu_actions',[]).append(dict(screen=screen,action=action,argument=argument,reply=list(reply[0])))
   if reply[0][4]!=1:raise RuntimeError('Menu action refused: '+repr(report['menu_actions'][-1]))
  def hold(seconds):
   until=time.monotonic()+seconds
   while time.monotonic()<until:
    pump();beat()
    if process.poll() is not None:raise RuntimeError('Original game exited during campaign journey')
    time.sleep(.02)
  def campaignReload():
   x=c.CDLL('libX11.so.6');xt=c.CDLL('libXtst.so.6');x.XOpenDisplay.argtypes=[c.c_char_p];x.XOpenDisplay.restype=c.c_void_p;x.XFlush.argtypes=x.XCloseDisplay.argtypes=[c.c_void_p]
   x.XDefaultRootWindow.argtypes=[c.c_void_p];x.XDefaultRootWindow.restype=c.c_ulong;x.XQueryTree.argtypes=[c.c_void_p,c.c_ulong,c.POINTER(c.c_ulong),c.POINTER(c.c_ulong),c.POINTER(c.POINTER(c.c_ulong)),c.POINTER(c.c_uint)];x.XFetchName.argtypes=[c.c_void_p,c.c_ulong,c.POINTER(c.c_void_p)];x.XFree.argtypes=[c.c_void_p];x.XSetInputFocus.argtypes=[c.c_void_p,c.c_ulong,c.c_int,c.c_ulong];x.XKeysymToKeycode.argtypes=[c.c_void_p,c.c_ulong];x.XKeysymToKeycode.restype=c.c_uint
   xt.XTestFakeMotionEvent.argtypes=[c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong];xt.XTestFakeButtonEvent.argtypes=xt.XTestFakeKeyEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
   display=x.XOpenDisplay(env['DISPLAY'].encode())
   if not display:raise RuntimeError('Private campaign input display absent')
   def focus():
    todo=[x.XDefaultRootWindow(display)];seen=set()
    while todo:
     window=todo.pop()
     if window in seen:continue
     seen.add(window)
     if len(seen)>512:raise RuntimeError('Private window budget')
     name=c.c_void_p();label=''
     if x.XFetchName(display,window,c.byref(name)) and name.value:label=c.string_at(name.value).decode(errors='replace');x.XFree(name)
     if any(v in label.lower() for v in ['magic','mayhem','chaos']):x.XSetInputFocus(display,window,2,0);x.XFlush(display);return
     returned=c.c_ulong();parent=c.c_ulong();children=c.POINTER(c.c_ulong)();count=c.c_uint()
     if x.XQueryTree(display,window,c.byref(returned),c.byref(parent),c.byref(children),c.byref(count)):
      todo.extend(children[i] for i in range(count.value))
      if children:x.XFree(children)
    raise RuntimeError('Original campaign window absent')
   def escape():
    code=x.XKeysymToKeycode(display,0xff1b);xt.XTestFakeKeyEvent(display,code,1,0);x.XFlush(display);hold(.15);xt.XTestFakeKeyEvent(display,code,0,0);x.XFlush(display);hold(.2);report.setdefault('physical_inputs',[]).append(dict(kind='escape'))
   def dismiss():
    xt.XTestFakeMotionEvent(display,-1,400,160,0);xt.XTestFakeButtonEvent(display,1,1,0);x.XFlush(display);hold(.12);xt.XTestFakeButtonEvent(display,1,0,0);x.XFlush(display);hold(.3);report.setdefault('physical_inputs',[]).append(dict(kind='guide-dismiss',point=[400,160]))
   def dismissRestoredGuide():
    # Validation-only recognition of the known 800x600 initial Hermes panel.
    # Several original pages can pause the restored World independently.
    from PIL import ImageGrab
    hold(.5)
    for attempt in range(32):
     shot=ImageGrab.grab(xdisplay=env['DISPLAY']).convert('RGB');colours=[shot.getpixel(point) for point in [(210,90),(580,210),(210,210),(210,200),(580,90)]]
     panel=all(r>=120 and g>=90 and b<=160 and r>=g+10 and g>=b+15 for r,g,b in colours)
     report.setdefault('restored_guide_observations',[]).append(dict(panel=panel,colours=colours))
     if not panel:return
     dismiss()
    raise RuntimeError('Original restored tutorial page budget exceeded')
   def mini():
    focus()
    current=state()
    if current and current[0][1]==17:
     try:return wait(17,timeout=5)
     except RuntimeError:pass
    for attempt in range(4):
     escape()
     try:return wait(17,timeout=5)
     except RuntimeError:
      if process.poll() is not None:raise
      dismiss()
    raise RuntimeError('Original campaign Mini not ready')
   try:
    send(3,22);send(18,23,1);send(18,25);report['battle_started']=True;hold(5);wait_lighting();
    if a.journey=='campaign-reload':
     mini();send(17,29)
     stem='Light'+hashlib.sha256(out.name.encode()).hexdigest()[:8]
     if any(p.name.casefold()==(stem+'.sav').casefold() for p in (experiment/'game/Save').iterdir()):raise RuntimeError('Private fixture save name already exists')
     send(13,31,name=stem);deadline=time.monotonic()+20;saves=[]
     while time.monotonic()<deadline:
      saves=[p for p in (experiment/'game/Save').iterdir() if p.is_file() and p.name.casefold()==(stem+'.sav').casefold()]
      current=state()
      if len(saves)==1 and saves[0].stat().st_size and current and current[0][1]!=13:break
      hold(.05)
     else:raise RuntimeError('Named private campaign save did not return to World')
     data=saves[0].read_bytes();saved=retain('saved-game.bin',data);record=saved['path'];digest=saved['sha256'];report['saved_game']=dict(path=str(saves[0].relative_to(experiment/'game')),record=record,sha256=digest,bytes=len(data))
     current=state()
     if not current or current[0][1]!=17 or not current[0][2]:mini()
     send(17,28);wait(12);deadline=time.monotonic()+10;selected=0
     while time.monotonic()<deadline:
      beat();first=struct.unpack_from('<I',channel,128)[0];payload=bytes(channel[110592:123552])
      if not first&1 and first==struct.unpack_from('<I',channel,128)[0] and struct.unpack_from('<I',payload)[0]==12:
       count=struct.unpack_from('<I',payload,24)[0]
       if count>100:raise RuntimeError('Storage row budget')
       rows=[payload[160+i*128:288+i*128].split(b'\0',1)[0].decode('cp1252') for i in range(count)]
       selected=next((i+1 for i,v in enumerate(rows) if v.casefold()==stem.casefold()),0)
       if selected:break
      hold(.05)
     if not selected:raise RuntimeError('Named original load row absent')
     struct.pack_into('<I',lighting,52,1);report['selected_load_row']=selected;send(12,31,selected);report['reload_requested']=True;report['saved_game_unchanged']=sha(saves[0])==digest;hold(2);focus();dismissRestoredGuide();wait_lighting(reload=True)
    if a.worker_loss:lose_worker()
    mini();send(17,15)
    helper=subprocess.Popen(['python3',str(ROOT/'tools/test-live-qt-campaign-quit.py'),'--answer','yes','--origin-x','0','--origin-y','0','--experiment',str(experiment),'--capture',str(out/'quit.png')],env=env,stdout=log,stderr=subprocess.STDOUT)
    deadline=time.monotonic()+60
    while helper.poll() is None and time.monotonic()<deadline:beat();time.sleep(.02)
    if helper.poll() is None:helper.terminate();helper.wait(timeout=5);raise RuntimeError('Original Quit timed out')
    if helper.returncode:raise RuntimeError('Original Quit confirmation/report failed')
    send(3,3)
   finally:x.XCloseDisplay(display)
  log=(out/'game.log').open('x');process=subprocess.Popen(['python3',str(ROOT/'tools/run-menu-observer.py'),str(experiment),'--menu-channel',str(experiment/'channel.bin'),'--prefix',str(prefix)],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
  campaignReload();code=process.wait(timeout=45);report['launcher_exit']=code
  if (code!=0)!=a.worker_loss:raise RuntimeError('Unexpected normal launcher result: '+str(code))
  assert (experiment/'original-process-exit.txt').read_text().strip()=='0','Original process did not quit normally'
  with sqlite3.connect(experiment/'terrain-light-session/outcome.sqlite3') as db:outcome=json.loads(db.execute('SELECT payload FROM results').fetchone()[0]);pid=db.execute('SELECT pid FROM lifecycle').fetchone()[0]
  report['worker_outcome']=outcome;report['worker_reaped']=not Path('/proc',str(pid)).exists();assert report['worker_reaped'] and outcome['success']!=a.worker_loss
  with (experiment/'terrain-light-session/corpus.bin').open('rb') as f:
   assert f.read(8)==b'MNMLSC01'
   while lens:=f.read(12):
    assert len(report['results'])<32 and len(lens)==12
    ni,nr,no=struct.unpack('<3I',lens);assert all(0<n<=lighting_session.CAPACITY for n in [ni,nr,no]);data,reply,result=f.read(ni),f.read(nr),f.read(no);assert len(data)==ni and len(reply)==nr and len(result)==no
    report['results'].append(compare_frame('frame-%02d'%len(report['results']),data,reply,result))
  assert len(report['results'])==32
  if a.journey=='campaign-reload':assert report['results'][0]['caller']==0x472201 and report['saved_game_unchanged']
  report.update(native_matches=32,normal_exit=True,success=True)
 except Exception as exc:
  report['error']=type(exc).__name__+': '+str(exc)
  try:
   from PIL import ImageGrab
   ImageGrab.grab(xdisplay=env['DISPLAY']).save(out/'failure.png');report['failure_image']=retain('failure.png',(out/'failure.png').read_bytes())
  except Exception as shot:report['failure_image_error']=str(shot)
 finally:
  if process is not None and process.poll() is None:
   os.killpg(process.pid,signal.SIGTERM)
   try:process.wait(timeout=5)
   except subprocess.TimeoutExpired:os.killpg(process.pid,signal.SIGKILL);process.wait(timeout=5)
  if log:log.close()
  if channel:channel.close()
  if lighting:lighting.close()
  if prefix.exists():
   subprocess.run(['wineserver','-k'],env=env,capture_output=True,timeout=10);subprocess.run(['wineserver','-w'],env=env,capture_output=True,timeout=20)
  if lease:lease.close()
  if (out/'game.log').exists():report['game_log']=retain('game.log',(out/'game.log').read_bytes())
  if experiment:
   for path in ['terrain-light-session/worker.log','terrain-light-session/outcome.sqlite3','terrain-light-session/channel.bin','terrain-light-session/corpus.bin','original-process-exit.txt','manifest.json','events.bin','channel.bin','channel.bin.admission','game/MnmLight.dll','game/MnmMenu.dll','game/Chaos.exe','game/CFG/prefs.cfg','game/CFG/Encrypted/prefs.cfg','lighting-build/manifest.json','lighting-build/mnm-terrain-light-session']:
    if (experiment/path).is_file():report.setdefault('retained_session',{})[path]=retain(path.replace('/','-')+'.bin',(experiment/path).read_bytes())
  for path in out.glob('*.png'):
   report.setdefault('images',{})[path.name]=retain(path.name,path.read_bytes())
  try:run('original-after',[main_root/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
  except Exception as exc:report['success']=False;report['manifest_error']=str(exc)
  report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable']
  for path,data in pending_evidence.items():assert write_report(ROOT,path,data)==hashlib.sha256(data).hexdigest()
  digest=write_report(ROOT,target,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=report['success'],error=report.get('error'),sha256=digest)),flush=True)
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
