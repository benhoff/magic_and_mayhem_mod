#!/usr/bin/env python3
"""Export exact original direct CALLs to the opt-in88entry terminal cohort."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from drawing_census import ROOT, BUILD_SHA256, instructions, calls_for
import importlib.util


def export(executable):
    paths=['tools/export-drawing-terminal-callers.py','tools/build-drawing-terminal-census.py',
        'tools/build-drawing-census.py','tools/drawing_census.py','tools/drawing_terminal_crosswalk.py',
        'research/runtime/drawing-canvas-release-original-range-20261011.json',
        'research/runtime/drawing-inventory.json','research/runtime/coverage/binary-nocd.json',
        'tools/export-menu-support.py','runtime/scene/canvas_producers.c']
    sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in paths}
    spec=importlib.util.spec_from_file_location('terminal_caller_entries',ROOT/'tools/build-drawing-terminal-census.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
    hooks,refused=m.hook_rows(executable);inventory=json.loads((ROOT/'research/runtime/coverage/binary-nocd.json').read_text())
    code=instructions(subprocess.check_output(['objdump','-w','-d','-Mintel',str(executable)],text=True));entries={r['entry'] for r in hooks};calls=calls_for(code,entries,inventory)
    if len(hooks)!=88 or hashlib.sha256(executable.read_bytes()).hexdigest()!=BUILD_SHA256 or sources!={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in paths}:raise ValueError('Terminal caller sources/original/hook scope changed')
    return {'schema':1,'source_sha256':BUILD_SHA256,'image_base':0x400000,'input_unchanged':True,
        'sources_stable':True,'sources':sources,'entries':sorted(entries),'callers':calls,
        'scope':'Exact original direct CALLs to88terminal/core entries. Discovered containing owners only; no indirect target/registration, function/semantic, runtime flow or DRAW-01 closure.','refused_core_entries':refused}


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--executable',type=Path,required=True);p.add_argument('--output',type=Path,required=True);args=p.parse_args();out=args.output.resolve()
    if not out.is_relative_to(ROOT) or out.relative_to(ROOT).parts[0] in {'original','.git'} or out.exists():raise ValueError('Caller output must be new and repository-owned')
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        r=export(args.executable)
        with out.open('x') as f:json.dump(r,f,indent=2);f.write('\n')
        print(json.dumps({'entries':len(r['entries']),'calls':len(r['callers'])}))
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
