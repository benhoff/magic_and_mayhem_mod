#!/usr/bin/env python3
"""Source-stable synthetic Qt/PE32 finite history execution; no original media."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
import xml.etree.ElementTree as ET
from coverage_claims import behavior_contract, required_sources, scenario_contract
from world_channel import create, validate_fresh

ROOT = Path(__file__).resolve().parents[1]


def declaration():
    spec = importlib.util.spec_from_file_location('preparation', ROOT/'tools/test-world-preparation.py')
    preparation = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(preparation)
    claims, sources = preparation.declaration()
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    builds = {b['id']: b for b in register['builds']}
    for bid, sid in [('NR.world-history-channel', 'native-world-history-channel'),
                     ('NR.world-live-history', 'native-world-live-history-synthetic')]:
        behavior = next(b for b in register['behaviors'] if b['id'] == bid)
        scenario = next(s for s in register['scenarios'] if s['id'] == sid)
        claims.append({'behavior': bid, 'contract_sha256': behavior_contract(behavior, builds),
                       'scenarios': {sid: scenario_contract(scenario)}})
        paths = required_sources(behavior) | set(scenario['tests'])
        sources.update({p: hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in paths})
    return claims, sources


def main():
    parent = ROOT/'working/tests/world-live-history'
    parent.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(output, flush=True)
    claims, sources = declaration()
    report = {'success': False, 'claims': claims, 'sources': sources,
              'scope': 'Synthetic Qt retained history and actual PE32 finite publication; no original artifacts, installed timings or raster equivalence.'}
    env = {k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(output/'wineprefix'), WINEDEBUG='-all')
    env.pop('WAYLAND_DISPLAY', None)
    build = ROOT/'working/build/world-frame'
    producer = ROOT/'working/build/scene-observer-selftest/world-stream-selftest.exe'

    def run(name, command, environment=None):
        with (output/(name+'.log')).open('x') as log:
            subprocess.run([str(c) for c in command], env=environment, stdout=log,
                           stderr=subprocess.STDOUT, timeout=300, check=True)

    try:
        run('configure', ['cmake', '-S', ROOT/'compat/legacy', '-B', build, '-DCMAKE_BUILD_TYPE=Debug'])
        run('build', ['cmake', '--build', build, '-j4'])
        run('ctest', ['ctest', '--test-dir', build, '--output-on-failure', '--output-junit', output/'ctest.xml'])
        cases = ET.parse(output/'ctest.xml').findall('.//testcase')
        if not cases or any(c.find('failure') is not None or c.find('skipped') is not None for c in cases):
            raise RuntimeError('Native history regressions failed')
        run('producer-build', ['python3', ROOT/'tools/build-scene-observer.py', '--selftest'])
        run('prefix-copy', ['cp', '-a', '--reflink=auto', ROOT/'working/wineprefix-x86_64', env['WINEPREFIX']])
        run('wineboot', ['wineboot', '-u'], env)
        modes = []
        for history, verify, mode in [(False, False, ''), (False, True, '')] + [
                (True, verify, mode) for verify in (False, True) for mode in ('', 'gap', 'missing', 'failure', 'changed')]:
            name = f'{"history" if history else "legacy"}-{int(verify)}-{mode or "complete"}'
            path = output/(name+'.bin')
            create(path, verify, 16 if history else 0)
            if validate_fresh(path) != history:
                raise RuntimeError('Python envelope mode changed')
            run(name, ['wine', producer], env | {'MNM_WORLD_CHANNEL': 'Z:'+str(path).replace('/', '\\'), 'MNM_HISTORY_TEST': mode})
            with path.open('rb') as channel:
                words = struct.unpack('<32I', channel.read(128))
            expected = ((2, 16, 0) if not mode else (3, 1 if mode == 'changed' else 0, {'gap':103, 'missing':103, 'failure':8, 'changed':104}[mode])) if history else (3, 0 if verify else 3, 8 if verify else 2)
            if (words[5], words[7], words[10]) != expected:
                raise RuntimeError('Producer final state differs: '+name)
            modes.append({'history': history, 'verify': verify, 'mode': mode or 'complete',
                          'state': words[5], 'published': words[7], 'reason': words[10], 'exit_code': 0})
        current_claims, current_sources = declaration()
        if current_sources != sources or current_claims != claims:
            raise RuntimeError('History sources or declared contracts changed during execution')
        report.update(success=True, sources_stable=True, ctests_passed=len(cases),
                      tests=[c.attrib['name'] for c in cases], producer_modes=modes,
                      outputs={c.attrib['name']:c.findtext('system-out', default='') for c in cases
                               if c.attrib['name'] in ('native-world-live-session', 'native-world-channel', 'opengl-scene-upload-budget')},
                      producer_sha256=hashlib.sha256(producer.read_bytes()).hexdigest())
    finally:
        if Path(env['WINEPREFIX']).exists():
            subprocess.run(['wineserver', '-k'], env=env, timeout=10, check=False)
        (output/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(output/'report.json', flush=True)


if __name__ == '__main__':
    main()
