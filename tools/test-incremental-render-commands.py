#!/usr/bin/env python3
"""Record bounded progressive execution and timer-driven Qt replay evidence."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCES = [
    'renderer/blit.cpp', 'renderer/blit.hpp', 'renderer/capture.cpp', 'renderer/capture.hpp',
    'renderer/commands.cpp', 'renderer/commands.hpp', 'renderer/command_state.hpp',
    'renderer/command_consumer.cpp', 'renderer/CMakeLists.txt',
    'apps/qt-shell/gl_viewport.cpp', 'apps/qt-shell/gl_viewport.hpp',
    'apps/qt-shell/command_replay.cpp', 'apps/qt-shell/command_replay.hpp',
    'tests/render-command-consumer-test.cpp', 'tests/command-replay-shell.cpp',
    'tools/test-incremental-render-commands.py',
]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def words(*values):
    return struct.pack('<'+'I'*len(values), *values)


def fixture(poison=False, gap=False):
    records = [(1, words(1, 2, 2, 8, 0, 0, 0)+bytes([0, 1, 2, 3])),
               (4, words(1, 0, 4)+bytes.fromhex('ff000000ff000000ffffffff'))]
    for i in range(40):
        pixels = bytes([3, 2, 1, 0]) if i%2 else bytes([0, 1, 2, 3])
        records += [(2, words(1, 0, 0, 2, 2)+pixels),
                    (5, words(1)+(bytes(4) if poison else pixels)), (6, words(1))]
    records += [(7, words(1)), (8, b'')]
    if gap:
        records.insert(34, (9, words(1)))
    return b'MNMCMD01'+words(1, 16)+b''.join(words(op, i+1, len(data))+data for i, (op, data) in enumerate(records))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path)
    parser.add_argument('--sanitized-build', type=Path)
    parser.add_argument('--qt-shell', type=Path, help='Also check the production --commands entry point')
    args = parser.parse_args()
    parent = ROOT/'working/tests/incremental-render-commands'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    initial = {path: sha(ROOT/path) for path in SOURCES}
    report = {'schema': 1, 'scope': 'Synthetic decoded incremental native integration and Qt replay service; no live transport, original artifacts or performance equivalence.',
              'source_sha256': initial, 'run_directory': str(run.relative_to(ROOT)), 'runs': [], 'artifacts': {}}

    def execute(name, command, expected=0, sanitized=False):
        env = dict(os.environ, QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', QT_SCALE_FACTOR='1')
        if sanitized:
            env['ASAN_OPTIONS'] = 'detect_leaks=0'
        result = subprocess.run(['xvfb-run', '-a', *map(str, command)], cwd=ROOT, env=env,
                                capture_output=True, text=True, timeout=60)
        (run/(name+'.stdout')).write_text(result.stdout)
        (run/(name+'.stderr')).write_text(result.stderr)
        report['runs'].append({'name': name, 'command': list(map(str, command)), 'returncode': result.returncode,
                               'stdout_sha256': sha(run/(name+'.stdout')), 'stderr_sha256': sha(run/(name+'.stderr'))})
        assert result.returncode == expected, (name, result.returncode, result.stdout, result.stderr)
        return result

    try:
        tests = []
        builds = [('normal', args.build.resolve())]
        if args.sanitized_build:
            builds.append(('sanitized', args.sanitized_build.resolve()))
        for label, build in builds:
            executable = build/'render-command-consumer-test'
            result = execute(label+'-partitions', [executable], sanitized=label=='sanitized')
            data = json.loads(result.stdout)
            assert data['success'] and data['ordinary_readbacks'] == 0 and data['gpu_viewport_uploads'] == 0
            tests.append(data)
            report['artifacts'][label+'-consumer'] = sha(executable)
        files = {}
        for name, poison, gap in [('valid', False, False), ('poisoned', True, False), ('gap', False, True)]:
            files[name] = run/(name+'.bin')
            files[name].write_bytes(fixture(poison, gap))
        for label, build in builds:
            shell = build/'command-replay-test-shell'
            skipped = execute(label+'-qt-skipped', [shell, files['valid']], sanitized=label=='sanitized')
            assert '40 presentations, 0 native CHECK readbacks, 0 RGBA CHECK readbacks, 0 viewport uploads; 4 batches, 40 skipped native checks' in skipped.stderr
            verified = execute(label+'-qt-verified', [shell, files['valid'], '--command-checks'], sanitized=label=='sanitized')
            assert '40 presentations, 40 native CHECK readbacks, 0 RGBA CHECK readbacks, 0 viewport uploads; 4 batches, 0 skipped native checks' in verified.stderr
            # No smoke reference for poisoned diagnostics: ordinary service checked
            # above; explicit comparison must refuse this stream.
            execute(label+'-qt-poison-refusal', [shell, files['poisoned'], '--command-checks'], 8, label=='sanitized')
            execute(label+'-qt-gap-refusal', [shell, files['gap']], 8, label=='sanitized')
            report['artifacts'][label+'-qt-service'] = sha(shell)
        if args.qt_shell:
            shell = args.qt_shell.resolve()
            result = execute('production-qt-shell', [shell, '--commands', files['valid'], '--smoke-test'])
            assert '4 batches, 40 skipped native checks' in result.stderr
            result = execute('production-qt-shell-verified', [shell, '--commands', files['valid'], '--command-checks', '--smoke-test'])
            assert '40 native CHECK readbacks' in result.stderr
            report['artifacts']['production-qt-shell'] = sha(shell)
        assert initial == {path: sha(ROOT/path) for path in SOURCES}, 'Sources changed during execution'
        report.update(success=True, tests=tests, native_integration={
            'all_match': True, 'full_frame_comparisons': sum(t['full_frame_comparisons'] for t in tests),
            'partitioned_sessions': sum(t['partitioned_sessions'] for t in tests),
            'failure_cases': sum(t['failure_cases'] for t in tests), 'ordinary_readbacks': 0,
            'gpu_viewport_uploads': 0, 'timer_batches': 4, 'diagnostics_explicit': True,
            'qt_service_replay': True, 'normal_and_sanitized': bool(args.sanitized_build),
            'production_qt_shell_checked': bool(args.qt_shell),
        })
    except Exception as error:
        report.update(success=False, error=str(error))
    (run/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps({'success': report['success'], 'report': str(run/'report.json')}))
    if not report['success']:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
