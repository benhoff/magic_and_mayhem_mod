#!/usr/bin/env python3
"""Offline native-word/raster and DC admission checks against retained Surface2 output."""
import argparse,base64,gzip,importlib.util,json,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
DEFAULT=ROOT/'research/runtime/surface-dib-ddraw-capture-20261006.json'
spec=importlib.util.spec_from_file_location('gdi',ROOT/'tools/check-surface-dib-gdi.py');gdi=importlib.util.module_from_spec(spec);spec.loader.exec_module(gdi)
require=gdi.require;sha=gdi.sha

def load(report=DEFAULT):
 capture=json.loads(report.read_text());require(capture['schema']==1 and capture['success'] is True,'Invalid capture report');p=capture['corpus']['path'];require(p=='tests/fixtures/surfaces/surface-dib-ddraw.json.gz','Unsafe fixture path');corpus=ROOT/p;require(sha(corpus)==capture['corpus']['sha256'],'DirectDraw fixture differs')
 with gzip.open(corpus,'rb') as f:raw=f.read(32*1024*1024+1)
 require(len(raw)<=32*1024*1024,'Expanded fixture limit');payload=json.loads(raw);require(payload['schema']==1 and len(payload['rows'])==capture['cases']==72,'DirectDraw case count differs');return capture,payload

def pack565(rgb):return ((rgb>>19)&31)<<11|((rgb>>10)&63)<<5|((rgb>>3)&31)
def expand565(word):return (((word>>11)*255//31)<<16)|((((word>>5)&63)*255//63)<<8)|((word&31)*255//31)
def colorref(rgb):return ((rgb&255)<<16)|(rgb&0xff00)|((rgb>>16)&255)
def replicated565(word):
 r=(word>>11)&31;g=(word>>5)&63;b=word&31
 return (((r<<3)|(r>>2))<<16)|(((g<<2)|(g>>4))<<8)|((b<<3)|(b>>2))

def compare(payload,assets,check_admission=True):
 counts=dict(cases=0,native_pixels=0,dc_samples=0,presentation_policy_differences=0);states=[]
 for id,row in enumerate(payload['rows']):
  c,o=row['input'],row['observation'];require(c['id']==o['id']==id,'Case identity differs');require(c['bits'] in [16,32] and c['caps'] in [0x840,0x40],'Unknown target request');w,h,rgb=gdi.source_image(base64.b64decode(assets[c['asset']],validate=True));require((c['source_width'],c['source_height'])==(w,h),'Source dimensions differ');sizes={'equal':(w,h),'cropped':(max(1,w-1),max(1,h-1)),'larger':(w+1,h+1)};require((c['width'],c['height'])==sizes[c['shape']],'Target dimensions differ');tw,th=c['width'],c['height'];bits=c['bits'];require((o['width'],o['height'],o['bits'])==(tw,th,bits),'Native descriptor differs');require((o['red_mask'],o['green_mask'],o['blue_mask'])==((0xf800,0x7e0,0x1f) if bits==16 else (0xff0000,0xff00,0xff)),'Native masks differ');require(o['pitch']>=tw*(bits//8),'Native pitch differs')
  for field in ['create','initial_lock','initial_unlock','final_lock','final_unlock']:require(o[field]==0,'Surface initialization/read failed')
  initial=0x2bab if bits==16 else 0x556677
  expected=[(pack565(rgb[y*w+x]) if bits==16 else rgb[y*w+x]) if x<w and y<h and o['get_dc']==0 else initial for y in range(th) for x in range(tw)]
  actual=[p&0xffffff if bits==32 else p for p in row['pixels']];require(actual==expected,f'DirectDraw native pixels differ: case{id}')
  if o['get_dc']==0:
   require(o['dc_out_state']==2 and 0<o['dib_result']<=h and o['flush']!=0 and o['release_dc']==0,'DC raster pipeline failed')
   require(o['bitmap_result']==24 and (o['bitmap_width'],o['bitmap_height'],o['bitmap_bits'])==(tw,th,bits),'Selected DC bitmap differs')
   points=[(0,0),(tw//2,th//2),(tw-1,th-1)]
   for field,(x,y) in zip(['dc_first_rgb','dc_middle_rgb','dc_last_rgb'],points,strict=True):
    word=expected[y*tw+x];display=replicated565(word) if bits==16 else word;require(o[field]==colorref(display),f'DC GetPixel sample differs: case{id}')
    counts['dc_samples']+=1
    if bits==16 and display!=expand565(word):counts['presentation_policy_differences']+=1
  # These exact outcomes are observation claims of this Wine build, not an API
  # implementation or a generic Windows guarantee. Updated after capture review.
  if check_admission:
   for field,value in ADMISSION.items():require(o[field]==value,f'DC admission differs: {field} case{id}')
  counts['cases']+=1;counts['native_pixels']+=len(expected);states.append(dict(input=c,expected=expected))
 return counts,states

ADMISSION=dict(get_dc_while_locked=0,locked_out_state=2,get_dc=0,dc_out_state=2,repeated_get_dc=0x8876026c,repeated_out_state=0,lock_while_dc=0x887601ae,release_dc=0,repeated_release_dc=0x8876024a)
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path);a=p.parse_args();capture,payload=load();original,_=gdi.sentinel.load();gdi.sentinel.compare(original);counts,_=compare(payload,original['assets']);names=['tools/check-surface-dib-ddraw.py','tests/test-surface-dib-ddraw.py','tools/check-surface-dib-gdi.py',str(DEFAULT.relative_to(ROOT)),capture['corpus']['path'],'tests/fixtures/surfaces/original-sentinel-corpus.json','tests/fixtures/surfaces/original-sentinel.json.gz'];report=dict(schema=1,success=True,sources={n:sha(ROOT/n) for n in names},**counts,admission=ADMISSION,scope='Offline input-derived RGB565/RGB32 native words compare72 real Surface2 GetDC/GDI/ReleaseDC outcomes and216 DC GetPixel samples. Exact driver admission/out-handle observations are build-scoped; native floor-expanded RGB565 presentation is separately classified from DC bit-replicated RGB. No original image, actual loss, indexed/DC clipping or live equivalence.')
 if a.report:
  with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
 print(json.dumps(counts))
if __name__=='__main__':main()
