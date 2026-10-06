#!/usr/bin/env python3
"""Offline GPU backend and v3 overlap replay against independent original outputs."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('overlap',ROOT/'tools/check-original-surface-overlap.py');o=importlib.util.module_from_spec(spec);spec.loader.exec_module(o)
words,pixels,require=o.words,o.pixels,o.require

def stream(rows,poison=None,flags=None,legacy=False):
    records=[]
    def add(op,payload=b''):
        sequence=len(records)+1;records.append(words(op,sequence,len(payload))+payload);return sequence
    for i,r in enumerate(rows):
        c=r['input'];id=i+1;add(1,words(id,8,6,16,0xf800,0x7e0,31)+pixels(r['destination_before']))
        phase=c['phase'];regions={0:[],1:[[2,1,6,5]],2:[[0,0,3,2],[5,3,8,6]],3:[],4:[],5:[[0,0,8,3],[0,4,8,6]]}[phase]
        add(16,words(id,0 if not phase else 2 if phase==3 else 1,len(regions),*(v for rect in regions for v in rect)))
        callflags=0 if c['no_wait'] else 0x10 if c['fast'] else 0x1000000
        if flags is not None:callflags=flags
        if legacy:add(3,words(id,id,*c['source'],*c['destination'][:2],0,0))
        else:
            seq=add(17,words(id,id,c['fast'],callflags,0,*c['source'],*c['destination']))
            add(18,words(seq,r['hresult']^(1 if poison=='hresult' else 0)))
        out=r['destination_after'].copy()
        if poison=='pixels':out[0]^=1
        add(5,words(id)+pixels(out))
        if i==len(rows)-1:add(6,words(id))
        add(7,words(id))
    add(8);return b'MNMCMD03'+words(3,16)+b''.join(records)

def check(build,report):
    rows,m=o.load(o.DEFAULT)
    for r in rows:require(o.cpu(r)==(r['hresult'],r['destination_after']),'CPU comparison mismatch')
    parent=ROOT/'working/tests/native-surface-overlap';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')
    def command(binary,input,output):
        r=subprocess.run([str(build/binary),str(input),str(output)] if binary=='render-surface-copy-fixtures' else [str(build/binary),str(input),'--output',str(output)],env=env,capture_output=True,text=True,timeout=60)
        (run/(input.stem+'.log')).write_text(r.stdout+r.stderr);return r
    # No expected pixels or HRESULTs are passed to the backend fixture executable.
    inputs=[dict(id=r['input']['id'],shared=True,source_pixels=r['source_before'],destination_pixels=r['destination_before'],source=r['input']['source'],destination=r['input']['destination'],fast=r['input']['fast'],flags=0 if r['input']['no_wait'] else 0x10 if r['input']['fast'] else 0x1000000,held=0,clip=r['input']['phase']) for r in rows]
    file=run/'inputs.json';file.write_text(json.dumps(inputs));out=run/'backend.json';result=command('render-surface-copy-fixtures',file,out);require(result.returncode==0,result.stdout+result.stderr);backend=json.loads(out.read_text())
    require(backend['ordinary_readbacks']==backend['ordinary_uploads']==0 and backend['policy_tests']==20,'GPU path/policy check mismatch')
    for r,actual in zip(rows,backend['cases'],strict=True):require(actual['hresult']==r['hresult'] and actual['source_after']==r['source_after'] and actual['destination_after']==r['destination_after'],'Native backend result/pixels mismatch')
    chunks=[]
    for index in range(0,len(rows),256):
        subset=rows[index:index+256];file=run/f'stream-{index}.cmd';file.write_bytes(stream(subset));out=run/f'stream-{index}.pixels';result=command('mnm-render-commands',file,out);require(result.returncode==0,result.stdout+result.stderr);chunk=json.loads(result.stdout);require(chunk['checks']==chunk['result_checks']==chunk['surface_copies']==len(subset),'Stream checks missing');require(out.read_bytes()==pixels(subset[-1]['destination_after']),'Published pixels mismatch');chunks.append(chunk)
    negatives=[('bad-result',dict(poison='hresult')),('bad-pixels',dict(poison='pixels')),('source-key-self',dict(flags=0x8000)),('fast-source-key-self',dict(flags=1)),('unknown-flag',dict(flags=3)),('legacy-self',dict(legacy=True))]
    for name,options in negatives:
        file=run/(name+'.cmd');file.write_bytes(stream([next(r for r in rows if r['input']['fast'])] if name=='fast-source-key-self' else rows[:1],**options));out=run/(name+'.pixels');result=command('mnm-render-commands',file,out);require(result.returncode!=0 and not out.exists(),'Invalid stream accepted or published pixels')
    paths=['tools/check-native-surface-overlap.py','tools/check-original-surface-overlap.py','tools/check-original-surface-keys.py','tests/test-original-surface-overlap.py','tests/render-surface-copy-fixtures.cpp','renderer/blit.cpp','renderer/blit.hpp','renderer/surface_copy.cpp','renderer/surface_copy.hpp','renderer/command_consumer.cpp','renderer/commands.cpp','renderer/commands.hpp','renderer/command_state.hpp','renderer/commands_main.cpp','research/formats/render-surface-commands-v3.md','tools/check-native-surface-clipping.py','tests/fixtures/surfaces/'+m['path'],str(o.DEFAULT.relative_to(ROOT))]
    data=dict(schema=1,success=True,original_sha256=o.BUILD_HASH,sources={n:o.sha(ROOT/n) for n in paths},backend='opengl_native_integer',cases=len(rows),backend_hresult_checks=len(rows),backend_pixel_checks=2*len(rows),stream_hresult_checks=len(rows),stream_pixel_checks=len(rows),ordinary_uploads=0,ordinary_readbacks=0,refused_streams=len(negatives),policy_tests=backend['policy_tests'],partial_error_cases=sum(bool(r['hresult']) and r['destination_before']!=r['destination_after'] for r in rows),vendor=backend['vendor'],renderer=backend['renderer'],chunks=chunks,artifacts=str(run.relative_to(ROOT)),scope='Offline original opaque RGB565 shared backing outputs match GPU per-piece snapshots and v3 same-ID logical replay. Separate COM aliases normalize to one verified logical backing ID; no live pointer tracking, keyed/masked/other-format overlap, original Restore/retry or Windows-driver equivalence.')
    with report.open('x') as f:json.dump(data,f,indent=2);f.write('\n')
    return data
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build',type=Path,default=ROOT/'working/build/renderer');p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    try:
        r=check(a.build.resolve(),a.report);print(json.dumps({k:r[k] for k in ['success','cases','partial_error_cases','refused_streams']}))
    except (ValueError,KeyError,OSError,subprocess.SubprocessError) as e:p.exit(2,str(e)+'\n')
