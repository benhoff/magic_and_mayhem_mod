#!/usr/bin/env python3
"""Join a bounded set of retained rendering proofs; finish/account once afterward."""
import argparse
import json
from pathlib import Path
import subprocess
import sys
from time import perf_counter
from coverage_evidence import Reader, write_report
from rendering_closeout import ADAPTERS, BID, SID, batch

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-closeout-batch-results-20261107/'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--parent', required=True)
    parser.add_argument('--component', nargs=2, action='append', required=True,
                        metavar=('ADAPTER', 'ANALYSIS'), help='Adapters: ' + ', '.join(ADAPTERS))
    parser.add_argument('--report', required=True)
    parser.add_argument('--run-guards', action='store_true', help='Run actual-data, adverse admission and real UI-reader tests')
    args = parser.parse_args()
    path = Path(args.report)
    if (not args.report.startswith(PREFIX) or '..' in path.parts or path.suffix != '.json'
            or (ROOT / path).exists()):
        parser.error('Fresh owned immutable report required')
    try:
        with Reader(ROOT) as reader:
            result = batch(ROOT, reader, args.parent, args.component)
            if args.run_guards:
                from coverage_claims import behavior_contract, scenario_contract
                tick = perf_counter()
                command = [sys.executable, '-B', 'tests/test-rendering-closeout-batch.py']
                check = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=120)
                guards = dict(command=command, exit_code=check.returncode,
                              stdout=check.stdout, stderr=check.stderr)
                if check.returncode:
                    raise ValueError('Closeout guards failed: ' + check.stderr)
                if any(reader.digest(n) != h for n, h in result['analysis_sources'].items()):
                    raise ValueError('Runner sources changed during guards')
                result['guards'] = guards
                result['timings_seconds']['guards'] = perf_counter() - tick
                register = reader.json('research/runtime/coverage/register.json')
                b = next(b for b in register['behaviors'] if b['id'] == BID)
                s = next(s for s in register['scenarios'] if s['id'] == SID)
                result['claims'] = [dict(behavior=BID, contract_sha256=behavior_contract(
                    b, {v['id']: v for v in register['builds']}), scenarios={SID: scenario_contract(s)})]
    except Exception as error:
        result = dict(schema=1, kind='retained_rendering_closeout_batch', success=False,
                      error=str(error), original_game_runs=0,
                      scope='Failed retained-data admission; no partial row promotions.')
    sha = write_report(ROOT, args.report, (json.dumps(result, indent=2) + '\n').encode())
    print(json.dumps(dict(record=args.report, sha256=sha,
                         **{k: result.get(k) for k in ('success', 'error', 'passed_requirements',
                            'pending_requirements', 'newly_passed_requirements', 'timings_seconds')})))
    return int(not result['success'])


if __name__ == '__main__':
    raise SystemExit(main())
