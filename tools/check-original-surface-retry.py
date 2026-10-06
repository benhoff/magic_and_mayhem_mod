#!/usr/bin/env python3
"""Offline input-derived recovery ordering versus captured original fault traces."""
import argparse,gzip,hashlib,io,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
BUILD_HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
DEFAULT=ROOT/'tests/fixtures/surfaces/original-retry-corpus.json'
LOST=0x887601c2

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def require(ok,message):
    if not ok:raise ValueError(message)
def load(path=DEFAULT):
    m=json.loads(path.read_text());require(m['schema']==1 and m['original_sha256']==BUILD_HASH and m['oracle']=='unchanged_original_wrapper_scripted_com_faults','Unsupported provenance')
    require(m['path']=='original-retry.json.gz','Escaping corpus path');file=path.parent/m['path'];require(file.resolve().is_relative_to(path.parent.resolve()) and sha(file)==m['sha256'],'Corpus path/hash mismatch')
    with gzip.GzipFile(fileobj=io.BytesIO(file.read_bytes())) as f:raw=f.read(16*1024*1024+1)
    require(len(raw)<=16*1024*1024 and hashlib.sha256(raw).hexdigest()==m['raw_sha256'],'Raw corpus hash/limit mismatch');rows=json.loads(raw);require(len(rows)==m['cases'] and rows,'Case count');seen=set()
    for r in rows:
        c=r['input'];o=r['original'];require(c['id'] not in seen and c['id']==o['id'],'Duplicate identity');seen.add(c['id'])
        require(c['entry'] in [0x58bac0,0x58bc10,0x58c4a0,0x58c8a0] and c['fast'] in (0,1) and c['no_wait'] in (0,1) and c['key_enabled'] in (0,1) and c['reload_bits'] in range(4),'Unsupported case')
        require(0<len(c['draw_results'])<=16 and all(type(x)==int and 0<=x<=0xffffffff for x in c['draw_results']),'Invalid fault schedule')
        require(0<len(o['events'])<=128 and all(len(e)==6 and all(type(x)==int and 0<=x<=0xffffffff for x in e) for e in o['events']),'Malformed events')
    return rows,m

def model(c):
    events=[];at=0;entry=c['entry'];fill=entry in (0x58bac0,0x58bc10)
    def report(h):
        if h and h!=LOST:events.extend([[6,0,0,0,0,0],[7,0,0,0,0,0]])
    def draw():
        nonlocal at
        require(at<len(c['draw_results']),'Fault schedule exhausted');h=c['draw_results'][at];at+=1
        flags=0x1000400 if fill else ((1 if entry==0x58c8a0 else 0) if c['fast'] else (0x8000 if entry==0x58c8a0 else 0))|(0 if c['no_wait'] else 0x10 if c['fast'] else 0x1000000)
        events.append([2 if c['fast'] and not fill else 1,2,h,flags,0 if fill else 1,(c['color']&65535) if fill else 0]);return h
    def restore(id):
        h=c['source_restore' if id==1 else 'destination_restore'];events.append([3,id,h,0,0,0])
        if not h:
            if c['key_enabled']:events.append([4,id,c['key_result'],8,0x5678 if id==1 else 0xe123,0x5678 if id==1 else 0xe123])
            if c['reload_bits']&(1 if id==1 else 2):events.append([5,id,0,0,0,0])
        return h
    if fill:
        h=draw();report(h)
        if h==LOST:
            result=restore(2)
            if entry==0x58bc10:report(result);report(draw())
    else:
        while True:
            h=draw()
            if not h:break
            report(h)
            if h==LOST:
                for id in [1,2]:restore(id)
    return dict(id=c['id'],draws_consumed=at,events=events)

def check(path=DEFAULT):
    rows,m=load(path)
    for r in rows:require(model(r['input'])==r['original'],f"Original scheduling mismatch in {r['input']['id']}")
    report=dict(schema=1,success=True,original_sha256=BUILD_HASH,cases=len(rows),matched=len(rows),events=sum(len(r['original']['events']) for r in rows),real_driver_validation=False,scope='Offline input-derived wrapper control-flow model versus unchanged original/scripted COM fault traces. Native storage validity, real driver loss/Restore/palette and asset reload remain separate pending boundaries.')
    names=['tools/check-original-surface-retry.py','tests/test-original-surface-retry.py',str(path.relative_to(ROOT)),str((path.parent/m['path']).relative_to(ROOT))];report['sources']={n:sha(ROOT/n) for n in names};return report
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing overwrite')
    r=check();a.report.write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(dict(success=True,cases=r['cases'])))
