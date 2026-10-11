#!/usr/bin/env python3
"""Replay inherited terminal edges from the retained88entry original cohort."""
import argparse
import importlib.util
import hashlib
import json
from pathlib import Path
import subprocess
from drawing_terminal_edges import verified_contracts, review

ROOT = Path(__file__).resolve().parents[1]
PROOF = ROOT/'research/runtime/drawing-canvas-release-original-range-20261011.json'
CALLERS = ROOT/'research/runtime/drawing-terminal-callers-static-20261011.json'
INVENTORY = ROOT/'research/runtime/coverage/binary-nocd.json'


def fingerprint(path):
    with path.open('rb') as source:digest = hashlib.file_digest(source,'sha256').hexdigest()
    return {'sha256':digest,'bytes':path.stat().st_size}


def run(args):
    output = args.output.resolve()
    if output.exists() or not output.is_relative_to(ROOT) or output.relative_to(ROOT).parts[0] in {'original','.git'}:
        raise ValueError('Terminal edge output must be new and repository-owned')
    load = lambda p:json.loads(p.read_text())
    claim_path = args.claims.resolve();claims = load(claim_path)
    if not any(c['behavior'] == 'TOOL.drawing-terminal-edges' for c in claims['claims']):
        raise ValueError('Missing prospective terminal edge scope')
    def stable():return all(fingerprint(ROOT/p)['sha256'] == h for p,h in claims['sources'].items())
    if not stable():raise ValueError('Declared terminal edge sources changed')
    experiment = args.run.resolve()
    if not experiment.is_relative_to(ROOT/'working'):
        raise ValueError('Replay requires a retained repository experiment')
    paths = [experiment/'capture/census.bin',experiment/'capture/core-bindings.bin',
        experiment/'census-build.json',experiment/'manifest.json',experiment/'journey.json',
        experiment/'staged-binary-artifacts/Chaos.exe',PROOF,CALLERS,INVENTORY,claim_path]
    artifacts = {str(p.relative_to(ROOT)):fingerprint(p) for p in paths}
    staged = paths[5]
    if fingerprint(staged)['sha256'] != load(paths[3])['binaries']['Chaos.exe']:
        raise ValueError('Retained staged binary differs from original cohort')
    spec = importlib.util.spec_from_file_location('edge_pe',ROOT/'tools/export-menu-support.py')
    pe = importlib.util.module_from_spec(spec);spec.loader.exec_module(pe)
    contracts = verified_contracts(PROOF,staged.read_bytes(),pe.image_offset)
    result = review(paths[0],load(paths[2]),paths[1],load(CALLERS),load(INVENTORY),contracts)
    if not stable() or any(fingerprint(ROOT/p) != v for p,v in artifacts.items()):
        raise ValueError('Terminal edge sources or artifacts changed during replay')
    result.update(sources=claims['sources'],sources_stable=True,claims=claims['claims'],
        artifacts=artifacts,original_execution_repeated=False,
        original_execution_provenance='research/runtime/drawing-terminal-observer-original-menu-20261011.json')
    with output.open('x') as f:json.dump(result,f,indent=2);f.write('\n')
    print(json.dumps({k:result[k] for k in ['integrity','reviewed_terminal_tail_edges','remaining_inherited_edges','sources_stable']}))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--run',type=Path,required=True)
    parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args = parser.parse_args()
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:run(args)
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
