#!/usr/bin/env python3
"""Record asset-free native PCM core execution and allocation/lifetime sanitizers."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]
SOURCES = ['audio/buffers.hpp', 'audio/buffers.cpp', 'audio/voices.cpp', 'audio/mixer.cpp',
           'audio/CMakeLists.txt', 'audio/README.md', 'research/runtime/native-audio-boundaries.md',
           'audio/wave_loader.cpp', 'audio/wave_loader.hpp', 'tests/test-audio-asset-input.py',
           'tests/audio-boundaries-test.cpp', 'tests/audio-buffers-test.cpp',
           'tests/audio-voice-state-test.cpp', 'tests/audio-mixer-test.cpp',
           'reconstruction/audio/dsound_setup.hpp', 'reconstruction/audio/dsound_setup.cpp',
           'reconstruction/audio/voice_contract.hpp', 'reconstruction/audio/voice_contract.cpp',
           'tools/test-native-audio-boundaries.py']


def main():
    parent = REPO / 'working/tests/native-audio-boundaries'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    hashes = {p: hashlib.sha256((REPO / p).read_bytes()).hexdigest() for p in SOURCES}
    build = REPO / 'working/build/audio-boundaries'
    for name, command in (
        ('configure', ['cmake', '-S', str(REPO / 'audio'), '-B', str(build)]),
        ('build', ['cmake', '--build', str(build), '--parallel', '4']),
        ('ctest', ['ctest', '--test-dir', str(build), '--output-on-failure']),
    ):
        result = subprocess.run(command, cwd=REPO, capture_output=True, text=True)
        (run / (name + '.log')).write_text(result.stdout + result.stderr)
        result.check_returncode()
    results = []
    for sanitize in (False, True):
        for fixture in ('boundaries', 'buffers', 'voice-state', 'mixer'):
            name = fixture + ('-sanitized' if sanitize else '-normal')
            binary = run / name
            command = ['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror', '-pedantic', '-g',
                       '-Iaudio', '-Ireconstruction/audio', 'tests/audio-' + fixture + '-test.cpp',
                       'audio/buffers.cpp', 'audio/voices.cpp', 'audio/mixer.cpp',
                       'reconstruction/audio/dsound_setup.cpp', 'reconstruction/audio/voice_contract.cpp',
                       '-o', str(binary)]
            if sanitize:
                command += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie']
            env = dict(os.environ, ASAN_OPTIONS='detect_leaks=1:halt_on_error=1', UBSAN_OPTIONS='halt_on_error=1')
            for suffix, cmd in (('build', command), ('execute', [str(binary)])):
                result = subprocess.run(cmd, cwd=REPO, env=env, capture_output=True, text=True)
                (run / (name + '-' + suffix + '.log')).write_text(result.stdout + result.stderr)
                result.check_returncode()
            results.append({'fixture': fixture, 'sanitized': sanitize, 'passed': True,
                            'binary_sha256': hashlib.sha256(binary.read_bytes()).hexdigest(), 'build_command': command,
                            'output': result.stdout.strip()})
            print(name + ': passed', flush=True)
    assert hashes == {p: hashlib.sha256((REPO / p).read_bytes()).hexdigest() for p in SOURCES}
    report = {'schema': 1, 'scope': 'Synthetic native PCM core: supported formats, parser, ownership, lock/voice lifetime, failure atomicity and PCM mixing.',
              'sources': hashes, 'tests_passed': True, 'sanitizers_passed': True,
              'standalone_ctest_passed': True, 'ctest_output': (run / 'ctest.log').read_text(),
              'fixtures': results, 'pcm_combinations': 24, 'allocation_operation_sweeps': 5,
              'game_assets_read': False, 'original_execution': False, 'device_output': False,
              'live_replacement': False, 'limitations': ['No OS allocation exhaustion or identity/ticket exhaustion execution.',
                  'No compressed audio, physical device, thread concurrency, live manager or original equivalence claim.']}
    with (run / 'report.json').open('x') as out:
        json.dump(report, out, indent=2)
        out.write('\n')
    print('Evidence: ' + str(run / 'report.json'))


if __name__ == '__main__':
    main()
