#!/usr/bin/env python3
"""One owned native backend versus retained original/driver operations and DC sequences."""
import argparse,base64,gzip,hashlib,importlib.util,json,os,shlex,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def module(name,path):
 s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
fills=module('fills','tools/check-original-surface-fills.py');keys=fills.key_check
overlap=module('overlap','tools/check-original-surface-overlap.py')
clipping=module('clipping','tools/check-native-surface-clipping.py')
reuse=module('reuse','tools/check-surface-dc-reuse.py')
access=module('access','tools/check-surface-access.py')
require=keys.require;sha=keys.sha

def inputs(combined=False):
 f,_=fills.load(fills.DEFAULT);k,_=keys.load(keys.DEFAULT);o,_=overlap.load(overlap.DEFAULT);c=clipping.corpus();rc,rd=reuse.load();ac,ad=access.load();original,_=reuse.p.dc.gdi.sentinel.load();reuse.compare(rd,original['assets']);access.compare(ad);draws=[];expected=[]
 groups=[('fills',f),('keys',k),('clipping',c),('overlap',o)]
 if combined:
  ko=module('keyed_overlap','tools/check-original-surface-keyed-overlap.py');kk,_=ko.load();ko.compare(kk);groups.append(('keyed-overlap',kk))
 for category,rows in groups:
  for row in rows:
   i=row['input'];fill=category=='fills';keyed=category=='keys';phase=i.get('phase',0);x=dict(tag=f"{category}:{i['id']}:{phase}",kind='fill' if fill else 'copy',destination_pixels=row['destination_before'],destination=i['destination'],clip=i['mode'] if fill else phase if category in ['overlap','keyed-overlap'] else i.get('clip',0) if category=='clipping' else 0,held=i.get('held',0),shared=category in ['overlap','keyed-overlap'],continues=keyed and phase==1,keep=keyed and i['mode']==2 and phase==0)
   if fill:x.update(color=i['key']&0xffff,null_rect=bool(phase),flags=row['trace'][2])
   else:x.update(source_pixels=row['source_before'],source=i['source'],fast=bool(i['fast']),flags=row['trace'][2] if keyed or category in ['overlap','keyed-overlap'] else i['flags'],key=row['key_before'][1] if category=='keyed-overlap' or keyed and i['mode']!=1 else None)
   draws.append(x);expected.append((category,row))
 if combined:
  borrowed=module('borrowed','tools/check-surface-borrowed-draw.py');_,bd=borrowed.load();borrowed.compare(bd)
  for row in bd['rows']:
   i=row['input'];fill=i['kind']==0;draws.append(dict(tag=f"borrowed:{i['id']}:0",kind='fill' if fill else 'copy',source_pixels=row['source_before'],destination_pixels=row['destination_before'],source=[0,0,8,6],destination=[0,0,8,6],clip=i['clip'],held=i['held'],shared=False,continues=False,keep=False,aux_source=True,fast=i['kind']>=3,flags=row['flags'],key=0x8000 if i['kind'] in [2,4] else None,color=0xffff,null_rect=False));expected.append(('borrowed',row))
 inp=dict(defaults=rd['default_dc_palette'],draws=draws,reuse=[dict(r['input'],initial_palette=bool(r['input']['initial_palette'])) for r in rd['rows']],access=[r['input'] for r in ad['rows']]);return inp,expected,rd,ad,original

def compare(actual,expected,rd,ad):
 require(len(actual['draws'])==len(expected),'Draw extent');counts=dict(draws=0,hresult_checks=0,pixel_checks=0,reuse_cases=0,reuse_phases=0,palette_entries=0,access_cases=0,access_steps=0,access_descriptor_fields=0,unstable_caps_excluded=0)
 for observed,(kind,row) in zip(actual['draws'],expected,strict=True):
  i=row['input'];require(observed['tag']==f"{kind}:{i['id']}:{i.get('phase',0)}",'Draw identity');require(observed['hresult']==row['hresult'],'Owned HRESULT differs');require(observed['destination_after']==row['destination_after'],'Owned destination differs');counts['pixel_checks']+=48
  if kind!='fills':require(observed['source_after']==row['source_after'],'Owned source differs');counts['pixel_checks']+=48
  if kind=='borrowed' and i['kind']==0 and i['held'] in [2,5]:require(observed['locked_after']==row['destination_after'],'Borrowed fill CPU snapshot differs')
  counts['draws']+=1;counts['hresult_checks']+=1
 require(len(actual['reuse'])==len(rd['rows']),'Reuse extent')
 for observed,row in zip(actual['reuse'],rd['rows'],strict=True):
  require(len(observed)==3,'Lease extent');bits=row['input']['bits']
  for got,want in zip(observed,row['phases'],strict=True):
   for name in ['table_before','table_draw']:require(got[name]==want[name],'Owned palette snapshot differs');counts['palette_entries']+=256 if bits==8 else 0
   require(got['pixels']==want['pixels'],'Owned DC native pixels differ');require(got['rgb']==[reuse.p.dc.colorref(v) for v in want['dc_rgb']],'Owned DC display differs');counts['pixel_checks']+=96;counts['reuse_phases']+=1
  counts['reuse_cases']+=1
 require(len(actual['access'])==len(ad['rows']),'Access extent')
 for observed,row in zip(actual['access'],ad['rows'],strict=True):
  require(len(observed['steps'])==len(row['steps']),'Access step extent')
  for op,got,want in zip(row['input']['operations'],observed['steps'],row['steps'],strict=True):
   require((got['result'],got['dc_out_state'])==(want['result'],want['dc_out_state']),'Owned access result/output differs')
   if op==0:
    n=26 if got['result'] else 27;require(got['descriptor'][:n]==want['descriptor'][:n],'Owned lock descriptor differs');counts['access_descriptor_fields']+=n;counts['unstable_caps_excluded']+=int(n==26)
   counts['access_steps']+=1
  got,want=observed['final_lock'],row['final_lock'];require(got['result']==want['result'],'Owned terminal lock differs');n=26 if got['result'] else 27;require(got['descriptor'][:n]==want['descriptor'][:n],'Owned terminal descriptor differs');counts['access_descriptor_fields']+=n;counts['unstable_caps_excluded']+=int(n==26);require(observed['pixels']==row['pixels'],'Owned terminal pixels differ');counts['pixel_checks']+=len(row['pixels'] or []);counts['access_cases']+=1
 require(actual['owner_resets']==12 and actual['guards']>=39,'Native guards/poisoned owner cleanup missing');return counts

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build',type=Path,default=ROOT/'working/build/surface-backend');p.add_argument('--report',type=Path);p.add_argument('--executable',type=Path);p.add_argument('--combined',action='store_true');args=p.parse_args()
 if args.report and args.report.exists():p.error('Refusing retained evidence overwrite')
 inp,expected,rd,ad,original=inputs(args.combined);names=['renderer/surface_backend.hpp','renderer/surface_backend.cpp','renderer/surface_access.hpp','renderer/bitmap_dc.hpp','renderer/blit.hpp','renderer/blit.cpp','renderer/surface_copy.hpp','renderer/surface_copy.cpp','renderer/known_pixels.hpp','renderer/dib.hpp','renderer/dib.cpp','renderer/CMakeLists.txt','reconstruction/rendering/surface_bitmap.hpp','tests/render-surface-backend-test.cpp','tools/check-native-surface-backend.py','tools/check-original-surface-fills.py','tools/check-original-surface-keys.py','tools/check-original-surface-overlap.py','tools/check-native-surface-clipping.py','tools/check-surface-dc-reuse.py','tools/check-surface-access.py','tools/capture-surface-access.py','tools/check-surface-dc-palette.py','tools/check-surface-dib-ddraw.py','tools/check-surface-dib-gdi.py','tests/fixtures/surfaces/original-sentinel-corpus.json','tests/fixtures/surfaces/original-sentinel.json.gz']
 for name in ['original-rgb565-fills','original-rgb565-keyed','original-rgb565-overlap']:names+=['tests/fixtures/surfaces/'+name+'-corpus.json','tests/fixtures/surfaces/'+name+'.json.gz']
 names+=['tests/fixtures/surfaces/driver-clipping-surface2-corpus.json','tests/fixtures/surfaces/driver-clipping-surface2-original.json.gz','tests/fixtures/surfaces/surface-dc-reuse.json.gz','research/runtime/surface-dc-reuse-capture-20261006.json','tests/fixtures/surfaces/surface-access.json.gz','research/runtime/surface-access-capture-20261006.json'];
 if args.combined:names+=['tools/check-original-surface-keyed-overlap.py','tools/capture-original-surface-keyed-overlap.py','tests/test-original-surface-keyed-overlap.py','tests/fixtures/surfaces/original-rgb565-keyed-overlap-corpus.json','tests/fixtures/surfaces/original-rgb565-keyed-overlap.json.gz','research/runtime/original-surface-keyed-overlap-capture-20261006.json','research/runtime/original-surface-keyed-overlap-offline-20261006.json','tools/check-surface-borrowed-draw.py','tools/capture-surface-borrowed-draw.py','tests/test-surface-borrowed-draw.py','tests/fixtures/surfaces/surface-borrowed-draw.json.gz','research/runtime/surface-borrowed-draw-capture-20261006.json','research/runtime/surface-borrowed-draw-offline-20261006.json']
 sources={n:sha(ROOT/n) for n in names};parent=ROOT/'working/tests/native-surface-backend';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));args.report=args.report or run/'report.json';assets=run/'assets';assets.mkdir()
 for name,raw in original['assets'].items():
  if raw is not None:(assets/(name+'.bmp')).write_bytes(base64.b64decode(raw))
 manifest=run/'inputs.json';manifest.write_text(json.dumps(inp));binary=args.executable.resolve() if args.executable else run/'native';library=args.build.resolve()/'libmnm-renderer.a';qt=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Gui','Qt6OpenGL'],text=True));
 if not args.executable:subprocess.run(['g++','-std=c++17','-O2','-fPIC','-Wall','-Wextra','-Werror','-pedantic','-I'+str(ROOT/'renderer'),str(ROOT/'tests/render-surface-backend-test.cpp'),str(library),*qt,'-o',str(binary)],check=True)
 output=run/'outputs.json';proc=subprocess.run([str(binary),str(manifest),str(assets),str(output)],capture_output=True,text=True,env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1'),timeout=240);(run/'native.log').write_text(proc.stdout+proc.stderr);require(proc.returncode==0,proc.stdout+proc.stderr);native=json.loads(output.read_text());counts=compare(native,expected,rd,ad);require(all(sha(ROOT/n)==h for n,h in sources.items()),'Sources changed during comparison');report=dict(schema=1,success=True,combined=args.combined,original_sha256=keys.BUILD_HASH,sources=sources,**counts,native_guards=native['guards'],poisoned_owner_resets=native['owner_resets'],vendor=native['vendor'],renderer=native['renderer'],inputs_sha256=sha(manifest),outputs_sha256=sha(output),binary_sha256=sha(binary),library_sha256=sha(library),artifacts=str(run.relative_to(ROOT)),scope='Unified owned SurfaceBackend compares retained original250 fill/436 keyed/1536 opaque overlap and408 standalone clipping results,192three-lease DC raster cases and96access sequences through actual persistent GL storage. All fill/keyed failure HRESULTs included; continued key/DC phases keep the same IDs.84unstable failure caps excluded;12poisoned owner states tear down explicitly. Combined mode additionally compares2304new original keyed-overlap calls and180borrowed driver draws, including forward same-ID reads, mapped fills and empty-list borrowed admission; keyed overlap is capped4096ordered texels with zero ordinary uploads/readbacks. Uncaptured busy/error/geometry combinations remain pending; no shared COM palette/alias lifetime, actual loss/Restore, new formats, wire or live replacement.')
 with args.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
 print(json.dumps(counts))
if __name__=='__main__':main()
