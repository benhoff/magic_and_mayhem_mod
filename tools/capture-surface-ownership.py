#!/usr/bin/env python3
"""Capture standalone Surface2 ownership and two-buffer flips."""
import argparse, gzip, importlib.util, json, os, struct, subprocess, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('capture_keys',ROOT/'tools/capture-original-surface-keys.py');keys=importlib.util.module_from_spec(spec);spec.loader.exec_module(keys)
SOURCES=['tests/surface-ownership-reference.c','tests/surface-palette-restore-reference.c','tools/capture-surface-ownership.py','tools/capture-original-surface-keys.py','runtime/shadow/win32_min.h']
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--corpus',type=Path,default=ROOT/'tests/fixtures/surfaces/surface-ownership-final.json.gz');p.add_argument('--report',type=Path,required=True);p.add_argument('--prefix-template',type=Path,default=ROOT/'working/wineprefix-x86_64');a=p.parse_args()
 corpus=a.corpus.resolve()
 if not corpus.is_relative_to(ROOT):p.error('Corpus must be inside the repository')
 if a.report.exists() or corpus.exists():p.error('Refusing to overwrite retained evidence')
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
 parent=ROOT/'working/tests/surface-ownership';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
 sources={n:keys.sha(ROOT/n) for n in SOURCES};env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.pop('WAYLAND_DISPLAY',None);env.update(WINEPREFIX=str(run/'wineprefix'),WINEDEBUG='-all',WINEDLLOVERRIDES='ddraw=b',LIBGL_ALWAYS_SOFTWARE='1')
 def command(name,argv,timeout=60):
  with (run/(name+'.log')).open('w') as log:subprocess.run(argv,cwd=run,env=env,stdout=log,stderr=log,check=True,timeout=timeout)
 try:
  command('original-before',[str(ROOT/'tools/original-manifest.sh'),'verify'],120)
  command('prefix',['cp','-a','--reflink=auto',str(a.prefix_template.resolve()),env['WINEPREFIX']],120);command('wineboot',['wineboot','-u'],120)
  imports={'Sleep':4,'LoadLibraryA':4,'GetProcAddress':8,'CreateFileA':28,'WriteFile':20,'CloseHandle':4,'ExitProcess':4};definition=run/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in imports.items()))
  command('imports',['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(run/'kernel32.lib')]);command('compile',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-DPROBE_BPP=8','-c',str(ROOT/SOURCES[0]),'-o',str(run/'probe.obj')]);command('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(run/'probe.exe'),str(run/'probe.obj'),str(run/'kernel32.lib')]);command('driver',['wine','explorer','/desktop=restore-probe,1024x768',str(run/'probe.exe')],60)
  raw=(run/'outputs.bin').read_bytes();assert len(raw)%24==0;rows=[list(x) for x in struct.iter_unpack('<6I',raw)];assert rows and rows[-1]==[46,99,0,0,0,0], 'Probe did not complete';assert all(keys.sha(ROOT/n)==h for n,h in sources.items())
  payload=gzip.compress(raw,mtime=0);corpus.parent.mkdir(parents=True,exist_ok=True)
  with corpus.open('xb') as f:f.write(payload)
  sources[str(corpus.relative_to(ROOT))]=keys.sha(corpus)
  report=dict(schema=1,success=True,sources=sources,scope='Standalone Wine Surface2 indexed8 alias/palette lifetime, shared partial update/detach/rebind, actual primary two-buffer flips, CPU lease transfer and cached-DC/storage separation, including busy results after swapping and attached-buffer reacquisition. No original game instructions or native/live/Windows equivalence.',original_game_executed=False,original_manifest_verified_before_after=True,environment=dict(wine=subprocess.check_output(['wine','--version'],text=True).strip(),ddraw_sha256=keys.sha(run/'wineprefix/drive_c/windows/syswow64/ddraw.dll'),override='ddraw=b',display=os.environ['DISPLAY'],virtual_desktop='restore-probe,1024x768',display_bpp=8,cooperative_level='EXCLUSIVE|FULLSCREEN'),artifacts=str(run.relative_to(ROOT)),probe_sha256=keys.sha(run/'probe.exe'),outputs_sha256=keys.sha(run/'outputs.bin'),row_fields=['kind','phase','surface','result','a','b'],kinds={'1':'CreateSurface','2':'IsLost','3':'GetColorKey','4':'Lock','5':'pixel','6':'pixel_hash','7':'Unlock','8':'BltFill','9':'SetCooperativeLevel','10':'SetDisplayMode','11':'SetColorKey','12':'Restore','13':'RestoreDisplayMode','14':'SetForegroundWindow','15':'format_dimensions','16':'RGB_masks','17':'complete','18':'GetDisplayMode','19':'display_dimensions','20':'CreatePalette','21':'GetPalette_identity','22':'palette_entry','23':'SetEntries','24':'SetPalette','25':'GetEntries','26':'reload_Lock','27':'reload_Unlock','30':'QueryInterface','31':'IUnknown_identity','32':'sample_Lock','33':'sample_byte','34':'sample_key','35':'GetPalette_identity','36':'GetEntries_range','37':'palette_entry','38':'SetColorKey','39':'Release_diagnostic','40':'Flip','41':'detach_palette','42':'create_primary_double_buffer','43':'GetAttachedSurface_identity','44':'hold_Lock_or_DC','45':'return_original_surface_lease','46':'complete','47':'return_other_surface_lease','48':'DC_flipback','49':'return_restored_DC'},surface_caps=[0x840,0x840,0x218],row_count=len(rows),corpus=dict(path=str(corpus.relative_to(ROOT)),sha256=keys.sha(corpus),raw_sha256=keys.sha(run/'outputs.bin'),raw_bytes=len(raw)))
 finally:
  subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10);subprocess.run(['wineserver','-w'],env=env,check=False,timeout=10);
  try:command('original-after',[str(ROOT/'tools/original-manifest.sh'),'verify'],120)
  finally:reservation.close()
 a.report.parent.mkdir(parents=True,exist_ok=True)
 with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
 print(json.dumps(dict(success=True,rows=len(rows),flips=[r for r in rows if r[0]==40])))
if __name__=='__main__':main()
