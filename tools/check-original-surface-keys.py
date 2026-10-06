#!/usr/bin/env python3
"""Offline CPU/OpenGL comparison with independently captured original keyed pixels."""
import argparse
import gzip
import hashlib
import io
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
BUILD_HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
DEFAULT=ROOT/'tests/fixtures/surfaces/original-rgb565-keyed-corpus.json'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def words(*v):return struct.pack('<'+'I'*len(v),*(x&0xffffffff for x in v))
def pixels(v):return struct.pack('<'+'H'*len(v),*v)
def require(ok,reason):
    if not ok:raise ValueError(reason)
def load(path):
    m=json.loads(path.read_text());require(m.get('schema')==1 and m.get('original_sha256')==BUILD_HASH and m.get('entry')==0x58ca90 and m.get('oracle')=='unchanged_original_wrapper_real_surface2_post_call_lock','Missing original wrapper provenance')
    name=m['path'];require(isinstance(name,str) and Path(name).name==name and name=='original-rgb565-keyed.json.gz','Escaping or unsupported capture path')
    file=path.parent/name;require(file.resolve().is_relative_to(path.parent.resolve()),'Escaping capture symlink');require(sha(file)==m['sha256'],'Compressed capture hash mismatch')
    with gzip.GzipFile(fileobj=io.BytesIO(file.read_bytes())) as f:raw=f.read(8*1024*1024+1)
    require(len(raw)<=8*1024*1024 and hashlib.sha256(raw).hexdigest()==m['raw_sha256'],'Raw capture size/hash mismatch')
    rows=json.loads(raw);require(len(rows)==m['cases'] and rows,'Case count mismatch');seen=set()
    for r in rows:
        c=r['input'];identity=(c['id'],c['phase']);require(identity not in seen,'Duplicate case');seen.add(identity)
        require(c['entry']==0x58ca90 and c['fast'] in (0,1) and c['no_wait'] in (0,1) and c['mode'] in (0,1,2) and c['phase'] in (0,1),'Unsupported original case')
        require(c['phase']==0 or c['mode']==2,'Invalid mutation phase')
        for name in ('source_before','source_after','destination_before','destination_after'):
            v=r[name];require(len(v)==48 and all(type(x)==int and 0<=x<=65535 for x in v),'Invalid complete pixels')
        for name in ('source_desc','destination_desc'):
            d=r[name];require(len(d)==27 and d[0]==108 and d[2:4]==[6,8] and d[9]==0 and d[19]==0x40 and d[21:25]==[16,0xf800,0x7e0,31],'Unsupported descriptor')
        t=r['trace'];require(len(t)==16 and t[0]==1 and t[1]==1+c['fast'] and t[2]==((1 if c['fast'] else 0x8000)|(0 if c['no_wait'] else 0x10 if c['fast'] else 0x1000000)) and t[3]==r['hresult'] and t[14:]==[0,1],'Incorrect forwarded operation')
        require(t[4:8]==c['source'] and t[8:10]==c['destination'][:2],'Forwarded rectangle mismatch')
        if not c['fast']:require(t[8:12]==c['destination'],'Blt rectangle mismatch')
        sr,dr=c['source'],c['destination'];require(0<=sr[0]<sr[2]<=8 and 0<=sr[1]<sr[3]<=6 and 0<=dr[0]<dr[2]<=8 and 0<=dr[1]<dr[3]<=6 and sr[2]-sr[0]==dr[2]-dr[0] and sr[3]-sr[1]==dr[3]-dr[1],'Out-of-scope rectangles')
        k=r['key_before'];require(len(k)==3 and r['key_after']==k,'Key changed during draw')
        if c['mode']!=1:require(k==[0,c['key']^(0xffff if c['phase'] else 0),c['key']^(0xffff if c['phase'] else 0)] and r['hresult']==0,'Exact-key state/result mismatch')
        else:require(k[0]!=0,'Missing key unexpectedly present')
    # Mutation second calls share complete state with the immediately prior output.
    for i,r in enumerate(rows):
        if r['input']['phase']:
            require(i>0 and rows[i-1]['input']['id']==r['input']['id'] and rows[i-1]['input']['phase']==0,'Missing mutation predecessor')
            require(r['source_before']==rows[i-1]['source_after'] and r['destination_before']==rows[i-1]['destination_after'],'Mutation state continuity mismatch')
    return rows,m

def cpu(r):
    c=r['input'];out=r['destination_before'].copy()
    if r['hresult']:return out
    key=None if c['mode']==1 else r['key_before'][1]
    l,t,right,bottom=c['source'];dx,dy=c['destination'][:2]
    for y in range(t,bottom):
        for x in range(l,right):
            v=r['source_before'][y*8+x]
            if v!=key:out[(dy+y-t)*8+dx+x-l]=v
    return out

def stream(rows):
    records=[]
    def add(op,payload=b''):records.append(words(op,len(records)+1,len(payload))+payload)
    admitted=[r for r in rows if not r['hresult']]
    for i,r in enumerate(admitted):
        s,d=2*i+1,2*i+2;c=r['input'];keyed=c['mode']!=1
        add(1,words(s,8,6,16,0xf800,0x7e0,31)+pixels(r['source_before']));add(1,words(d,8,6,16,0xf800,0x7e0,31)+pixels(r['destination_before']))
        add(3,words(s,d,*c['source'],*c['destination'][:2],int(keyed),r['key_before'][1] if keyed else 0))
        add(5,words(s)+pixels(r['source_after']));add(5,words(d)+pixels(r['destination_after']))
        if i==len(admitted)-1:add(6,words(d))
        add(7,words(s));add(7,words(d))
    add(8);return b'MNMCMD01'+words(1,16)+b''.join(records)

def check(path=DEFAULT,build=None):
    rows,m=load(path)
    require(all(r['source_before']==r['source_after'] and cpu(r)==r['destination_after'] for r in rows),'Independent CPU pixels disagree')
    report=dict(schema=1,success=True,original_sha256=BUILD_HASH,cases=len(rows),cpu_destination_checks=len(rows),source_checks=len(rows),native_success_cases=sum(not r['hresult'] for r in rows),native_hresult_validation=False,pending_native_failure_cases=sum(bool(r['hresult']) for r in rows),mutation_second_calls=sum(r['input']['phase'] for r in rows),hresults={f'0x{h:08x}':sum(r['hresult']==h for r in rows) for h in sorted({r['hresult'] for r in rows})},scope='Offline RGB565 pixel comparison with unchanged original keyed wrapper0x58ca90 through Wine Surface2; native COPY primitive compares successful pixels only. Failed HRESULTs retained, no native keyed API HRESULT/retry/Restore equivalence. Missing-key successful BltFast is opaque driver behavior.')
    paths=['tools/check-original-surface-keys.py','tests/test-original-surface-keys.py','renderer/blit.cpp','renderer/blit.hpp','renderer/commands.cpp','renderer/commands.hpp','renderer/command_consumer.cpp','renderer/command_state.hpp','renderer/commands_main.cpp']
    if build:
        parent=ROOT/'working/tests/original-surface-key-replay';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));file=run/'keys.cmd';file.write_bytes(stream(rows));output=run/'native.pixels'
        env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1');binary=build.resolve()/'mnm-render-commands';result=subprocess.run([str(binary),str(file),'--output',str(output)],env=env,capture_output=True,text=True,timeout=60);(run/'replay.log').write_text(result.stdout+result.stderr)
        require(result.returncode==0,result.stdout+result.stderr);gl=json.loads(result.stdout);require(gl['checks']==2*report['native_success_cases'] and gl['rendered'],'Missing native pixel checks')
        require(output.read_bytes()==pixels(next(r for r in reversed(rows) if not r['hresult'])['destination_after']),'Native final pixels disagree')
        report.update(backend='opengl_native_integer',native=gl,binary_sha256=sha(binary),stream_sha256=sha(file),artifacts=str(run.relative_to(ROOT)))
    else:report['backend']='cpu'
    paths += [str(path.resolve().relative_to(ROOT)),str((path.parent/m['path']).resolve().relative_to(ROOT))]
    report['sources']={n:sha(ROOT/n) for n in paths};return report

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--corpus',type=Path,default=DEFAULT);p.add_argument('--build',type=Path);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing to overwrite evidence')
    try:
        r=check(a.corpus,a.build);a.report.parent.mkdir(parents=True,exist_ok=True)
        with a.report.open('x') as f:json.dump(r,f,indent=2);f.write('\n')
        print(json.dumps({k:r[k] for k in ['success','cases','native_success_cases','pending_native_failure_cases']}))
    except (ValueError,KeyError,TypeError,OSError,EOFError,subprocess.SubprocessError) as e:p.exit(2,str(e)+'\n')
if __name__=='__main__':main()
