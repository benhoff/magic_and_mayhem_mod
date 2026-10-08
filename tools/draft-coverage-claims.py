#!/usr/bin/env python3
"""Prepare scope/source declarations before an experiment; never execution proof."""
import argparse
import hashlib
import json
from pathlib import Path

from coverage_claims import behavior_contract, required_sources, scenario_contract

ROOT = Path(__file__).resolve().parents[1]


def declaration(root, register, behavior_ids, scenario_ids):
    behaviors = {b['id']: b for b in register['behaviors']}
    scenarios = {s['id']: s for s in register['scenarios']}
    builds = {b['id']: b for b in register['builds']}
    claims, paths = [], set()
    if not behavior_ids or len(set(behavior_ids)) != len(behavior_ids) or len(set(scenario_ids)) != len(scenario_ids):
        raise ValueError('Select at least one unique behavior and unique scenarios')
    for sid in scenario_ids:
        if sid not in scenarios or not set(scenarios[sid]['behaviors']) & set(behavior_ids):
            raise ValueError('Scenario does not exercise a selected behavior: ' + sid)
    for bid in behavior_ids:
        if bid not in behaviors:
            raise ValueError('Unknown behavior: ' + bid)
        behavior = behaviors[bid]
        selected = [scenarios[sid] for sid in scenario_ids if bid in scenarios[sid]['behaviors']]
        claims.append({'behavior': bid, 'contract_sha256': behavior_contract(behavior, builds),
                       'scenarios': {s['id']: scenario_contract(s) for s in selected}})
        paths.update(required_sources(behavior))
        paths.update(p for s in selected for p in s['tests'])
    hashes = {}
    for path in sorted(paths):
        target = (root/path).resolve()
        if Path(path).is_absolute() or not target.is_relative_to(root.resolve()) or target.relative_to(root.resolve()).parts[0] in {'original', 'working', '.git'}:
            raise ValueError('Unsafe declaration source: ' + path)
        hashes[path] = hashlib.sha256(target.read_bytes()).hexdigest()
    return {'schema': 1, 'scope': 'Prospective scope/source declaration only. Execute the experiment and verify source stability before registering its result.',
            'claims': claims, 'sources': hashes}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--register', type=Path, default=ROOT/'research/runtime/coverage/register.json')
    parser.add_argument('--behavior', action='append', required=True)
    parser.add_argument('--scenario', action='append', default=[])
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    value = declaration(ROOT, json.loads(args.register.read_text()), args.behavior, args.scenario)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open('x') as out:
        json.dump(value, out, indent=2)
        out.write('\n')


if __name__ == '__main__':
    main()
