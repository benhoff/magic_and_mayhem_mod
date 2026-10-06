#!/usr/bin/env python3
"""Replay argument-derived overlap against independent original wrapper outputs offline."""
import argparse
import gzip
import hashlib
import importlib.util
import io
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('key_check',ROOT/'tools/check-original-surface-keys.py');key_check=importlib.util.module_from_spec(spec);spec.loader.exec_module(key_check)
sha,words,pixels,require=key_check.sha,key_check.words,key_check.pixels,key_check.require
BUILD_HASH=key_check.BUILD_HASH
DEFAULT=ROOT/'tests/fixtures/surfaces/original-rgb565-overlap-corpus.json'
def signed(v):return v if v<0x80000000 else v-0x100000000

def load(path):
    m=json.loads(path.read_text());require(m.get('schema')==1 and m.get('original_sha256')==BUILD_HASH and m.get('entry')==0x58c360 and m.get('oracle')=='unchanged_original_overlap_wrapper_real_surface2_post_call_lock','Missing original overlap provenance')
    name=m['path'];require(name=='original-rgb565-overlap.json.gz','Escaping or unsupported capture path');file=path.parent/name
    require(file.resolve().is_relative_to(path.parent.resolve()) and sha(file)==m['sha256'],'Capture path/hash mismatch')
    with gzip.GzipFile(fileobj=io.BytesIO(file.read_bytes())) as f:raw=f.read(8*1024*1024+1)
    require(len(raw)<=8*1024*1024 and hashlib.sha256(raw).hexdigest()==m['raw_sha256'],'Raw capture size/hash mismatch')
    rows=json.loads(raw);require(rows and len(rows)==m['cases'],'Case count mismatch');seen=set()
    for r in rows:
        c=r['input'];require(c['id'] not in seen and c['entry']==0x58c360 and c['mode'] in (0,1) and c['phase'] in range(6) and c['fast'] in (0,1) and c['no_wait'] in (0,1),'Unsupported overlap case');seen.add(c['id'])
        for name in ('source_desc','destination_desc'):
            d=r[name];require(len(d)==27 and d[0]==108 and d[2:4]==[6,8] and d[9]==0 and d[19]==0x40 and d[21:25]==[16,0xf800,0x7e0,31],'Unsupported descriptor')
        for name in ('source_before','source_after','destination_before','destination_after'):
            v=r[name];require(len(v)==48 and all(type(x)==int and 0<=x<=65535 for x in v),'Incomplete native pixels')
        require(r['source_before']==r['destination_before'] and r['source_after']==r['destination_after'],'Shared storage pixels disagree')
        require(r['identity']==[1-c['mode'],1,1 if c['mode'] else 2,2],'Shared COM identity mismatch')
        require(r['key_before']==r['key_after']==[0x887600d7,0,0],'Opaque key state mismatch')
        t=r['trace'];require(len(t)==16 and t[:3]==[1,1+c['fast'],0 if c['no_wait'] else 0x10 if c['fast'] else 0x1000000] and t[3]==r['hresult'] and t[12:]==[0,0,0,1],'Original API/flags/result mismatch')
        require([signed(x) for x in t[4:8]]==c['source'] and [signed(x) for x in t[8:10]]==c['destination'][:2],'Forwarded rectangle mismatch')
        if not c['fast']:require([signed(x) for x in t[8:12]]==c['destination'],'Forwarded destination mismatch')
        clip=r['clip_before'];require(len(clip)==18 and clip==r['clip_after'],'Clip state changed')
        phase=c['phase']
        if phase==0:require(clip==[0]*18,'Detached clip mismatch')
        elif phase==3:require(clip[1]==0x887600cd,'Missing clip list mismatch')
        else:
            count={1:1,2:2,4:0,5:2}[phase];require(clip[1]==0 and clip[2:6]==[32,1,count,count*16] and clip[0]==32+count*16,'Explicit clip descriptor mismatch')
            require(clip[10:10+count*4]=={1:[2,1,6,5],2:[0,0,3,2,5,3,8,6],4:[],5:[0,0,8,3,0,4,8,6]}[phase],'Explicit clip region mismatch')
    return rows,m

def plan(r):
    c=r['input'];sl,st,sr,sb=c['source'];dl,dt,dr,db=c['destination'];w,h=sr-sl,sb-st
    inside=lambda l,t,x,y:0<=l<x<=8 and 0<=t<y<=6
    if c['fast']:
        if w<0 or h<0 or dl<0 or dt<0 or dl+w>8 or dt+h>6:return 0x88760096,[]
        if c['phase']:return 0x8876023e,[]
        if not inside(sl,st,sr,sb):return 0x88760096,[]
    else:
        if w<=0 or h<=0 or dl>=dr or dt>=db:return 0x88760096,[]
        require(w==dr-dl and h==db-dt,'Stretch outside scope')
        if c['phase']==3:return 0x887600cd,[]
        if not c['phase'] and (not inside(sl,st,sr,sb) or not inside(dl,dt,dr,db)):return 0x88760096,[]
    regions={0:[[0,0,8,6]],1:[[2,1,6,5]],2:[[0,0,3,2],[5,3,8,6]],4:[],5:[[0,0,8,3],[0,4,8,6]]}[c['phase']];pieces=[]
    for a,b,x,y in regions:
        l,t,right,bottom=max(dl,a),max(dt,b),min(dr,x),min(db,y)
        if l>=right or t>=bottom:continue
        src=[sl+l-dl,st+t-dt,sl+right-dl,st+bottom-dt]
        if not inside(*src):return 0x88760096,pieces
        pieces.append((src,l,t))
    return 0,pieces

def cpu(r):
    result,pieces=plan(r);out=r['destination_before'].copy()
    for (l,t,right,bottom),dx,dy in pieces:
        frozen=out.copy()
        for y in range(t,bottom):
            for x in range(l,right):out[(dy+y-t)*8+dx+x-l]=frozen[y*8+x]
    return result,out

def check(path=DEFAULT,build=None):
    rows,m=load(path)
    for r in rows:
        h,out=cpu(r);require(h==r['hresult'] and out==r['destination_after'],f"Overlap model disagrees for {r['input']['id']} {r['input']['label']}: {h:#x} vs {r['hresult']:#x}")
    report=dict(schema=1,success=True,original_sha256=BUILD_HASH,cases=len(rows),cpu_hresult_checks=len(rows),cpu_shared_pixel_checks=len(rows),alias_cases=sum(r['input']['mode'] for r in rows),partial_error_cases=sum(bool(r['hresult']) and r['destination_before']!=r['destination_after'] for r in rows),native_hresult_validation=False,backend='cpu',scope='Opaque RGB565 self-copy through unchanged original wrapper0x58c360 and Wine Surface2; per-clip-piece source snapshot model, same-pointer and Surface1 aliases. No keyed/masked/other formats, original Restore/retry, Windows-driver or live replacement equivalence.')
    paths=['tools/check-original-surface-overlap.py','tools/check-original-surface-keys.py','tests/test-original-surface-overlap.py',str(path.resolve().relative_to(ROOT)),str((path.parent/m['path']).resolve().relative_to(ROOT))]
    report['sources']={n:sha(ROOT/n) for n in paths};return report

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--corpus',type=Path,default=DEFAULT);p.add_argument('--build',type=Path);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing to overwrite evidence')
    try:
        r=check(a.corpus,a.build);a.report.parent.mkdir(parents=True,exist_ok=True)
        with a.report.open('x') as f:json.dump(r,f,indent=2);f.write('\n')
        print(json.dumps({k:r[k] for k in ['success','cases','alias_cases','partial_error_cases']}))
    except (ValueError,KeyError,TypeError,OSError,EOFError,subprocess.SubprocessError) as e:p.exit(2,str(e)+'\n')
if __name__=='__main__':main()
