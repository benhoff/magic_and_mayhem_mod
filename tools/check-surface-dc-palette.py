#!/usr/bin/env python3
"""Offline input-derived indexed palette and independent DC/DirectDraw clipping checks."""
import argparse,base64,gzip,importlib.util,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
DEFAULT=ROOT/'research/runtime/surface-dc-palette-capture-20261006.json'
spec=importlib.util.spec_from_file_location('dc',ROOT/'tools/check-surface-dib-ddraw.py');dc=importlib.util.module_from_spec(spec);spec.loader.exec_module(dc)
require=dc.require;sha=dc.sha
PAIRS=[(0,0),(1,0),(2,0),(3,0),(4,0),(0,1),(0,2),(0,3),(0,4),(2,1)]
REGIONS={0:None,1:[(2,1,6,5)],2:[(0,0,3,2),(5,3,8,6)],3:[],4:[(2,1,6,5),(4,0,8,3)]}
def palette(style):
 if style==1:return [i*0x010101 for i in range(256)]
 if style==2:return [(255-i)*0x010101 for i in range(256)]
 if style==3:return [((((i>>5)&7)*255//7)<<16)|((((i>>2)&7)*255//7)<<8)|((i&3)*85) if i!=255 else 0 for i in range(256)]
 raise ValueError('Absent palette has no input-derived color table')
def nearest(rgb,colors):
 r,g,b=(rgb>>16)&255,(rgb>>8)&255,rgb&255
 return min(range(256),key=lambda i:(r-((colors[i]>>16)&255))**2+(g-((colors[i]>>8)&255))**2+(b-(colors[i]&255))**2)
def inside(x,y,regions):return regions is None or any(l<=x<r and t<=y<b for l,t,r,b in regions)
def load(report=DEFAULT):
 c=json.loads(report.read_text());require(c['schema']==1 and c['success'] is True,'Invalid DC palette capture');p=c['corpus']['path'];require(p=='tests/fixtures/surfaces/surface-dc-palette.json.gz','Unsafe corpus path');require(sha(ROOT/p)==c['corpus']['sha256'],'DC palette fixture differs')
 with gzip.open(ROOT/p,'rb') as f:raw=f.read(16*1024*1024+1)
 require(len(raw)<=16*1024*1024,'Expanded fixture limit');data=json.loads(raw);require(data['schema']==1 and len(data['rows'])==c['cases']==360,'DC palette case count');return c,data

def compare(payload,assets):
 counts=dict(cases=0,native_pixels=0,dc_rgb_pixels=0,clipped_samples=0,absent_palette_cases=0);states=[]
 for id,row in enumerate(payload['rows']):
  c,o=row['input'],row['observation'];bits,style=c['bits'],c['palette_style'];require(c['id']==o['id']==id,'Case identity differs');require(bits in [8,16,32] and (c['ddclip'],c['gdiclip']) in PAIRS and style in ([0,1,2,3] if bits==8 else [0]),'Unknown palette/clip inputs');require((c['width'],c['height'],o['width'],o['height'],o['bits'])==(8,6,8,6,bits),'Native descriptor differs')
  for field in ['create','initial_lock','initial_unlock','get_dc','release_dc','final_lock','final_unlock']:require(o[field]==0,f'Pipeline failed:{field}')
  require(o['actual_caps']==0x840 and o['dc_out_state']==2 and o['flush']==1,'Caps/DC/flush differs');require((o['red_mask'],o['green_mask'],o['blue_mask'])==({8:(0,0,0),16:(0xf800,0x7e0,0x1f),32:(0xff0000,0xff00,0xff)}[bits]),'Native masks differ');require(o['pitch']>=8*(bits//8),'Native pitch differs')
  if bits==8 and style:require(o['create_palette']==o['bind_palette']==0,'Palette setup differs')
  if c['ddclip']:
   require(o['create_clipper']==o['attach_clipper']==0,'Clipper setup differs')
   if c['ddclip']!=3:require(o['set_clip_list']==0,'Clip list failed')
  require((o['clip_box_result'],o['clip_left'],o['clip_top'],o['clip_right'],o['clip_bottom'])==(2,0,0,8,6),'Attached DirectDraw clipper changed DC clip box')
  if c['gdiclip']:require(o['select_clip']==({1:2,2:3,3:1,4:3}[c['gdiclip']]),'GDI clip complexity differs')
  require(o['clear_clip']==2,'Cleared clip complexity differs')
  sw,sh,rgb=dc.gdi.source_image(base64.b64decode(assets[c['asset']],validate=True));require((sw,sh)==(c['source_width'],c['source_height']),'Source size differs')
  # No-list, empty and region-bearing DirectDraw clippers do not constrain this
  # measured GetDC raster; explicit application GDI regions do.
  regions=REGIONS[c['gdiclip']];colors=palette(style) if bits==8 and style else None
  if bits==8:
   require(o['table_count']==256,'Indexed DC table length differs')
   if colors is not None:require([v&0xffffff for v in row['color_table']]==colors,'DC color table differs from installed input')
   else:counts['absent_palette_cases']+=1
  else:require(o['table_count']==0 and row['color_table']==[0]*256,'RGB target unexpectedly has palette')
  initial={8:19,16:0x2bab,32:0x556677}[bits]
  write=[x<sw and y<sh and inside(x,y,regions) for y in range(6) for x in range(8)]
  if bits==8 and colors is None:
   require(len(row['pixels'])==48 and all(0<=v<256 for v in row['pixels']),'Absent palette native extent')
   require(all(v==initial for v,yes in zip(row['pixels'],write,strict=True) if not yes),'Absent palette untouched border differs')
   states.append(dict(input=c,expected=None,write=write));counts['cases']+=1;continue
  expected=[];write=[]
  for y in range(6):
   for x in range(8):
    yes=x<sw and y<sh and inside(x,y,regions);write.append(yes);p=rgb[y*sw+x] if yes else None
    expected.append(initial if p is None else dc.pack565(p) if bits==16 else p if bits==32 else nearest(((p&0xf8f8f8)+0x040404) if c['usage']==1 else p,colors))
  actual=[p&0xffffff if bits==32 else p for p in row['pixels']];require(actual==expected,f'Native clipped palette raster differs: case{id}')
  display=lambda word:dc.replicated565(word) if bits==16 else colors[word] if bits==8 else word
  before=[dc.colorref(display(initial))]*48;clear=[dc.colorref(display(p)) for p in expected];clipped=list(clear) # Wine GetPixel ignores the application clip in these cases.
  require(row['dc_before']==before,f'Initial DC colors differ: case{id}');require(row['dc_rgb']==clear,f'Cleared DC colors differ: case{id}');require(row['dc_clipped']==clipped,f'Clipped DC colors differ: case{id}')
  require(o['dib_result']==sh,'StretchDIBits return differs')
  counts['cases']+=1;counts['native_pixels']+=48;counts['dc_rgb_pixels']+=48;counts['clipped_samples']+=48;states.append(dict(input=c,expected=expected,write=write))
 return counts,states

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path);a=p.parse_args();cap,data=load();original,_=dc.gdi.sentinel.load();dc.gdi.sentinel.compare(original);counts,_=compare(data,original['assets']);names=['tools/check-surface-dc-palette.py','tests/test-surface-dc-palette.py','tools/check-surface-dib-ddraw.py','tools/check-surface-dib-gdi.py',str(DEFAULT.relative_to(ROOT)),cap['corpus']['path'],'tests/fixtures/surfaces/original-sentinel-corpus.json','tests/fixtures/surfaces/original-sentinel.json.gz'];out=dict(schema=1,success=True,sources={n:sha(ROOT/n) for n in names},**counts,scope='Input-derived installed palette/explicit GDI clip conversion against independent Surface2 outputs. Absent-palette conversion remains observation-only, no expected conversion or RGB equivalence claimed. Attached DirectDraw clipper admission/box scoped separately from GDI regions. No original instructions, actual loss, live/wire or Windows hardware equivalence.')
 if a.report:
  with a.report.open('x') as f:json.dump(out,f,indent=2);f.write('\n')
 print(json.dumps(counts))
if __name__=='__main__':main()
