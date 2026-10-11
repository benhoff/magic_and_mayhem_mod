#!/usr/bin/env python3
"""Stage explicit88core/128CPU terminal-flow observation in a private game."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import subprocess
from types import SimpleNamespace
from drawing_census import ROOT
from drawing_cpu_sites import SLOTS


def load(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m


def approved_plan(path,expected):
    if not isinstance(expected,str) or not re.fullmatch('[0-9a-f]{64}',expected):raise ValueError('Terminal plan needs an exact reviewed SHA256')
    path=path.resolve()
    if not path.is_relative_to(ROOT) or path.relative_to(ROOT).parts[0] in {'original','.git'}:raise ValueError('Terminal plan must be repository-owned research/working metadata')
    data=path.read_bytes()
    if hashlib.sha256(data).hexdigest()!=expected:raise ValueError('Terminal plan hash mismatch')
    record=json.loads(data)
    if record.get('terminal_flow_observation') is not True or record.get('cohort')!='core_api' or record.get('core_hook_count')!=88 or len(record['hooks'])!=128 or record['refused']:raise ValueError('Expected separate88core/128CPU terminal plan')
    if any(type(r['tag']) is not int for r in record['hooks']) or [r['tag'] for r in record['hooks']]!=list(range(128)):raise ValueError('Invalid CPU terminal tags')
    for hook in record['hooks']:
        mode=hook['target_mode']
        if not (mode.get('kind')=='absolute_memory' and mode.get('address') in SLOTS or hook['site']==0x59d98a and mode=={'kind':'register','register':'eax'}):raise ValueError('Terminal cohort acquired a non-CPU CALL')
    return path,record


def prepare(source,plan,expected,movies=False,gameplay=False):
    path,record=approved_plan(plan,expected)
    own='tools/prepare-drawing-terminal-census.py';own_hash=hashlib.sha256((ROOT/own).read_bytes()).hexdigest()
    def cpu_build(executable,selected):
        if selected!='core_api':raise ValueError('Terminal CPU cohort requires original-active core')
        core=ROOT/'working/build/drawing-census/manifest.json';before=core.read_bytes();meta=json.loads(before)
        if not meta.get('terminal_flow_observation') or len(meta['hooks'])!=88:raise ValueError('Terminal CPU build lacks its actual88entry core manifest')
        if any(c['entry']<a['site']+a['length'] and a['site']<c['entry']+c['length'] for c in meta['hooks'] for a in record['hooks']):raise ValueError('Terminal CPU/core prefix overlap')
        backend=load('terminal_cpu_backend','tools/build-drawing-api-census.py');backend.PLANS={'core_api':(str(path.relative_to(ROOT)),expected)}
        dll=backend.build(executable,selected);out=dll.parent/'manifest.json';m=json.loads(out.read_text())
        if core.read_bytes()!=before or len(m['hooks'])!=128 or m['refused'] or any(h['details']['formal_words']!=0xffffffff or h['details']['snapshots'] for h in m['hooks']):raise ValueError('Terminal build changed actual core metadata or untyped CPU scope')
        m['sources'][own]=own_hash;m['call_boundary_role']='cpu_backend_dispatch';m['terminal_flow_observation']=True
        m['actual_core_build_sha256']=hashlib.sha256(before).hexdigest();m['actual_core_hook_count']=88
        out.write_text(json.dumps(m,indent=2)+'\n');return dll
    backend=load('terminal_staging','tools/prepare-drawing-census.py');original_load=backend.load
    def selected_load(name,path):
        if name=='drawing_build':return load(name,'tools/build-drawing-terminal-census.py')
        if name=='drawing_api_build':return SimpleNamespace(build=cpu_build)
        return original_load(name,path)
    backend.load=selected_load
    root=backend.prepare(source,movies=movies,gameplay=gameplay,cpu=True)
    if hashlib.sha256((ROOT/own).read_bytes()).hexdigest()!=own_hash or hashlib.sha256(path.read_bytes()).hexdigest()!=expected:raise ValueError('Terminal staging sources/plan changed')
    manifest=root/'manifest.json';m=json.loads(manifest.read_text());m['terminal_flow_observation']=True
    m['terminal_cpu_plan']={'path':str(path.relative_to(ROOT)),'sha256':expected}
    manifest.write_text(json.dumps(m,indent=2)+'\n');return root


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--source-game',type=Path,required=True);parser.add_argument('--cpu-plan',type=Path,required=True);parser.add_argument('--cpu-plan-sha256',required=True);parser.add_argument('--movies',action='store_true');parser.add_argument('--gameplay-observation',action='store_true');args=parser.parse_args()
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:print(prepare(args.source_game,args.cpu_plan,args.cpu_plan_sha256,args.movies,args.gameplay_observation),flush=True)
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
