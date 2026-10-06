#!/usr/bin/env python3
"""Owned native DC raster context versus independent three-lease Surface2 frames."""
import argparse,base64,importlib.util,json,os,shlex,struct,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('r',ROOT/'tools/check-surface-dc-reuse.py');r=importlib.util.module_from_spec(spec);spec.loader.exec_module(r)
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build',type=Path,default=ROOT/'working/build/ddraw-dib');p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 if a.report.exists():p.error('Refusing retained report overwrite')
 cap,data=r.load();original,_=r.p.dc.gdi.sentinel.load();r.compare(data,original['assets']);names=['renderer/bitmap_dc.hpp','renderer/blit.hpp','renderer/blit.cpp','renderer/dib.hpp','renderer/dib.cpp','renderer/surface_copy.hpp','renderer/surface_copy.cpp','renderer/known_pixels.hpp','renderer/CMakeLists.txt','reconstruction/rendering/surface_bitmap.hpp','tests/render-dc-reuse-test.cpp','tools/check-native-dc-reuse.py','tools/check-surface-dc-reuse.py','tools/check-surface-dc-palette.py','tools/check-surface-dib-ddraw.py','tools/check-surface-dib-gdi.py','tests/fixtures/surfaces/original-sentinel.json.gz','tests/fixtures/surfaces/original-sentinel-corpus.json',str(r.DEFAULT.relative_to(ROOT)),cap['corpus']['path']];sources={n:r.sha(ROOT/n) for n in names}
 parent=ROOT/'working/tests/native-dc-reuse';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));assets=run/'assets';assets.mkdir()
 for name,raw in original['assets'].items():
  if raw is not None:(assets/(name+'.bmp')).write_bytes(base64.b64decode(raw))
 manifest=run/'inputs.txt';manifest.write_text(' '.join(str(v) for v in data['default_dc_palette'])+'\n'+''.join(f"{c['id']} {c['asset']} {c['bits']} {c['initial_palette']} {c['action']} {c['clip']}\n" for c in (row['input'] for row in data['rows'])))
 binary=run/'native';library=a.build.resolve()/'libmnm-renderer.a';qt=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Gui','Qt6OpenGL'],text=True));subprocess.run(['g++','-std=c++17','-O2','-fPIC','-Wall','-Wextra','-Werror','-pedantic','-I'+str(ROOT/'renderer'),str(ROOT/'tests/render-dc-reuse-test.cpp'),str(library),*qt,'-o',str(binary)],check=True)
 output=run/'pixels.bin';proc=subprocess.run([str(binary),str(manifest),str(assets),str(output)],capture_output=True,text=True,env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1'),timeout=120);(run/'native.log').write_text(proc.stdout+proc.stderr);r.require(proc.returncode==0,proc.stdout+proc.stderr);native=json.loads(proc.stdout);raw=output.read_bytes();at=0;pixels=0;entries=0
 for row in data['rows']:
  for phase,frame in enumerate(row['phases']):
   id,ph,bits=struct.unpack_from('<3I',raw,at);at+=12;r.require((id,ph,bits)==(row['input']['id'],phase,row['input']['bits']),'Native phase identity differs')
   for field in ['table_before','table_draw']:
    values=list(struct.unpack_from('<256I',raw,at));at+=1024;r.require(values==frame[field],'Native DC palette state differs');entries+=256 if bits==8 else 0
   for expected in [[v&0xffffff if bits==32 else v for v in frame['pixels']],[r.p.dc.colorref(v) for v in frame['dc_rgb']]]:
    actual=list(struct.unpack_from('<48I',raw,at));at+=192;r.require(actual==expected,f'Native DC pixels differ: case{id} phase{ph}');pixels+=48
 r.require(at==len(raw),'Native output extent');r.require(all(r.sha(ROOT/n)==h for n,h in sources.items()),'Sources changed during native comparison')
 report=dict(schema=1,success=True,sources=sources,native=native,driver_pixel_comparisons=pixels,palette_entry_comparisons=entries,default_palette_context_sha256=r.sha(run/'inputs.txt'),library_sha256=r.sha(library),binary_sha256=r.sha(binary),outputs_sha256=r.sha(output),artifacts=str(run.relative_to(ROOT)),scope='Owned BitmapDcState and GlBlitter raster target192cases/576leases compare independent Surface2 before/draw tables/native/DC RGB outputs. Default table is separate preflight environment input; snapshots/deferred binding/reset clipping/private DC edits plus native guard/validity policies. No COM HDC, global busy/lock ownership, original engine/loss/Windows/live/wire replacement.')
 with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
 print(json.dumps(dict(native=native,pixel_comparisons=pixels,palette_entries=entries)))
if __name__=='__main__':main()
