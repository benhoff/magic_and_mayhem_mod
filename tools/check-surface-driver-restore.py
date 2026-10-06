#!/usr/bin/env python3
"""Offline verification of bounded Wine driver observations, not game equivalence."""
import argparse, hashlib, json, struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
DEFAULT=ROOT/'tests/fixtures/surfaces/surface-driver-restore-corpus.json'
LOST=0x887601c2
WRONGMODE=0x8876024b

def require(ok,message):
 if not ok:raise ValueError(message)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def validate(r,bpp):
 require(r['schema']==1 and r['success'] is True and r['environment']['display_bpp']==bpp and r['environment']['override']=='ddraw=b','Unsupported capture')
 require(r['surface_caps']==[0x840,0x4040,0x40,0x200],'Unsupported caps')
 rows=r['rows'];require(0<len(rows)<5000 and all(len(x)==6 and all(type(v)==int and 0<=v<=0xffffffff for v in x) for x in rows),'Malformed rows')
 raw=b''.join(struct.pack('<6I',*x) for x in rows);require(hashlib.sha256(raw).hexdigest()==r['outputs_sha256'],'Output fingerprint')
 pos=0;checks=0
 def take(kind,phase,id):
  nonlocal pos,checks
  require(pos<len(rows) and rows[pos][:3]==[kind,phase,id],f'Incomplete/reordered trace at {pos}: expected {kind,phase,id}')
  row=rows[pos];pos+=1;checks+=1;return row
 def result(kind,phase,id,h):
  row=take(kind,phase,id);require(row[3]==h,f'HRESULT mismatch for {kind,phase,id}');return row
 def mode(phase,w,h):
  row=result(10,phase,0,0);require(row[4:]==[w,h],'Mode dimensions');row=result(18,phase,0,0);require(row[4:]==[bpp,0xf800 if bpp==16 else 0xff0000],'Reported display format');row=take(19,phase,0);require(row[4:]==[w,h],'Reported display dimensions');activate(phase)
 def activate(phase):
  row=take(14,phase,0);require(row[3:5]==[1,1],'Foreground ownership missing')
 def fill(phase,id,color):
  row=result(8,phase,id,0);require(row[4:]==[color,0],'Fill input')
 def lost(phase,id):return LOST if id and (phase in (1,3) or (id==3 and phase==2)) else 0
 def inspect(phase,id):
  h=lost(phase,id);result(2,phase,id,h);row=result(3,phase,id,0);require(row[4:]==[0x5678,0x5678],'Key retention')
  row=result(4,phase,id,h)
  if h:return
  fmt=16 if id<3 else bpp;require(row[4]==fmt and row[5]>=8*(fmt//8),'Lock format/pitch')
  row=result(15,phase,id,fmt);require(row[4:]==([8,6] if id<3 else [640,480]),'Surface dimensions')
  row=take(16,phase,id);require(row[3:]==([0xf800,0x7e0,31] if fmt==16 else [0xff0000,0xff00,255]),'RGB masks')
  values=[]
  for i in range(48):
   row=result(5,phase,id,0);require(row[4]==i and row[5]<(1<<fmt),'Pixel index/range');values.append(row[5])
  # Only fully defined initial/final fills establish expected bytes. Bytes after
  # Restore remain observations, even when this Wine allocation retains them.
  if phase in (0,6):require([v&(0xffff if fmt==16 else 0xffffff) for v in values]==[0x1234 if phase==0 else 0x9abc]*48,'Defined fill pixels')
  hash=2166136261
  for v in values:hash=((hash^v)*16777619)&0xffffffff
  row=result(6,phase,id,hash);require(row[4:]==([0xf800,0x7e0] if fmt==16 else [0xff0000,0xff00]),'Hash masks');result(7,phase,id,0)
 activate(8);row=result(9,0,0,0);require(row[4:]==[0x11,0],'Cooperative flags');mode(0,640,480)
 for id,caps in enumerate(r['surface_caps']):
  row=result(1,0,id,0);require(row[4:]==[caps,0],'Create caps');result(12,0,id,0);row=result(11,0,id,0);require(row[4:]==[0x5678,0x5678],'Set key');fill(0,id,0x1234);inspect(0,id)
 mode(1,800,600)
 for id in range(4):inspect(1,id);fill(1,id,0xabcd)
 for id in range(4):result(12,2,id,WRONGMODE if id==3 else 0);inspect(2,id)
 mode(3,640,480)
 for id in range(4):inspect(3,id)
 for id in range(4):
  for phase in (4,5):result(12,phase,id,0);inspect(phase,id)
  fill(6,id,0x9abc);inspect(6,id)
 result(13,7,0,0);row=result(9,7,0,0);require(row[4:]==[8,0],'Normal cleanup');result(17,7,0,0);require(pos==len(rows),'Trailing trace rows')
 return checks

def check(path=DEFAULT):
 m=json.loads(path.read_text());require(m['schema']==1 and m['oracle']=='wine_builtin_surface2_api' and len(m['captures'])==2,'Unsupported corpus')
 sources={str(path.relative_to(ROOT)):sha(path)};checks=0;seen=set()
 for c in m['captures']:
  require(c['bpp'] in (16,32) and c['bpp'] not in seen,'Duplicate capture');seen.add(c['bpp']);p=ROOT/c['path'];require(p.resolve().is_relative_to(ROOT) and sha(p)==c['sha256'],'Capture path/hash')
  r=json.loads(p.read_text());checks+=validate(r,c['bpp']);sources[c['path']]=sha(p)
  for n,h in r['sources'].items():require((ROOT/n).resolve().is_relative_to(ROOT) and sha(ROOT/n)==h,'Capture source freshness: '+n);sources[n]=h
 for n in ['tools/check-surface-driver-restore.py','tests/test-surface-driver-restore.py']:sources[n]=sha(ROOT/n)
 return dict(schema=1,success=True,sources=sources,captures=len(seen),checked_rows=checks,original_wrapper_executed=False,native_renderer_executed=False,scope='Offline full trace/provenance verification of two real Wine API captures. Captured HRESULT/key states and explicitly filled bytes checked; unknown post-Restore bytes only have internal hash checks. No game-wrapper, Windows-driver, palette or live replacement equivalence.')
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 if a.report.exists():p.error('Refusing overwrite')
 r=check();a.report.write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(dict(success=True,captures=r['captures'],checked_rows=r['checked_rows'])))
