#!/usr/bin/env python3
"""Retain fresh real-GL loss/restart controls from a separate RAM build."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

from coverage_evidence import Reader, write_report
from rendering_build_binding import native_runtime_source_paths
from rendering_pilot import require
from rendering_surface_loss import linked_sources

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/broader-player-acceptance-results-20261110/'
TESTS = (
    ('renderer-target', 'renderer/render-surface-loss-test'),
    ('worker-target', 'native-surface-loss-worker-test'),
    ('gui-target', 'native-gui-surface-loss-test'),
    ('terminal-restart', 'resident-terminal-restart-test'),
    ('ordinary-worker', 'native-render-worker-test'),
    ('quiescent-context', 'renderer/render-context-recovery-test'),
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts
            and not (ROOT/args.report).exists(), 'Fresh owned controls required')
    build = args.build.resolve()
    names = native_runtime_source_paths(ROOT) | {'tools/run-broader-player-controls.py',
        'tools/rendering_surface_loss.py', 'tests/test-rendering-surface-loss.py',
        'tests/test-broader-player-acceptance.py', 'tools/broader_player_acceptance.py'}
    for _, name in TESTS:
        binary = build/name
        names |= linked_sources(binary.parent, ROOT, binary.name)
    with Reader(ROOT) as reader:
        sources = {n: reader.digest(n) for n in sorted(names)}
    result = dict(schema=1, kind='broader_player_native_recovery_controls', success=False,
                  original_game_runs=0, sources=sources, commands=[],
                  physical_hardware_reset=False, current_application_acceptance_established=False)

    def retain(label, raw):
        name = args.report.removesuffix('.json')+'-'+label
        return dict(record=name, sha256=write_report(ROOT, name, raw), bytes=len(raw))

    result['producer'] = retain('producer.bin', Path(__file__).read_bytes())
    environment = dict(os.environ, QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1',
        __GLX_VENDOR_LIBRARY_NAME='mesa', LP_NUM_THREADS='4',
        __EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json')
    for key in ('MNM_SURFACE_LOSS_TEST', 'MNM_NATIVE_DEVICE_RECOVERY',
                'MNM_ACCEPTANCE_CAPTURE_DIRECTORY', 'MNM_ACCEPTANCE_CAPTURE_ID'):
        environment.pop(key, None)
    try:
        for label, name in TESTS:
            binary = build/name
            artifact = retain(label+'.bin', binary.read_bytes())
            selected = dict(environment)
            if label == 'ordinary-worker':
                selected['MNM_NATIVE_FRAME_TRACE'] = '1'
            run = subprocess.run(['xvfb-run', '-a', str(binary)], cwd=ROOT,
                env=selected, capture_output=True, timeout=120)
            row = dict(label=label, binary=artifact, returncode=run.returncode,
                       log=retain(label+'.log', run.stdout+run.stderr))
            result['commands'].append(row)
            require(hashlib.sha256(binary.read_bytes()).hexdigest() == artifact['sha256'],
                    'Tested native control binary changed')
            require(run.returncode == 0, 'Native control failed: '+label)
            if label == 'quiescent-context':
                require(run.stdout.strip() == b'Native contexts rebuilt twice; pixels, palettes, clipping, partial validity and outstanding frame leases preserved',
                        'Missing context recovery completion')
                row['result'] = dict(success=True, context_rebuilds=2)
            else:
                values = [json.loads(line) for line in run.stdout.splitlines() if line.startswith(b'{')]
                require(values and values[-1].get('success') is True,
                        'Missing native success: '+label)
                row['result'] = values[-1]
        result['python_controls'] = []
        for test in ('tests/test-rendering-surface-loss.py', 'tests/test-broader-player-acceptance.py'):
            run = subprocess.run(['python3', '-B', test], cwd=ROOT, capture_output=True, timeout=30)
            result['python_controls'].append(dict(test=test, returncode=run.returncode,
                log=retain(Path(test).stem+'.log', run.stdout+run.stderr)))
            require(run.returncode == 0, 'Reader/provenance control failed: '+test)
        result['success'] = True
    except Exception as error:
        result['error'] = str(error)
    with Reader(ROOT) as reader:
        result['sources_stable'] = all(reader.digest(n) == h for n, h in sources.items())
    result['success'] &= result['sources_stable']
    sha = write_report(ROOT, args.report, (json.dumps(result, indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'], sha256=sha,
                          error=result.get('error'), native_tests=len(result['commands']))), flush=True)
    return int(not result['success'])


if __name__ == '__main__':
    raise SystemExit(main())
