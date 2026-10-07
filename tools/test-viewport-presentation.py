#!/usr/bin/env python3
"""Record display-only scaling/fullscreen and input regression checks on Xvfb/Mesa."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

from importlib.util import module_from_spec, spec_from_file_location

ROOT = Path(__file__).resolve().parents[1]
spec = spec_from_file_location('gpu_fixtures', ROOT/'tools/test-gpu-presentation.py')
gpu = module_from_spec(spec)
spec.loader.exec_module(gpu)
SOURCES = sorted(set(gpu.SOURCES + [
    'apps/qt-shell/viewport_presentation.cpp', 'apps/qt-shell/viewport_presentation.hpp',
    'apps/qt-shell/command_replay.cpp', 'apps/qt-shell/command_replay.hpp',
    'apps/qt-shell/input_forwarder.cpp', 'apps/qt-shell/input_forwarder.hpp',
    'apps/qt-shell/input_state.cpp', 'apps/qt-shell/input_state.hpp',
    'apps/qt-shell/window_host.cpp', 'apps/qt-shell/window_host.hpp',
    'tests/viewport-presentation-test.cpp', 'tests/qt-input-test.cpp',
    'tests/gl-viewport-upload-test.cpp', 'tools/test-viewport-presentation.py',
]))


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path)
    args = parser.parse_args()
    build = args.build.resolve()
    parent = ROOT/'working/tests/viewport-presentation'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    initial = {name: sha(ROOT/name) for name in SOURCES}
    report = {'schema': 1, 'scope': 'Synthetic Qt display policy and input, isolated Xvfb/Mesa at DPI scales 1 and 1.5; no original artifacts, gameplay equivalence or hardware performance claim.',
              'source_sha256': initial, 'run_directory': str(run.relative_to(ROOT)), 'runs': []}
    binaries = ['mnm-qt-shell', 'viewport-presentation-test', 'gpu-presentation-test', 'qt-input-test', 'gl-viewport-upload-test']
    report['artifacts'] = {name: sha(build/name) for name in binaries}

    def execute(name, arguments, env, expected=0):
        command = ['xvfb-run', '-a', '-s', '-screen 0 1920x1440x24']+arguments
        result = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, text=True, timeout=60)
        (run/(name+'.stdout')).write_text(result.stdout)
        (run/(name+'.stderr')).write_text(result.stderr)
        report['runs'].append({'name': name, 'command': command, 'returncode': result.returncode,
                               'stdout_sha256': sha(run/(name+'.stdout')), 'stderr_sha256': sha(run/(name+'.stderr'))})
        assert result.returncode == expected, (name, result.returncode, result.stdout, result.stderr)
        return result

    try:
        presentations, regressions = [], []
        commands = run/'commands.bin'
        commands.write_bytes(gpu.fixture())
        for scale in ('1', '1.5'):
            env = dict(os.environ, QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', QT_SCALE_FACTOR=scale)
            presentations.append(json.loads(execute('presentation-'+scale, [str(build/'viewport-presentation-test')], env).stdout))
            regressions.append(json.loads(execute('gpu-'+scale, [str(build/'gpu-presentation-test')], env).stdout))
            execute('input-'+scale, [str(build/'qt-input-test')], env)
            execute('uploads-'+scale, [str(build/'gl-viewport-upload-test')], env)
            for mode in ('sharp', 'smooth', 'integer'):
                result = execute('replay-'+scale+'-'+mode, [str(build/'mnm-qt-shell'), '--commands', str(commands), '--smoke-test', '--fullscreen', '--scaling', mode], env)
                assert '2 presentations, 0 native CHECK readbacks, 0 RGBA CHECK readbacks, 0 viewport uploads' in result.stderr
            execute('shell-fullscreen-'+scale, [str(build/'mnm-qt-shell'), '--fullscreen', '--scaling', 'smooth', '--smoke-test'], env)
        for name, options in [('bad-scaling', ['--scaling', 'wrong']), ('embedded', ['--renderer', 'native', '--fullscreen']), ('live-menus', ['--live-menus', '--scaling', 'smooth'])]:
            execute(name, [str(build/'mnm-qt-shell'), '--smoke-test']+options, env, 2)
        assert all(p['success'] for p in presentations+regressions)
        assert all(p['onscreen_comparisons']==17 and p['empty_frame_controls'] for p in presentations)
        assert initial == {name: sha(ROOT/name) for name in SOURCES}, 'Sources changed during validation'
        assert report['artifacts'] == {name: sha(build/name) for name in binaries}, 'Binaries changed during validation'
        report.update(success=True, presentations=presentations, gpu_regressions=regressions,
                      native_integration={'all_match': True, 'normal_and_hidpi': True,
                                          'full_frame_comparisons': sum(p['full_frame_comparisons'] for p in presentations+regressions),
                                          'presentation_readbacks': 0, 'gpu_scaling_uploads': 0,
                                          'logical_input_preserved': True, 'fullscreen_restore': True,
                                          'onscreen_comparisons': sum(p['onscreen_comparisons'] for p in presentations),
                                          'empty_frame_controls': True,
                                          'f11_consumed': True, 'escape_forwarded': True, 'cli_modes': True})
    except Exception as error:
        report.update(success=False, error=str(error))
    (run/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps({'success': report['success'], 'report': str(run/'report.json')}))
    if not report['success']:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
