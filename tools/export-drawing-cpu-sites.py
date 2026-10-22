#!/usr/bin/env python3
"""Export unchanged original raster-slot CALLs and exact immediate assignments."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
from drawing_census import ROOT,BUILD_SHA256,instructions
from drawing_cpu_sites import boundaries,assignments,SLOTS


def export(executable):
    paths=['tools/export-drawing-cpu-sites.py','tools/drawing_cpu_sites.py','tools/drawing_api_sites.py','tools/drawing_census.py','tools/export-menu-support.py','research/runtime/coverage/binary-nocd.json']
    sources={p:hashlib.sha256(evidence_bytes(ROOT/p)).hexdigest() for p in paths}
    raw=evidence_bytes(executable)
    if hashlib.sha256(raw).hexdigest()!=BUILD_SHA256:raise ValueError('Unsupported original executable')
    inventory=json.loads(evidence_text(ROOT/'research/runtime/coverage/binary-nocd.json'))
    if inventory['source_sha256']!=BUILD_SHA256 or inventory['image_base']!=0x400000:raise ValueError('CPU discovery build/base mismatch')
    assembly=subprocess.check_output(['objdump','-w','-d','-Mintel',str(executable)],text=True);code=instructions(assembly)
    calls=boundaries(code,inventory);writes=assignments(code,inventory)
    spec=importlib.util.spec_from_file_location('cpu_pe',ROOT/'tools/export-menu-support.py');pe=importlib.util.module_from_spec(spec);spec.loader.exec_module(pe)
    for row in calls+writes:
        expected=bytes.fromhex(row['bytes']);at=pe.image_offset(raw,row['site'],len(expected))
        if raw[at:at+len(expected)]!=expected:raise ValueError('CPU selected bytes disagree with original file')
    if hashlib.sha256(evidence_bytes(executable)).hexdigest()!=BUILD_SHA256 or sources!={p:hashlib.sha256(evidence_bytes(ROOT/p)).hexdigest() for p in paths}:raise ValueError('CPU export sources/input changed')
    return {'schema':1,'source_sha256':BUILD_SHA256,'image_base':0x400000,'input_unchanged':True,'sources_stable':True,'sources':sources,'assembly_sha256':hashlib.sha256(assembly.encode()).hexdigest(),'slots':sorted(SLOTS),'calls':calls,'direct_immediate_assignments':writes,'counts':{'calls':len(calls),'global_calls':sum(r['target_mode']['kind']=='absolute_memory' for r in calls),'exit_callback_calls':sum(r['target_mode']['kind']=='register' for r in calls),'direct_immediate_assignments':len(writes)},'scope':'Whole-image exact CALL census for four original raster globals and one exact CRT exit callback CALL; exact C7 05 immediate assignments only. Empty discovered owners remain empty. No permanent/dynamic target, universal writer/alias exclusion, raster semantics, API/COM/lifetime, installed-hook or DRAW-01 closure.'}


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--executable',type=Path,required=True);p.add_argument('--output',type=Path,required=True);args=p.parse_args();out=args.output.resolve()
    if not out.is_relative_to(ROOT) or out.relative_to(ROOT).parts[0] in {'original','.git'} or out.exists():raise ValueError('Output must be new and repository-owned')
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        r=export(args.executable);out.parent.mkdir(parents=True,exist_ok=True)
        with out.open('x') as f:json.dump(r,f,indent=2);f.write('\n')
        print(json.dumps(r['counts']))
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)


if __name__=='__main__':main()
