#!/usr/bin/env python3
"""Validate a connected synthetic Realm/Region/Character presentation journey."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

from coverage_claims import behavior_contract, required_sources, scenario_contract

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    args = parser.parse_args()
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior = next(b for b in register['behaviors'] if b['id'] == 'NP.campaign-menu-fixture-workflow')
    scenario = next(s for s in register['scenarios'] if s['id'] == 'campaign-menu-fixture-20261012')
    builds = {b['id']: b for b in register['builds']}
    claims = [{'behavior': behavior['id'], 'contract_sha256': behavior_contract(behavior, builds),
               'scenarios': {scenario['id']: scenario_contract(scenario)}}]
    paths = required_sources(behavior) | set(scenario['tests'])

    def fingerprints():
        return {p: hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(paths)}

    sources = fingerprints()
    declaration = json.loads(args.claims.read_text())
    if declaration['claims'] != claims or declaration['sources'] != sources:
        raise ValueError('Prospective declaration differs from current fixture scope/sources')
    parent = ROOT/'working/tests/campaign-menu-fixture'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    (out/'.retain').write_text('Scope-bound synthetic campaign menu fixture evidence.\n')
    (out/'claims.json').write_bytes(args.claims.read_bytes())
    print(out, flush=True)
    commands = []

    def run(command, name, timeout=180):
        command = list(map(str, command))
        commands.append(command)
        result = subprocess.run(command, capture_output=True, text=True, timeout=timeout,
                                env=dict(os.environ, QT_QPA_PLATFORM='offscreen'))
        (out/(name+'.log')).write_text(result.stdout+result.stderr)
        result.check_returncode()
        return result.stdout

    build = ROOT/'working/build/qt-shell'
    run(['cmake', '-S', ROOT/'apps/qt-shell', '-B', build], 'configure')
    run(['cmake', '--build', build, '--target', 'campaign-menu-fixture',
         'menu-character-bridge-test', 'realm-viewer-test', 'character-screen-test', '-j4'], 'build', 600)
    run(['ctest', '--test-dir', build, '-R', '^(qt-menu-character|qt-realm-viewer|qt-character-screen)$',
         '--output-on-failure', '--output-junit', out/'qt.xml'], 'ctest', 60)
    fixture = json.loads(run([build/'campaign-menu-fixture', '--capture', out/'realm.png'], 'journey', 30))
    if not (fixture['success'] and fixture['character_parent_returns'] == 6 and fixture['callers'] == 2
            and fixture['synthetic_campaign'] and not fixture['live_validated']
            and not fixture['original_callbacks_executed']):
        raise ValueError('Incomplete or overclaimed synthetic campaign journey')
    if fingerprints() != sources:
        raise ValueError('Fixture sources changed during execution')
    report = dict(success=True, fixture=fixture, qt_tests=4, source_stable=True,
                  execution_claims=claims, sources=sources, commands=commands,
                  scope=behavior['scope'], live_validated=False, original_work_bypassed=False,
                  artifacts={p.name: hashlib.sha256(p.read_bytes()).hexdigest()
                             for p in sorted(out.iterdir()) if p.is_file()})
    (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print('Connected synthetic campaign journey and3 companion Qt tests passed', flush=True)


if __name__ == '__main__':
    main()
