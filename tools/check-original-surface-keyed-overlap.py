#!/usr/bin/env python3
"""Offline keyed overlap comparison with unchanged original wrapper/driver outputs."""
import argparse,gzip,hashlib,importlib.util,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def module(name,path):
 s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
opaque=module('opaque','tools/check-original-surface-overlap.py');capture=module('capture','tools/capture-original-surface-keyed-overlap.py');require=opaque.require;sha=opaque.sha
DEFAULT=ROOT/'tests/fixtures/surfaces/original-rgb565-keyed-overlap-corpus.json'
def load():
 m=json.loads(DEFAULT.read_text());require(m['schema']==1 and m['entry']==0x58ca90 and m['original_sha256']==opaque.BUILD_HASH and m['oracle']=='unchanged_original_keyed_overlap_wrapper_real_surface2_post_call_lock','Missing keyed overlap provenance');require(m['path']=='original-rgb565-keyed-overlap.json.gz','Escaping keyed fixture path');p=DEFAULT.parent/m['path'];require(sha(p)==m['sha256'],'Keyed overlap compressed hash')
 with gzip.open(p,'rb') as f:raw=f.read(8*1024*1024+1)
 require(len(raw)<=8*1024*1024 and hashlib.sha256(raw).hexdigest()==m['raw_sha256'],'Keyed overlap raw size/hash');rows=json.loads(raw);require(len(rows)==m['cases']==2304 and [r['input'] for r in rows]==capture.cases(),'Keyed overlap complete matrix')
 for r in rows:
  c=r['input'];require(r['identity']==[1-c['mode'],1,1 if c['mode'] else 2,2],'Keyed COM identity differs');require(r['key_before']==r['key_after']==[0,c['key'],c['key']],'Keyed binding differs');require(r['clip_before']==r['clip_after'],'Keyed clip changed');t=r['trace'];require(len(t)==16 and t[:4]==[1,1+c['fast'],(1 if c['fast'] else 0x8000)|(0 if c['no_wait'] else 0x10 if c['fast'] else 0x1000000),r['hresult']] and t[4:8]==c['source'] and [opaque.signed(v) for v in t[8:10]]==c['destination'][:2] and t[14:]==[0,1],'Original forwarded keyed operation differs');require(r['source_before']==r['destination_before'] and r['source_after']==r['destination_after'],'Shared native storage differs')
  for name in ['source_before','destination_before','source_after','destination_after']:require(len(r[name])==48 and all(type(v)==int and 0<=v<=65535 for v in r[name]),'Incomplete keyed pixels')
  for name in ['source_desc','destination_desc']:
   d=r[name];require(len(d)==27 and d[0]==108 and d[2:4]==[6,8] and d[9]==0 and d[21:25]==[16,0xf800,0x7e0,31],'Keyed format differs')
  clip=c['phase'];n={0:0,1:1,2:2,3:0,4:0,5:2}[clip];state=r['clip_before'];require(len(state)==18,'Keyed clip extent')
  if clip==0:require(state==[0]*18,'Detached keyed clip')
  elif clip==3:require(state[1]==0x887600cd,'Missing keyed clip list')
  else:require(state[:6]==[32+n*16,0,32,1,n,n*16] and state[10:10+n*4]=={1:[2,1,6,5],2:[0,0,3,2,5,3,8,6],4:[],5:[0,0,8,3,0,4,8,6]}[clip],'Explicit keyed clip differs')
 return rows,m

def cpu(row):
 h,pieces=opaque.plan(row);out=list(row['destination_before']);key=row['input']['key']
 for (l,t,r,b),dx,dy in pieces:
  for y in range(t,b):
   for x in range(l,r):
    value=out[y*8+x]
    if value!=key:out[(dy+y-t)*8+dx+x-l]=value
 return h,out

def compare(rows):
 counts=dict(cases=0,pixels=0,partial_error_cases=0)
 require([r['input'] for r in rows]==capture.cases(),'Keyed matrix differs')
 for r in rows:
  h,pixels=cpu(r);require(h==r['hresult'] and pixels==r['source_after']==r['destination_after'],f"Keyed overlap model differs:{r['input']['id']} {r['input']['label']}");counts['cases']+=1;counts['pixels']+=48;counts['partial_error_cases']+=int(bool(h) and pixels!=r['destination_before'])
 return counts

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path);args=p.parse_args();rows,m=load();counts=compare(rows);names=['tools/check-original-surface-keyed-overlap.py','tools/check-original-surface-overlap.py','tools/check-original-surface-keys.py','tools/capture-original-surface-keyed-overlap.py','tests/test-original-surface-keyed-overlap.py',str(DEFAULT.relative_to(ROOT)),'tests/fixtures/surfaces/'+m['path']];out=dict(schema=1,success=True,original_sha256=opaque.BUILD_HASH,sources={n:sha(ROOT/n) for n in names},**counts,scope='2304unchanged original0x58ca90 keyed same-backing RGB565 calls match input-derived ordered forward per-pixel mutation and discard equality, including self-read of preceding writes; same/Surface1 alias,3keys,mixed/all-key,8geometries,6clip states,WAITpairs. No other formats/masks/original retry/loss/Windows/live equivalence.')
 if args.report:
  with args.report.open('x') as f:json.dump(out,f,indent=2);f.write('\n')
 print(json.dumps(counts))
if __name__=='__main__':main()
