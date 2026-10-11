#!/usr/bin/env python3
"""Opt-in five terminal parent frames beside the unchanged default core census."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
from drawing_census import ROOT, BUILD_SHA256, instructions, splice_conflicts
from drawing_terminal_crosswalk import PROOF_SHA256

PROOF='research/runtime/drawing-canvas-release-original-range-20261011.json'
TARGETS=(0x4bdd10,0x4bdcb0,0x4a2f00,0x4a2f70,0x4a37d0)


def load(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module


def terminal_rows(raw, base_rows, code, proof, image_offset):
    """Exact original prefix/RET0 composition; no guessed function boundaries."""
    if hashlib.sha256(proof.read_bytes()).hexdigest()!=PROOF_SHA256:raise ValueError('Unreviewed original terminal range proof')
    record=json.loads(proof.read_text());ranges={r['start']:r for r in record['original_instruction_contracts']}
    if record['original_sha256']!=BUILD_SHA256 or not record['success'] or not record['original_wrapper_instructions_unchanged']:raise ValueError('Missing original terminal wrapper execution')
    rows=list(base_rows);occupied=[(r['entry'],r['entry']+r['length']) for r in rows]
    for entry in TARGETS:
        contract=ranges[entry];body=bytes.fromhex(record['original_entry_anchors'][hex(entry)])
        offset=image_offset(raw,entry,len(body))
        if raw[offset:offset+len(body)]!=body or hashlib.sha256(body).hexdigest()!=contract['bytes_sha256']:raise ValueError('Original terminal body bytes differ')
        if contract['return_cleanup']!=0 or any(r['bytes']!='c3' for r in contract['return_instructions']):raise ValueError('Terminal RET0 composition differs')
        prefix=bytearray();cursor=entry
        for instruction in contract['instructions']:
            if instruction['address']!=cursor:raise ValueError('Terminal prefix instruction identity differs')
            data=bytes.fromhex(instruction['bytes'])
            if instruction['mnemonic']!='mov':raise ValueError('Terminal prefix is outside reviewed MOV-only scope')
            prefix.extend(data);cursor+=len(data)
            if len(prefix)>=5:break
        if not 5<=len(prefix)<=16 or body[:len(prefix)]!=prefix:raise ValueError('Terminal prefix extent mismatch')
        if any(entry<end and start<cursor for start,end in occupied):raise ValueError('Terminal prefix overlaps another core hook')
        if splice_conflicts(entry,len(prefix),code):raise ValueError('Incoming original branch enters terminal prefix interior')
        rows.append({'tag':len(rows),'entry':entry,'signature':prefix.hex(),'length':len(prefix),
            'pop':0,'site':False,'relocations':[],'route':'DRAW.terminal-entry.'+format(entry,'08x')})
        occupied.append((entry,cursor))
    if len(rows)>256:raise ValueError('Extended core count exceeds native capacity')
    return rows


def hook_rows(executable):
    backend=load('terminal_base_rows','tools/build-drawing-census.py')
    rows,refused=backend.hook_rows(executable);raw=executable.read_bytes()
    if hashlib.sha256(raw).hexdigest()!=BUILD_SHA256:raise ValueError('Wrong original terminal input')
    code=instructions(subprocess.check_output(['objdump','-w','-d','-Mintel',str(executable)],text=True))
    rows=terminal_rows(raw,rows,code,ROOT/PROOF,load('terminal_pe','tools/export-menu-support.py').image_offset)
    if hashlib.sha256(executable.read_bytes()).hexdigest()!=BUILD_SHA256:raise ValueError('Original terminal input changed')
    return rows,refused


def build(executable):
    paths=['tools/build-drawing-terminal-census.py','tools/drawing_terminal_crosswalk.py',PROOF]
    sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in paths}
    backend=load('terminal_base_build','tools/build-drawing-census.py');backend.hook_rows=hook_rows
    dll=backend.build(executable);path=dll.parent/'manifest.json';record=json.loads(path.read_text())
    if len(record['hooks'])!=88 or {r['entry'] for r in record['hooks'][-5:]}!=set(TARGETS):raise ValueError('Expected83default and five terminal hooks')
    if sources!={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in paths}:raise ValueError('Terminal build sources changed')
    record['sources'].update(sources);record['terminal_flow_observation']=True
    record['scope']='Separate opt-in88entry core cohort:83safe default hooks plus five byte-pinned terminal wrapper parent frames with exact original RET0 composition. Native writer/forwarder unchanged; no original replacement, generic ownership, all side effects/lifetimes, caller or scenario/dispatch closure from compilation.'
    path.write_text(json.dumps(record,indent=2)+'\n');return dll


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--executable',required=True,type=Path);args=parser.parse_args()
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:build(args.executable)
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
