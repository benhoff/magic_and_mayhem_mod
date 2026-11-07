#!/usr/bin/env python3
"""Bounded native Sharp and physical menu-publication regressions; no game runs."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time

from coverage_evidence import write_report

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-defect-fixes-results-20261106/'


def physical_child(binary, output):
    from rendering_menu_accessibility import AccessibleMenus, HostInput
    env = dict(os.environ, QT_QPA_PLATFORM='xcb', QT_ACCESSIBILITY='1',
               QT_LINUX_ACCESSIBILITY_ALWAYS_ON='1', QT_SCALE_FACTOR='1')
    child = subprocess.Popen([str(binary), '--physical', str(output)], env=env,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    inputs = None
    try:
        inputs = HostInput()
        access = AccessibleMenus(child.pid)
        for phase, name in enumerate(('Main action', 'Map action', 'Save action')):
            deadline = time.monotonic() + 8
            while time.monotonic() < deadline:
                if child.poll() is not None:
                    raise ValueError('Physical fixture exited before action: ' + name)
                try:
                    _, node = access.find(name, 'button')
                    if node['enabled']:
                        inputs.activate()
                        inputs.click(node['rect'])
                        break
                except ValueError:
                    pass
                time.sleep(.05)
            else:
                raise ValueError('Physical control readiness deadline: ' + name)
            deadline = time.monotonic() + 3
            while time.monotonic() < deadline:
                if output.exists():
                    value = json.loads(output.read_text())
                    if value['failed']:
                        raise ValueError('Physical request was lost: ' + name)
                    if value['phase'] > phase:
                        break
                time.sleep(.025)
            else:
                raise ValueError('Physical request completion deadline: ' + name)
        stdout, stderr = child.communicate(timeout=3)
        if child.returncode:
            raise ValueError('Physical fixture failed: ' + stderr)
        result = json.loads(output.read_text())
        if not (result['success'] and result['physical_presses'] == 3
                and result['physical_releases'] == 3
                and all(r['accepted_intent'] and r['request_before_stable'] == 0
                        for r in result['receipts'])):
            raise ValueError('Incomplete physical input/publication proof')
        print(json.dumps(result))
    finally:
        if inputs:
            inputs.close()
        if child.poll() is None:
            child.kill()
            child.communicate(timeout=3)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--report')
    parser.add_argument('--declaration', type=Path)
    parser.add_argument('--physical-child', type=Path)
    args = parser.parse_args()
    build = args.build.resolve()
    if args.physical_child:
        physical_child(build / 'menu-storage-bridge-test', args.physical_child)
        return 0
    if not args.report or not args.report.startswith(PREFIX) or '..' in Path(args.report).parts:
        parser.error('A fresh owned immutable --report is required')
    if (ROOT / args.report).exists() or not args.declaration:
        parser.error('Report must be new and --declaration must precede execution')
    declaration = json.loads(args.declaration.read_text())
    sources = declaration['sources']
    digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
    if not all(digest(ROOT / name) == value for name, value in sources.items()):
        raise ValueError('Prospective source declaration changed')
    binaries = ['viewport-presentation-test', 'gpu-presentation-test', 'menu-storage-bridge-test', 'mnm-qt-shell']
    artifacts = {name: digest(build / name) for name in binaries}
    result = dict(schema=1, kind='native_rendering_defect_regressions', success=False,
                  sources=sources, claims=declaration['claims'], binaries=artifacts,
                  runs=[], original_game_runs=0,
                  scope='Synthetic Qt/Mesa exact Sharp pixels and externally delivered XTest menu actions during controlled original-lane seqlock publication. No original-game outcome, historical failure retirement or acceptance-row promotion.')
    env = {k: v for k, v in os.environ.items() if not k.startswith(('MNM_', 'WINE'))}
    env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', LP_NUM_THREADS='4')

    def execute(name, command, expected=0, overrides=None):
        current = dict(env, **(overrides or {}))
        try:
            done = subprocess.run(command, cwd=ROOT, env=current, capture_output=True,
                                  text=True, timeout=60)
            row = dict(name=name, command=command, returncode=done.returncode,
                       stdout=done.stdout, stderr=done.stderr)
        except subprocess.TimeoutExpired as error:
            row = dict(name=name, command=command, returncode=None,
                       stdout=str(error.stdout), stderr=str(error.stderr), timeout=True)
        result['runs'].append(row)
        print(name, row['returncode'], flush=True)
        if row['returncode'] != expected:
            raise ValueError('Regression failed: ' + name + '\n' + row['stderr'])

    try:
        execute('menu-publication-guards', [str(build / 'menu-storage-bridge-test')],
                overrides={'QT_QPA_PLATFORM': 'offscreen'})
        for scale in ('1', '1.5'):
            for name in ('viewport-presentation-test', 'gpu-presentation-test'):
                execute(name + '-' + scale,
                        ['xvfb-run', '-a', '-s', '-screen 0 1920x1440x24', str(build / name)],
                        overrides={'QT_SCALE_FACTOR': scale})
        with tempfile.TemporaryDirectory(prefix='mnm-physical-publication-', dir='/tmp') as temporary:
            execute('physical-menu-publication',
                    ['xvfb-run', '-a', 'dbus-run-session', '--', sys.executable,
                     str(Path(__file__).resolve()), '--build', str(build),
                     '--physical-child', str(Path(temporary) / 'physical.json')])
        if not all(digest(ROOT / name) == value for name, value in sources.items()):
            raise ValueError('Sources changed during execution')
        if artifacts != {name: digest(build / name) for name in binaries}:
            raise ValueError('Executables changed during execution')
        result.update(success=True, sources_stable=True, binaries_stable=True)
    except Exception as error:
        result['error'] = str(error)
    sha = write_report(ROOT, args.report, (json.dumps(result, indent=2) + '\n').encode())
    print(json.dumps(dict(success=result['success'], record=args.report, sha256=sha)))
    return 0 if result['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
