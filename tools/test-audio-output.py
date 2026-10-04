#!/usr/bin/env python3
"""Verify Qt PCM output using fixtures; optional bounded host-device smoke test."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--device-test', action='store_true',
                        help='probe the default device and play a quiet two-second tone with restart')
    args = parser.parse_args()
    parent = REPO / 'working/tests/audio-output'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    build = REPO / 'working/build/audio-output'
    print(f'Evidence: {root}', flush=True)
    commands = [
        ('configure', ['cmake', '-S', str(REPO / 'audio'), '-B', str(build)]),
        ('build', ['cmake', '--build', str(build), '--parallel', '4']),
        ('ctest', ['ctest', '--test-dir', str(build), '--output-on-failure']),
    ]
    if args.device_test:
        commands += [('probe', [str(build / 'mnm-audio-output'), '--probe']),
                     ('tone', [str(build / 'mnm-audio-output'), '--tone'])]
    for name, command in commands:
        try:
            result = subprocess.run(command, text=True, capture_output=True,
                                    timeout=15 if name in ('probe', 'tone') else 180)
        except subprocess.TimeoutExpired as error:
            (root / (name+'.log')).write_bytes((error.stdout or b'')+(error.stderr or b''))
            raise
        (root / (name+'.log')).write_text(result.stdout+result.stderr)
        print(result.stdout+result.stderr, end='', flush=True)
        result.check_returncode()
    sources = ['audio/qt_output.hpp', 'audio/qt_output.cpp', 'audio/output_main.cpp',
               'audio/CMakeLists.txt', 'audio/buffers.hpp', 'audio/buffers.cpp',
               'audio/voices.cpp', 'audio/mixer.cpp', 'tests/audio-output-test.cpp']
    report = {'scope': 'native Qt push PCM output adapter', 'tests_passed': True,
              'native_fixture_sha256': hashlib.sha256((build / 'audio-output-test').read_bytes()).hexdigest(),
              'output_cli_sha256': hashlib.sha256((build / 'mnm-audio-output').read_bytes()).hexdigest(),
              'source_sha256': {name: hashlib.sha256((REPO / name).read_bytes()).hexdigest() for name in sources},
              'host_backend_delivery_validated': args.device_test,
              'speaker_audibility_confirmed': False, 'game_assets_read': False,
              'live_game_validated': False, 'game_hooks_installed': False,
              'checks': ['stereo_int16_rate_negotiation', 'unsupported_and_null_devices',
                         'partial_byte_writes', 'zero_write_backpressure', 'bounded_blocks',
                         'byte_exact_pcm', 'write_errors', 'queue_discard',
                         'repeated_stop_and_failed_start']}
    (root / 'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(f'Qt audio output evidence: {root / "report.json"}')


if __name__ == '__main__':
    main()
