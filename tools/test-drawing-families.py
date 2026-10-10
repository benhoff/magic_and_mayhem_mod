#!/usr/bin/env python3
"""Run bounded drawing-family regressions; retained reports never imply full takeover."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
FAMILIES = {
    'RI.copies': ('surfaces', 'Complete caller coverage and original copy suppression'),
    'RI.fills': ('surfaces', 'Original fill suppression and all retry/failure branches'),
    'RI.locked-writes': ('surfaces', 'JPEG and unclassified CPU writers'),
    'RI.gdi-text': ('surfaces', 'Active GDI operation/layout trace and painting suppression'),
    'RI.terrain': ('terrain', 'Live traversal/submission and water/overlay/picking routes'),
    'RI.sprites': ('sprites', 'Alternate/clipped dispatch and sustained gameplay'),
    'RI.effects': ('effects', 'Effect-to-queue-to-pixel attribution; tests cover upstream ANI state only'),
    'RI.ui': ('ui', 'Original HUD/tooltips/font/cursor composition and suppression'),
    'RI.movies': ('media', 'Installed enabled-movie playback and coherent World return'),
}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def sources():
    """Explicit conservative source provenance, including shared build inputs."""
    paths = {Path(__file__).relative_to(ROOT)}
    for directory in ('renderer', 'assets', 'reconstruction/rendering',
                      'reconstruction/animation', 'apps/qt-shell', 'runtime/render',
                      'runtime/scene', 'runtime/shadow', 'protocols', 'compat/legacy',
                      'tests', 'tools'):
        for path in (ROOT / directory).rglob('*'):
            if path.is_file() and (path.suffix in {'.c', '.cpp', '.h', '.hpp', '.S', '.py',
                                                  '.cmake', '.json', '.gz', '.txt'}):
                paths.add(path.relative_to(ROOT))
    return {str(p): sha(ROOT / p) for p in sorted(paths)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--skip-live', action='store_true', help='Leave fresh sprite takeover explicitly untested')
    args = parser.parse_args()
    declaration = json.loads(args.claims.read_text())
    fingerprints = sources()
    for path, expected in declaration['sources'].items():
        if fingerprints.get(path, sha(ROOT / path)) != expected:
            raise ValueError('Prospective source changed: ' + path)
        fingerprints[path] = expected
    parent = ROOT / 'working/tests/drawing-families'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1',
               ASAN_OPTIONS='detect_leaks=0', UBSAN_OPTIONS='halt_on_error=1')
    report = dict(schema=1, success=False, claims=declaration['claims'], sources=fingerprints,
                  scope='Bounded current-source regression executions across nine inventoried families. '
                        'Transport, native UI/media policy and effect state tests do not prove original '
                        'producer equivalence. Fresh direct-word takeover remains its existing subset. '
                        'No family completeness, additional bypass family or whole-game promotion.',
                  cases=[], families={}, original_work_bypassed=False,
                  new_live_bypass_families=[], overall_completion_percent=None)

    def run(name, command, group=None, timeout=600):
        start = time.monotonic()
        log = out / (name + '.log')
        try:
            with log.open('x') as stream:
                result = subprocess.run(list(map(str, command)), cwd=ROOT, env=env,
                                        stdout=stream, stderr=subprocess.STDOUT, timeout=timeout)
            status = result.returncode
            error = None
        except subprocess.TimeoutExpired:
            status, error = -1, 'timeout'
        case = dict(id=name, group=group, command=list(map(str, command)), returncode=status,
                    success=status == 0, seconds=round(time.monotonic()-start, 3),
                    log=str(log.relative_to(ROOT)), log_sha256=sha(log))
        if error:
            case['error'] = error
        report['cases'].append(case)
        print(name + ': ' + ('passed' if case['success'] else 'FAILED'), flush=True)
        return case

    before = run('original-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
    try:
        if not before['success']:
            raise ValueError('Immutable input manifest failed before tests')
        build = out / 'renderer'
        configured = run('renderer-configure', ['cmake', '-S', ROOT / 'renderer', '-B', build,
                                               '-DCMAKE_BUILD_TYPE=Debug'])
        built = configured['success'] and run('renderer-build', ['cmake', '--build', build, '-j4'])['success']
        if built:
            run('surface-regressions', ['ctest', '--test-dir', build, '--output-on-failure'], 'surfaces')
            run('drawing-recovery', ['xvfb-run', '-a', 'python3', ROOT / 'tools/test-render-drawing-recovery.py',
                                    build], 'surfaces', 900)
        run('terrain-submission', ['python3', ROOT / 'tools/test-terrain-submission.py'], 'terrain')
        run('effect-animation', ['python3', ROOT / 'tools/test-effect-animation-binding.py'], 'effects')
        preview_build = out / 'sprite-render'
        configured = run('sprite-configure', ['cmake', '-S', ROOT / 'renderer/sprites', '-B', preview_build])
        built = configured['success'] and run('sprite-build', ['cmake', '--build', preview_build, '-j4'])['success']
        if built:
            run('sprite-original-gpu', ['python3', ROOT / 'tools/test-sprite-render.py',
                                       '--preview', preview_build / 'mnm-sprite-preview'], 'sprites')
        run('media-bridge', ['python3', ROOT / 'tools/test-native-media.py'], 'media', 900)
        run('menu-controls', ['ctest', '--test-dir', ROOT / 'working/build/qt-shell', '--output-on-failure',
                              '--no-tests=error', '-R', '^qt-menu-(fonts|sprites|images)$'], 'ui')
        if not args.skip_live:
            capture = run('word-takeover-capture', ['xvfb-run', '-a', '-s', '-screen 0 1280x1024x24',
                          'python3', ROOT / 'tools/capture-scene-game.py', '--word-sprites', 'takeover',
                          '--samples', '4', '--claims', args.claims.resolve()], 'sprites', 600)
            # Use only this execution's explicit report path, never a latest-run lookup.
            if capture['success']:
                raw = (ROOT / capture['log']).read_text()
                paths = re.findall(r'^([^\n]+/report\.json)\s*$', raw, re.M)
                reports = [Path(p) for p in paths if Path(p).is_file()]
                scene = [p for p in reports if 'scene-observer' in str(p)]
                if len(scene) != 1:
                    raise ValueError('Capture did not identify exactly one scene report')
                directory = scene[0].parent / 'word-sprites'
                run('word-takeover-original', ['python3', ROOT / 'tools/test-word-sprites.py',
                                              '--live-directory', directory,
                                              '--report', out / 'word-original.json'], 'sprites')
        for fid, (group, gap) in FAMILIES.items():
            cases = [c for c in report['cases'] if c['group'] == group]
            report['families'][fid] = dict(test_group=group, cases=[c['id'] for c in cases],
                                          tested_subset_passed=bool(cases) and all(c['success'] for c in cases),
                                          full_family_validated=False, remaining=gap)
        report['success'] = all(c['success'] for c in report['cases']) and all(
            v['tested_subset_passed'] for v in report['families'].values())
    except Exception as error:
        report['error'] = str(error)
    finally:
        after = run('original-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
        changed = [p for p, h in fingerprints.items() if not (ROOT / p).is_file() or sha(ROOT / p) != h]
        report.update(sources_stable=not changed, changed_sources=changed,
                      success=report['success'] and after['success'] and not changed)
        with (out / 'report.json').open('x') as f:
            json.dump(report, f, indent=2)
            f.write('\n')
        print(out / 'report.json', flush=True)
    raise SystemExit(0 if report['success'] else 1)


if __name__ == '__main__':
    main()
