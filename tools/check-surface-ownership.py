#!/usr/bin/env python3
"""Offline Wine ownership and flipping outputs; COM counts remain diagnostic."""
import argparse,gzip,hashlib,json,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
DEFAULT=ROOT/'research/runtime/surface-ownership-final-capture-20261006.json'
def require(ok,message):
 if not ok:raise ValueError(message)
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def color(i,id,generation):return ((i*17+id*37+generation*11)&255)|(((i*29+3+generation*19)&255)<<8)|(((i*43+9+generation*23)&255)<<16)
def load(path=DEFAULT):
 report=json.loads(path.read_text());require(report['schema']==1 and report['success'] and report['original_game_executed'] is False and report['original_manifest_verified_before_after'] is True,'Capture scope')
 c=report['corpus'];p=ROOT/c['path'];require(p.resolve().is_relative_to(ROOT) and sha(p)==c['sha256'],'Corpus hash/path');raw=gzip.decompress(p.read_bytes());require(len(raw)==c['raw_bytes'] and hashlib.sha256(raw).hexdigest()==c['raw_sha256']==report['outputs_sha256'] and len(raw)<1024*1024 and len(raw)%24==0,'Raw bytes/hash');rows=[list(v) for v in struct.iter_unpack('<6I',raw)];require(len(rows)==report['row_count'],'Row count');return report,rows
def validate(report,rows):
 at=0;frames={};tables={};flips={};cleanups={};detached=None;counts=[];palette=[color(i,0,0) for i in range(256)];other=[color(i,1,0) for i in range(256)]
 def take(kind,phase,id,result=None):
  nonlocal at
  require(at<len(rows) and rows[at][:3]==[kind,phase,id],f'Missing/reordered operation{kind,phase,id} at{at}');r=rows[at];at+=1
  require(len(r)==6 and all(type(v)==int and 0<=v<=0xffffffff for v in r),'Malformed row')
  if result is not None:require(r[3]==result,f'Operation result{kind,phase,id}')
  return r
 def query(phase,id):require(take(30,phase,id,0)[4:]==[1,0],'QueryInterface output')
 def identity(phase,id):query(phase,id);query(phase,id);require(take(31,phase,id,0)[4:]==[1,0],'IUnknown identity')
 def drop(phase,id):counts.append(take(39,phase,id)[3])
 def sample(phase,id,value,key):
  r=take(32,phase,id,0);require(r[4]==8 and r[5]>=8,'Lock layout');p=[]
  for i in range(48):r=take(33,phase,id,0);require(r[4:]==[i,value],'Independent frame bytes');p.append(r[5])
  require(take(34,phase,id,0)[4:]==[key,key],'Object key metadata');frames[(phase,id)]=p
 def colors(phase,id,expected):
  r=take(35,phase,id,0 if expected is not None else 0x8876023c)
  require(r[4:]==([1,1] if expected is not None else [1,0]),'Palette identity/output')
  if expected is None:return r[3]
  require(take(36,phase,id,0)[4:]==[0,256],'Palette read range');p=[]
  for i in range(256):r=take(37,phase,id,0);require(r[4:]==[i,expected[i]],'Shared palette entries');p.append(r[5])
  tables[(phase,id)]=p
 def bind(phase,id,p):require(take(24,phase,id,0)[4:]==[p,0],'Binding input')
 def key(id,k):require(take(38,0,id,0)[4:]==[k,0],'Key input')
 def fill(id,c):require(take(8,0,id,0)[4:]==[c,0],'Fill input')
 require(take(18,0,0,0)[4:]==[8,0],'Display format');require(take(19,0,0)[3:]==[0,640,480],'Display dimensions');require(take(14,0,0)[3:5]==[1,1],'Exclusive foreground')
 for id in range(2):require(take(20,0,id,0)[4:]==[0x44,256],'Palette creation')
 for id in range(2):require(take(1,0,id,0)[4:]==[0x840,0],'Offscreen creation')
 bind(0,0,0);bind(0,1,0);key(0,0x35);key(1,0x46);fill(0,0x12);fill(1,0x23);query(0,0);identity(0,0);sample(0,0,0x12,0x35);drop(1,0);sample(1,0,0x12,0x35);require(take(38,1,0,0)[4:]==[0x57,0],'Alias key mutation');sample(2,0,0x12,0x57)
 drop(2,10);colors(2,0,palette);colors(2,1,palette);require(take(23,3,0,0)[4:]==[37,7],'Partial palette update');palette[37:44]=[color(i,0,1) for i in range(37,44)];colors(3,0,palette);colors(3,1,palette);sample(3,0,0x12,0x57);sample(3,1,0x23,0x46);require(take(41,4,0,0)[4:]==[0,0],'Detach');detached=colors(4,0,None);colors(4,1,palette);drop(4,10);drop(4,0);colors(5,1,palette);bind(5,1,1);colors(6,1,other);sample(6,1,0x23,0x46);drop(6,1)
 require(take(42,0,3,0)[4:]==[0x218,1],'Primary two-buffer creation');query(0,3);require(take(43,0,4,0)[4:]==[1,0],'Attached back buffer');query(0,3);query(0,4);identity(0,3);identity(0,4);bind(0,3,1);bind(0,4,1);key(3,0x61);key(4,0x72);fill(3,0x31);fill(4,0xa5);front,back=0x31,0xa5
 for phase in range(10,14):
  sample(phase,3,front,0x61);sample(phase,4,back,0x72);colors(phase,3,other);colors(phase,4,other);r=take(40,phase,3,0);require(r[4:]==[int(phase==12),1],'Flip target/flags');front,back=back,front;flips[phase]=r[3];sample(phase+10,3,front,0x61);sample(phase+10,4,back,0x72)
 for held in range(4):
  phase=30+held;id=4 if held%2 else 3;other_id=3 if held%2 else 4;require(take(44,phase,id,0)[4:]==[int(held>=2),0],'Held borrower input');r=take(40,phase,3,0x887601ae if held%2 else 0);require(r[4:]==[0,0],'Held flip flags');flips[phase]=r[3];front,back=back,front
  same=take(45,phase,id,0x88760248 if held<2 else 0x8876086c)[3];opposite=take(47,phase,other_id,0 if held<2 else 0x8876024a)[3];undo=returned=0
  if held>=2:
   r=take(48,phase,3,0 if held%2 else 0x887601ae);require(r[4:]==[0,1],'DC flipback flags');undo=r[3];flips[phase+100]=undo;front,back=back,front;returned=take(49,phase,id,0)[3]
  cleanups[phase]=[same,opposite,undo,returned];sample(phase,3,front,0x61);sample(phase,4,back,0x72)
 drop(40,4);drop(40,4);require(take(43,40,4,0)[4:]==[1,0],'Reacquire attachment');sample(40,4,back,0x72);drop(41,3);sample(41,3,front,0x61);require(take(40,41,3,0)[4:]==[0,1],'Surviving alias flip');flips[41]=0;front,back=back,front;sample(42,3,front,0x61);sample(42,4,back,0x72);drop(43,4);drop(43,3);drop(43,11);require(take(46,99,0,0)[4:]==[0,0] and at==len(rows),'Terminal/trailing rows')
 return dict(frames=frames,tables=tables,flips=flips,cleanups=cleanups,detached=detached,reference_results=counts)
def check(path=DEFAULT):
 r,rows=load(path);d=validate(r,rows);sources={str(path.relative_to(ROOT)):sha(path)}
 for n,h in r['sources'].items():require(sha(ROOT/n)==h,'Capture source stale: '+n);sources[n]=h
 for n in ['tools/check-surface-ownership.py','tests/test-surface-ownership.py']:sources[n]=sha(ROOT/n)
 return dict(schema=1,success=True,sources=sources,rows=len(rows),frames=len(d['frames']),pixels=len(d['frames'])*48,palette_entries=len(d['tables'])*256,flips=len(d['flips']),scope='Offline standalone Wine Surface2 ownership/flip observation and provenance. Independent pixel/table/results and normalized IUnknown identity, surviving aliases, caller-released bound palettes, detach/rebind and attachment reacquisition. COM reference-count return values retained diagnostically; native canonical counts are separate policy. No original wrapper/live/Windows comparison.')
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path,required=True);a=p.parse_args();r=check();a.report.parent.mkdir(parents=True,exist_ok=True)
 with a.report.open('x') as f:json.dump(r,f,indent=2);f.write('\n')
 print(json.dumps({k:r[k] for k in ['success','rows','frames','pixels','palette_entries','flips']}))
