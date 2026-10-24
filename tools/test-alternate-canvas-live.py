#!/usr/bin/env python3
"""Bounded all-created-thread pointer watches during original Quick Battle."""
import argparse
import ctypes as C
import ctypes.util
import hashlib
import importlib.util
import json
import mmap
import os
from pathlib import Path
import re
import resource
import secrets
import shlex
import shutil
import socket
import struct
import subprocess
import time
from coverage_claims import behavior_contract,required_sources
from coverage_evidence import evidence_digest,read_json,write_report
from drawing_fixture_prefix import prepare
from original_x86 import PE32
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
ID='RS.alternate-canvas-provenance'
class OriginalConfigurationRefused(Exception):pass
def load(name,path):
 spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def win(path):return 'Z:'+str(path.resolve()).replace('/','\\')
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def decode(path):
 raw=path.read_bytes();h=struct.unpack('<64I',raw[:256]);version=h[2];tail=256 if version==2 else 0;rows=list(struct.iter_unpack('<16I',raw[256:len(raw)-tail]));
 if h[:2]!=(0x414d4e4d,0x31305743) or version not in (1,2) or h[3]!=256 or len(rows)!=h[16]:raise ValueError('Incomplete debugger record')
 return dict(complete=bool(h[4]),error=h[5],pid=h[6],main_thread=h[7],image_base=h[8],threads_armed=h[9],threads_retired=h[10],write_events=h[11],dispatch_calls=h[12],shutdown_calls=h[13],slot_writes=list(h[14:16]),rows=rows,code_checks=h[17],bounded_termination=bool(h[18]),loader_breakpoints=h[19],first_chance_exceptions=h[20],child_exit=h[21],debug_events=h[22],observed_video=h[23],offscreen_height=h[24],offscreen_pitch_bytes=h[25],idle_context=list(h[28:32]),idle_samples=h[32],error_dialog=bool(h[34]),error_message=raw[-256:].split(b'\0')[0].decode('cp1252') if version==2 else '',error_config=list(h[36:39]),sha256=digest(path))
def main():
 resource.setrlimit(resource.RLIMIT_CORE,(0,0));p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('--output',type=Path,required=True);p.add_argument('--report',required=True);p.add_argument('--source-game',type=Path,required=True)
 p.add_argument('--prefix-owner-root',type=Path,required=True);p.add_argument('--wine-prefix',type=Path,required=True)
 p.add_argument('--trace-errors',action='store_true');p.add_argument('--trace-blt',action='store_true');p.add_argument('--enabled-first',action='store_true');p.add_argument('--frames',type=int,default=40,choices=range(8,129));a=p.parse_args();out=a.output.resolve()
 if a.trace_blt and a.trace_errors:p.error('Select one diagnostic watch mode')
 if out.exists() or not out.is_relative_to(ROOT/'working/tests'):p.error('Require new owned output')
 out.mkdir(parents=True);(out/'.retain').write_text('All-thread original alternate pointer and dispatch observations,including negative evidence.\n')
 j=read_json(ROOT/'research/runtime/coverage/register.json',root=ROOT);b=next(b for b in j['behaviors'] if b['id']==ID)
 paths=required_sources(b);todo=list(paths)
 while todo:
  name=todo.pop();path=ROOT/name
  if path.suffix not in {'.c','.h','.S','.cpp','.hpp'}:continue
  for inc in re.findall(r'^\s*#\s*include\s*"([^"]+)"',path.read_text(),re.M):
   child=(path.parent/inc).resolve()
   if child.is_file() and child.is_relative_to(ROOT):
    key=child.relative_to(ROOT).as_posix()
    if key not in paths:paths.add(key);todo.append(key)
 sources={n:evidence_digest(ROOT/n,root=ROOT) for n in sorted(paths)}
 report=dict(schema=1,success=False,sources=sources,claims=[dict(behavior=ID,contract_sha256=behavior_contract(b,{v['id']:v for v in j['builds']}))],original_sha256=BUILD,commands=[],cohorts=[],live_replacement=False,all_writers_recovered=False,original_allocation_recovered=False,
  scope='Read-only hardware watchpoints on both alternate pointer slots and original dispatch plus the selected shutdown/error-reporter/copy call-return diagnostic for every created process thread; authored positive control and natural original Quick Battle sessions across video/configuration choices. Menu navigation is an explicit adapter. Bounded termination is not normal cleanup or universal reachability evidence.')
 report['diagnostic_mode']='copy_call_return' if a.trace_blt else 'error_reporter' if a.trace_errors else 'shutdown'
 env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))};env.update(WINEDEBUG='-all',LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe',LP_NUM_THREADS='4',__EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json',__GLX_VENDOR_LIBRARY_NAME='mesa')
 env.pop('MESA_LOADER_DRIVER_OVERRIDE',None);env.pop('WAYLAND_DISPLAY',None)
 lock=server=process=xlog=None
 def run(label,cmd,settings=None,cwd=out,timeout=180):
  print(label,flush=True);path=out/(label+'.log')
  with path.open('x') as f:r=subprocess.run(list(map(str,cmd)),env=env|dict(settings or {}),cwd=cwd,stdout=f,stderr=subprocess.STDOUT,timeout=timeout)
  report['commands'].append(dict(label=label,argv=list(map(str,cmd)),exit=r.returncode,sha256=digest(path),log=path.relative_to(ROOT).as_posix()))
  if r.returncode:raise ValueError(label+': '+path.read_text(errors='replace')[-2500:])
 try:
  run('manifest-before',[ROOT/'tools/original-manifest.sh','verify'])
  image=PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes());assert image.sha256==BUILD
  report['original_code_witnesses']={hex(address):image.read(address,16).hex() for address in [0x4e415a,0x4e41be,0x5597ea,0x58bf40,0x58c03a,0x58c05a,0x58c067,0x484c00]}
  prefix,lock=prepare(a.prefix_owner_root.resolve(),out,a.wine_prefix.resolve(),a.wine_prefix.resolve());env['WINEPREFIX']=str(prefix)
  def common(path):return (path/subprocess.check_output(['git','-C',str(path),'rev-parse','--git-common-dir'],text=True).strip()).resolve()
  if common(a.prefix_owner_root.resolve())!=common(ROOT):raise ValueError('Prefix must share repository')
  dp=load('alternate_directplay','tools/directplay_dx6_launch.py');dp.check_prefix(prefix)
  for display in range(190,290):
   with socket.socket() as s:
    try:s.bind(('127.0.0.1',6000+display))
    except OSError:continue
    break
  else:raise ValueError('No private X display')
  authority=out/'Xauthority';fields=(b'',str(display).encode(),b'MIT-MAGIC-COOKIE-1',secrets.token_bytes(16));authority.write_bytes(struct.pack('>H',65535)+b''.join(struct.pack('>H',len(v))+v for v in fields));authority.chmod(0o600)
  env.update(DISPLAY='127.0.0.1:'+str(display),XAUTHORITY=str(authority))
  xlog=(out/'xserver.log').open('x');server=subprocess.Popen(['Xvfb',':'+str(display),'-screen','0','1024x768x24','-nolisten','unix','-listen','tcp','-auth',str(authority)],env=env,stdout=xlog,stderr=subprocess.STDOUT)
  for _ in range(100):
   if server.poll() is not None:raise ValueError('Private display exited')
   try:
    with socket.create_connection(('127.0.0.1',6000+display),timeout=.1):break
   except OSError:time.sleep(.05)
  else:raise ValueError('Private display readiness')
  run('prefix-keepalive',['wineserver','-p'])
  imports={'CreateFileA':28,'WriteFile':20,'CloseHandle':4,'SetFilePointer':16,'FlushFileBuffers':4,'ExitProcess':4,'GetEnvironmentVariableA':12,'GetCommandLineA':0,'CreateProcessA':40,'WaitForDebugEvent':8,'ContinueDebugEvent':12,'GetThreadContext':8,'SetThreadContext':8,'ReadProcessMemory':20,'GetFileAttributesA':4,'GetLastError':0,'TerminateProcess':8,'VirtualProtect':16,'CreateThread':24,'WaitForSingleObject':8,'SuspendThread':4,'ResumeThread':4}
  definition=out/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(n+'@'+str(v)+'\n' for n,v in imports.items()));run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
  flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-I'+str(out)]
  user=out/'user32.def';user.write_text('LIBRARY USER32.dll\nEXPORTS\nFindWindowA@8\nGetWindowThreadProcessId@8\nGetWindow@8\nGetWindowTextA@12\n');run('user-imports',['llvm-dlltool','-m','i386','--kill-at','-d',user,'-l',out/'user32.lib'])
  pin=out/'alternate_canvas_pinned.h'
  def build(label,source,extra=()):
   run(label+'-build',[*flags,'-MMD','-MF',out/(label+'.d'),'-c',ROOT/source,'-o',out/(label+'.obj')])
   for name in shlex.split((out/(label+'.d')).read_text().replace('\\\n',' ').split(':',1)[1]):
    path=Path(name).resolve()
    if path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working') and path.relative_to(ROOT).as_posix() not in sources:raise ValueError('Undeclared compiler source '+name)
   run(label+'-link',['lld-link','/machine:x86','/entry:'+('start' if label=='positive-target' else 'mainCRTStartup'),'/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/(label+'.exe')),*extra,out/(label+'.obj'),out/'kernel32.lib',out/'user32.lib'])
  pin.write_text('static const unsigned char alternate_canvas_pinned[16]={0},alternate_error_pinned[16]={0},alternate_blt_pinned[16]={0},alternate_call_pinned[16]={0},alternate_fast_pinned[16]={0};\n');build('positive-debugger','runtime/diagnostics/alternate_canvas_debug.c')
  pad=out/'pad.S';pad.write_text('.text\n.globl _alternate_pad\n_alternate_pad:\n.space 0x400000\n');run('pad-build',['clang','--target=i686-pc-windows-msvc','-c',pad,'-o',out/'pad.obj'])
  build('positive-target','tests/alternate-canvas-debug-target.c',['/base:0x400000','/dynamicbase:no','/include:_alternate_pad',str(out/'pad.obj')])
  positive=out/'positive';positive.mkdir();run('positive-control',['wine',out/'positive-debugger.exe',out/'positive-target.exe'],cwd=positive)
  proof=decode(positive/'alternate-watch.bin');report['positive_control']=proof
  if not proof['complete'] or proof['slot_writes']!=[4,4] or proof['dispatch_calls']!=2 or proof['shutdown_calls']!=2 or len({r[2] for r in proof['rows'] if r[1]==2})!=2:raise ValueError('Missing transient write/thread positive controls')
  pin.write_text(''.join('static const unsigned char '+name+'[]={'+','.join(hex(v) for v in image.read(address,16))+'};\n' for name,address in [('alternate_canvas_pinned',0x4fd438),('alternate_error_pinned',0x484c00),('alternate_blt_pinned',0x58bf40),('alternate_call_pinned',0x58c03a),('alternate_fast_pinned',0x58c05a)]));build('debugger','runtime/diagnostics/alternate_canvas_debug.c')
  menu=load('alternate_menu_builder','tools/build-menu-observer.py').build(out/'menu-build');manifest=json.loads((out/'menu-build/manifest.json').read_text())
  if set(manifest['compiled_sources'])-set(sources):raise ValueError('Undeclared menu compiler input')
  game=out/'game';source=a.source_game.resolve();installation_executable=digest(source/'Chaos.exe');raw=image.raw
  report['source_installation_inputs']={p.relative_to(source).as_posix():dict(bytes=p.stat().st_size,sha256=digest(p)) for p in sorted(source.rglob('*')) if p.is_file()}
  provider={name:digest(Path('/usr/lib/wine/i386-windows')/name) for name in ['ddraw.dll','wined3d.dll']};report['provider']=dict(wine_version=subprocess.check_output(['wine','--version'],text=True).strip(),installed_modules=provider,ddraw_override='builtin',physical_driver_equivalence=False)
  # Installation supplies assets only. Always stage the hash-pinned immutable
  # executable,even if this working installation has an earlier adapter.
  report['installation_executable']=dict(sha256=installation_executable,used_as_executable_input=False)
  run('installation-copy',['cp','-a','--reflink=auto',source,game])
  for name in ['Chaos.exe','MnmMenu.dll','CFG/prefs.cfg','CFG/Encrypted/prefs.cfg','CFG/chaos.cfg','CFG/Encrypted/chaos.cfg']:
   if not (game/name).resolve().is_relative_to(game.resolve()):raise ValueError('Escaping staged write target '+name)
  shutil.copyfile(menu,game/'MnmMenu.dll')
  patch=load('alternate_import','tools/prepare-shadow-experiment.py').add_import;staged=patch(raw,'MnmMenu.dll','MenuAnchor',b'.altmenu')
  for section in image.sections:
   at=section['offset'];size=section['stored']
   if staged[at:at+size]!=raw[at:at+size]:raise ValueError('Original section changed')
  (game/'Chaos.exe').write_bytes(staged)
  settings=load('alternate_settings','tools/run-opengl-game.py');settings.disable_cd_music(game);settings.skip_movies(game)
  encoder=load('alternate_encoder','tools/encode-cfg.py');decoder=encoder.load_decoder(ROOT)
  config_original={path:path.read_bytes() for path in [game/'CFG/chaos.cfg',game/'CFG/Encrypted/chaos.cfg'] if path.is_file()}
  report['staging']=dict(source_sha256=BUILD,staged_sha256=digest(game/'Chaos.exe'),menu_dll_sha256=digest(menu),original_sections_unchanged=True,debugger_sha256=digest(out/'debugger.exe'),prefix=str(prefix),display=env['DISPLAY'])
  cohorts=[(False,True),(False,False),(True,True),(True,False)] if a.enabled_first else [(False,False),(False,True),(True,False),(True,True)]
  for video,enabled in cohorts:
   case=out/('video'+str(int(video))+'-malloc'+str(int(enabled)));case.mkdir();print(case.name,flush=True)
   settings.patch_staged_preferences(game,'VIDEO',{'IsHighRes':'TRUE' if video else 'FALSE'})
   cfg=[]
   for path,data in config_original.items():
    packed=path.parent.name=='Encrypted';text=decoder.decode(data)[1] if packed else data
    updated,n=re.subn(rb'(?im)^\s*;?UseMallocedBackBuffer\s*=\s*(TRUE|FALSE)[^\r\n]*',b'UseMallocedBackBuffer='+str(enabled).upper().encode(),text)
    if n!=1:raise ValueError('Ambiguous original configuration')
    encoded=encoder.encode(updated,decoder)[0] if packed else updated
    if packed and decoder.decode(encoded)[1]!=updated:raise ValueError('CFG round trip')
    path.write_bytes(encoded);cfg.append(dict(path=path.relative_to(game).as_posix(),sha256=digest(path)))
   channel_path=case/'channel.bin';raw_channel=bytearray(32768);raw_channel[:16]=b'MNMMCMD2'+struct.pack('<II',2,32768);struct.pack_into('<3I',raw_channel,16,2,1,1);channel_path.write_bytes(raw_channel)
   channel_file=channel_path.open('r+b');channel=mmap.mmap(channel_file.fileno(),0);heartbeat=1;sequence=2;request=0;trace=[]
   child_env=env|dict(MNM_MENU_CHANNEL=win(channel_path),MNM_MENU_OBSERVE=win(case/'menus.bin'),MNM_ALTERNATE_CWD=win(game),WINEDLLOVERRIDES='ddraw=b;dplay,dplayx,dpwsockx=n')
   if a.trace_errors or a.trace_blt:child_env['MNM_ALTERNATE_ERROR_TRACE']='2' if a.trace_blt else '1'
   log=(case/'game.log').open('x');process=subprocess.Popen(['wine','explorer','/desktop=alternate-watch,1024x768',str(out/'debugger.exe'),str(game/'Chaos.exe')],cwd=case,env=child_env,stdout=log,stderr=subprocess.STDOUT)
   def beat():
    nonlocal heartbeat,sequence
    heartbeat+=1;sequence+=2;struct.pack_into('<I',channel,16,sequence-1);struct.pack_into('<I',channel,24,heartbeat);struct.pack_into('<I',channel,16,sequence)
   def state():
    seq=struct.unpack_from('<I',channel,128)[0];row=struct.unpack_from('<7I',channel,132);payload=bytes(channel[160:16924]);return (row,payload) if not seq&1 and seq==struct.unpack_from('<I',channel,128)[0] else None
   def focus():
    # Only this runner's authenticated private X server is addressed.
    os.environ['XAUTHORITY']=env['XAUTHORITY'];x=C.CDLL(ctypes.util.find_library('X11'))
    callback_type=C.CFUNCTYPE(C.c_int,C.c_void_p,C.c_void_p);errors=[]
    @callback_type
    def ignore_destroyed_window(display,error):errors.append(1);return 0
    x.XSetErrorHandler.argtypes=[C.c_void_p];x.XSetErrorHandler.restype=C.c_void_p;previous_handler=x.XSetErrorHandler(C.cast(ignore_destroyed_window,C.c_void_p))
    x.XOpenDisplay.argtypes=[C.c_char_p];x.XOpenDisplay.restype=C.c_void_p;d=x.XOpenDisplay(env['DISPLAY'].encode())
    if not d:raise ValueError('Private focus display')
    x.XDefaultRootWindow.argtypes=[C.c_void_p];x.XDefaultRootWindow.restype=C.c_ulong
    x.XQueryTree.argtypes=[C.c_void_p,C.c_ulong,C.POINTER(C.c_ulong),C.POINTER(C.c_ulong),C.POINTER(C.POINTER(C.c_ulong)),C.POINTER(C.c_uint)]
    x.XFetchName.argtypes=[C.c_void_p,C.c_ulong,C.POINTER(C.c_void_p)];x.XFree.argtypes=[C.c_void_p]
    x.XSetInputFocus.argtypes=[C.c_void_p,C.c_ulong,C.c_int,C.c_ulong];x.XFlush.argtypes=[C.c_void_p];x.XCloseDisplay.argtypes=[C.c_void_p]
    found=[]
    def visit(w,depth=0):
     name=C.c_void_p();x.XFetchName(d,w,C.byref(name))
     if name.value:
      title=C.string_at(name).decode(errors='replace');x.XFree(name)
      if 'Magic' in title or 'alternate-watch' in title:found.append((depth,w,title))
     root=C.c_ulong();parent=C.c_ulong();children=C.POINTER(C.c_ulong)();count=C.c_uint()
     x.XQueryTree(d,w,C.byref(root),C.byref(parent),C.byref(children),C.byref(count))
     if depth<5:
      for i in range(count.value):visit(children[i],depth+1)
     if children:x.XFree(children)
    visit(x.XDefaultRootWindow(d))
    if found:x.XSetInputFocus(d,max(found)[1],1,0);x.XFlush(d)
    x.XCloseDisplay(d);x.XSetErrorHandler(previous_handler)
    report['focus_window_races']=report.get('focus_window_races',0)+len(errors)
   def wait(screen=None,ack=None):
    end=time.monotonic()+60;focused=0
    while time.monotonic()<end:
     s=state()
     watch=case/'alternate-watch.bin'
     if watch.exists():
      try:observation=decode(watch)
      except (ValueError,struct.error):observation={}
      if observation.get('error_dialog'):raise OriginalConfigurationRefused()
     if s and (screen is None or s[0][1]==screen and s[0][2]) and (ack is None or s[0][3]==ack):return s
     if process.poll() is not None:raise ValueError('Original exited before menu action: '+log.name)
     beat()
     if time.monotonic()>focused+1:focus();focused=time.monotonic()
     time.sleep(.02)
    raise ValueError('Menu timeout: '+str(state()[0] if state() else None))
   def send(screen,action,argument=0,rules=None):
    nonlocal request,sequence,heartbeat
    s=wait(screen);request+=1;heartbeat+=1;sequence+=2;struct.pack_into('<I',channel,16,sequence-1);struct.pack_into('<6I',channel,20,1,heartbeat,request,action,s[0][0],argument)
    if rules is not None:struct.pack_into('<17I',channel,44,*rules)
    struct.pack_into('<I',channel,16,sequence);reply=wait(ack=request)
    if reply[0][4]!=1:raise ValueError('Menu refused '+str(reply[0]))
    trace.append(dict(screen=screen,action=action,argument=argument,ack=request))
   try:
    send(3,1);send(22,4);s=wait(14);rules=list(struct.unpack_from('<13I',s[1],8))+[struct.unpack_from('<I',s[1],60+i*48+12)[0] for i in range(4)];rules[2]=0
    send(14,6,rules=rules);send(25,9,2);send(14,7,rules=rules)
    deadline=time.monotonic()+90
    while time.monotonic()<deadline:
     beat()
     if process.poll() is not None:raise ValueError('Original exited before dispatch observation')
     path=case/'alternate-watch.bin'
     if path.exists():
      try:progress=decode(path)
      except (ValueError,struct.error):progress={}
      if progress.get('dispatch_calls',0)>=a.frames:break
     time.sleep(.1)
    else:raise ValueError('No bounded original dispatch prefix')
    (case/'alternate-stop.flag').touch();process.wait(timeout=20)
    if process.returncode:raise ValueError('Debugger refused original observation')
    proof=decode(case/'alternate-watch.bin')
    if not proof['complete'] or not proof['bounded_termination'] or proof['dispatch_calls']<a.frames or proof['image_base']!=0x400000:raise ValueError('Incomplete live observation')
    dispatch=[r for r in proof['rows'] if r[1]==3]
    if not dispatch or any(bool(r[9])!=enabled for r in dispatch) or bool(proof['observed_video'])!=video:raise ValueError('Original video/DEBUG choices were not observed')
    report['cohorts'].append(dict(video=video,use_malloced_backbuffer=enabled,configuration=cfg,menu_actions=trace,proof=proof,outcome='dispatch_observed'))
   except OriginalConfigurationRefused:
    if not enabled:raise ValueError('Default original configuration raised a DirectDraw error')
    (case/'alternate-stop.flag').touch();process.wait(timeout=20);proof=decode(case/'alternate-watch.bin')
    if process.returncode or not proof['complete'] or not proof['bounded_termination'] or 'Rectangle provided was invalid.' not in proof['error_message'] or proof['error_config']!=[1,96,0] or bool(proof['observed_video'])!=video:raise ValueError('Unclassified original configuration failure')
    report['cohorts'].append(dict(video=video,use_malloced_backbuffer=enabled,configuration=cfg,menu_actions=trace,proof=proof,outcome='original_configuration_refused'))
   finally:
    if process.poll() is None:
     (case/'alternate-stop.flag').touch()
     try:process.wait(timeout=10)
     except subprocess.TimeoutExpired:process.terminate();process.wait(timeout=10)
    process=None;log.close();channel.close();channel_file.close()
   if a.trace_blt or a.trace_errors and enabled:
    path=case/('alternate-blt.bin' if a.trace_blt else 'alternate-error.bin');words=struct.unpack('<768I',path.read_bytes())
    if words[:3]!=(0x45524e4d,2,3072) or words[15]!=1:raise ValueError('Error trace format')
    report['cohorts'][-1]['error_trace']=dict(registers=list(words[3:13]),mode=words[13],hresult=words[14],stack=list(words[16:528]),source_wrapper=list(words[528:560]),destination_wrapper=list(words[560:592]),arguments=list(words[592:598]),destination_rectangle=list(words[608:612]),source_rectangle=list(words[612:616]),read_complete=list(words[616:620]),fast_route=words[650],sha256=digest(path))
   if a.trace_blt or a.trace_errors and enabled:
    captured=report['cohorts'][-1]['error_trace'];base_width,height=(800,600) if video else (640,480);width=base_width+(96 if enabled else 0)
    if captured['source_wrapper'][4:7]!=[width,height,width*2] or captured['destination_wrapper'][4:6]!=[base_width,height] or captured['hresult']!=(0x88760096 if enabled else 0):raise ValueError('Unexpected original surface shape/result')
    if captured['read_complete'][:2]!=[1,1] or captured['fast_route']!=1:raise ValueError('Unadmitted original presentation route')
    if a.trace_blt:
     args=captured['arguments']
     if captured['registers'][1]!=0x58c05a or args[:4]!=[captured['destination_wrapper'][2],0,0,captured['source_wrapper'][2]] or args[5]!=0x10 or captured['source_rectangle']!=[0,0,width,height] or captured['read_complete'][3]!=1 or captured['stack'][22]!=0x5597fd:raise ValueError('Original copy arguments/caller mismatch')
    elif captured['stack'][:2]!=[0x58c072,0x88760096] or captured['stack'][18]!=0x5597fd:raise ValueError('Original reporter caller mismatch')
    captured['validated']=True
   if server.poll() is not None:raise ValueError('Private display died')
   run(case.name+'-prefix-stop',['wineserver','-k']);run(case.name+'-prefix-idle',['wineserver','-w']);run(case.name+'-prefix-keepalive',['wineserver','-p'])
  if digest(source/'Chaos.exe')!=installation_executable:raise ValueError('Source installation changed')
  if any(digest(source/path)!=v['sha256'] for path,v in report['source_installation_inputs'].items()):raise ValueError('Source installation input changed')
  if any(digest(Path('/usr/lib/wine/i386-windows')/name)!=h for name,h in provider.items()):raise ValueError('Provider changed')
  report.update(error_attribution_validated=bool(a.trace_blt or a.trace_errors) and all(c.get('error_trace',{}).get('validated',False) for c in report['cohorts'] if a.trace_blt or c['use_malloced_backbuffer']),success=True,original_game_executed=True,observed_pointer_writes=any(c['proof']['write_events'] for c in report['cohorts']),observed_natural_publications=any(r[1]==2 and (r[5] or r[6]) for c in report['cohorts'] for r in c['proof']['rows']),observed_alternate_dispatch=any(r[5]!=0 for c in report['cohorts'] for r in c['proof']['rows'] if r[1]==3))
 except (OSError,ValueError,subprocess.SubprocessError,AssertionError) as error:report['error']=str(error)
 finally:
  if process is not None and process.poll() is None:process.terminate();process.wait(timeout=10)
  if lock is not None:
   try:run('prefix-stop',['wineserver','-k']);run('prefix-idle',['wineserver','-w'])
   except (OSError,ValueError,subprocess.SubprocessError) as error:report.update(success=False,prefix_error=str(error))
   lock.close()
  if server is not None:
   server.terminate()
   try:server.wait(timeout=10)
   except subprocess.TimeoutExpired:server.kill();server.wait()
  if xlog is not None:xlog.close()
  try:run('manifest-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
  except (OSError,ValueError,subprocess.SubprocessError) as error:report.update(success=False,manifest_error=str(error))
  report['sources_stable']=all(evidence_digest(ROOT/n,root=ROOT)==h for n,h in sources.items());report['success']&=report['sources_stable']
  sha=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=report['success'],sha256=sha,error=report.get('error'))),flush=True)
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
