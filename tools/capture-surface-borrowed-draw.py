#!/usr/bin/env python3
"""Capture real Surface2 Lock/Unlock/GetDC/ReleaseDC transitions, one Wine session."""
import argparse,gzip,importlib.util,json,os,struct,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def module(name,path):
 s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
keys=module('keys','tools/capture-original-surface-keys.py')
SOURCES=['tests/surface-borrowed-draw-reference.c','tests/surface-access-reference.c','runtime/shadow/win32_min.h','tools/capture-surface-borrowed-draw.py','tools/capture-original-surface-keys.py']
def inputs():return [dict(id=i,kind=kind,held=held,clip=clip) for i,(kind,held,clip) in enumerate((kind,held,clip) for kind in range(5) for held in range(6) for clip in range(6))]

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path,required=True);p.add_argument('--corpus',type=Path,default=ROOT/'tests/fixtures/surfaces/surface-borrowed-draw.json.gz');p.add_argument('--prefix-template',type=Path,default=ROOT/'working/wineprefix-x86_64');a=p.parse_args()
 if a.report.exists() or a.corpus.exists():p.error('Refusing retained evidence overwrite')
 if not os.environ.get('DISPLAY'):p.error('Run under Xvfb/display')
 reservation=keys.reserve_wine();parent=ROOT/'working/tests/surface-borrowed-draw';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
 env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.pop('WAYLAND_DISPLAY',None);env.update(WINEPREFIX=str(run/'wineprefix'),WINEDEBUG='-all',LIBGL_ALWAYS_SOFTWARE='1',WINEDLLOVERRIDES='ddraw=b')
 sources={n:keys.sha(ROOT/n) for n in SOURCES}
 def command(name,argv,timeout=60):
  with (run/(name+'.log')).open('w') as f:subprocess.run(argv,cwd=run,env=env,stdout=f,stderr=f,check=True,timeout=timeout)
 try:
  command('original-before',[str(ROOT/'tools/original-manifest.sh'),'verify'],120)
  command('prefix',['cp','-a','--reflink=auto',str(a.prefix_template.resolve()),env['WINEPREFIX']],120);command('wineboot',['wineboot','-u'],120)
  imports={'LoadLibraryA':4,'GetProcAddress':8,'CreateFileA':28,'ReadFile':20,'WriteFile':20,'CloseHandle':4,'ExitProcess':4};definition=run/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{v}\n' for n,v in imports.items()))
  command('imports',['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(run/'kernel32.lib')]);command('compile',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',str(ROOT/SOURCES[0]),'-o',str(run/'probe.obj')]);command('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(run/'probe.exe'),str(run/'probe.obj'),str(run/'kernel32.lib')])
  cases=inputs();data=struct.pack('<I',len(cases))+b''.join(struct.pack('<4I',c['id'],c['kind'],c['held'],c['clip']) for c in cases);(run/'inputs.bin').write_bytes(data)
  command('driver',['wine',str(run/'probe.exe')]);raw=(run/'outputs.bin').read_bytes();at=0;rows=[]
  for c in cases:
   values=list(struct.unpack_from('<6I',raw,at));at+=24;assert values[:4]==[c['id'],c['kind'],c['held'],c['clip']];row=dict(input=c,flags=values[4],hresult=values[5])
   for field in ['source_before','destination_before','source_after','destination_after']:row[field]=list(struct.unpack_from('<48I',raw,at));at+=192
   rows.append(row)
  assert raw[at:]==struct.pack('<I',0x42524431)
  assert all(keys.sha(ROOT/n)==v for n,v in sources.items())
  a.corpus.parent.mkdir(parents=True,exist_ok=True)
  with a.corpus.open('xb') as f:f.write(gzip.compress(json.dumps(dict(schema=1,rows=rows),separators=(',',':')).encode(),mtime=0))
  sources[str(a.corpus.relative_to(ROOT))]=keys.sha(a.corpus)
  report=dict(schema=1,success=True,sources=sources,cases=len(rows),pixels=len(rows)*96,corpus=dict(path=str(a.corpus.relative_to(ROOT)),sha256=keys.sha(a.corpus)),inputs_sha256=keys.sha(run/'inputs.bin'),outputs_sha256=keys.sha(run/'outputs.bin'),probe_sha256=keys.sha(run/'probe.exe'),artifacts=str(run.relative_to(ROOT)),environment=dict(wine=subprocess.check_output(['wine','--version'],text=True).strip(),gdi32_sha256=keys.sha(run/'wineprefix/drive_c/windows/syswow64/gdi32.dll'),ddraw_sha256=keys.sha(run/'wineprefix/drive_c/windows/syswow64/ddraw.dll'),override='ddraw=b'),original_game_executed=False,scope='Standalone Wine Surface2 RGB565180draws:COLORFILL/opaque-keyed Blt-BltFast with source/destination Lock/DC and destination Lock+DC,6clip states. Independent results and full before/after bytes. No original game, actual loss, Windows/live equivalence.')
 finally:
  subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10);subprocess.run(['wineserver','-w'],env=env,check=False,timeout=10)
  try:command('original-after',[str(ROOT/'tools/original-manifest.sh'),'verify'],120)
  finally:reservation.close()
 report['original_manifest_verified_before_after']=True
 with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
 print(json.dumps(dict(success=True,cases=len(rows),hresults=sorted({r['hresult'] for r in rows}))))
if __name__=='__main__':main()
