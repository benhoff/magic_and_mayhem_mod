#!/usr/bin/env python3
"""Offline independent five-format draw outputs; rejected memory layouts stay rejected."""
import argparse,gzip,hashlib,importlib.util,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
DEFAULT=ROOT/'research/runtime/surface-formats-final-capture-20261006.json'
DEPTHS=[8,16,16,24,32]
MASKS=[[0,0,0],[0x7c00,0x3e0,31],[0xf800,0x7e0,31],[0xff0000,0xff00,255],[0xff0000,0xff00,255]]
def require(ok,message):
 if not ok:raise ValueError(message)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def key(f):return [0x81,0x9234,0x9234,0x901234,0x7f901234][f]
def active_mask(f):return 255 if f==0 else MASKS[f][0]|MASKS[f][1]|MASKS[f][2]
def pixels(f,destination=False):
 out=[];k=key(f);maximum=(1<<DEPTHS[f])-1
 for i in range(35):
  v=(0xa0b0c0d0+i*73)&maximum if destination else (k^(0x1234+i*73))&maximum
  if not destination:
   if i%4==0:v=k
   elif i%4==1:v=0x82 if f==0 else k^0x8000 if f==1 else k^0xff000000 if f==4 else k^1
  out.append(v)
 return out
def clips(mode):return [[0,0,7,5]] if mode==0 else [[1,1,6,4]] if mode==1 else [[0,0,3,2],[4,3,7,5]] if mode==2 else []
def packed(values,f,pitch):
 data=bytearray([0xcd]*256);size=DEPTHS[f]//8
 for i,v in enumerate(values):at=16+(i//7)*pitch+(i%7)*size;data[at:at+size]=v.to_bytes(size,'little')
 return data.hex()
def load(path=DEFAULT):
 r=json.loads(path.read_text());require(r['schema']==1 and r['success'] and not r['original_game_executed'] and r['original_manifest_verified_before_after'],'Scope/manifest')
 p=ROOT/r['corpus']['path'];require(p.resolve().is_relative_to(ROOT) and sha(p)==r['corpus']['sha256'],'Corpus path/hash')
 d=json.loads(gzip.decompress(p.read_bytes()));require(d['schema']==1 and len(d['rows'])==r['cases'],'Corpus schema/count');return r,d['rows']
def expected(c):
 f,kind,clip,mode=[c[k] for k in ['format','kind','clip','key_mode']];source=pixels(f);dest=pixels(f,kind<5);installed=None if mode==2 else key(f)^(1 if mode==1 else 0)
 if mode==3:installed=key(f)&active_mask(f)
 result=0
 if kind in (3,4) and clip:result=0x8876023e
 elif kind in (2,6) and installed is None:result=0x80070057
 elif clip==4:result=0x887600cd
 if result:return result,source,dest
 for region in clips(clip):
  left,top,right,bottom=region
  if kind>=5:left=max(left,1);top=max(top,1)
  snapshot=dest[:] if kind==5 else source
  for y in range(top,bottom):
   for x in range(left,right):
    at=y*7+x
    if kind==0:dest[at]=0xd3e2f197&active_mask(f);continue
    sx,sy=(x-1,y-1) if kind>=5 else (x,y)
    v=dest[sy*7+sx] if kind==6 else snapshot[sy*7+sx]
    if kind in (2,4,6) and installed is not None and v&active_mask(f)==installed:continue
    dest[at]=v
 return result,dest[:] if kind>=5 else source,dest
def validate(r,rows):
 count=0;failed=0;word_checks=0
 require(len(rows)==3500,'Complete matrix')
 for i,row in enumerate(rows):
  f,l,k,c,m=(i//700,(i//140)%5,(i//20)%7,(i//4)%5,i%4)
  case=dict(id=i,format=f,layout=l,kind=k,clip=c,key_mode=m);require(row['input']==case,'Input sequence')
  direct=0 if l==0 else 0x80070057
  setting=0xffffffff if l==0 else 0 if (l==1 and f!=3) or (l==3 and f==4) else 0x80070057
  creation=0 if l==0 or setting==0 else setting
  require(row['direct_create']==[direct]*2 and row['set_descriptor']==[setting]*2 and row['create']==[creation]*2,'Creation/binding results')
  if creation:
   require(row==dict(input=case,direct_create=[direct]*2,set_descriptor=[setting]*2,create=[creation]*2),'Invented failed outputs');failed+=1;continue
  require(row['create']==[0,0],'Allocated creation');count+=1
  descriptor=[0]*27;descriptor[0]=108;descriptor[1]=0x100f;descriptor[2]=5;descriptor[3]=7;descriptor[4]=(7*(DEPTHS[f]//8)+7)&~7;descriptor[4]=((7*(DEPTHS[f]//8)+3)&~3)+4 if l==1 else 28 if l==3 else descriptor[4];descriptor[18]=32;descriptor[19]=0x60 if f==0 else 0x40;descriptor[21]=DEPTHS[f];descriptor[22:25]=MASKS[f];descriptor[26]=0x840
  require(row['source_before']['pixels']==pixels(f) and row['destination_before']['pixels']==pixels(f,k<5),'Initialized words');word_checks+=70
  installed=None if m==2 else key(f)^(1 if m==1 else 0)
  if m==3:installed=key(f)&active_mask(f)
  for name in ['source_before','destination_before','source_after','destination_after']:
   expected_descriptor=descriptor[:]
   if installed is not None and (name=='source_after' or (name=='destination_after' and k>=5)):expected_descriptor[1]|=0x10000;expected_descriptor[16]=expected_descriptor[17]=installed
   require(row[name]['descriptor']==expected_descriptor and row[name]['pointer_matches']==1,'Descriptor/pointer identity');word_checks+=27
  require(row['key_results']==([0xffffffff,0xffffffff,0x887600d7] if m==2 else [0,0 if m==1 else 0xffffffff,0]),'Key mutation/get results')
  require(row['key_range']==([0xabababab]*2 if m==2 else [installed]*2),'Key output preservation')
  flags=0x400 if k==0 else 0x8000 if k in (2,6) else 1 if k==4 else 0;require(row['flags']==flags,'Flags')
  result,source,dest=expected(case);require(row['hresult']==result and row['source_after']['pixels']==source and row['destination_after']['pixels']==dest,'Independent draw mismatch: '+str(case));word_checks+=70
  if l:
   for side,values in [('source',source),('destination',dest)]:require(row[side+'_storage']==packed(values,f,descriptor[4]),'Independent storage guards/padding');word_checks+=128
 return dict(admitted_draws=count,rejected_layout_cases=failed,word_checks=word_checks)
def check():
 r,rows=load();counts=validate(r,rows);sources={str(DEFAULT.relative_to(ROOT)):sha(DEFAULT)}
 for n,h in r['sources'].items():require(sha(ROOT/n)==h,'Stale capture: '+n);sources[n]=h
 for n in ['tools/check-surface-formats.py','tests/test-surface-formats.py']:sources[n]=sha(ROOT/n)
 return dict(schema=1,success=True,sources=sources,**counts,scope='Independent standalone DirectDraw2/Surface2 words/results/descriptors only. Caller LPSURFACE creation rejected; Surface3 SetSurfaceDesc admits sampled positive padded indexed/16/32 rows and tight RGB32; RGB24/negative/other tight layouts rejected, not manufactured. No original game format reachability or negative-driver-pitch acceptance.')
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path,required=True);a=p.parse_args();r=check()
 with a.report.open('x') as f:json.dump(r,f,indent=2);f.write('\n')
 print(json.dumps({k:r[k] for k in ['success','admitted_draws','rejected_layout_cases','word_checks']}))
