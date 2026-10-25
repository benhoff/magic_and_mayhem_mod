#!/usr/bin/env python3
"""Bounded original-active lighting comparison, then explicitly gated replacement."""
import argparse,fcntl,hashlib,importlib.util,json,mmap,os,signal,struct,subprocess,time,shlex
from pathlib import Path
from coverage_evidence import write_report,read_json
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def module(name,path):
 s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--main-root',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--report',type=Path,required=True);p.add_argument('--mode',choices=['observe','replace'],default='observe');p.add_argument('--validated-observation',type=Path);p.add_argument('--reuse-prefix',type=Path,help='Stopped prefix from this task, exclusively locked and registry-snapshotted before reuse');p.add_argument('--samples',type=int,choices=range(8,33),default=16);a=p.parse_args()
 if not os.environ.get('DISPLAY'):p.error('Run under isolated xvfb-run')
 out=a.output.resolve();target=a.report.absolute();main_root=a.main_root.resolve()
 if out.exists() or target.exists() or not out.is_relative_to(ROOT/'working') or not target.is_relative_to(ROOT/'research/runtime'):p.error('Require new owned working output and research report')
 if (a.mode=='replace')!=(a.validated_observation is not None):p.error('Replacement requires the matching successful original-active observation')
 out.mkdir(parents=True);(out/'.retain').write_text('Live lighting evidence and private installation/prefix; preserve.\n');(out/'capture').mkdir();(out/'tmp').mkdir()
 env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(TMPDIR=str(out/'tmp'),WINEPREFIX=str(out/'wineprefix'),WINEDEBUG='-all',WINEDLLOVERRIDES='mscoree,mshtml=;winemenubuilder.exe=d')
 report=dict(schema=1,success=False,mode=a.mode,original_sha256=BUILD,results=[],commands=[],live_replacement=False,original_manifest_verified_before_after=False)
 reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());ids={'TL.live-light-inputs','TL.live-light-scheduler','TL.live-light-replacement'};behaviors=[b for b in reg['behaviors'] if b['id'] in ids];assert len(behaviors)==3
 if a.mode=='observe':behaviors=[b for b in behaviors if b['id']!='TL.live-light-replacement']
 builds={b['id']:b for b in reg['builds']};scenarios=[s for s in reg['scenarios'] if s['id']==('TLCL.live-'+a.mode+'-20261025')]
 paths=set().union(*(required_sources(b)|set(b['documents']) for b in behaviors));paths.update(['tools/coverage_claims.py','tools/coverage_evidence.py','tools/capture-fade-writer-game.py','tools/fade_live_capture.py','tools/encode-cfg.py','tools/decode-cfg.py'])
 # Enumerate compiler inputs before declaring execution claims.
 for unit,compiler,flags in [(ROOT/'runtime/menu/observer.c','clang',['--target=i686-pc-windows-msvc','-ffreestanding']), (ROOT/'runtime/lighting/observer.c','clang',['--target=i686-pc-windows-msvc','-ffreestanding']),(ROOT/'tools/terrain-light-replay.cpp','g++',['-std=c++17','-DORIGINAL_REFERENCE','-m32']),*[(ROOT/n,'g++',['-std=c++17']) for n in ['tools/terrain-light-replay.cpp','reconstruction/rendering/terrain_lighting.cpp','reconstruction/rendering/terrain_creature_lighting.cpp','reconstruction/rendering/terrain_static_lighting.cpp','reconstruction/rendering/terrain_lighting_cycle.cpp']]]:
  listing=subprocess.check_output([compiler,*flags,'-MM',str(unit)],cwd=ROOT,text=True).replace('\\\n',' ')
  for name in shlex.split(listing.split(':',1)[1]):
   dep=Path(name).resolve()
   if dep.is_relative_to(ROOT):paths.add(dep.relative_to(ROOT).as_posix())
 sources={n:sha(ROOT/n) for n in sorted(paths)};report.update(sources=sources,claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,builds),scenarios={s['id']:scenario_contract(s) for s in scenarios if b['id'] in s['behaviors']}) for b in behaviors])
 process=None;channel=None;log=None;prefix_lease=None
 def run(label,cmd,timeout=180,environment=None):
  try:r=subprocess.run(list(map(str,cmd)),cwd=ROOT,env=environment or env,capture_output=True,text=True,timeout=timeout)
  except subprocess.TimeoutExpired as exc:
   def decoded(v):return v.decode(errors='replace') if isinstance(v,bytes) else v or ''
   (out/(label+'.log')).write_text(decoded(exc.stdout)+decoded(exc.stderr));report['commands'].append(dict(label=label,timed_out=True,log_sha256=sha(out/(label+'.log'))));raise
  (out/(label+'.log')).write_text(r.stdout+r.stderr);report['commands'].append(dict(label=label,exit_code=r.returncode,log_sha256=sha(out/(label+'.log'))));r.check_returncode();return r.stdout
 try:
  run('original-before',[main_root/'tools/original-manifest.sh','verify']);pe=main_root/'original/Arcane_Nocd/Chaos.exe';assert sha(pe)==BUILD
  if a.validated_observation:
   prior=read_json(a.validated_observation.absolute(),root=ROOT);assert prior['success'] and prior['mode']=='observe' and prior['native_matches']>=8 and all(sources.get(n)==h for n,h in prior['sources'].items()),'Observation does not bind current live source inputs';report['validated_observation']=str(a.validated_observation)
  light=module('light_build','tools/build-terrain-light-observer.py').build(out/'light-build')
  menu=module('menu_build','tools/build-menu-observer.py').build(out/'menu-build')
  menu_inputs=json.loads((out/'menu-build/manifest.json').read_text())['compiled_sources'];assert all(n in sources and sources[n]==h for n,h in menu_inputs.items()),'Missing menu compiler provenance'
  raw=pe.read_bytes();stager=module('light_import','tools/prepare-shadow-experiment.py');staged=stager.add_import(raw,'MnmMenu.dll','MenuAnchor',b'.mnmenu');staged=stager.add_import(staged,'MnmLight.dll','LightAnchor',b'.mnlight')
  # Preserve all original section bytes. Only PE import/header data is appended.
  head=struct.unpack_from('<I',raw,60)[0];table=head+24+struct.unpack_from('<H',raw,head+20)[0]
  for i in range(struct.unpack_from('<H',raw,head+6)[0]):
   size,offset=struct.unpack_from('<II',raw,table+i*40+16);assert raw[offset:offset+size]==staged[offset:offset+size]
  run('copy-game',['cp','-a','--reflink=auto',main_root/'working/game-clean',out/'game'])
  (out/'game/Chaos.exe').write_bytes(staged)
  report['staged_preferences']=module('light_preferences','tools/capture-fade-writer-game.py').stage_preferences(out/'game')
  import shutil
  shutil.copy2(light,out/'game/MnmLight.dll');shutil.copy2(menu,out/'game/MnmMenu.dll')
  report['staged_executable_sha256']=sha(out/'game/Chaos.exe');report['light_dll_sha256']=sha(light);report['menu_dll_sha256']=sha(menu)
  # The bounded original battle route uses a stopped copy of the established
  # prefix. No template/user prefix is killed or modified.
  if a.reuse_prefix:
   prefix=a.reuse_prefix.resolve()
   if not prefix.is_relative_to(ROOT/'working/light-closure') or prefix.name!='wineprefix' or not (prefix.parent/'.retain').is_file():raise ValueError('Reuse requires this task\'s retained private prefix')
   prefix_lease=(prefix.parent/'lighting-prefix.lock').open('a+b');fcntl.flock(prefix_lease,fcntl.LOCK_EX|fcntl.LOCK_NB)
   env['WINEPREFIX']=str(prefix);run('reused-prefix-stopped',['wineserver','-w'],timeout=5);(out/'wineprefix').symlink_to(prefix)
   report['reused_prefix']=str(prefix.relative_to(ROOT));report['prefix_registry_before']={}
   for name in ['system.reg','user.reg','userdef.reg']:
    data=(prefix/name).read_bytes();record='research/runtime/terrain-light-closure-results-20261025/'+out.name+'-prefix-'+name+'.bin';report['prefix_registry_before'][name]=dict(record=record,sha256=write_report(ROOT,record,data))
  else:
   template=main_root/'working/wineprefix-x86_64';run('template-stopped',['wineserver','-w'],timeout=5,environment={**env,'WINEPREFIX':str(template)})
   run('clone-prefix',['cp','-a','--reflink=auto',template,out/'wineprefix'])
   run('wineboot',['wineboot','-u'],timeout=120);run('wineboot-idle',['wineserver','-w'],timeout=120)
  models=[ROOT/'reconstruction/rendering'/n for n in ['terrain_lighting.cpp','terrain_creature_lighting.cpp','terrain_static_lighting.cpp','terrain_lighting_cycle.cpp']]
  binary=out/'terrain-light-replay';run('native-build',['g++','-std=c++17','-O2',ROOT/'tools/terrain-light-replay.cpp',*models,'-o',binary])
  original_binary=out/'original-light-replay';run('original-build',['g++','-std=c++17','-O2','-m32','-fno-pie','-no-pie','-DORIGINAL_REFERENCE',ROOT/'tools/terrain-light-replay.cpp',*models,'-o',original_binary])
  sanitized_binary=out/'sanitized-light-replay';run('sanitized-build',['g++','-std=c++17','-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie',ROOT/'tools/terrain-light-replay.cpp',*models,'-o',sanitized_binary])
  env.update(ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
  raw=bytearray(32768);raw[:16]=b'MNMMCMD2'+struct.pack('<II',2,32768);struct.pack_into('<III',raw,16,2,1,1);(out/'channel.bin').write_bytes(raw)
  f=(out/'channel.bin').open('r+b');channel=mmap.mmap(f.fileno(),0);heartbeat=1;request=0;seq=2;processed=set()
  env.update(MNM_MENU_OBSERVE='Z:'+str(out/'events.bin').replace('/','\\'),MNM_MENU_CHANNEL='Z:'+str(out/'channel.bin').replace('/','\\'),MNM_LIGHT_DIR='Z:'+str(out/'capture').replace('/','\\'),MNM_LIGHT_MODE=a.mode,MNM_LIGHT_LIMIT=str(a.samples),LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe')
  def state():
   before=struct.unpack_from('<I',channel,128)[0];row=struct.unpack_from('<7I',channel,132);payload=bytes(channel[160:16924]);return (row,payload) if not before&1 and before==struct.unpack_from('<I',channel,128)[0] else None
  def beat(action=0,generation=0,argument=0,rules=None):
   nonlocal heartbeat,request,seq
   heartbeat+=1;seq+=2
   if action:request+=1
   struct.pack_into('<I',channel,16,seq-1)
   if action:struct.pack_into('<6I',channel,20,1,heartbeat,request,action,generation,argument)
   else:struct.pack_into('<I',channel,24,heartbeat)
   if rules is not None:struct.pack_into('<17I',channel,44,*rules)
   struct.pack_into('<I',channel,16,seq)
  def pump():
   for pre in sorted((out/'capture').glob('*.in')):
    if pre.name in processed:continue
    temporary=pre.with_suffix('.reply-tmp');text=run('replay-'+pre.stem,[binary,pre,temporary],timeout=20)
    os.replace(temporary,pre.with_suffix('.reply'));processed.add(pre.name)
    run('original-replay-'+pre.stem,[original_binary,pre,pre.with_suffix('.original-replay'),pe],timeout=20)
    run('sanitized-replay-'+pre.stem,[sanitized_binary,pre,pre.with_suffix('.sanitized-replay')],timeout=20)
  def wait(screen=None,ack=None,timeout=90):
   until=time.monotonic()+timeout
   while time.monotonic()<until:
    pump();s=state()
    if s and (screen is None or s[0][1]==screen and s[0][2]) and (ack is None or s[0][3]==ack):return s
    beat()
    if process.poll() is not None:raise RuntimeError('Original game exited during startup')
    time.sleep(.02)
   raise RuntimeError('Menu startup timed out')
  def send(screen,action,argument=0,rules=None):
   s=wait(screen);beat(action,s[0][0],argument,rules);reply=wait(ack=request)
   report.setdefault('menu_actions',[]).append(dict(screen=screen,action=action,argument=argument,reply=list(reply[0])))
   if reply[0][4]!=1:raise RuntimeError('Menu action refused: '+repr(report['menu_actions'][-1]))
  exe='Z:'+str(out/'game/Chaos.exe').replace('/','\\');exitfile='Z:'+str(out/'process-exit.txt').replace('/','\\')
  assert all(c not in exe+exitfile for c in '\r\n%&|<>^\"')
  (out/'wait-game.cmd').write_bytes(('@echo off\r\nstart \"\" /wait /b \"'+exe+'\"\r\nset MNM_LIGHT_EXIT=%ERRORLEVEL%\r\n> \"'+exitfile+'\" echo %MNM_LIGHT_EXIT%\r\nexit /b %MNM_LIGHT_EXIT%\r\n').encode('ascii'))
  log=(out/'game.log').open('x');process=subprocess.Popen(['wine','explorer','/desktop=LightObserver,800x600','cmd.exe','/d','/q','/c','Z:'+str(out/'wait-game.cmd').replace('/','\\')],cwd=out/'game',env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
  send(3,1);send(22,4);s=wait(14);rules=list(struct.unpack_from('<13I',s[1],8));rules+=[struct.unpack_from('<I',s[1],60+i*48+12)[0] for i in range(4)];rules[2]=0;send(14,6,rules=rules);send(25,9,2);send(14,7,rules=rules);report['battle_started']=True
  until=time.monotonic()+120
  while time.monotonic()<until:
   pump();beat();done=sorted((out/'capture').glob('*.done'));refused=sorted((out/'capture').glob('*.refused'))
   if len(done)+len(refused)>=a.samples:break
   if process.poll() is not None:raise RuntimeError('Original game exited during lighting capture')
   time.sleep(.02)
  pump();frames=[]
  for pre in sorted((out/'capture').glob('*.in')):
   oracle=pre.with_suffix('.oracle');done=pre.with_suffix('.done');reply=pre.with_suffix('.reply')
   if not oracle.exists() or not done.exists() or not reply.exists():raise RuntimeError('Incomplete light transaction')
   before=pre.read_bytes();answer=reply.read_bytes();original=oracle.read_bytes();ack=struct.unpack('<4I',done.read_bytes());match=answer[32:]==original
   original_match=pre.with_suffix('.original-replay').read_bytes()[32:]==original;sanitized_match=pre.with_suffix('.sanitized-replay').read_bytes()[32:]==original
   path='research/runtime/terrain-light-closure-results-20261025/'+out.name+'-'+pre.stem
   write_report(ROOT,path+'-input.bin',before);write_report(ROOT,path+'-result.bin',original)
   frames.append(dict(sequence=ack[0],replaced=ack[1],clock=ack[3],caller=struct.unpack_from('<I',before,108)[0],match=match,independent_original_match=original_match,sanitized_match=sanitized_match,input=path+'-input.bin',result=path+'-result.bin',input_sha256=sha(pre),result_sha256=sha(oracle)))
  report.update(results=frames,native_matches=sum(v['match'] for v in frames),independent_original_matches=sum(v['independent_original_match'] for v in frames),sanitized_matches=sum(v['sanitized_match'] for v in frames),replacements=sum(v['replaced'] for v in frames),refused=[v.name for v in (out/'capture').glob('*.refused')],live_replacement=a.mode=='replace',native_binary_sha256=sha(binary),original_binary_sha256=sha(original_binary),sanitized_binary_sha256=sha(sanitized_binary))
  report['original_fallbacks']=len(frames)-report['replacements'] if a.mode=='replace' else 0
  if len(frames)<8 or not all(v['match'] and v['independent_original_match'] and v['sanitized_match'] for v in frames) or (a.mode=='replace' and report['replacements']<8):raise RuntimeError('Incomplete lighting comparison/replacement')
  report['wire_refusals']=json.loads(run('wire-refusals',['python3',ROOT/'tests/test-terrain-light-wire.py','--binary',binary,'--input',out/'capture/light-00000001.in','--output',out/'wire-refusals']))
  until=time.monotonic()+1
  while time.monotonic()<until:
   beat()
   if process.poll() is not None:raise RuntimeError('Original scene exited after lighting capture')
   time.sleep(.02)
  from PIL import ImageGrab
  ImageGrab.grab(xdisplay=env['DISPLAY']).save(out/'scene.png');scene_path='research/runtime/terrain-light-closure-results-20261025/'+out.name+'-scene.png';report['scene_image']=scene_path;report['scene_image_sha256']=write_report(ROOT,scene_path,(out/'scene.png').read_bytes());report['scene_continued_seconds']=1
  report['success']=True
 except Exception as exc:
  report['error']=str(exc)
  if process is not None:
   try:
    from PIL import ImageGrab
    ImageGrab.grab(xdisplay=env['DISPLAY']).save(out/'failure.png');report['failure_image_sha256']=sha(out/'failure.png')
    write_report(ROOT,'research/runtime/terrain-light-closure-results-20261025/'+out.name+'-failure.png',(out/'failure.png').read_bytes())
   except Exception as shot:report['failure_image_error']=str(shot)
 finally:
  if process is not None and process.poll() is None:
   os.killpg(process.pid,signal.SIGTERM)
   try:process.wait(timeout=5)
   except subprocess.TimeoutExpired:os.killpg(process.pid,signal.SIGKILL);process.wait(timeout=5)
  if log:log.close()
  if channel:channel.close()
  if (out/'wineprefix').exists():
   subprocess.run(['wineserver','-k'],env=env,capture_output=True,timeout=10);subprocess.run(['wineserver','-w'],env=env,capture_output=True,timeout=20)
  if prefix_lease:prefix_lease.close()
  try:run('original-after',[main_root/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
  except Exception as exc:report['success']=False;report['manifest_error']=str(exc)
  report['inputs_stable']=sha(main_root/'original/Arcane_Nocd/Chaos.exe')==BUILD
  report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable'] and report['inputs_stable'];digest=write_report(ROOT,target,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=report['success'],mode=a.mode,native_matches=report.get('native_matches',0),replacements=report.get('replacements',0),error=report.get('error'),sha256=digest)),flush=True)
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
