#!/usr/bin/env python3
"""Retain production-encoder controls and their exact source contract in SQLite."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, write_report

ROOT = Path(__file__).resolve().parents[1]
BID = 'NP.scene-actor-generation-observation'
SID = 'native.scene-actor-observation-controls'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    path = Path(args.report)
    if not args.report.startswith('research/runtime/attachment-combat-results-20261107/') or '..' in path.parts or (ROOT/path).exists():
        parser.error('New owned immutable report required')
    with Reader(ROOT) as reader:
        register = reader.json('research/runtime/coverage/register.json')
        behavior = next(b for b in register['behaviors'] if b['id'] == BID)
        scenario = next(s for s in register['scenarios'] if s['id'] == SID)
        names = set(required_sources(behavior)) | {'tools/check-scene-actor-observation.py'}
        sources = {name: reader.digest(name) for name in sorted(names)}
        execution = subprocess.run([sys.executable, '-B', 'tests/test-scene-actor-observation.py'],
                                   cwd=ROOT, capture_output=True, timeout=60)
        stable = all(reader.digest(name) == sha for name, sha in sources.items())
    log = str(path.with_suffix('')) + '-log.bin'
    log_sha = write_report(ROOT, log, execution.stdout + execution.stderr)
    result = dict(schema=1, kind='scene_actor_production_encoder_controls',
                  success=execution.returncode == 0 and stable, exit_code=execution.returncode,
                  sources=sources, sources_stable=stable, original_game_runs=0,
                  attachment_placement_proved=False, combat_output_proved=False,
                  log=dict(record=log, sha256=log_sha),
                  claims=[dict(behavior=BID, contract_sha256=behavior_contract(
                      behavior, {b['id']: b for b in register['builds']}),
                      scenarios={SID: scenario_contract(scenario)})])
    sha = write_report(ROOT, args.report, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'], record=args.report, sha256=sha)))
    return int(not result['success'])


if __name__ == '__main__':
    raise SystemExit(main())
