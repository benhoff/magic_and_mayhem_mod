#!/usr/bin/env python3
"""Validate reusable command ring independently of original hooks and game media."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path, help='Renderer build with render-command-ring-gpu-test')
    args = parser.parse_args()
    if not os.environ.get('DISPLAY'):
        parser.error('Run under xvfb-run -a')
    names = ['protocols/schemas/render_commands-v2.json', 'protocols/include/mnm/render_commands_v2.h',
             'protocols/python/mnm_protocols/render_commands_v2.py', 'protocols/include/mnm/render_command_ring.h',
             'protocols/generate.py', 'tests/render-command-ring-test.c', 'tests/render-command-ring-gpu-test.cpp',
             'tools/test-render-command-ring.py', 'renderer/CMakeLists.txt', 'renderer/commands.cpp',
             'renderer/commands.hpp', 'renderer/command_consumer.cpp', 'renderer/command_state.hpp',
             'renderer/blit.cpp', 'renderer/blit.hpp', 'apps/qt-shell/gl_viewport.cpp', 'apps/qt-shell/gl_viewport.hpp']
    def sha(path):
        return hashlib.sha256(path.read_bytes()).hexdigest()
    sources = {p: sha(ROOT/p) for p in names}
    parent = ROOT/'working/tests/render-command-ring'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(run, flush=True)
    subprocess.run(['python3', 'protocols/generate.py', '--check'], cwd=ROOT, check=True)
    env = dict(os.environ, QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', ASAN_OPTIONS='detect_leaks=1')
    results = {}
    for label, flags in [('native', ['-O2']), ('sanitized', ['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer'])]:
        binary = run/label
        subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror', '-pedantic', *flags,
                        str(ROOT/'tests/render-command-ring-test.c'), '-o', str(binary)], check=True)
        result = subprocess.run([str(binary)], env=env, capture_output=True, text=True, timeout=45)
        (run/(label+'.log')).write_text(result.stdout+result.stderr)
        assert result.returncode == 0, result.stderr
        results[label] = json.loads(result.stdout)
        assert results[label]['success'] and results[label]['cross_process_bytes'] > 64*1024*1024
    result = subprocess.run([str(args.build.resolve()/'render-command-ring-gpu-test')], env=env,
                            capture_output=True, text=True, timeout=45)
    (run/'gpu.log').write_text(result.stdout+result.stderr)
    assert result.returncode == 0, result.stderr
    results['gpu'] = json.loads(result.stdout)
    assert results['gpu']['success'] and results['gpu']['frames'] == 4
    assert results['gpu']['ordinary_readbacks'] == results['gpu']['viewport_uploads'] == 0
    assert all(sha(ROOT/p) == h for p, h in sources.items()), 'Source changed during validation'
    report = dict(schema=1, success=True, sources=sources, scope='Native/sanitized x86 cross-process '
                  'byte ring plus four direct GPU frames through existing bounded decoder/consumer; '
                  'no PE32 hook/live game integration, queue scheduling or unbounded decoder claim.', **results)
    (run/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps(dict(success=True, report=str(run/'report.json'))), flush=True)


if __name__ == '__main__':
    main()
