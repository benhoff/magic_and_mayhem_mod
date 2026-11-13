#!/usr/bin/env python3
"""Retain the precise current-source inventory before broader player acquisition."""
import argparse
import json
from pathlib import Path

from broader_player_acceptance import PREFIX, inventory
from coverage_evidence import Reader, write_report
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts
            and not (ROOT/args.report).exists(), 'Fresh owned inventory required')
    with Reader(ROOT, memoize=True) as reader:
        names = ('tools/broader_player_acceptance.py', 'tools/inspect-broader-player-acceptance.py',
                 'tools/rendering_build_binding.py')
        before = {n: reader.digest(n) for n in names}
        result = inventory(ROOT, reader)
        require(all(reader.digest(n) == h for n, h in before.items()), 'Inventory source changed')
        result.update(sources=before, sources_stable=True)
    digest = write_report(ROOT, args.report, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(dict(success=True, record=args.report, sha256=digest, cases=len(result['cases']),
                          exact_current_candidates=sum(c['phase']['current_native_source_candidate']
                                                       for c in result['cases']))), flush=True)


if __name__ == '__main__':
    raise SystemExit(main())
