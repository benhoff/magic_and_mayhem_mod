#!/usr/bin/env python3
"""Retain rendering branch/scenario validation gaps directly in SQLite."""
import argparse
import json
from pathlib import Path
from coverage_evidence import write_report
from rendering_frontier import build_report, markdown

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--markdown', type=Path)
    args = parser.parse_args()
    paths = [p.absolute() for p in (args.report, args.markdown) if p]
    if len(paths) != len(set(paths)) or any(p.exists() for p in paths):
        parser.error('Outputs must be distinct new paths')
    if any(not p.resolve().is_relative_to(ROOT/'research/runtime') for p in paths):
        parser.error('Frontier evidence belongs under research/runtime')
    if args.markdown and (args.markdown.parent.resolve() != ROOT/'research/runtime/coverage' or not args.markdown.name.startswith('summary')):
        parser.error('Generated Markdown must be a coverage/summary*.md SQLite reference')
    report = build_report(ROOT)
    write_report(ROOT, args.report, (json.dumps(report, indent=2)+'\n').encode())
    if args.markdown:
        write_report(ROOT, args.markdown, markdown(report).encode())
    print(json.dumps(dict(audit_valid=report['audit_valid'], errors=report['errors'], counts=report['counts']), indent=2))
    return int(not report['audit_valid'])


if __name__ == '__main__':
    raise SystemExit(main())
