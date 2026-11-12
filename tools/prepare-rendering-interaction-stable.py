#!/usr/bin/env python3
"""Freeze one missing640 repair and the two remaining software domains."""
import json
from pathlib import Path
from coverage_evidence import Reader, write_report
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-interaction-results-20261107/'


def main():
    target = PREFIX + 'input-plan-08.json'
    require(not (ROOT / target).exists(), 'Fresh immutable stable plan required')
    with Reader(ROOT) as reader:
        parent = PREFIX + 'input-plan-07.json'
        plan = reader.json(parent)
        require(all(reader.digest(n) == h for section in ('sources', 'compiler_sources', 'runtime_sources')
                    for n, h in plan[section].items()), 'Original frozen player/runtime/controller changed')
        plan['parent'] = dict(record=parent, sha256=reader.digest(parent))
        plan['kind'] = 'finite_interaction_stable_collection_plan'
        plan['bounds'].update(maximum_requests=10, maximum_original_games=9, repairs=6)
        for n in ('tools/prepare-rendering-interaction-stable.py', 'tools/run-rendering-interaction-stable.py',
                  'tools/rendering_interaction_stability.py'):
            plan['sources'][n] = reader.digest(n)
        plan['input_policy']['pointer_readiness'] = dict(minimum_seconds=1.2, maximum_seconds=5,
            minimum_distinct_actually_displayed_source_generations=3,
            cursor='Completed original cursor point equals the prescribed source-only target',
            hover='Same living original wizard hit; no coordinate search or extra original poll')
        plan['repair_reason'] = 'The retained640 acquisition lacks three admitted displayed generations for one away action. Preserve the sufficient800 capture; collect only missing640 plus unexecuted software domains. Strict comparison gates remain unchanged.'
        print(write_report(ROOT, target, (json.dumps(plan, indent=2) + '\n').encode()))


if __name__ == '__main__':
    main()
