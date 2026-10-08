#!/usr/bin/env python3
"""Retain synthetic native preparation/Qt regressions and prospective claims."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
import xml.etree.ElementTree as ET

from coverage_claims import behavior_contract, required_sources, scenario_contract

ROOT = Path(__file__).resolve().parents[1]
BEHAVIORS = ('AS.resource-preparation', 'NR.world-preparation', 'NR.upload-preparation')
SCENARIOS = ('native-resource-preparation', 'native-world-preparation', 'native-world-upload-preparation')


def declaration(focused=False):
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behaviors = {b['id']: b for b in register['behaviors']}
    scenarios = {s['id']: s for s in register['scenarios']}
    builds = {b['id']: b for b in register['builds']}
    claims, paths = [], set()
    for bid in BEHAVIORS:
        behavior = behaviors[bid]
        selected = [scenarios[sid] for sid in SCENARIOS if bid in scenarios[sid]['behaviors']]
        claims.append({'behavior': bid, 'contract_sha256': behavior_contract(behavior, builds),
                       'scenarios': {s['id']: scenario_contract(s) for s in selected}})
        paths.update(required_sources(behavior))
        paths.update(p for s in selected for p in s['tests'])
    # Full dependency regression corpus, beyond the two newly claimed policies.
    for directory in (() if focused else ('assets', 'renderer', 'compat/legacy')):
        paths.update(str(p.relative_to(ROOT)) for p in (ROOT/directory).rglob('*')
                     if p.is_file() and (p.suffix in ('.cpp', '.hpp') or p.name == 'CMakeLists.txt'))
    return claims, {p: hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(paths)}


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--focused',action='store_true',help='Execute the four preparation/GUI/upload tests with their reviewed policy dependency closure')
    args=parser.parse_args()
    parent = ROOT/'working/tests/world-preparation'
    parent.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    build = ROOT/'working/build/world-frame'
    claims, sources = declaration(args.focused)
    report = {'success': False, 'sources': sources, 'claims': claims,
              'scope': 'Synthetic owned CPU preparation/adoption, resumable GPU uploads and Qt responsiveness; no installed game, original comparison, driver latency or live replacement.'}
    try:
        with (output/'execution.log').open('x') as log:
            for command in (
                    ['cmake', '-S', str(ROOT/'compat/legacy'), '-B', str(build), '-DCMAKE_BUILD_TYPE=Debug'],
                    ['cmake', '--build', str(build), *(['--target','resource-preparation-test','world-preparation-test','scene-upload-test','world-live-session-test'] if args.focused else []), '-j4'],
                    ['ctest', '--test-dir', str(build), '--output-on-failure', *(['-R','^(native-resource-preparation|native-world-preparation|opengl-scene-upload-budget|native-world-live-session)$'] if args.focused else []), '--output-junit', str(output/'ctest.xml')]):
                subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=240)
        cases = ET.parse(output/'ctest.xml').findall('.//testcase')
        if not cases or any(c.find('failure') is not None or c.find('skipped') is not None for c in cases):
            raise RuntimeError('Native preparation regression failure')
        current_claims, current_sources = declaration(args.focused)
        if current_sources != sources or current_claims != claims:
            raise RuntimeError('Preparation sources or declared contracts changed during execution')
        report.update(success=True, sources_stable=True, ctests_passed=len(cases),
                      tests=[c.attrib['name'] for c in cases],
                      outputs={c.attrib['name']: c.findtext('system-out', default='') for c in cases
                               if c.attrib['name'] in ('native-resource-preparation', 'native-world-preparation', 'native-world-live-session', 'opengl-scene-upload-budget')})
    finally:
        (output/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(output/'report.json', flush=True)


if __name__ == '__main__':
    main()
