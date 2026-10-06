#!/usr/bin/env python3
"""Native indexed bitmap conversion/DC region validity versus independent Surface2 output."""
import argparse,base64,importlib.util,json,os,shlex,struct,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def module(name,path):
 s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
p=module('p','tools/check-surface-dc-palette.py')
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--build',type=Path,default=ROOT/'working/build/ddraw-dib');parser.add_argument('--report',type=Path,required=True);parser.add_argument('--identity',action='store_true');a=parser.parse_args()
 if a.report.exists():parser.error('Refusing retained report overwrite')
 cap,data=p.load();original,_=p.dc.gdi.sentinel.load();p.compare(data,original['assets']);rows=[r for r in data['rows'] if r['input']['bits']!=8 or r['input']['palette_style']];assets=original['assets'];extra=[]
 if a.identity:
  identity=module('identity','tools/check-surface-dc-palette-identity.py');icap,idata=identity.load();identity.compare(idata);rows+=idata['rows'];assets=dict(assets,**idata['assets']);extra=['tools/check-surface-dc-palette-identity.py',str(identity.DEFAULT.relative_to(ROOT)),icap['corpus']['path']]
 names=['renderer/blit.cpp','renderer/blit.hpp','renderer/dib.cpp','renderer/dib.hpp','renderer/known_pixels.hpp','renderer/surface_copy.cpp','renderer/surface_copy.hpp','renderer/CMakeLists.txt','reconstruction/rendering/surface_bitmap.hpp','tests/render-dc-palette-test.cpp','tools/check-native-dc-palette.py','tools/check-surface-dc-palette.py','tools/check-surface-dib-ddraw.py','tools/check-surface-dib-gdi.py','tests/fixtures/surfaces/original-sentinel-corpus.json','tests/fixtures/surfaces/original-sentinel.json.gz',str(p.DEFAULT.relative_to(ROOT)),cap['corpus']['path'],*extra];sources={n:p.sha(ROOT/n) for n in names}
 parent=ROOT/'working/tests/native-dc-palette';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));directory=run/'assets';directory.mkdir()
 for name,data in assets.items():
  if data is not None:(directory/(name+'.bmp')).write_bytes(base64.b64decode(data))
 manifest=run/'cases.txt';manifest.write_text(''.join(f"{i} {r['input']['asset']} {r['input']['bits']} {r['input']['palette_style']} {r['input']['ddclip']} {r['input']['gdiclip']}\n" for i,r in enumerate(rows)))
 binary=run/'native';library=a.build.resolve()/'libmnm-renderer.a';flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Gui','Qt6OpenGL'],text=True));subprocess.run(['g++','-std=c++17','-O2','-fPIC','-Wall','-Wextra','-Werror','-pedantic','-I'+str(ROOT/'renderer'),str(ROOT/'tests/render-dc-palette-test.cpp'),str(library),*flags,'-o',str(binary)],check=True)
 output=run/'pixels.bin';proc=subprocess.run([str(binary),str(manifest),str(directory),str(output)],capture_output=True,text=True,env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1'),timeout=120);(run/'native.log').write_text(proc.stdout+proc.stderr);p.require(proc.returncode==0,proc.stdout+proc.stderr);native=json.loads(proc.stdout);raw=output.read_bytes();at=0;comparisons=0
 for i,row in enumerate(rows):
  id,bits=struct.unpack_from('<2I',raw,at);at+=8;p.require((id,bits)==(i,row['input']['bits']),'Native row identity differs')
  for expected in [[v&0xffffff if bits==32 else v for v in row['pixels']],[p.dc.colorref(v) for v in row['dc_rgb']]]:
   actual=list(struct.unpack_from('<48I',raw,at));at+=192;p.require(actual==expected,f'Native indexed/DC output differs: case{i}');comparisons+=48
 p.require(at==len(raw),'Native output extent differs');p.require(all(p.sha(ROOT/n)==h for n,h in sources.items()),'Sources changed during native test')
 report=dict(schema=1,success=True,sources=sources,native=native,driver_pixel_comparisons=comparisons,library_sha256=p.sha(library),native_binary_sha256=p.sha(binary),pixels_sha256=p.sha(output),artifacts=str(run.relative_to(ROOT)),scope='Native installed indexed palettes and RGB565/32 DIB reload with separate owned GDI region union versus independent Surface2 native/DC frames. Clipped written-region validity/partial palette/refusals checked without using original restored bytes. Absent palettes, native COM/DC lifetime, actual loss/live/wire/Windows excluded.')
 with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
 print(json.dumps(dict(native=native,pixel_comparisons=comparisons)))
if __name__=='__main__':main()
