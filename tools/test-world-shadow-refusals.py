#!/usr/bin/env python3
"""Validate actual PE32 World refusal publication and native Qt recovery."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import secrets
import struct
import subprocess
import tempfile
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    if not os.environ.get('DISPLAY'):
        raise RuntimeError('Run under xvfb-run -a')
    spec = importlib.util.spec_from_file_location('native_world', ROOT/'tools/test-native-world.py')
    native = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(native)
    extra = ['tools/test-world-shadow-refusals.py', 'runtime/shadow/win32_min.h',
             'research/runtime/native-world-shadow-refusal.md', 'research/formats/world-frame-v1.md']

    def sources():
        return native.sources() | {path:sha(ROOT/path) for path in extra}

    parent = ROOT/'working/tests/world-shadow-refusals'
    parent.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(output, flush=True)
    fingerprints = sources()
    report = {'success':False, 'sources':fingerprints,
              'scope':'Actual PE32 producer transport refusal/recovery and synthetic native Qt session recovery; no original rendering equivalence',
              'original_pixels_used_as_native_inputs':False, 'original_work_bypassed':False}
    env = {k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(output/'wineprefix'), WINEDEBUG='-all')
    env.pop('WAYLAND_DISPLAY', None)
    build = ROOT/'working/build/world-frame'
    producer = ROOT/'working/build/scene-observer-selftest/world-stream-selftest.exe'

    def run(name, command, environment=None):
        with (output/(name+'.log')).open('x') as log:
            subprocess.run([str(c) for c in command], env=environment, stdout=log,
                           stderr=subprocess.STDOUT, timeout=180, check=True)

    try:
        run('producer-build', ['python3', ROOT/'tools/build-scene-observer.py', '--selftest'])
        run('configure', ['cmake', '-S', ROOT/'compat/legacy', '-B', build, '-DCMAKE_BUILD_TYPE=Debug'])
        run('native-build', ['cmake', '--build', build, '--target', 'world-live-session-test', 'world-channel-test', 'world-frame-test', '-j4'])
        run('native-tests', ['ctest', '--test-dir', build, '--output-on-failure', '-R',
                            '^(native-world-live-session|native-world-channel|native-world-frame)$',
                            '--output-junit', output/'ctest.xml'])
        cases = ET.parse(output/'ctest.xml').findall('.//testcase')
        if len(cases)!=3 or any(c.find('failure') is not None or c.find('skipped') is not None for c in cases):
            raise RuntimeError('Native World refusal regressions failed')
        run('prefix-copy', ['cp', '-a', '--reflink=auto', ROOT/'working/wineprefix-x86_64', env['WINEPREFIX']])
        run('wineboot', ['wineboot', '-u'], env)
        results = []
        for verify in [False, True]:
            name = 'verify' if verify else 'normal'
            channel = output/(name+'.bin')
            size = 128+2*(16+32*1024*1024+8*1024*1024)
            header = bytearray(128)
            header[:8] = b'MNMWCH01'
            struct.pack_into('<15I', header, 8, 1, size, secrets.randbelow(0xffffffff)+1, 0, 0, 0, 0, 0, 0,
                             int(verify), 0, 2, 32*1024*1024, 8*1024*1024, 0x40209ca7)
            with channel.open('xb') as f:
                f.write(header)
                f.truncate(size)
            run(name, ['wine', producer], env | {'MNM_WORLD_CHANNEL':'Z:'+str(channel).replace('/', '\\')})
            with channel.open('rb') as f:
                final = f.read(128)
            state, published, reason = (struct.unpack_from('<I', final, offset)[0] for offset in [20, 28, 40])
            if (state, published, reason)!=(3, 0 if verify else 3, 8 if verify else 2):
                raise RuntimeError('Producer refusal mode/state differs from asserted contract')
            results.append({'verify':verify, 'exit_code':0, 'final_state':state, 'published':published,
                            'final_reason':reason, 'header_sha256':hashlib.sha256(final).hexdigest()})
        if sources()!=fingerprints:
            raise RuntimeError('Refusal test sources changed during execution')
        report.update(success=True, sources_stable=True, ctests_passed=len(cases),
                      tests=[c.attrib['name'] for c in cases], producer_modes=results,
                      producer_sha256=sha(producer),
                      normal_refusals_remain_active=True, normal_recovery_publishes=True,
                      verify_refusal_is_fatal=True, structural_refusal_is_fatal=True,
                      normal_partial_payload_excluded=True,
                      producer_success_packet_scope='Transport publication only; placeholder raster record is not a native drawing fixture')
    finally:
        if Path(env['WINEPREFIX']).exists():
            subprocess.run(['wineserver', '-k'], env=env, timeout=10, check=False)
        (output/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(output/'report.json', flush=True)


if __name__=='__main__':
    main()
