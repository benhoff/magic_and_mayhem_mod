#!/usr/bin/env python3
"""Execute bounded native session lifecycle tests; no original acquisition."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--build', type=Path, default=ROOT/'working/build/draw03-session')
    p.add_argument('--claims', type=Path, required=True)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    report = a.report.resolve()
    if report.exists() or not report.is_relative_to(ROOT) or report.relative_to(ROOT).parts[0] in {'original', '.git'}:
        p.error('Choose a new report in the worktree')
    claims = json.loads(a.claims.read_text())
    paths = set(claims['sources'])
    for directory in ('assets', 'renderer', 'compat/legacy'):
        paths.update(str(f.relative_to(ROOT)) for f in (ROOT/directory).rglob('*')
                     if f.is_file() and (f.suffix in {'.cpp', '.hpp', '.c', '.h', '.S'} or f.name == 'CMakeLists.txt'))
    paths.update(['tests/resource-fixtures.hpp', 'tools/coverage_claims.py'])
    sources = {n: sha(ROOT/n) for n in sorted(paths)}
    if any(sources[n] != h for n, h in claims['sources'].items()):
        p.error('Prospective native session sources changed')
    parent = ROOT/'working/tests/draw03-session'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='lifecycle-', dir=parent))
    build = a.build.resolve()
    env = dict(os.environ, QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
    def execute(name, command, timeout=180):
        result = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, timeout=timeout)
        (run/(name+'.log')).write_bytes(result.stdout+result.stderr)
        if result.returncode:
            raise RuntimeError('Native session check failed: '+str(run/(name+'.log')))
        return result
    execute('configure', ['cmake', '-S', str(ROOT/'compat/legacy/drawing-scene'), '-B', str(build), '-DCMAKE_BUILD_TYPE=Debug'])
    execute('build', ['cmake', '--build', str(build), '--target', 'drawing-session-test', 'drawing-scene-test', 'mnm-drawing-session-replay', '--parallel', '4'])
    tested = execute('session', ['xvfb-run', '-a', str(build/'drawing-session-test')], 60)
    observed = json.loads(tested.stdout)
    if not observed['success'] or observed['remaining_surfaces'] != 0 or len(observed['cases']) != 12:
        raise RuntimeError('Incomplete native lifecycle checks')
    execute('scene-regression', ['xvfb-run', '-a', str(build/'drawing-scene-test')], 60)
    tested_runner = execute('runner', ['python3', str(ROOT/'tools/test-native-drawing-session-runner.py'),
                            '--binary', str(build/'mnm-drawing-session-replay'), '--artifacts', str(run/'runner')])
    runner = json.loads(tested_runner.stdout)
    if not runner['success'] or runner['remaining_surfaces'] != 0 or len(runner['cases']) != 30:
        raise RuntimeError('Incomplete native session runner checks')
    if any(sha(ROOT/n) != h for n, h in sources.items()):
        raise RuntimeError('Native session sources changed during execution')
    value = dict(schema=1, success=True, execution_performed=True, sources=sources, sources_stable=True,
                 claims=claims['claims'], native=observed, runner=runner, scene_regression=True,
                 binaries={n: sha(build/n) for n in ['drawing-session-test', 'drawing-scene-test', 'mnm-drawing-session-replay']},
                 artifacts=str(run.relative_to(ROOT)),
                 logs={str(f.relative_to(run)): sha(f) for f in sorted(run.rglob('*.log'))},
                 original_execution=False, original_pixels_used_as_native_inputs=False,
                 scope='Native lifecycle policy with owned synthetic inputs and real OpenGL storage: session/segment/frame order, generation changes, interruption, immutable leases, refusal, bounded fresh recovery and cleanup. Native owned-input CLI passes30cases, including6independently expected complete planes across6segments, hash/path/schema/catalogue/refusal checks and zero resources after actual execution failures. Asset-backed synthetic World completion and existing scene regression pass. No recovered transition/action policy, original session comparison, full presented journey, media integration or live replacement.')
    report.parent.mkdir(parents=True, exist_ok=True)
    with report.open('x') as out:
        json.dump(value, out, indent=2)
        out.write('\n')
    print(json.dumps({'success': True, 'lifecycle_cases': len(observed['cases']), 'runner_cases':len(runner['cases']), 'scene_regression': True, 'report': str(report)}))


if __name__ == '__main__':
    main()
