#!/usr/bin/env python3
"""Reproduce callback-side logging contention using production C admission code.

Linux pthread/Win32 API stubs exercise native policy, not original gameplay.
The pinned previous logger deliberately blocks in WriteFile; both versions must
forward original drawing exactly once. Output and scope declarations are owned.
"""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path

import importlib.util

spec = importlib.util.spec_from_file_location("draft_claims", Path(__file__).with_name("draft-coverage-claims.py"))
draft_claims = importlib.util.module_from_spec(spec)
spec.loader.exec_module(draft_claims)
declaration = draft_claims.declaration

ROOT = Path(__file__).resolve().parents[1]
BASE = 'e529af2f41707247b70b01040e5ceb52e49f69ef'
BASE_SHA = 'e21d9134b6f2d0a9d7624a017a2ea0b52ee05aae6a249a1c84ccf8740959b5a0'


def run(argv, output):
    result = subprocess.run(argv, cwd=ROOT, text=True, capture_output=True, timeout=30)
    output.write_text(result.stdout + result.stderr)
    if result.returncode:
        raise RuntimeError(f'{argv[0]} failed ({result.returncode}); see {output}')
    return result.stdout


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    proposed = json.loads(args.claims.read_text())
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    bids = [c['behavior'] for c in proposed['claims']]
    sids = sorted({s for c in proposed['claims'] for s in c.get('scenarios', {})})
    actual = declaration(ROOT, register, bids, sids)
    if actual != proposed:
        raise ValueError('Prospective scope/source declaration changed before execution')
    args.output.mkdir(parents=True, exist_ok=False)
    baseline = subprocess.check_output(['git', 'show', BASE+':runtime/render/lock_diagnostics.h'], cwd=ROOT)
    if hashlib.sha256(baseline).hexdigest() != BASE_SHA:
        raise ValueError('Pinned previous native logger hash differs')
    header = args.output.resolve()/'baseline-lock-diagnostics.h'
    header.write_bytes(baseline)
    flags = ['gcc', '-m32', '-std=gnu11', '-O2', '-pthread', '-Wall', '-Wextra', '-Werror', '-Wno-unused-function']
    fixture = str(ROOT/'tests/lock-diagnostic-contention-test.c')
    runs = {}
    for version in ('baseline', 'current'):
        binary = args.output.resolve()/version
        command = flags + [fixture, '-o', str(binary)]
        if version == 'baseline':
            command += [f'-DLOCK_DIAGNOSTICS_HEADER="{header}"']
        run(command, args.output/(version+'-compile.log'))
        cases = ('baseline', 'finite', 'finite-concurrent') if version == 'baseline' else ('stall', 'guards', 'limits', 'thread-failure', 'file-failure', 'finite', 'finite-concurrent')
        for case in cases:
            raw = run([str(binary), case], args.output/(version+'-'+case+'.log'))
            runs[version+'-'+case] = json.loads(raw)
    stable = declaration(ROOT, register, bids, sids) == proposed
    if not stable:
        raise ValueError('Sources changed during execution')
    report = {'schema': 1, 'success': True, 'sources_stable': stable,
              'scope': __doc__, 'live_validated': False,
              'baseline_commit': BASE, 'baseline_header_sha256': BASE_SHA,
              'sources': proposed['sources'], 'claims': proposed['claims'],
              'compiler': subprocess.check_output(['gcc', '--version'], text=True).splitlines()[0],
              'runs': runs}
    with (args.output/'report.json').open('x') as output:
        json.dump(report, output, indent=2)
        output.write('\n')
    print(json.dumps({'success': True, 'cases': len(runs), 'report': str(args.output/'report.json')}))


if __name__ == '__main__':
    main()
