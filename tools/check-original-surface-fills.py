#!/usr/bin/env python3
"""Replay argument-derived fills against independent original wrapper outputs offline."""
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
DEFAULT=ROOT/'tests/fixtures/surfaces/original-rgb565-fills-corpus.json'
def signed(v):return v if v<0x80000000 else v-0x100000000

def load(path):
    m=json.loads(path.read_text());require(m.get('schema')==1 and m.get('original_sha256')==BUILD_HASH and m.get('entry')==0x58bac0 and m.get('oracle')=='unchanged_original_fill_wrapper_real_surface2_post_call_lock','Missing original fill provenance')
    name=m['path'];require(name=='original-rgb565-fills.json.gz','Escaping or unsupported capture path');file=path.parent/name
    require(file.resolve().is_relative_to(path.parent.resolve()) and sha(file)==m['sha256'],'Capture path/hash mismatch')
    with gzip.GzipFile(fileobj=io.BytesIO(file.read_bytes())) as f:raw=f.read(8*1024*1024+1)
    require(len(raw)<=8*1024*1024 and hashlib.sha256(raw).hexdigest()==m['raw_sha256'],'Raw capture size/hash mismatch')
    rows=json.loads(raw);require(rows and len(rows)==m['cases'],'Case count mismatch');seen=set()
    for r in rows:
        c=r['input'];require(c['id'] not in seen and c['entry']==0x58bac0 and c['mode'] in range(5) and c['phase'] in (0,1) and 0<=c['key']<=0xffffffff,'Unsupported fill case');seen.add(c['id'])
        d=r['destination_desc'];require(len(d)==27 and d[0]==108 and d[2:4]==[6,8] and d[9]==0 and d[19]==0x40 and d[21:25]==[16,0xf800,0x7e0,31],'Unsupported destination descriptor')
        for name in ('destination_before','destination_after'):
            v=r[name];require(len(v)==48 and all(type(x)==int and 0<=x<=65535 for x in v),'Incomplete native pixels')
        t=r['trace'];require(len(t)==16 and t[:3]==[1,1,0x1000400] and t[3]==r['hresult'] and t[4:8]==[0,0,0,0] and t[12:]==[100,c['key']&0xffff,1,0],'Original fill effects/flags/result mismatch')
        require([signed(x) for x in t[8:12]]==([0]*4 if c['phase'] else c['destination']),'Original fill rectangle mismatch')
        clip=r['clip_before'];require(len(clip)==18 and clip==r['clip_after'],'Clip state changed during fill')
        if c['mode']==0:require(clip==[0]*18,'Detached clip state mismatch')
        elif c['mode']==3:require(clip[1]==0x887600cd,'Missing clip list mismatch')
        else:
            count={1:1,2:2,4:0}[c['mode']];require(clip[1]==0 and clip[2:6]==[32,1,count,count*16] and clip[0]==32+count*16,'Explicit clip descriptor mismatch')
            expected={1:[2,1,6,5],2:[0,0,3,2,5,3,8,6],4:[]}[c['mode']];require(clip[10:10+count*4]==expected,'Explicit clip regions mismatch')
        require(r['hresult']!=0x887601c2,'Lost/Restore is outside scope')
        require(r['dialogs']==r['destroy_calls']==int(bool(r['hresult'])),'Original error-report UI call count mismatch')
    return rows,m

def plan(r):
    c=r['input'];l,t,right,bottom=[0,0,8,6] if c['phase'] else c['destination']
    if l>=right or t>=bottom:return 0x88760096,[]
    if c['mode']==3:return 0x887600cd,[]
    if c['mode']==0:
        if l<0 or t<0 or right>8 or bottom>6:return 0x88760096,[]
        regions=[[0,0,8,6]]
    else:
        clip=r['clip_before'];regions=[clip[10+i*4:14+i*4] for i in range(clip[4])]
    pieces=[]
    for a,b,x,y in regions:
        rect=[max(l,a),max(t,b),min(right,x),min(bottom,y)]
        if rect[0]<rect[2] and rect[1]<rect[3]:pieces.append(rect)
    return 0,pieces

def cpu(r):
    h,pieces=plan(r);out=r['destination_before'].copy();color=r['input']['key']&0xffff
    for l,t,right,bottom in pieces:
        for y in range(t,bottom):
            for x in range(l,right):out[y*8+x]=color
    return h,out

def stream(rows):
    records=[]
    def add(op,payload=b''):records.append(words(op,len(records)+1,len(payload))+payload)
    admitted=[r for r in rows if plan(r)[0]==0]
    for i,r in enumerate(admitted):
        id=i+1;add(1,words(id,8,6,16,0xf800,0x7e0,31)+pixels(r['destination_before']))
        for l,t,right,bottom in plan(r)[1]:add(2,words(id,l,t,right-l,bottom-t)+pixels([r['input']['key']&0xffff]*((right-l)*(bottom-t))))
        add(5,words(id)+pixels(r['destination_after']))
        if i==len(admitted)-1:add(6,words(id))
        add(7,words(id))
    add(8);return b'MNMCMD01'+words(1,16)+b''.join(records)

def check(path=DEFAULT,build=None):
    rows,m=load(path)
    for r in rows:
        h,out=cpu(r);require(h==r['hresult'] and out==r['destination_after'],f"Fill model disagrees for {r['input']['id']}: {h:#x} vs {r['hresult']:#x}")
    report=dict(schema=1,success=True,original_sha256=BUILD_HASH,cases=len(rows),cpu_hresult_checks=len(rows),cpu_destination_checks=len(rows),native_success_cases=sum(not r['hresult'] for r in rows),pending_native_failure_cases=sum(bool(r['hresult']) for r in rows),native_hresult_validation=False,high_color_truncation_cases=sum(r['input']['key']>65535 for r in rows),clipped_cases=sum(r['input']['mode']!=0 for r in rows),hresults={f'0x{h:08x}':sum(r['hresult']==h for r in rows) for h in sorted({r['hresult'] for r in rows})},scope='Offline comparison with unchanged original fill wrapper0x58bac0 against Wine Surface2. CPU RGB565 fill admission/result model; native argument-derived constant UPDATE pieces compare successful output pixels. No native fill API HRESULT, loss/Restore/retry, initial unknown storage, indexed/masked formats, live gameplay or Windows-driver equivalence.')
    paths=['tools/check-original-surface-fills.py','tools/check-original-surface-keys.py','tests/test-original-surface-fills.py','renderer/blit.cpp','renderer/blit.hpp','renderer/commands.cpp','renderer/commands.hpp','renderer/command_consumer.cpp','renderer/command_state.hpp','renderer/commands_main.cpp']
    if build:
        parent=ROOT/'working/tests/original-surface-fill-replay';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));file=run/'fills.cmd';file.write_bytes(stream(rows));output=run/'native.pixels'
        env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1');binary=build.resolve()/'mnm-render-commands';result=subprocess.run([str(binary),str(file),'--output',str(output)],env=env,capture_output=True,text=True,timeout=60);(run/'replay.log').write_text(result.stdout+result.stderr)
        require(result.returncode==0,result.stdout+result.stderr);gl=json.loads(result.stdout);require(gl['checks']==report['native_success_cases'] and gl['rendered'],'Missing native fill checks');require(output.read_bytes()==pixels(next(r for r in reversed(rows) if plan(r)[0]==0)['destination_after']),'Native fill final output mismatch')
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
