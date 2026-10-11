#!/usr/bin/env python3
"""Compare the complete declared DRAW-03 canvas from source-owned startup inputs."""
import argparse
import json
import os
from pathlib import Path
import subprocess

from drawing_scene_corpus import CATALOG, ROOT, check, sha


def sources():
    paths = set()
    for directory in ('renderer', 'assets', 'compat/legacy', 'protocols/include', 'reconstruction/rendering'):
        for path in (ROOT / directory).rglob('*'):
            if path.is_file() and (path.suffix in {'.cpp', '.hpp', '.c', '.h', '.S', '.cmake'} or path.name == 'CMakeLists.txt'):
                paths.add(path)
    paths.update(p for p in (ROOT / 'tests/fixtures/drawing-scene').rglob('*') if p.is_file())
    paths.update(ROOT / p for p in ('tools/check-drawing-scene.py', 'tools/drawing_scene_corpus.py',
                                   'tools/retain-drawing-scene.py', 'tools/drawing_corpus.py',
                                   'tools/inspect-canvas-producers.py', 'tools/rendering_coverage.py',
                                   'tools/coverage_audit.py', 'tests/drawing-scene-test.cpp',
                                   'tests/test-drawing-scene.py', 'tests/resource-fixtures.hpp',
                                   'research/runtime/draw03-native-scene.md'))
    return {str(p.relative_to(ROOT)): sha(p.read_bytes()) for p in sorted(paths)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--corpus', default=CATALOG)
    parser.add_argument('--retention', default='NR.drawing-scene.retention-20261010')
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--headless', action='store_true')
    parser.add_argument('--test-executable', type=Path, help='Defaults to drawing-scene-test beside the replay executable')
    parser.add_argument('--claims', type=Path)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--require-complete', action='store_true')
    args = parser.parse_args()
    try:
        report = args.report.resolve()
        if report.exists() or not report.is_relative_to(ROOT) or any(report.is_relative_to((ROOT / p).resolve()) for p in ('original', '.git')):
            raise ValueError('Report must be new and within the repository outside original/.git')
        hashes = sources()
        claims = json.loads(args.claims.read_text()) if args.claims else None
        if claims:
            for name, expected in claims['sources'].items():
                if sha((ROOT / name).read_bytes()) != expected:
                    raise ValueError('Prospective scene source changed: ' + name)
                hashes[name] = expected
        test_binary = (args.test_executable or args.executable.parent / 'drawing-scene-test').resolve()
        test_hash = sha(test_binary.read_bytes())
        command = [str(test_binary)]
        environment = dict(os.environ)
        if args.headless:
            command = ['xvfb-run', '-a', *command]
            environment.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
        native_tests = subprocess.run(command, capture_output=True, timeout=60, env=environment)
        if native_tests.returncode or sha(test_binary.read_bytes()) != test_hash:
            raise ValueError('Native scene refusal/order tests failed: ' + native_tests.stderr.decode(errors='replace'))
        guards = subprocess.run(['python3', '-B', str(ROOT / 'tests/test-drawing-scene.py')], capture_output=True, timeout=60)
        if guards.returncode:
            raise ValueError('Scene corpus guards failed: ' + guards.stderr.decode(errors='replace'))
        if args.retention == 'NR.drawing-journey.retention-20261010':
            journey_guards = subprocess.run(['python3', '-B', str(ROOT / 'tests/test-drawing-journey.py')], capture_output=True, timeout=60)
            if journey_guards.returncode:
                raise ValueError('Journey corpus guards failed: ' + journey_guards.stderr.decode(errors='replace'))
        result = check(catalog_path=args.corpus, executable=args.executable, headless=args.headless, retention=args.retention)
        result['tests'] = {'native_scene': True, 'portable_guards': True, 'test_binary_sha256': test_hash,
                           'native_output': native_tests.stdout.decode(errors='replace'), 'guard_output': guards.stderr.decode(errors='replace')}
        if args.retention == 'NR.drawing-journey.retention-20261010':
            result['tests']['journey_guards'] = journey_guards.stderr.decode(errors='replace')
        if any(sha((ROOT / name).read_bytes()) != h for name, h in hashes.items()):
            raise ValueError('Scene comparison sources changed during execution')
        result.update(sources=hashes, sources_stable=True, inputs_stable=True)
        if claims:
            result['claims'] = claims['claims']
        report.parent.mkdir(parents=True, exist_ok=True)
        with report.open('x') as out:
            json.dump(result, out, indent=2)
            out.write('\n')
        print(json.dumps({k: result[k] for k in ('success', 'pixels_compared', 'mismatches', 'declared_canvas_complete', 'draw03_complete', 'artifacts')}))
        return int(not result['success'] or args.require_complete and not result['draw03_complete'])
    except (ValueError, OSError, KeyError, TypeError, StopIteration, subprocess.TimeoutExpired) as error:
        parser.exit(2, str(error) + '\n')


if __name__ == '__main__':
    raise SystemExit(main())
