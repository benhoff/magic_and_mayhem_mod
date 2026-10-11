#!/usr/bin/env python3
"""Execute unchanged original canvas release and terminal wrapper clients."""
import argparse
from collections import Counter
import hashlib
import itertools
import json
from pathlib import Path
import subprocess
import tempfile
from drawing_census import ROOT, BUILD_SHA256, instructions


def run(executable, claims):
    declaration=json.loads(claims.read_text()); sources=declaration['sources']
    def source_hashes():
        return {p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sources}
    if source_hashes()!=sources:raise ValueError('Prospective sources changed')
    if hashlib.sha256(executable.read_bytes()).hexdigest()!=BUILD_SHA256:raise ValueError('Wrong original input')
    inventory=json.loads((ROOT/'research/runtime/coverage/binary-nocd.json').read_text())
    imports=[x for x in inventory['imports'] if x['iat_va']==0x5c50a0]
    if len(imports)!=1 or imports[0]['name']!='CloseHandle':raise ValueError('Wrong original cleanup import')
    ranges=[(0x58b3e0,0x58b4b8),(0x4bdd10,0x4bdd1a),(0x4bdcb0,0x4bdcba),
            (0x4a2f00,0x4a2f1f),(0x4a2f70,0x4a2f8f),(0x4a37d0,0x4a37fe)]
    contracts=[]
    for start,end in ranges:
        assembly=subprocess.check_output(['objdump','-d','-Mintel','--wide',
            '--start-address='+hex(start),'--stop-address='+hex(end),str(executable)],text=True)
        code=instructions(assembly); cursor=start
        while cursor<end:
            row=code.get(cursor)
            if row is None:raise ValueError('Missing exact instruction in bounded original range')
            cursor+=len(bytes.fromhex(row['bytes']))
        if cursor!=end:raise ValueError('Original client range ends inside an instruction')
        returns=[row for row in code.values() if row['mnemonic']=='ret']
        if any(row['bytes']!='c3' for row in returns):raise ValueError('Original RET0 contract differs')
        contracts.append({'start':start,'end':end,'instructions':[code[x] for x in sorted(code)],
            'bytes_sha256':hashlib.sha256(b''.join(bytes.fromhex(code[x]['bytes']) for x in sorted(code))).hexdigest(),
            'return_cleanup':0,'return_instructions':returns})
    parent=ROOT/'working/tests/drawing-canvas-release';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent)); print(out,flush=True)
    with (out/'build.log').open('x') as log:
        subprocess.run(['g++','-m32','-O2','-std=c++17','-fno-pie','-no-pie','-mno-sse','-mno-mmx',
            '-Wall','-Wextra','-Werror',str(ROOT/'tests/drawing-canvas-release-reference.cpp'),
            '-o',str(out/'reference')],stdout=log,stderr=subprocess.STDOUT,check=True,timeout=90)
    with (out/'cases.jsonl').open('x') as output,(out/'execution.log').open('x') as log:
        process=subprocess.run([str(out/'reference'),str(executable)],stdout=output,stderr=log,timeout=90)
    (out/'execution-status.json').write_text(json.dumps({'return_code':process.returncode},indent=2)+'\n')
    if process.returncode:raise ValueError('Original release fixture failed: '+str(out))
    axes={'present':{0,1},'aux':{0,1},'first':{0,2,1001},'second':{0,2,1001},
          'clipper':{0,1},'current':{0,1},'index':{0,1,126,127},'tail':{0,1,3},'count':{0,1,7}}
    seen=set();callbacks=set();outer=set();totals=Counter();shared_cap=underflow=0
    for line in (out/'cases.jsonl').read_text().splitlines():
        row=json.loads(line)
        if row['kind']=='release':
            key=tuple(row[k] for k in axes)
            if any(type(row[k]) is not int or row[k] not in values for k,values in axes.items()) or key in seen:raise ValueError('Invalid/duplicate release cell')
            seen.add(key); totals['release4']+=row['calls4'];totals['release8']+=row['calls8'];totals['release_clipper']+=row['calls_clipper']
            if row['present'] and row['first']==1001:
                if (row['calls4'],row['calls8'])!=(1000,1):raise ValueError('Shared cap did not execute second Release')
                shared_cap+=1
            underflow+=row['present']==1 and row['count']==0
        elif row['kind']=='callback':
            key=(row['target'],row['flags'])
            if key in callbacks:raise ValueError('Duplicate callback cell')
            callbacks.add(key);totals['callback_release']+=row['calls']
        elif row['kind']=='outer':
            key=(row['handle'],row['result'])
            if key in outer or row['calls']!=2:raise ValueError('Invalid/duplicate outer cleanup cell')
            outer.add(key);totals['outer_release']+=row['calls']
        else:raise ValueError('Unknown original fixture cell')
    if seen!=set(itertools.product(*axes.values())) or len(seen)!=5184:raise ValueError('Incomplete release matrix')
    if callbacks!=set(itertools.product([0x4bdd10,0x4bdcb0,0x4a2f00,0x4a2f70],range(256))):raise ValueError('Incomplete callback flag matrix')
    if outer!=set(itertools.product([0,0x12345678,0xffffffff],[0,1,0xffffffff])):raise ValueError('Incomplete handle matrix')
    if source_hashes()!=sources or hashlib.sha256(executable.read_bytes()).hexdigest()!=BUILD_SHA256:raise ValueError('Sources/input changed during execution')
    report={'schema':1,'success':True,'scope':'Unchanged original58b3e0 canvas release client with owned nonnull scripted interfaces,5184unique present/aux/Release-result/list/current/count cells, exact shared1000iteration cap, optional slot152, clipper/current retirement, list compaction/index updates, counter decrement and object writes. Separately execute four unchanged fixed-receiver/one-shot callbacks over all256flag states twice and outer4a37d0 over nine handle/result cells, with58b3e0 mocked only in that second phase. No real COM/refcounts/object deallocation, driver, external mutation/reentrancy, invalid index/null+4, arbitrary list shapes, callback registration, actual exit flow correlation, pixels/lifetime, native equivalence or scenario/dispatch closure.',
        'sources':sources,'claims':declaration['claims'],'sources_stable':True,'original_sha256':BUILD_SHA256,
        'input_unchanged':True,'original_release_instructions_unchanged_during_release_cases':True,
        'original_wrapper_instructions_unchanged':True,'release_cases':len(seen),'callback_cases':len(callbacks),
        'outer_cases':len(outer),'original_cases':len(seen)+len(callbacks)+len(outer),
        'shared_cap_second_release_cells':shared_cap,'zero_count_decrement_cells':underflow,
        'matrix_axes':{k:sorted(v) for k,v in axes.items()},'calls':dict(totals),
        'original_instruction_contracts':contracts,
        'original_entry_anchors':{hex(c['start']):''.join(row['bytes'] for row in c['instructions']) for c in contracts},
        'close_handle_import':imports[0],
        'artifacts':{str(p.relative_to(ROOT)):{'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'bytes':p.stat().st_size}
            for p in [out/'cases.jsonl',out/'reference',out/'build.log',out/'execution.log',out/'execution-status.json']}}
    with (out/'report.json').open('x') as f:json.dump(report,f,indent=2);f.write('\n')
    print(json.dumps({'output':str(out),'release_cases':len(seen),'callback_cases':len(callbacks),'outer_cases':len(outer),'calls':dict(totals)}),flush=True)
    return report


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--executable',type=Path,required=True);parser.add_argument('--claims',type=Path,required=True);args=parser.parse_args()
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:run(args.executable,args.claims)
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
