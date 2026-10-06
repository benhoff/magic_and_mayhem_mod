#!/usr/bin/env python3
"""Compare retained real GDI raster outputs with input-derived pixels, offline."""
import argparse,base64,gzip,hashlib,importlib.util,json,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
DEFAULT=ROOT/'research/runtime/surface-dib-gdi-capture-20261006.json'
spec=importlib.util.spec_from_file_location('sentinel',ROOT/'tools/check-original-surface-sentinel.py');sentinel=importlib.util.module_from_spec(spec);spec.loader.exec_module(sentinel)
require=sentinel.require;sha=sentinel.sha

def source_image(asset):
 header=asset[14:54];w,h=struct.unpack_from('<II',header,4);bits=struct.unpack_from('<H',header,14)[0];require(0<w<=2048 and 0<h<=2048 and bits in [1,4,8,24],'Unsupported DIB dimensions/depth')
 palette_bytes={1:8,4:64,8:1024}.get(bits,0);palette=asset[54:54+palette_bytes];start=54+palette_bytes;length=struct.unpack_from('<I',asset,2)[0]-struct.unpack_from('<I',asset,10)[0];data=asset[start:start+length];stride=((w*bits+31)//32)*4;require(len(data)>=stride*h,'Short DIB input')
 rgb=[]
 for y in range(h):
  for x in range(w):
   row=(h-y-1)*stride
   if bits==24:q=row+x*3;b,g,r=data[q:q+3]
   else:
    index=data[row+x] if bits==8 else (data[row+x//2]>>(4 if x%2==0 else 0))&15 if bits==4 else (data[row+x//8]>>(7-x%8))&1
    b,g,r=palette[index*4:index*4+3]
   rgb.append((r<<16)|(g<<8)|b)
 return w,h,rgb

def load(report=DEFAULT):
 capture=json.loads(report.read_text());require(capture['schema']==1 and capture['success'] is True,'Invalid capture report');p=capture['corpus']['path'];require(p=='tests/fixtures/surfaces/surface-dib-gdi.json.gz','Unsafe fixture path');corpus=ROOT/p;require(sha(corpus)==capture['corpus']['sha256'],'GDI fixture differs')
 with gzip.open(corpus,'rb') as f:raw=f.read(16*1024*1024+1)
 require(len(raw)<=16*1024*1024,'Expanded GDI fixture limit');payload=json.loads(raw);require(payload['schema']==1 and len(payload['rows'])==capture['cases']==18,'GDI case count differs');return capture,payload

def compare(payload,assets):
 pixels=0;states=[]
 for id,row in enumerate(payload['rows']):
  c=row['input'];require(c['id']==id,'Case identity differs');w,h,rgb=source_image(base64.b64decode(assets[c['asset']],validate=True));require((c['source_width'],c['source_height'])==(w,h),'Source dimensions differ');require(c['usage']==(1 if c['asset'] in ['rgb24','installed-cursors'] else 0),'Original usage differs')
  sizes={'equal':(w,h),'cropped':(max(1,w-1),max(1,h-1)),'larger':(w+1,h+1)};require((c['width'],c['height'])==sizes[c['shape']],'Target dimensions differ')
  tw,th=c['width'],c['height'];expected=[rgb[y*w+x] if x<w and y<h else 0x556677 for y in range(th) for x in range(tw)]
  require(0<row['result']<=h,'GDI drawing failed or unsupported result');require(len(row['pixels'])==tw*th and [p&0xffffff for p in row['pixels']]==expected,f'GDI RGB pixels differ: case{id}');pixels+=len(expected);states.append(dict(input=c,expected=expected))
 return dict(cases=len(states),rgb_pixels=pixels),states

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path);a=p.parse_args();capture,payload=load();original,_=sentinel.load();sentinel.compare(original);counts,_=compare(payload,original['assets']);names=['tools/check-surface-dib-gdi.py','tests/test-surface-dib-gdi.py','tools/check-original-surface-sentinel.py',str(DEFAULT.relative_to(ROOT)),capture['corpus']['path'],'tests/fixtures/surfaces/original-sentinel-corpus.json','tests/fixtures/surfaces/original-sentinel.json.gz'];out=dict(schema=1,success=True,sources={n:sha(ROOT/n) for n in names},**counts,scope='Input-derived RGB model matches independent standalone Wine GDI StretchDIBits outputs from original submitted DIB inputs,18 equal/cropped/larger cases. High byte excluded; no DirectDraw DC/format, real loss or live equivalence.')
 if a.report:
  with a.report.open('x') as f:json.dump(out,f,indent=2);f.write('\n')
 print(json.dumps(counts))
if __name__=='__main__':main()
