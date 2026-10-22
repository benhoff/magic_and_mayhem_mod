#!/usr/bin/env python3
"""Plan the separate128CPU CALL cohort beside88terminal/core entries."""
from coverage_evidence import read_bytes as evidence_bytes
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
from drawing_census import ROOT


def load(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m


def export(executable,boundaries):
    paths=['tools/export-drawing-terminal-plan.py','tools/build-drawing-terminal-census.py',
        'tools/drawing_terminal_crosswalk.py','research/runtime/drawing-canvas-release-original-range-20261011.json']
    sources={p:hashlib.sha256(evidence_bytes(ROOT/p)).hexdigest() for p in paths}
    backend=load('terminal_call_planner','tools/export-drawing-api-splices.py');original_load=backend.load
    backend.load=lambda name,path:load(name,'tools/build-drawing-terminal-census.py') if name=='api_core_splices' else original_load(name,path)
    record=backend.export(executable,boundaries.resolve())
    if record['core_hook_count']!=88 or len(record['hooks'])!=128 or record['refused']:raise ValueError('Terminal CPU plan must admit128CALLs beside88entries')
    if sources!={p:hashlib.sha256(evidence_bytes(ROOT/p)).hexdigest() for p in paths}:raise ValueError('Terminal planner sources changed')
    record['sources'].update(sources);record['terminal_flow_observation']=True
    return record


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--executable',type=Path,required=True);p.add_argument('--boundaries',type=Path,required=True);p.add_argument('--output',type=Path,required=True);args=p.parse_args();out=args.output.resolve()
    if not out.is_relative_to(ROOT) or out.relative_to(ROOT).parts[0] in {'original','.git'} or out.exists():raise ValueError('Terminal plan output must be new and repository-owned')
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        result=export(args.executable,args.boundaries)
        with out.open('x') as f:json.dump(result,f,indent=2);f.write('\n')
        print(json.dumps(result['counts']))
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
