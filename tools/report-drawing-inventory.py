#!/usr/bin/env python3
"""Check DRAW-01 route accounting and retain completion blockers."""
import argparse
import json
from pathlib import Path

from drawing_inventory import ROOT, build_report, markdown, producer_trace


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
    parser.add_argument('--require-accounted', action='store_true', help='Fail on identities absent from the reviewed route register')
    parser.add_argument('--require-complete', action='store_true', help='Fail until every required scenario has a complete census')
    parser.add_argument('--producer-stream', type=Path, help='Optional existing closed producer journal')
    parser.add_argument('--done', type=Path, help='Matching MNMPDONE marker')
    parser.add_argument('--stream-sha256', help='Expected journal hash from its retained capture report')
    parser.add_argument('--stream-scenario', help='Reviewed scenario ID for this journal')
    parser.add_argument('--runtime-image-base', help='Observed process module base; never inferred from caller pointers')
    args = parser.parse_args()
    try:
        output_paths((args.json, args.markdown))
        report = build_report()
        trace_args = (args.producer_stream, args.done, args.stream_sha256, args.stream_scenario, args.runtime_image_base)
        if any(trace_args):
            if not all(trace_args):
                raise ValueError('Producer import requires stream, marker, pinned hash, scenario and runtime image base')
            trace = producer_trace(*trace_args, report)
            report['producer_trace'] = trace
            report['unaccounted'] += trace['unaccounted_operations']
            report['unaccounted'] += [{'kind': 'caller', 'behavior': 'DRAW.unattributed-caller', **r} for r in trace['unattributed_callers']]
            report['completion_blockers'] += [{'kind': 'unattributed_caller', 'detail': r} for r in trace['unattributed_callers']]
            report['draw01_complete'] = report['draw01_complete'] and not trace['unattributed_callers'] and not trace['unaccounted_operations']
            report['counts']['unaccounted'] = len(report['unaccounted'])
        for path, content in ((args.json, json.dumps(report, indent=2) + '\n'), (args.markdown, markdown(report))):
            if path:
                path.parent.mkdir(parents=True, exist_ok=True)
                with path.open('x') as output:
                    output.write(content)
        print(json.dumps({'inventory_valid': report['inventory_valid'], 'draw01_complete': report['draw01_complete'],
                          'counts': report['counts'], 'completion_blockers': len(report['completion_blockers'])}, indent=2)
              if args.json or args.markdown else markdown(report))
        return int(args.require_accounted and bool(report['unaccounted']) or args.require_complete and not report['draw01_complete'])
    except (KeyError, ValueError, OSError, TypeError) as error:
        parser.exit(2, str(error) + '\n')


if __name__ == '__main__':
    raise SystemExit(main())
