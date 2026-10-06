#!/usr/bin/env python3
"""Offline three-lease DC state and default-context raster comparison."""
import argparse,base64,gzip,importlib.util,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];DEFAULT=ROOT/'research/runtime/surface-dc-reuse-capture-20261006.json'
spec=importlib.util.spec_from_file_location('p',ROOT/'tools/check-surface-dc-palette.py');p=importlib.util.module_from_spec(spec);spec.loader.exec_module(p)
require=p.require;sha=p.sha

def load(report=DEFAULT):
 c=json.loads(report.read_text());require(c['schema']==1 and c['success'] is True,'Invalid reuse capture');path=c['corpus']['path'];require(path=='tests/fixtures/surfaces/surface-dc-reuse.json.gz','Unsafe reuse fixture path');require(sha(ROOT/path)==c['corpus']['sha256'],'Reuse fixture differs')
 with gzip.open(ROOT/path,'rb') as f:raw=f.read(16*1024*1024+1)
 require(len(raw)<=16*1024*1024,'Expanded reuse limit');data=json.loads(raw);require(data['schema']==1 and len(data['rows'])==c['cases']==192,'Reuse case count');require(len(data['default_dc_palette'])==256 and all(0<=v<=0xffffff for v in data['default_dc_palette']),'Default context table invalid');return c,data

def changed_colors():return [((255-i)<<16)|(((i*5)&255)<<8)|((i*7)&255) for i in range(64,96)]
def compare(data,assets):
 counts=dict(cases=0,phases=0,native_pixels=0,dc_rgb_pixels=0,palette_entries=0,default_palette_phases=0);states=[]
 expected_cases=[(name,bits,initial,action,clip) for name in ['indexed1','indexed4','indexed8','rgb24','offset-gap','installed-cursors'] for bits in [8,16,32] for initial in ([0,1] if bits==8 else [0]) for action in (range(7) if bits==8 else [0]) for clip in [0,1]]
 require([(r['input']['asset'],r['input']['bits'],r['input']['initial_palette'],r['input']['action'],r['input']['clip']) for r in data['rows']]==expected_cases,'Reuse case matrix differs')
 for id,row in enumerate(data['rows']):
  c=row['input'];bits=c['bits'];require(c['id']==id and (c['width'],c['height'])==(8,6) and len(row['phases'])==3,'Reuse input identity differs');sw,sh,rgb=p.dc.gdi.source_image(base64.b64decode(assets[c['asset']],validate=True));require((sw,sh)==(c['source_width'],c['source_height']),'Source size differs');binding=p.palette(1) if c['initial_palette'] else None;pixels=[{8:19,16:0x2bab,32:0x556677}[bits]]*48;case_states=[]
  for phase,frame in enumerate(row['phases']):
   o=frame['observation'];require((o['id'],o['phase'],o['bits'])==(id,phase,bits),'Phase identity differs')
   for field in ['get_dc','release_dc','lock','unlock']:require(o[field]==0,'Lease pipeline failed:'+field)
   require(o['dc_out_state']==2 and o['same_dc']==(1 if phase==0 else 0) and o['flush']==1 and o['dib_result']==sh,'DC output/identity/flush/return differs');require((o['clip_box_result'],o['clip_left'],o['clip_top'],o['clip_right'],o['clip_bottom'])==(2,0,0,8,6),'Clip persisted into fresh lease');require(o['pitch']>=8*(bits//8) and (o['red_mask'],o['green_mask'],o['blue_mask'])=={8:(0,0,0),16:(0xf800,0x7e0,0x1f),32:(0xff0000,0xff00,0xff)}[bits],'Native descriptor differs')
   action=c['action'];mutates=phase==1 and action in [1,2,3] or phase==0 and action in [4,5,6];require(o['mutation_result']==(32 if action==4 else 0) if mutates else o['mutation_result']==0xffffffff,'Mutation result differs')
   if phase==1:
    if action==1 and binding is not None:binding[64:96]=p.palette(2)[64:96]
    if action==2:binding=p.palette(3)
    if action==3:binding=None
   colors=list(binding if binding is not None else data['default_dc_palette']) if bits==8 else [0]*256
   require(frame['table_before']==colors,'Acquired table differs from input binding/default context');before=list(colors)
   if phase==0:
    if action==4:colors[64:96]=changed_colors()
    if action==5:binding=p.palette(3)
    if action==6 and binding is not None:binding[64:96]=p.palette(2)[64:96]
   require(frame['table_draw']==colors,'In-lease snapshot/table edit differs');require(o['table_before_count']==o['table_draw_count']==(256 if bits==8 else 0),'Palette count differs');require(o['select_clip']==2 if phase==2 or phase==0 and c['clip'] else o['select_clip']==0xffffffff,'Clip selection differs')
   regions=[(2,1,6,5)] if phase==0 and c['clip'] else None
   for y in range(6):
    for x in range(8):
     if x<sw and y<sh and p.inside(x,y,regions):
      color=rgb[y*sw+x];pixels[y*8+x]=p.dc.pack565(color) if bits==16 else color if bits==32 else p.nearest((color&0xf8f8f8)+0x040404 if c['usage']==1 else color,colors)
   actual=[v&0xffffff if bits==32 else v for v in frame['pixels']];require(actual==pixels,f'Reused native raster differs: case{id} phase{phase}')
   display=[p.dc.replicated565(v) if bits==16 else v if bits==32 else colors[v] for v in pixels];require(frame['dc_rgb']==[p.dc.colorref(v) for v in display],f'Reused DC RGB differs: case{id} phase{phase}')
   counts['phases']+=1;counts['native_pixels']+=48;counts['dc_rgb_pixels']+=48;counts['palette_entries']+=512 if bits==8 else 0;counts['default_palette_phases']+=int(bits==8 and before==data['default_dc_palette']);case_states.append(dict(before=before,draw=colors,pixels=list(pixels),rgb=display,regions=regions))
  states.append(case_states);counts['cases']+=1
 return counts,states

def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',type=Path);a=parser.parse_args();cap,data=load();original,_=p.dc.gdi.sentinel.load();p.dc.gdi.sentinel.compare(original);counts,_=compare(data,original['assets']);names=['tools/check-surface-dc-reuse.py','tests/test-surface-dc-reuse.py','tools/check-surface-dc-palette.py','tools/check-surface-dib-ddraw.py','tools/check-surface-dib-gdi.py',str(DEFAULT.relative_to(ROOT)),cap['corpus']['path'],'tests/fixtures/surfaces/original-sentinel-corpus.json','tests/fixtures/surfaces/original-sentinel.json.gz'];out=dict(schema=1,success=True,sources={n:sha(ROOT/n) for n in names},**counts,scope='192 standalone Surface2 cases/576 leases compare input-derived binding/update/DC edit/clip-reset behavior and default context from separate preflight read. Complete native/DC output frames independent of model. Default context is environment input, not portable palette formula. No original game/loss/COM implementation/Windows/live equivalence.')
 if a.report:
  with a.report.open('x') as f:json.dump(out,f,indent=2);f.write('\n')
 print(json.dumps(counts))
if __name__=='__main__':main()
