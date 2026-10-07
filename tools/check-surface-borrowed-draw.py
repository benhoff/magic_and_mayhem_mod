#!/usr/bin/env python3
"""Offline input-derived Surface2 fill/copy admission for Lock/DC operands."""
import argparse,gzip,hashlib,importlib.util,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];DEFAULT=ROOT/'research/runtime/surface-borrowed-draw-capture-20261006.json'
s=importlib.util.spec_from_file_location('capture',ROOT/'tools/capture-surface-borrowed-draw.py');capture=importlib.util.module_from_spec(s);s.loader.exec_module(capture)
def require(ok,reason):
 if not ok:raise ValueError(reason)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def load():
 cap=json.loads(DEFAULT.read_text());require(cap['schema']==1 and cap['success'] is True,'Borrowed capture status');require(cap['corpus']['path']=='tests/fixtures/surfaces/surface-borrowed-draw.json.gz','Unsafe borrowed corpus');path=ROOT/cap['corpus']['path'];require(sha(path)==cap['corpus']['sha256'],'Borrowed corpus hash')
 with gzip.open(path,'rb') as f:raw=f.read(2*1024*1024+1)
 require(len(raw)<=2*1024*1024,'Expanded borrowed limit');d=json.loads(raw);require(d['schema']==1 and len(d['rows'])==180,'Borrowed extent');return cap,d

def expected(row):
 c=row['input'];kind,held,clip=c['kind'],c['held'],c['clip'];s=[0x8000 if i%3==0 else (0x1234+i*73)&0xffff for i in range(48)];out=[0xa000+i for i in range(48)]
 if kind>=3 and clip:return 0x8876023e,s,out
 if clip==3:return 0x887600cd,s,out
 if kind and held and clip!=4:return 0x887601ae,s,out
 regions={0:[[0,0,8,6]],1:[[2,1,6,5]],2:[[0,0,3,2],[5,3,8,6]],4:[],5:[[0,0,8,3],[0,4,8,6]]}[clip]
 for l,t,r,b in regions:
  for y in range(t,b):
   for x in range(l,r):
    i=y*8+x
    if kind==0:out[i]=0xffff
    elif kind in [1,3] or s[i]!=0x8000:out[i]=s[i]
 return 0,s,out

def compare(data):
 require([r['input'] for r in data['rows']]==capture.inputs(),'Borrowed complete matrix');counts=dict(cases=0,pixels=0,borrowed_fill_successes=0,borrowed_empty_blt_successes=0)
 for row in data['rows']:
  c=row['input'];require(row['flags']==[0x400,0,0x8000,0,1][c['kind']],'Borrowed flags differ');h,s,d=expected(row);require(row['source_before']==s and row['destination_before']==[0xa000+i for i in range(48)],'Borrowed initial bytes differ');require(row['hresult']==h and row['source_after']==s and row['destination_after']==d,'Borrowed results/pixels differ');counts['cases']+=1;counts['pixels']+=96;counts['borrowed_fill_successes']+=int(c['kind']==0 and c['held']!=0 and h==0);counts['borrowed_empty_blt_successes']+=int(c['kind'] in [1,2] and c['held']!=0 and c['clip']==4 and h==0)
 return counts

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path);args=p.parse_args();cap,data=load();counts=compare(data);names=['tools/check-surface-borrowed-draw.py','tools/capture-surface-borrowed-draw.py','tests/test-surface-borrowed-draw.py',str(DEFAULT.relative_to(ROOT)),cap['corpus']['path']];report=dict(schema=1,success=True,sources={n:sha(ROOT/n) for n in names},**counts,scope='180standalone Surface2 RGB565 operation inputs match independent results/full frames with6clip states and no/source-destination Lock/DC/destinationLock+DC. Fill bypasses mapping busy; empty Blt clips skip borrowed copies, while BltFast clip refusal precedes busy. No new game execution, other geometry/error precedence/format/real loss/Windows/live claim.')
 if args.report:
  with args.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
 print(json.dumps(counts))
if __name__=='__main__':main()
