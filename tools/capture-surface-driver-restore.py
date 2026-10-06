#!/usr/bin/env python3
"""Bounded real Wine Surface2 loss/Restore evidence, without a game image."""
import argparse, importlib.util, json, os, struct, subprocess, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('capture_keys',ROOT/'tools/capture-original-surface-keys.py');keys=importlib.util.module_from_spec(spec);spec.loader.exec_module(keys)
SOURCES=['tests/surface-driver-restore-reference.c','tools/capture-surface-driver-restore.py','tools/capture-original-surface-keys.py','runtime/shadow/win32_min.h']
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--display-bpp',type=int,choices=(16,32),default=32);p.add_argument('--report',type=Path,required=True);p.add_argument('--prefix-template',type=Path,default=ROOT/'working/wineprefix-x86_64');a=p.parse_args()
 if a.report.exists():p.error('Refusing to overwrite retained evidence')
 if not os.environ.get('DISPLAY'):p.error('Run under Xvfb or a display')
 try:reservation=keys.reserve_wine()
 except RuntimeError:
  active=[]
  for process in Path('/proc').iterdir():
   if not process.name.isdecimal() or int(process.name)==os.getpid():continue
   try:
    comm=(process/'comm').read_text().strip();environment=(process/'environ').read_bytes().split(b'\0')
    if comm.startswith(('wine','wineserver')) or any(x.startswith(b'WINEPREFIX=') for x in environment):active.append((process.name,comm))
   except (FileNotFoundError,PermissionError,ProcessLookupError):continue
  print('Wine reservation refused; active process IDs/names:',active,flush=True);raise
 parent=ROOT/'working/tests/surface-driver-restore';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
 sources={n:keys.sha(ROOT/n) for n in SOURCES};env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.pop('WAYLAND_DISPLAY',None);env.update(WINEPREFIX=str(run/'wineprefix'),WINEDEBUG='-all',WINEDLLOVERRIDES='ddraw=b',LIBGL_ALWAYS_SOFTWARE='1')
 def command(name,argv,timeout=60):
  with (run/(name+'.log')).open('w') as log:subprocess.run(argv,cwd=run,env=env,stdout=log,stderr=log,check=True,timeout=timeout)
 try:
  command('prefix',['cp','-a','--reflink=auto',str(a.prefix_template.resolve()),env['WINEPREFIX']],120);command('wineboot',['wineboot','-u'],120)
  imports={'Sleep':4,'LoadLibraryA':4,'GetProcAddress':8,'CreateFileA':28,'WriteFile':20,'CloseHandle':4,'ExitProcess':4};definition=run/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in imports.items()))
  command('imports',['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(run/'kernel32.lib')]);command('compile',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-DPROBE_BPP='+str(a.display_bpp),'-c',str(ROOT/SOURCES[0]),'-o',str(run/'probe.obj')]);command('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(run/'probe.exe'),str(run/'probe.obj'),str(run/'kernel32.lib')]);command('driver',['wine','explorer','/desktop=restore-probe,1024x768',str(run/'probe.exe')],60)
  raw=(run/'outputs.bin').read_bytes();assert len(raw)%24==0;rows=[list(x) for x in struct.iter_unpack('<6I',raw)];assert rows and rows[-1]==[17,7,0,0,0,0], 'Probe did not complete';assert all(keys.sha(ROOT/n)==h for n,h in sources.items())
  report=dict(schema=1,success=True,sources=sources,scope='Real Wine built-in IDirectDrawSurface2 mode-change loss/Restore observations. No original game image/wrapper, native renderer or Windows-driver equivalence. Restored bytes are observed only; they are not a portable deterministic contract.',environment=dict(wine=subprocess.check_output(['wine','--version'],text=True).strip(),ddraw_sha256=keys.sha(run/'wineprefix/drive_c/windows/syswow64/ddraw.dll'),override='ddraw=b',display=os.environ['DISPLAY'],virtual_desktop='restore-probe,1024x768',display_bpp=a.display_bpp,cooperative_level='EXCLUSIVE|FULLSCREEN'),artifacts=str(run.relative_to(ROOT)),probe_sha256=keys.sha(run/'probe.exe'),outputs_sha256=keys.sha(run/'outputs.bin'),row_fields=['kind','phase','surface','result','a','b'],kinds={'1':'CreateSurface','2':'IsLost','3':'GetColorKey','4':'Lock','5':'pixel','6':'pixel_hash','7':'Unlock','8':'BltFill','9':'SetCooperativeLevel','10':'SetDisplayMode','11':'SetColorKey','12':'Restore','13':'RestoreDisplayMode','14':'SetForegroundWindow','15':'format_dimensions','16':'RGB_masks','17':'complete','18':'GetDisplayMode','19':'display_dimensions'},surface_caps=[0x840,0x4040,0x40,0x200],rows=rows)
 finally:
  subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10);subprocess.run(['wineserver','-w'],env=env,check=False,timeout=10);reservation.close()
 a.report.parent.mkdir(parents=True,exist_ok=True)
 with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
 print(json.dumps([r for r in rows if r[0] in (1,2,10,12)]))
if __name__=='__main__':main()
