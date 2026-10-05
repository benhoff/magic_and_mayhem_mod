#!/usr/bin/env python3
"""Audit binary/behavior/evidence/scenario links without consuming original media."""
import argparse
import json
from pathlib import Path
from coverage_audit import audit, markdown

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--register', type=Path, default=ROOT/'research/runtime/coverage/register.json')
    parser.add_argument('--json', type=Path, help='Write a new full machine-readable report')
    parser.add_argument('--markdown', type=Path, help='Write a new human-readable summary')
    parser.add_argument('--require-fresh', action='store_true', help='Also fail on changed evidence source hashes')
    args = parser.parse_args()
    for path in (args.json, args.markdown):
        if path and path.exists():
            parser.error(f'Refusing to overwrite report: {path}')
    report = audit(ROOT, json.loads(args.register.read_text()))
    for path, content in ((args.json, json.dumps(report, indent=2)+'\n'), (args.markdown, markdown(report))):
        if path:
            path.parent.mkdir(parents=True, exist_ok=True)
            with path.open('x') as out:
                out.write(content)
    print(json.dumps({'errors': report['errors'], 'warnings': report['warnings'], 'counts': report.get('counts', {})}, indent=2))
    return 1 if report['errors'] or (args.require_fresh and report['findings']['stale_evidence']) else 0


if __name__ == '__main__':
    raise SystemExit(main())
