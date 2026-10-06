#!/usr/bin/env python3
"""Recovered C++ cursor routing/BMP inputs and native GL versus retained GDI pixels."""
import argparse,base64,importlib.util,json,os,shlex,struct,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def module(name,path):
 s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
gdi=module('gdi','tools/check-surface-dib-gdi.py');s=gdi.sentinel

def projected(out):
 events=[]
 for e in out['events']:
  k=e['kind']
  if k=='draw':events.append([2 if e['fast'] else 1,2,e['result'],e['flags'],e['color'],0])
  elif k=='restore':events.append([3,e['surface'],e['result'],0,0,0])
  elif k=='key':events.append([4,e['surface'],e['result'],e['flags'],*e['range']])
  elif k=='callback':events.append([5,e['surface'],0,e['argument'],0,0])
  elif k=='report':events.append([6,0,e['result'],0,0,0])
  elif k=='cursor_reload':events.append([7,3,0,0,0,0])
 return dict(id=out['id'],draws_consumed=out['draws_consumed'],events=events)

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build',type=Path,default=ROOT/'working/build/cursor-reload');p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 if a.report.exists():p.error('Refusing retained evidence overwrite')
 payload,_=s.load();s.compare(payload);capture,gdi_payload=gdi.load();gdi.compare(gdi_payload,payload['assets']);rows=[r for r in payload['rows'] if r['input']['entry'] in [0x58bac0,0x58bc10,0x58c4a0,0x58c8a0]]
 names=['reconstruction/rendering/surface_retry.hpp','reconstruction/rendering/surface_bitmap.hpp','renderer/dib.hpp','renderer/dib.cpp','renderer/blit.hpp','renderer/blit.cpp','renderer/known_pixels.hpp','renderer/surface_copy.hpp','renderer/surface_copy.cpp','renderer/CMakeLists.txt','tests/surface-cursor-retry-model.cpp','tests/render-cursor-reload-test.cpp','tools/check-native-cursor-reload.py','tools/check-surface-dib-gdi.py','tools/check-original-surface-sentinel.py','tests/fixtures/surfaces/original-sentinel-corpus.json','tests/fixtures/surfaces/original-sentinel.json.gz',str(gdi.DEFAULT.relative_to(ROOT)),capture['corpus']['path']];sources={n:s.sha(ROOT/n) for n in names}
 parent=ROOT/'working/tests/native-cursor-reload';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));assets=run/'assets';assets.mkdir()
 for name,data in payload['assets'].items():
  if data is not None:(assets/(name+'.bmp')).write_bytes(base64.b64decode(data))
 schedule=run/'cases.txt';schedule.write_text(''.join(' '.join(str(v) for v in [*(c[k] for k in ['id','entry','fast','no_wait','key_enabled','source_reload','destination_reload','source_restore','destination_restore','key_result','color']),len(c['draw_results']),*c['draw_results']])+'\n' for c in (r['input'] for r in rows)))
 options=['g++','-std=c++17','-O2','-fPIC','-Wall','-Wextra','-Werror','-pedantic'];core=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Core'],text=True));model=run/'model';subprocess.run([*options,str(ROOT/'tests/surface-cursor-retry-model.cpp'),*core,'-o',str(model)],check=True)
 def traces(budget=None):
  out=subprocess.run([str(model),str(schedule),str(assets),*([str(budget)] if budget is not None else [])],capture_output=True,text=True,timeout=30);s.require(out.returncode==0,out.stderr);return json.loads(out.stdout)
 actual=traces();expected=[projected(r['original']) for r in rows]
 for e,o in zip(expected,actual['rows'],strict=True):s.require(o['returned'] and not o['budget_exhausted'] and {k:o[k] for k in e}==e,'Recovered cursor route mismatch')
 for item in actual['assets']:
  original=next(r['original'] for r in payload['rows'] if r['input']['entry']==0x58d1a0 and r['input']['asset']==item['asset'] and r['input']['global_surface'] and not r['input']['allocation_failure']);s.require(item['loaded']==original['return_value'],'Recovered file-load return mismatch')
  dib=next((e for e in original['events'] if e['kind']=='dib'),None)
  if dib:
   for k in ['usage','info_sha256','palette_sha256','pixels_sha256']:s.require(item[k]==dib[k],'Recovered submitted buffer mismatch')
 bounded=traces(1);stops=0
 for e,o in zip(expected,bounded['rows'],strict=True):
  s.require(o['events']==e['events'][:len(o['events'])] and o['draws_consumed']<=1,'Bounded route prefix mismatch')
  if o['budget_exhausted']:s.require(not o['returned'],'Budget disguised as return');stops+=1
 for budget in [0,65537]:
  out=subprocess.run([str(model),str(schedule),str(assets),str(budget)],capture_output=True,text=True,timeout=30);s.require(out.returncode!=0,'Invalid budget admitted')
 manifest=run/'gdi.txt';manifest.write_text(''.join(f"{r['input']['id']} {r['input']['asset']} {r['input']['width']} {r['input']['height']} {bits}\n" for bits in [24,32] for r in gdi_payload['rows']))
 qt=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Gui','Qt6OpenGL'],text=True));binary=run/'native';library=a.build.resolve()/'libmnm-renderer.a';subprocess.run([*options,'-I'+str(ROOT/'renderer'),str(ROOT/'tests/render-cursor-reload-test.cpp'),str(library),*qt,'-o',str(binary)],check=True)
 output=run/'pixels.bin';proc=subprocess.run([str(binary),str(manifest),str(assets),str(output)],env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1'),capture_output=True,text=True,timeout=60);(run/'native.log').write_text(proc.stdout+proc.stderr);s.require(proc.returncode==0,proc.stdout+proc.stderr);native=json.loads(proc.stdout);raw=output.read_bytes();at=0;pixels=0
 for bits in [24,32]:
  for r in gdi_payload['rows']:
   c=r['input'];id,w,h,b=struct.unpack_from('<4I',raw,at);at+=16;s.require((id,w,h,b)==(c['id'],c['width'],c['height'],bits),'Native output identity mismatch');reference=[p&0xffffff for p in r['pixels']]
   for _ in range(2):
    actual_pixels=list(struct.unpack_from('<'+'I'*(w*h),raw,at));at+=w*h*4;s.require(actual_pixels==reference,'Native reload/presentation differs from independent GDI pixels');pixels+=w*h
 s.require(at==len(raw),'Native output extent');s.require(all(s.sha(ROOT/n)==h for n,h in sources.items()),'Sources changed during native execution')
 report=dict(schema=1,success=True,original_sha256=s.BUILD_HASH,sources=sources,scheduling_cases=len(rows),matched=len(expected),asset_cases=len(actual['assets']),budget_stops=stops,invalid_budgets_refused=2,native=native,gdi_rgb_pixel_comparisons=pixels,model_binary_sha256=s.sha(model),native_binary_sha256=s.sha(binary),library_sha256=s.sha(library),native_pixels_sha256=s.sha(output),artifacts=str(run.relative_to(ROOT)),scope='Recovered C++ none/callback/global-cursor scheduling projected against original x86 captures and12 exact original file return/submitted-buffer cases. Native GL canonical RGB24/32 DIB reload/presentation versus18 independent Wine GDI32-bit DIBSection raster outputs;12 scripted recovery compositions preserve triggering-surface invalidity. No DirectDraw DC/format, original CRT, real loss or live equivalence.')
 with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
 print(json.dumps({k:report[k] for k in ['scheduling_cases','asset_cases','budget_stops','native','gdi_rgb_pixel_comparisons']}))
if __name__=='__main__':main()
