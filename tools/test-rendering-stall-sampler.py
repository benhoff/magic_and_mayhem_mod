#!/usr/bin/env python3
"""Retain synthetic timing and actual parent-debugger controls in SQLite."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
from coverage_evidence import write_report
ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report', required=True)
    a = p.parse_args()
    prefix = 'research/runtime/rendering-watchdog-results-20261101/'
    if not a.report.startswith(prefix) or (ROOT/a.report).exists():
        p.error('New owned immutable report required')
    out = ROOT/'working/watchdog-sampler'/Path(a.report).stem
    out.mkdir(parents=True, exist_ok=False)
    (out/'.retain').write_text('Sampler controls, including negative results; retain exact bytes.\n')
    names = ['tools/rendering_stall_sampler.py', 'tools/sample-rendering-stall.py',
             'tools/test-rendering-stall-sampler.py', 'tests/test-rendering-stall-sampler.py']
    sources = {n: hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in names}
    report = dict(schema=1, kind='parent_debugger_sampler_controls', success=False,
                  sources=sources, original_artifacts_consumed=False,
                  live_game_validated=False, historical_watchdog_resolved=False)
    try:
        with (out/'tests.log').open('xb') as log:
            result = subprocess.run([sys.executable, '-B', ROOT/'tests/test-rendering-stall-sampler.py'],
                                    stdout=log, stderr=subprocess.STDOUT, timeout=60,
                                    env=dict(os.environ, MNM_STALL_TEST_OUTPUT=str(out)))
        report['exit_code'] = result.returncode
        report['success'] = result.returncode == 0
    except Exception as exc:
        report['error'] = str(exc)
    report['sources_stable'] = all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest() == sha for n, sha in sources.items())
    report['success'] &= report['sources_stable']
    report['artifacts'] = []
    for path in sorted(out.rglob('*')):
        if not path.is_file() or path.is_symlink():continue
        raw = path.read_bytes()
        if len(raw) > 4*1024**2:raise ValueError('Unexpectedly unbounded synthetic output')
        record = str(Path(a.report).with_suffix(''))+'-'+hashlib.sha256(str(path.relative_to(out)).encode()).hexdigest()[:20]+'.bin'
        sha = write_report(ROOT, record, raw)
        report['artifacts'].append(dict(path=str(path.relative_to(out)), record=record, sha256=sha, bytes=len(raw)))
    sha = write_report(ROOT, a.report, (json.dumps(report, indent=2)+'\n').encode())
    print(json.dumps(dict(success=report['success'], record=a.report, sha256=sha)), flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':raise SystemExit(main())
