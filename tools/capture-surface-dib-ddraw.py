#!/usr/bin/env python3
"""Capture real Surface2 GetDC/GDI/ReleaseDC RGB565/RGB32 outputs, one Wine session."""
import argparse,base64,gzip,importlib.util,json,os,struct,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def module(name,path):
 s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
keys=module('keys','tools/capture-original-surface-keys.py')
sentinel=module('sentinel','tools/check-original-surface-sentinel.py')
SOURCES=['tests/surface-dib-ddraw-reference.c','runtime/shadow/win32_min.h','tools/capture-surface-dib-ddraw.py','tools/capture-original-surface-keys.py','tools/check-original-surface-sentinel.py','tests/fixtures/surfaces/original-sentinel-corpus.json','tests/fixtures/surfaces/original-sentinel.json.gz']
def inputs():
 payload,_=sentinel.load();sentinel.compare(payload);cases=[]
 for name in ['indexed1','indexed4','indexed8','rgb24','offset-gap','installed-cursors']:
  asset=base64.b64decode(payload['assets'][name]);header=asset[14:54];w,h=struct.unpack_from('<II',header,4);bits=struct.unpack_from('<H',header,14)[0];n={1:8,4:64,8:1024}.get(bits,0);length=struct.unpack_from('<I',asset,2)[0]-struct.unpack_from('<I',asset,10)[0];palette=asset[54:54+n];pixels=asset[54+n:54+n+length]
  for shape,tw,th in [('equal',w,h),('cropped',max(1,w-1),max(1,h-1)),('larger',w+1,h+1)]:
   cases.append(dict(id=len(cases),asset=name,shape=shape,width=tw,height=th,source_width=w,source_height=h,header=header,palette=palette,pixels=pixels,usage=1 if bits==24 else 0))
 return cases

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path,required=True);p.add_argument('--corpus',type=Path,default=ROOT/'tests/fixtures/surfaces/surface-dib-ddraw.json.gz');p.add_argument('--prefix-template',type=Path,default=ROOT/'working/wineprefix-x86_64');a=p.parse_args()
 if a.report.exists() or a.corpus.exists():p.error('Refusing retained evidence overwrite')
 if not os.environ.get('DISPLAY'):p.error('Run under Xvfb/display')
 reservation=keys.reserve_wine();parent=ROOT/'working/tests/surface-dib-ddraw';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
 env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.pop('WAYLAND_DISPLAY',None);env.update(WINEPREFIX=str(run/'wineprefix'),WINEDEBUG='-all',LIBGL_ALWAYS_SOFTWARE='1',WINEDLLOVERRIDES='ddraw=b')
 sources={n:keys.sha(ROOT/n) for n in SOURCES}
 def command(name,argv,timeout=60):
  with (run/(name+'.log')).open('w') as f:subprocess.run(argv,cwd=run,env=env,stdout=f,stderr=f,check=True,timeout=timeout)
 try:
  command('original-before',[str(ROOT/'tools/original-manifest.sh'),'verify'],120)
  command('prefix',['cp','-a','--reflink=auto',str(a.prefix_template.resolve()),env['WINEPREFIX']],120);command('wineboot',['wineboot','-u'],120)
  imports={'LoadLibraryA':4,'GetProcAddress':8,'CreateFileA':28,'ReadFile':20,'WriteFile':20,'CloseHandle':4,'ExitProcess':4};definition=run/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{v}\n' for n,v in imports.items()))
  command('imports',['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(run/'kernel32.lib')]);command('compile',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',str(ROOT/SOURCES[0]),'-o',str(run/'probe.obj')]);command('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(run/'probe.exe'),str(run/'probe.obj'),str(run/'kernel32.lib')])
  cases=[dict(c,id=i,bits=bits,caps=caps) for i,(c,bits,caps) in enumerate((c,bits,caps) for c in inputs() for bits in [16,32] for caps in [0x840,0x40])];data=struct.pack('<I',len(cases))+b''.join(struct.pack('<10I',c['id'],c['width'],c['height'],c['source_width'],c['source_height'],40+len(c['palette']),len(c['pixels']),c['usage'],c['bits'],c['caps'])+c['header']+c['palette']+c['pixels'] for c in cases);(run/'inputs.bin').write_bytes(data)
  command('gdi',['wine',str(run/'probe.exe')]);raw=(run/'outputs.bin').read_bytes();at=0;rows=[]
  for c in cases:
   fields=['id','create','actual_caps','initial_lock','get_dc_while_locked','locked_out_state','initial_unlock','get_dc','dc_out_state','repeated_get_dc','repeated_out_state','lock_while_dc','dib_result','flush','bitmap_result','bitmap_width','bitmap_height','bitmap_bits','release_dc','repeated_release_dc','final_lock','bits','width','height','pitch','red_mask','green_mask','blue_mask','final_unlock','dc_first_rgb','dc_middle_rgb','dc_last_rgb'];values=struct.unpack_from('<32I',raw,at);at+=128;observation=dict(zip(fields,values));w,h=observation['width'],observation['height'];assert (observation['id'],w,h,observation['bits'])==(c['id'],c['width'],c['height'],c['bits']);pixels=list(struct.unpack_from('<'+'I'*(w*h),raw,at));at+=w*h*4
   rows.append(dict(input={k:v for k,v in c.items() if k not in ['header','palette','pixels']},observation=observation,pixels=pixels))
  assert raw[at:]==struct.pack('<I',0x44444331)
  assert all(keys.sha(ROOT/n)==v for n,v in sources.items())
  a.corpus.parent.mkdir(parents=True,exist_ok=True)
  with a.corpus.open('xb') as f:f.write(gzip.compress(json.dumps(dict(schema=1,rows=rows),separators=(',',':')).encode(),mtime=0))
  sources[str(a.corpus.relative_to(ROOT))]=keys.sha(a.corpus)
  report=dict(schema=1,success=True,sources=sources,cases=len(rows),pixels=sum(len(r['pixels']) for r in rows),corpus=dict(path=str(a.corpus.relative_to(ROOT)),sha256=keys.sha(a.corpus)),inputs_sha256=keys.sha(run/'inputs.bin'),outputs_sha256=keys.sha(run/'outputs.bin'),probe_sha256=keys.sha(run/'probe.exe'),artifacts=str(run.relative_to(ROOT)),environment=dict(wine=subprocess.check_output(['wine','--version'],text=True).strip(),gdi32_sha256=keys.sha(run/'wineprefix/drive_c/windows/syswow64/gdi32.dll'),ddraw_sha256=keys.sha(run/'wineprefix/drive_c/windows/syswow64/ddraw.dll'),override='ddraw=b'),original_game_executed=False,scope='Standalone real Wine Surface2 GetDC/GDI/ReleaseDC to requested RGB565/RGB32 offscreen surfaces with two cap classes,72 cases from retained original loader inputs; locked/repeated DC admission, output-handle states, selected bitmap and native Lock pixel outputs. No original game execution, actual surface loss, indexed/DC clipping, Windows hardware or live equivalence.')
 finally:
  subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10);subprocess.run(['wineserver','-w'],env=env,check=False,timeout=10)
  try:command('original-after',[str(ROOT/'tools/original-manifest.sh'),'verify'],120)
  finally:reservation.close()
 report['original_manifest_verified_before_after']=True
 with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
 print(json.dumps(dict(success=True,cases=len(rows),get_dc_results=sorted({r['observation']['get_dc'] for r in rows}))))
if __name__=='__main__':main()
