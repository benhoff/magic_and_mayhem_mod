#!/usr/bin/env python3
"""Capture real Surface2 GetDC/GDI/ReleaseDC indexed/RGB565/RGB32 outputs, one Wine session."""
import argparse,base64,gzip,importlib.util,json,os,struct,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def module(name,path):
 s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
keys=module('keys','tools/capture-original-surface-keys.py')
sentinel=module('sentinel','tools/check-original-surface-sentinel.py')
SOURCES=['tests/surface-dc-palette-reference.c','runtime/shadow/win32_min.h','tools/capture-surface-dc-palette.py','tools/capture-original-surface-keys.py','tools/check-original-surface-sentinel.py','tests/fixtures/surfaces/original-sentinel-corpus.json','tests/fixtures/surfaces/original-sentinel.json.gz']
def inputs():
 payload,_=sentinel.load();sentinel.compare(payload);cases=[]
 for name in ['indexed1','indexed4','indexed8','rgb24','offset-gap','installed-cursors']:
  asset=base64.b64decode(payload['assets'][name]);header=asset[14:54];w,h=struct.unpack_from('<II',header,4);bits=struct.unpack_from('<H',header,14)[0];n={1:8,4:64,8:1024}.get(bits,0);length=struct.unpack_from('<I',asset,2)[0]-struct.unpack_from('<I',asset,10)[0];palette=asset[54:54+n];pixels=asset[54+n:54+n+length]
  for shape,tw,th in [('equal',w,h),('cropped',max(1,w-1),max(1,h-1)),('larger',w+1,h+1)]:
   cases.append(dict(id=len(cases),asset=name,shape=shape,width=tw,height=th,source_width=w,source_height=h,header=header,palette=palette,pixels=pixels,usage=1 if bits==24 else 0))
 return cases

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path,required=True);p.add_argument('--corpus',type=Path,default=ROOT/'tests/fixtures/surfaces/surface-dc-palette.json.gz');p.add_argument('--prefix-template',type=Path,default=ROOT/'working/wineprefix-x86_64');a=p.parse_args()
 if a.report.exists() or a.corpus.exists():p.error('Refusing retained evidence overwrite')
 if not os.environ.get('DISPLAY'):p.error('Run under Xvfb/display')
 reservation=keys.reserve_wine();parent=ROOT/'working/tests/surface-dc-palette';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
 env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.pop('WAYLAND_DISPLAY',None);env.update(WINEPREFIX=str(run/'wineprefix'),WINEDEBUG='-all',LIBGL_ALWAYS_SOFTWARE='1',WINEDLLOVERRIDES='ddraw=b')
 sources={n:keys.sha(ROOT/n) for n in SOURCES}
 def command(name,argv,timeout=60):
  with (run/(name+'.log')).open('w') as f:subprocess.run(argv,cwd=run,env=env,stdout=f,stderr=f,check=True,timeout=timeout)
 try:
  command('original-before',[str(ROOT/'tools/original-manifest.sh'),'verify'],120)
  command('prefix',['cp','-a','--reflink=auto',str(a.prefix_template.resolve()),env['WINEPREFIX']],120);command('wineboot',['wineboot','-u'],120)
  imports={'LoadLibraryA':4,'GetProcAddress':8,'CreateFileA':28,'ReadFile':20,'WriteFile':20,'CloseHandle':4,'ExitProcess':4};definition=run/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{v}\n' for n,v in imports.items()))
  command('imports',['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(run/'kernel32.lib')]);command('compile',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',str(ROOT/SOURCES[0]),'-o',str(run/'probe.obj')]);command('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(run/'probe.exe'),str(run/'probe.obj'),str(run/'kernel32.lib')])
  pairs=[(0,0),(1,0),(2,0),(3,0),(4,0),(0,1),(0,2),(0,3),(0,4),(2,1)]
  cases=[dict(c,id=i,width=8,height=6,bits=bits,palette_style=style,ddclip=ddclip,gdiclip=gdiclip) for i,(c,bits,style,ddclip,gdiclip) in enumerate((c,bits,style,ddclip,gdiclip) for c in inputs() if c['shape']=='equal' for bits in [8,16,32] for style in ([0,1,2,3] if bits==8 else [0]) for ddclip,gdiclip in pairs)]
  data=struct.pack('<I',len(cases))+b''.join(struct.pack('<12I',c['id'],8,6,c['source_width'],c['source_height'],40+len(c['palette']),len(c['pixels']),c['usage'],c['bits'],c['palette_style'],c['ddclip'],c['gdiclip'])+c['header']+c['palette']+c['pixels'] for c in cases);(run/'inputs.bin').write_bytes(data)
  command('gdi',['wine',str(run/'probe.exe')]);raw=(run/'outputs.bin').read_bytes();at=0;rows=[]
  for c in cases:
   fields=['id','create','actual_caps','bind_palette','create_palette','create_clipper','set_clip_list','attach_clipper','get_dc','dc_out_state','table_count','clip_box_result','clip_left','clip_top','clip_right','clip_bottom','select_clip','dib_result','flush','clear_clip','release_dc','final_lock','bits','width','height','pitch','red_mask','green_mask','blue_mask','final_unlock','initial_lock','initial_unlock'];values=struct.unpack_from('<32I',raw,at);at+=128;observation=dict(zip(fields,values));assert (observation['id'],observation['width'],observation['height'],observation['bits'])==(c['id'],8,6,c['bits']);colors=list(struct.unpack_from('<256I',raw,at));at+=1024;arrays={}
   for field in ['dc_before','dc_clipped','dc_rgb','pixels']:arrays[field]=list(struct.unpack_from('<48I',raw,at));at+=192
   rows.append(dict(input={k:v for k,v in c.items() if k not in ['header','palette','pixels']},observation=observation,color_table=colors,**arrays))
  assert raw[at:]==struct.pack('<I',0x50444331)
  assert all(keys.sha(ROOT/n)==v for n,v in sources.items())
  a.corpus.parent.mkdir(parents=True,exist_ok=True)
  with a.corpus.open('xb') as f:f.write(gzip.compress(json.dumps(dict(schema=1,rows=rows),separators=(',',':')).encode(),mtime=0))
  sources[str(a.corpus.relative_to(ROOT))]=keys.sha(a.corpus)
  report=dict(schema=1,success=True,sources=sources,cases=len(rows),pixels=sum(len(r['pixels']) for r in rows),dc_pixels=sum(len(r['dc_rgb']) for r in rows),corpus=dict(path=str(a.corpus.relative_to(ROOT)),sha256=keys.sha(a.corpus)),inputs_sha256=keys.sha(run/'inputs.bin'),outputs_sha256=keys.sha(run/'outputs.bin'),probe_sha256=keys.sha(run/'probe.exe'),artifacts=str(run.relative_to(ROOT)),environment=dict(wine=subprocess.check_output(['wine','--version'],text=True).strip(),gdi32_sha256=keys.sha(run/'wineprefix/drive_c/windows/syswow64/gdi32.dll'),ddraw_sha256=keys.sha(run/'wineprefix/drive_c/windows/syswow64/ddraw.dll'),override='ddraw=b'),original_game_executed=False,scope='Standalone real Wine Surface2 GetDC/GDI/ReleaseDC to requested indexed/RGB565/RGB32 offscreen surfaces with system-memory8x6 targets,360 cases from6 retained original loader inputs; absent/gray/reversed/cube destination palettes, attached one/two/missing/empty DirectDraw clip lists and explicit one/two/empty/overlapping GDI regions plus combined case. Native Lock words, actual DC color tables, before/clipped/cleared GetPixel frames. No original game instructions, actual loss, Windows hardware or live/wire equivalence.')
 finally:
  subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10);subprocess.run(['wineserver','-w'],env=env,check=False,timeout=10)
  try:command('original-after',[str(ROOT/'tools/original-manifest.sh'),'verify'],120)
  finally:reservation.close()
 report['original_manifest_verified_before_after']=True
 with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
 print(json.dumps(dict(success=True,cases=len(rows),get_dc_results=sorted({r['observation']['get_dc'] for r in rows}))))
if __name__=='__main__':main()
