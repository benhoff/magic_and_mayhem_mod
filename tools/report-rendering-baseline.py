#!/usr/bin/env python3
"""Review an immutable Git rendering baseline and retain its report in SQLite."""
import argparse
import json
from pathlib import Path
from rendering_baseline import build, write

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--revision', required=True, help='Committed source revision; excludes all pending edits')
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    output = args.report if args.report.is_absolute() else ROOT/args.report
    if output.exists() or not output.resolve().is_relative_to(ROOT/'research/runtime') or output.suffix != '.json':
        parser.error('Choose a new .json reference under research/runtime')
    report = build(ROOT, args.revision)
    digest = write(ROOT, output, report)
    print(json.dumps(dict(success=report['success'], subject=report['subject'],
        audit_errors=report['audit']['errors'], rendering_counts=report['rendering']['counts'],
        requirements=len(report['requirements']), sha256=digest), indent=2))
    return int(not report['success'])


if __name__ == '__main__':
    raise SystemExit(main())
