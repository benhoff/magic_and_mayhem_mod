#!/usr/bin/env python3
"""Replay retained CPU/core exit calls against exact original wrapper bytes."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
from drawing_terminal_crosswalk import contracts, review

ROOT=Path(__file__).resolve().parents[1]
PROOF=ROOT/'research/runtime/drawing-canvas-release-original-range-20261011.json'


def fingerprint(path):
    with path.open('rb') as source:digest=hashlib.file_digest(source,'sha256').hexdigest()
    return {'sha256':digest,'bytes':path.stat().st_size}


def run(args):
    if args.output.exists():raise ValueError('Refuse to overwrite terminal crosswalk evidence')
    load=lambda path:json.loads(path.read_text())
    declaration=load(args.claims);sources=declaration['sources']
    def stable():return all(fingerprint(ROOT/path)['sha256']==digest for path,digest in sources.items())
    if not stable() or not any(c['behavior']=='TOOL.drawing-terminal-crosswalk' for c in declaration['claims']):raise ValueError('Missing/current prospective terminal declaration')
    run=args.run.resolve();staged=run/'staged-binary-artifacts/Chaos.exe'
    paths=[run/'capture/census.bin',run/'capture/core-bindings.bin',run/'capture/api-calls.bin',
        run/'capture/api-bindings.bin',run/'census-build.json',run/'api-build.json',run/'manifest.json',
        run/'journey.json',staged,PROOF,args.claims.resolve(),
        ROOT/'research/runtime/drawing-callers-movie-sites-20261010.json',ROOT/'research/runtime/coverage/binary-nocd.json']
    artifacts={str(p.relative_to(ROOT)):fingerprint(p) for p in paths}
    manifest=load(paths[6])
    if fingerprint(staged)['sha256']!=manifest['binaries']['Chaos.exe']:raise ValueError('Retained staged binary differs from prepared original run')
    spec=importlib.util.spec_from_file_location('terminal_pe',ROOT/'tools/export-menu-support.py')
    pe=importlib.util.module_from_spec(spec);spec.loader.exec_module(pe)
    verified=contracts(PROOF,staged.read_bytes(),pe.image_offset)
    result=review(paths[0],load(paths[4]),paths[1],paths[2],load(paths[5]),paths[3],manifest,load(paths[11]),load(paths[12]),verified)
    if not stable() or any(fingerprint(ROOT/p)!=v for p,v in artifacts.items()):raise ValueError('Terminal sources/inputs changed during review')
    result.update(sources=sources,sources_stable=True,claims=declaration['claims'],artifacts=artifacts,
        original_execution_repeated=False,initial_flag_byte_observed=False,
        original_execution_provenance='research/runtime/drawing-cpu-observer-original-menu-20261011.json')
    with args.output.open('x') as output:json.dump(result,output,indent=2);output.write('\n')
    print(json.dumps({k:result[k] for k in ['integrity','matched_wrapper_tail_pairs','direct_cpu_core_pairs','pending_cpu_pairs','sources_stable']}))


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--run',type=Path,required=True);parser.add_argument('--claims',type=Path,required=True);parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:run(args)
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
