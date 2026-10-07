#!/usr/bin/env python3
"""Capture Surface2 native formats, keys and signed/padded rows; serialized Wine."""
import argparse,gzip,importlib.util,json,os,struct,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def module(name,path):
 s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
keys=module('keys','tools/capture-original-surface-keys.py')
SOURCES=['tests/surface-formats-reference.c','tests/surface-access-reference.c','runtime/shadow/win32_min.h','tools/capture-surface-formats.py','tools/capture-original-surface-keys.py']
def inputs():return [dict(id=i,format=f,layout=layout,kind=kind,clip=clip,key_mode=mode) for i,(f,layout,kind,clip,mode) in enumerate((f,l,k,c,m) for f in range(5) for l in range(5) for k in range(7) for c in range(5) for m in range(4))]

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path,required=True);p.add_argument('--corpus',type=Path,default=ROOT/'tests/fixtures/surfaces/surface-formats-final.json.gz');p.add_argument('--prefix-template',type=Path,default=ROOT/'working/wineprefix-x86_64');a=p.parse_args();a.report=a.report.resolve();a.corpus=a.corpus.resolve()
 if a.report.exists() or a.corpus.exists():p.error('Refusing retained evidence overwrite')
 if not os.environ.get('DISPLAY'):p.error('Run under Xvfb/display')
 reservation=keys.reserve_wine();parent=ROOT/'working/tests/surface-formats';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
 env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.pop('WAYLAND_DISPLAY',None);env.update(WINEPREFIX=str(run/'wineprefix'),WINEDEBUG='-all',LIBGL_ALWAYS_SOFTWARE='1',WINEDLLOVERRIDES='ddraw=b')
 sources={n:keys.sha(ROOT/n) for n in SOURCES}
 def command(name,argv,timeout=60):
  with (run/(name+'.log')).open('w') as f:subprocess.run(argv,cwd=run,env=env,stdout=f,stderr=f,check=True,timeout=timeout)
 try:
  command('original-before',[str(ROOT/'tools/original-manifest.sh'),'verify'],120)
  command('prefix',['cp','-a','--reflink=auto',str(a.prefix_template.resolve()),env['WINEPREFIX']],120);command('wineboot',['wineboot','-u'],120)
  imports={'LoadLibraryA':4,'GetProcAddress':8,'CreateFileA':28,'ReadFile':20,'WriteFile':20,'CloseHandle':4,'ExitProcess':4};definition=run/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{v}\n' for n,v in imports.items()))
  command('imports',['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(run/'kernel32.lib')]);command('compile',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',str(ROOT/SOURCES[0]),'-o',str(run/'probe.obj')]);command('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(run/'probe.exe'),str(run/'probe.obj'),str(run/'kernel32.lib')])
  cases=inputs();data=struct.pack('<I',len(cases))+b''.join(struct.pack('<6I',*(c[k] for k in ['id','format','layout','kind','clip','key_mode'])) for c in cases);(run/'inputs.bin').write_bytes(data)
  command('driver',['wine',str(run/'probe.exe')]);raw=(run/'outputs.bin').read_bytes();at=0;rows=[]
  def words(n):
   nonlocal at
   values=list(struct.unpack_from('<'+str(n)+'I',raw,at));at+=n*4;return values
  def sample():return dict(descriptor=words(27),pixels=words(35),pointer_matches=words(1)[0])
  for c in cases:
   values=words(6);assert values==[c[k] for k in ['id','format','layout','kind','clip','key_mode']];row=dict(input=c,direct_create=words(2),set_descriptor=words(2),create=words(2))
   if not any(row['create']):
    row.update(source_before=sample(),destination_before=sample(),key_results=words(3),key_range=words(2));draw=words(2);row.update(flags=draw[0],hresult=draw[1],source_after=sample(),destination_after=sample())
    if c['layout']:
     row['source_storage']=raw[at:at+256].hex();at+=256;row['destination_storage']=raw[at:at+256].hex();at+=256
   rows.append(row)
  assert raw[at:]==struct.pack('<I',0x464d5431)
  assert all(keys.sha(ROOT/n)==v for n,v in sources.items())
  a.corpus.parent.mkdir(parents=True,exist_ok=True)
  with a.corpus.open('xb') as f:f.write(gzip.compress(json.dumps(dict(schema=1,rows=rows),separators=(',',':')).encode(),mtime=0))
  sources[str(a.corpus.relative_to(ROOT))]=keys.sha(a.corpus)
  report=dict(schema=1,success=True,sources=sources,cases=len(rows),corpus=dict(path=str(a.corpus.relative_to(ROOT)),sha256=keys.sha(a.corpus)),inputs_sha256=keys.sha(run/'inputs.bin'),outputs_sha256=keys.sha(run/'outputs.bin'),probe_sha256=keys.sha(run/'probe.exe'),artifacts=str(run.relative_to(ROOT)),environment=dict(wine=subprocess.check_output(['wine','--version'],text=True).strip(),ddraw_sha256=keys.sha(run/'wineprefix/drive_c/windows/syswow64/ddraw.dll'),override='ddraw=b'),original_game_executed=False,scope='Standalone Wine DirectDraw2/Surface2 five explicit indexed8/RGB555/RGB565/RGB24/RGB32 formats, five default/padded/tight/negative layouts, seven fill/copy/self-keyed operation families, five clip states and four key states. Independent results/normalized descriptors/complete native pixels and caller-storage padding/guards. Actual caller-memory creation failures and Surface3 SetSurfaceDesc memory binding results retained. No original game instructions, original format reachability, cross-format conversion, physical display/Windows/live equivalence.')

 finally:
  subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10);subprocess.run(['wineserver','-w'],env=env,check=False,timeout=10)
  try:command('original-after',[str(ROOT/'tools/original-manifest.sh'),'verify'],120)
  finally:reservation.close()
 report['original_manifest_verified_before_after']=True
 with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
 print(json.dumps(dict(success=True,cases=len(rows),create_results=sorted({v for r in rows for v in r['create']}),hresults=sorted({r['hresult'] for r in rows if 'hresult' in r}))))
if __name__=='__main__':main()
