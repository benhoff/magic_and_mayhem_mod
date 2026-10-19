#!/usr/bin/env python3
"""Build and execute native checkpoint/lease recovery under actual OpenGL."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--qt-build', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    declaration = json.loads(args.claims.read_text())
    def stable():
        return all(hashlib.sha256((ROOT / p).read_bytes()).hexdigest() == h
                   for p, h in declaration['sources'].items())
    report = dict(schema=1, success=False, claims=declaration['claims'], sources=declaration['sources'],
                  scope=__doc__, physical_hardware_reset=False, original_game_executed=False,
                  in_flight_recovery=False, commands=[])
    def run(name, command, env=None, timeout=180):
        with (out / (name + '.log')).open('x') as log:
            result = subprocess.run(list(map(str, command)), cwd=ROOT,
                                    env=dict(os.environ) | (env or {}), stdout=log,
                                    stderr=subprocess.STDOUT, timeout=timeout)
        report['commands'].append(dict(name=name, exit_code=result.returncode))
        if result.returncode:
            raise RuntimeError(name + ': exit ' + str(result.returncode))
    try:
        if not stable():
            raise ValueError('Prospective inputs changed')
        run('renderer-build', ['cmake', '--build', args.build, '--target',
                              'render-context-recovery-test', 'gpu-production-queue-test', '-j2'])
        run('qt-build', ['cmake', '--build', args.qt_build, '--target', 'mnm-qt-shell', '-j2'], timeout=600)
        run('context-recovery', [args.build.resolve() / 'render-context-recovery-test'],
            {'MNM_RENDER_SERVICE': '1'})
        run('queue-recovery', [args.build.resolve() / 'gpu-production-queue-test'],
            {'MNM_NATIVE_DEVICE_RECOVERY': '1', 'MNM_RENDER_SERVICE': '1'})
        run('queue-ordinary', [args.build.resolve() / 'gpu-production-queue-test'],
            {'MNM_NATIVE_DEVICE_RECOVERY': '0'})
        report.update(success=True, context_rebuilds=2, different_sharing_group=True,
                      preserved=['pixels', 'palettes', 'partial_validity', 'clipping',
                                 'surface_handles', 'mutable_frames', 'immutable_frames'],
                      queue_backpressure_recovery=True, qt_application_build=True)
    except Exception as error:
        report['error'] = str(error)
    report['sources_stable'] = stable()
    report['success'] = report['success'] and report['sources_stable']
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: report.get(k) for k in ('success', 'sources_stable', 'error')}))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
