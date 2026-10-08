#!/usr/bin/env python3
"""Record source-bound stopped-journal attribution and strict negative fixtures."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path)
    args = parser.parse_args()
    paths = ['tools/world_refusal.py','tools/world_channel.py','tools/inspect-startup-queues.py',
             'tests/world-refusal-diagnostics-test.py','tools/test-world-refusal-diagnostics.py']
    claims = json.loads(args.claims.read_text()) if args.claims else None
    if claims:
        paths += list(claims['sources'])
        for name, digest in claims['sources'].items():
            if sha(ROOT/name) != digest:
                raise RuntimeError('Declared source changed: '+name)
    sources = {name:sha(ROOT/name) for name in sorted(set(paths))}
    parent = ROOT/'working/tests/world-kind-diagnostics'
    parent.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    with (output/'tests.log').open('x') as log:
        subprocess.run(['python3',ROOT/'tests/world-refusal-diagnostics-test.py','-v'],
                       cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, timeout=60, check=True)
    if sources != {name:sha(ROOT/name) for name in sources}:
        raise RuntimeError('Sources changed during diagnostic validation')
    report = {'success':True, 'sources':sources, 'sources_stable':True, 'tests':16,
              'scope':'Synthetic stopped-channel/raw-journal correlation and strict malformed/missing cases; no original raster or live launcher equivalence.',
              'input_fixture_sha256':sources['tests/world-refusal-diagnostics-test.py']}
    if claims:
        report['claims'] = claims['claims']
    with (output/'report.json').open('x') as result:
        json.dump(report, result, indent=2)
        result.write('\n')
    print(output/'report.json')


if __name__ == '__main__':
    main()
