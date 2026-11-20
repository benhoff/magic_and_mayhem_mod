#!/usr/bin/env python3
"""Bind focused synthetic gameplay-FPS controls to prospective source/contract identities."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import write_report

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior = next(b for b in register['behaviors'] if b['id'] == 'HOST.gameplay-fps')
    scenario = next(s for s in register['scenarios'] if s['id'] == 'hosting.gameplay-fps')
    claims = [dict(behavior=behavior['id'], contract_sha256=behavior_contract(
        behavior, {b['id']: b for b in register['builds']}),
        scenarios={scenario['id']: scenario_contract(scenario)})]
    paths = required_sources(behavior) | set(scenario['tests'])
    paths.update(p.relative_to(ROOT).as_posix() for parent in ('runtime/menu', 'renderer')
                 for p in (ROOT/parent).rglob('*') if p.suffix in ('.cpp', '.hpp', '.c', '.h', '.inc'))
    paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'protocols/include/mnm').rglob('*.h'))
    paths.add('tests/menu-campaign-restart-test.cpp')
    paths.update(('apps/qt-shell/gl_viewport.cpp', 'apps/qt-shell/gl_viewport.hpp', 'tools/build-menu-observer.py'))

    def sources():
        return {p: hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(paths)}

    before = sources()
    parent = ROOT/'working/tests/gameplay-fps'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    report = dict(schema=1, success=False, claims=claims, source_sha256=before,
                  scope=scenario['scope'], runs=[], limitations=[
                      'Synthetic native/PE32 controls only; original-game telemetry remains pending.',
                      'No original equivalence, full mode/reset semantics or hardware timing claim.'])
    try:
        commands = [
            ['ctest', '--test-dir', str(args.build.resolve()), '-R',
             '^(qt-viewport-presentation(-hidpi)?|qt-menu-startup-resources|qt-game-hard-restart|qt-campaign-restart)$', '--output-on-failure'],
            ['python3', str(ROOT/'tools/test-campaign-observer.py')]]
        for command in commands:
            result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=90)
            report['runs'].append(dict(command=command, returncode=result.returncode,
                                       stdout=result.stdout, stderr=result.stderr))
            result.check_returncode()
        assert sources() == before, 'Sources changed during focused controls'
        report['source_stable'] = True
        report['artifacts'] = {p: hashlib.sha256((args.build/p).read_bytes()).hexdigest()
                               for p in ('mnm-qt-shell', 'viewport-presentation-test')}
        report['qt_ctest_log'] = (args.build/'Testing/Temporary/LastTest.log').read_text()
        report['success'] = True
    except (AssertionError, OSError, subprocess.SubprocessError) as error:
        report['error'] = str(error)
    report['run_directory'] = str(out.relative_to(ROOT))
    digest = write_report(ROOT, args.report, (json.dumps(report, indent=2)+'\n').encode())
    print(json.dumps(dict(success=report['success'], report=args.report, sha256=digest)))
    if not report['success']:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
