#!/usr/bin/env python3
"""Build the existing exact CALL observer for the separate approved CPU cohort."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
from drawing_census import ROOT

CPU_PLAN=('research/runtime/drawing-cpu-core-splices-static-20261011.json',
          '5554492d37878a97e2aa1b079bcebbdd2299783f616e622530c4938a55274313')


def build(executable, selected='core_api'):
    if selected!='core_api':raise ValueError('CPU target observation requires the original-active core cohort')
    source='tools/build-drawing-cpu-census.py';before=hashlib.sha256((ROOT/source).read_bytes()).hexdigest()
    spec=importlib.util.spec_from_file_location('cpu_call_observer',ROOT/'tools/build-drawing-api-census.py');backend=importlib.util.module_from_spec(spec);spec.loader.exec_module(backend)
    # This private module instance changes only its approved input plan. The
    # repository's graphics builder/default plans and native callback code stay
    # separate. Its hash/source/signature/capacity checks still execute.
    backend.PLANS={'core_api':CPU_PLAN}
    dll=backend.build(executable,selected)
    p=dll.parent/'manifest.json';record=json.loads(p.read_text())
    if len(record['hooks'])!=128 or record['refused'] or any(r['details']['formal_words']!=0xffffffff or r['details']['snapshots'] for r in record['hooks']):raise ValueError('CPU cohort must retain 128 untyped admitted CALLs')
    if hashlib.sha256((ROOT/source).read_bytes()).hexdigest()!=before:raise ValueError('CPU builder changed during compilation')
    record['sources'][source]=before;record['call_boundary_role']='cpu_backend_dispatch'
    record['scope']='Separate original-active128site CPU-global/CRT CALL target cohort using the existing V2 writer/forwarder. Unknown formal layouts, no COM/graphics API semantics or lifetime inference; no original execution, universal timing/ABI, full writers or DRAW-01 closure from compilation.'
    p.write_text(json.dumps(record,indent=2)+'\n')
    return dll


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--executable',type=Path,required=True);args=p.parse_args()
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:build(args.executable)
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
