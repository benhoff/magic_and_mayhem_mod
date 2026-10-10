#!/usr/bin/env python3
"""Report scoped rendering support against the reviewed binary/API inventory."""
import argparse
import json
from pathlib import Path

from rendering_coverage import ROOT, build_report, markdown


def output_paths(paths):
    resolved = [p.resolve() for p in paths if p]
    if len(resolved) != len(set(resolved)) or any(p.exists() for p in resolved):
        raise ValueError('Report paths must be distinct and new')
    if any(p.is_relative_to((ROOT / directory).resolve()) for p in resolved for directory in ('original', '.git')):
        raise ValueError('Reports cannot be written into original/ or .git/')
    return resolved


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--json', type=Path, help='New machine-readable report')
    parser.add_argument('--markdown', type=Path, help='New readable report')
    args = parser.parse_args()
    try:
        paths = output_paths((args.json, args.markdown))
        report = build_report()
        for path, content in ((args.json, json.dumps(report, indent=2) + '\n'), (args.markdown, markdown(report))):
            if path:
                path.parent.mkdir(parents=True, exist_ok=True)
                with path.open('x') as output:
                    output.write(content)
        print(markdown(report) if not paths else json.dumps({
            'audit_valid': report['audit']['valid'],
            'surface_operations': {k: v for k, v in report['surface_operations'].items() if k != 'rows'},
            'drawing_routes': {k: v for k, v in report['drawing_routes'].items() if k != 'rows'},
        }, indent=2))
        return 0 if report['audit']['valid'] else 1
    except (ValueError, KeyError, OSError, TypeError) as error:
        parser.exit(2, str(error) + '\n')


if __name__ == '__main__':
    raise SystemExit(main())
