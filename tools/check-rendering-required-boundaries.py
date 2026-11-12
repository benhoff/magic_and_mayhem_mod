#!/usr/bin/env python3
"""Retain a current required-boundary review; optionally refuse an unready gate."""
import argparse
import importlib.util
import io
import json
import platform
import sqlite3
import unittest
from pathlib import Path
import numpy

from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from rendering_required_boundaries import ROOT, PREFIX, BID, SID, analyze, require


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    parser.add_argument('--require-ready', action='store_true')
    args = parser.parse_args()
    path = Path(args.report)
    require(args.report.startswith(PREFIX) and '..' not in path.parts and not (ROOT/path).exists(),
            'Fresh owned SQLite report required')
    result = dict(schema=1, kind='rendering_required_boundaries_review', success=False, gate_ready=False)
    try:
        with Reader(ROOT) as reader:
            register = reader.json('research/runtime/coverage/register.json')
            behavior = next(v for v in register['behaviors'] if v['id'] == BID)
            scenario = next(v for v in register['scenarios'] if v['id'] == SID)
            sources = {n: reader.digest(n) for n in sorted(required_sources(behavior))}
        snapshots = []
        for name in ('tools/rendering_required_boundaries.py', 'tools/check-rendering-required-boundaries.py',
                     'tests/test-rendering-required-boundaries.py'):
            target = args.report.removesuffix('.json')+'-source-'+Path(name).name+'.bin'
            snapshots.append(dict(path=name, record=target,
                                  sha256=write_report(ROOT, target, (ROOT/name).read_bytes())))
        result.update(sources=sources, source_snapshots=snapshots)
        spec = importlib.util.spec_from_file_location('required_boundary_controls', ROOT/'tests/test-rendering-required-boundaries.py')
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        stream = io.StringIO()
        tests = unittest.TextTestRunner(stream=stream, verbosity=2).run(unittest.defaultTestLoader.loadTestsFromModule(module))
        log_name = args.report.removesuffix('.json')+'-controls.log'
        controls = dict(tests=tests.testsRun, failures=len(tests.failures), errors=len(tests.errors),
                        skipped=len(tests.skipped),
                        log=dict(record=log_name, sha256=write_report(ROOT, log_name, stream.getvalue().encode())))
        require(tests.wasSuccessful() and not tests.skipped and tests.testsRun >= 20,
                'Required-boundary controls failed: '+stream.getvalue())
        with Reader(ROOT) as reader:
            result = analyze(reader)
            require(all(reader.digest(n) == h for n, h in sources.items()), 'Review sources changed during execution')
        result.update(sources=sources, sources_stable=True, source_snapshots=snapshots, controls=controls,
            claims=[dict(behavior=BID, contract_sha256=behavior_contract(behavior, {v['id']:v for v in register['builds']}),
                         scenarios={SID:scenario_contract(scenario)})],
            python=platform.python_version(), sqlite=sqlite3.sqlite_version, numpy=numpy.__version__, task_complete=False)
    except Exception as exc:
        result.update(success=False, error=str(exc))
    digest = write_report(ROOT, args.report, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'], gate_ready=result['gate_ready'],
                         record=args.report, sha256=digest, pending=result.get('pending_scope_ids'),
                         controls=result.get('controls'), error=result.get('error'))), flush=True)
    return 1 if not result['success'] else 2 if args.require_ready and not result['gate_ready'] else 0


if __name__ == '__main__':
    raise SystemExit(main())
