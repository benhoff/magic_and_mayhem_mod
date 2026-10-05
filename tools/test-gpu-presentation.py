#!/usr/bin/env python3
"""Record synthetic shared-texture presentation and command-replay checks."""
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
    'renderer/blit.cpp', 'renderer/blit.hpp', 'renderer/commands.cpp', 'renderer/commands.hpp',
    'renderer/capture.cpp', 'renderer/capture.hpp', 'renderer/CMakeLists.txt',
    'apps/qt-shell/gl_viewport.cpp', 'apps/qt-shell/gl_viewport.hpp',
    'apps/qt-shell/main.cpp', 'apps/qt-shell/CMakeLists.txt',
    'tests/gpu-presentation-test.cpp', 'tools/test-gpu-presentation.py',
]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def pack(*values):
    return struct.pack('<'+'I'*len(values), *values)


def fixture(gap=False):
    records = [
        (1, pack(1, 2, 2, 8, 0, 0, 0)+bytes([0, 1, 2, 3])),
        (4, pack(1, 0, 4)+bytes.fromhex('ff000000ff000000ffffffff')),
        (6, pack(1)),
        (2, pack(1, 0, 0, 2, 1)+bytes([3, 2])),
        (6, pack(1)),
        (7, pack(1)), (8, b''),
    ]
    if gap:
        records.insert(3, (9, pack(1)))
    return b'MNMCMD01'+pack(1, 16)+b''.join(pack(op, i+1, len(data))+data for i, (op, data) in enumerate(records))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path)
    parser.add_argument('--sanitized-build', type=Path, help='ASan/UBSan build containing gpu-presentation-test')
    args = parser.parse_args()
    build = args.build.resolve()
    parent = ROOT/'working/tests/gpu-presentation'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    initial = {name: sha(ROOT/name) for name in SOURCES}
    report = {'schema': 1, 'scope': 'Synthetic native integration on isolated Xvfb/Mesa; no original artifacts, live game or hardware performance claim.',
              'source_sha256': initial, 'run_directory': str(run.relative_to(ROOT)), 'runs': []}

    def execute(name, command, env=None, expected=0):
        result = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, text=True, timeout=60)
        (run/(name+'.stdout')).write_text(result.stdout)
        (run/(name+'.stderr')).write_text(result.stderr)
        report['runs'].append({'name': name, 'command': command, 'returncode': result.returncode,
                               'stdout_sha256': sha(run/(name+'.stdout')), 'stderr_sha256': sha(run/(name+'.stderr'))})
        assert result.returncode == expected, (name, result.returncode, result.stdout, result.stderr)
        return result

    try:
        frames = []
        for scale in ('1', '1.5'):
            env = dict(os.environ, QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', QT_SCALE_FACTOR=scale)
            result = execute('frames-'+scale, ['xvfb-run', '-a', str(build/'gpu-presentation-test')], env)
            frame = json.loads(result.stdout)
            assert frame['success'] and frame['rgba_readbacks'] == 0 and frame['gpu_viewport_uploads'] == 0
            frames.append(frame)
        if args.sanitized_build:
            sanitized = args.sanitized_build.resolve()/'gpu-presentation-test'
            for scale in ('1', '1.5'):
                env = dict(os.environ, QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', QT_SCALE_FACTOR=scale, ASAN_OPTIONS='detect_leaks=0')
                result = execute('sanitized-frames-'+scale, ['xvfb-run', '-a', str(sanitized)], env)
                frame = json.loads(result.stdout)
                assert frame['success'] and frame['rgba_readbacks'] == 0 and frame['gpu_viewport_uploads'] == 0
                frames.append(frame)
            report['sanitized_artifact_sha256'] = sha(sanitized)
        command_file = run/'commands.bin'
        command_file.write_bytes(fixture())
        gap_file = run/'gap.bin'
        gap_file.write_bytes(fixture(True))
        env = dict(os.environ, QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', QT_SCALE_FACTOR='1')
        result = execute('shell-replay', ['xvfb-run', '-a', str(build/'mnm-qt-shell'), '--commands', str(command_file), '--smoke-test'], env)
        assert '2 presentations, 0 native CHECK readbacks, 0 RGBA CHECK readbacks, 0 viewport uploads' in result.stderr
        execute('shell-gap-refusal', ['xvfb-run', '-a', str(build/'mnm-qt-shell'), '--commands', str(gap_file), '--smoke-test'], env, 8)
        assert initial == {name: sha(ROOT/name) for name in SOURCES}, 'Sources changed during validation'
        report.update(success=True, frames=frames,
                      native_integration={'all_match': True, 'full_frame_comparisons': sum(f['full_frame_comparisons'] for f in frames),
                                          'normal_and_hidpi': True, 'presentation_readbacks': 0, 'gpu_viewport_uploads': 0,
                                          'source_and_renderer_release': True, 'unshared_context_refused': True,
                                          'shell_command_replay': True, 'gap_refused': True})
    except Exception as error:
        report.update(success=False, error=str(error))
    report['artifacts'] = {str(path.relative_to(build)): sha(path) for path in
                           (build/'gpu-presentation-test', build/'mnm-qt-shell') if path.exists()}
    (run/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps({'success': report['success'], 'report': str(run/'report.json')}))
    if not report['success']:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
