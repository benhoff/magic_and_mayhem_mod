#!/usr/bin/env python3
"""Reproduce a CPU/core crosswalk from retained original-run artifacts."""
from coverage_evidence import read_text as evidence_text
import argparse
import hashlib
import json
from pathlib import Path
from drawing_cpu_crosswalk import review

ROOT = Path(__file__).resolve().parents[1]


def fingerprint(path):
    with path.open('rb') as source:
        digest = hashlib.file_digest(source, 'sha256').hexdigest()
    return {'sha256':digest, 'bytes':path.stat().st_size}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--run', type=Path, required=True)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists(): raise ValueError('Refuse to overwrite execution evidence')
    load = lambda path:json.loads(evidence_text(path))
    claims = load(args.claims)
    if not any(row['behavior']=='TOOL.drawing-cpu-crosswalk' for row in claims['claims']):
        raise ValueError('Missing prospective CPU crosswalk scope declaration')
    sources = claims['sources']
    def stable_sources():
        return all(fingerprint(ROOT/path)['sha256']==digest for path,digest in sources.items())
    if not stable_sources(): raise ValueError('Declared sources changed before execution')
    run = args.run.resolve()
    inputs = [run/'capture/census.bin', run/'capture/core-bindings.bin',
        run/'capture/api-calls.bin', run/'capture/api-bindings.bin',
        run/'census-build.json', run/'api-build.json', run/'manifest.json',
        run/'journey.json', args.claims.resolve(),
        ROOT/'research/runtime/drawing-callers-movie-sites-20261010.json',
        ROOT/'research/runtime/coverage/binary-nocd.json']
    artifacts = {str(path.relative_to(ROOT)):fingerprint(path) for path in inputs}
    result = review(inputs[0], load(inputs[4]), inputs[1], inputs[2],
        load(inputs[5]), inputs[3], load(inputs[6]), load(inputs[9]), load(inputs[10]))
    if not stable_sources() or any(fingerprint(ROOT/path)!=value for path,value in artifacts.items()):
        raise ValueError('Sources or retained input artifacts changed during execution')
    result.update(sources=sources, sources_stable=True, claims=claims['claims'],
        artifacts=artifacts, original_execution_repeated=False,
        original_execution_provenance='research/runtime/drawing-cpu-observer-original-menu-20261011.json')
    with args.output.open('x') as output:
        json.dump(result, output, indent=2); output.write('\n')
    print(json.dumps({key:result[key] for key in ('integrity','matched_direct_cpu_pairs',
        'pending_cpu_pairs','core_pairs_outside_direct_cpu_crosswalk','sources_stable')}))


if __name__ == '__main__': main()
