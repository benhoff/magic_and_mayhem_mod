#!/usr/bin/env python3
"""Compare owned four-view minimap terrain with an isolated pinned PE32 routine."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCES = [
    'renderer/minimap/minimap.hpp', 'renderer/minimap/minimap.cpp',
    'renderer/minimap/CMakeLists.txt', 'tests/minimap-fixture.hpp',
    'tests/minimap-native.cpp', 'tests/minimap-original.cpp',
    'tests/sprite-binary-reference.cpp', 'tools/test-minimap-terrain.py',
    'tools/original-manifest.sh',
]
EXPECTED = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def corpus():
    result = []
    seed = 0
    # Exhaustive centres on small asymmetric grids, then boundary centres on
    # larger grids; fog disabled, mixed, all hidden and enabled/all visible.
    for w, h in [(2, 1), (2, 2), (4, 3), (4, 4), (6, 5), (8, 8),
                 (16, 3), (16, 16), (32, 17), (64, 64), (128, 127), (256, 256)]:
        centres = [(x, y) for x in range(w) for y in range(h)] if w <= 6 else [
            (0, 0), (w - 1, h - 1), (w // 2, h // 2)]
        for cx, cy in centres:
            for view in range(4):
                for fog in range(4):
                    seed += 1
                    result.append((w, h, cx, cy, view, fog, seed))
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    args = parser.parse_args()
    declaration = json.loads(args.claims.read_text())
    sources = {p: sha(ROOT / p) for p in SOURCES}
    for p, digest in declaration['sources'].items():
        if sha(ROOT / p) != digest:
            raise ValueError('Prospective source changed: ' + p)
        sources[p] = digest
    parent = ROOT / 'working/tests/minimap-terrain'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    report = {'success': False, 'sources': sources, 'claims': declaration['claims'],
              'scope': 'Owned RGB565 terrain placement in four orientations; bounded centres, even widths, fog retained from owned auxiliary history, positive padded rows. Isolated original raster/side-effect comparison; no live hook, driver, marker or gameplay equivalence.',
              'original_pixels_used_as_native_inputs': False, 'original_work_bypassed': False}
    env = {**os.environ, 'ASAN_OPTIONS': 'detect_leaks=0'}

    def run(name, command):
        with (out / (name + '.log')).open('x') as log:
            subprocess.run([str(x) for x in command], cwd=ROOT, env=env,
                           stdout=log, stderr=subprocess.STDOUT, check=True, timeout=180)

    run('original-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
    try:
        pe = ROOT / 'original/Arcane_Nocd/Chaos.exe'
        if sha(pe) != EXPECTED:
            raise ValueError('Pinned original executable changed')
        cases = corpus()
        casefile = out / 'cases.txt'
        casefile.write_text(''.join(' '.join(map(str, c)) + '\n' for c in cases))
        build = out / 'build'
        run('configure', ['cmake', '-S', ROOT / 'renderer/minimap', '-B', build, '-DCMAKE_BUILD_TYPE=Debug'])
        run('build', ['cmake', '--build', build, '-j4'])
        run('native-refusals', ['ctest', '--test-dir', build, '--output-on-failure'])
        original = out / 'original-reference'
        run('original-build', ['g++', '-m32', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror', '-no-pie', ROOT / 'tests/minimap-original.cpp', '-o', original])
        native = build / 'minimap-native'
        sanitized = out / 'native-sanitized'
        run('sanitized-build', ['c++', '-std=c++17', '-O1', '-g', '-Wall', '-Wextra', '-Werror', '-fsanitize=address,undefined', '-fno-omit-frame-pointer', ROOT / 'tests/minimap-native.cpp', ROOT / 'renderer/minimap/minimap.cpp', '-o', sanitized])
        run('sanitized-refusals', [sanitized, '--selftest'])
        run('original', [original, pe, casefile, out / 'original.565'])
        run('native', [native, casefile, out / 'native.565'])
        run('sanitized', [sanitized, casefile, out / 'sanitized.565'])
        results = []
        pixels = 0
        with (out / 'original.565').open('rb') as old, (out / 'native.565').open('rb') as new, (out / 'sanitized.565').open('rb') as checked:
            for w, h, cx, cy, view, fog, seed in cases:
                count = (2 * (w + h) + 15) * (w + h + 8)
                a, b, c = old.read(count * 2), new.read(count * 2), checked.read(count * 2)
                equal = len(a) == count * 2 and a == b == c
                results.append({'width': w, 'height': h, 'center': [cx, cy], 'orientation': view, 'fog': fog, 'seed': seed, 'words': count, 'equal': equal})
                pixels += count
            if old.read(1) or new.read(1) or checked.read(1):
                raise ValueError('Unexpected trailing fixture output')
        (out / 'comparisons.json').write_text(json.dumps(results, indent=2) + '\n')
        report.update(success=all(r['equal'] for r in results), cases=len(cases),
                      equal=sum(r['equal'] for r in results), words_compared=pixels,
                      orientations=4, atomic_refusals=15,
                      original_auxiliary_and_source_unchanged=True,
                      original_object_only_ff_invalidated=True, original_unlocks_per_case=1,
                      sanitizer_passed=True, source_executable_sha256=EXPECTED,
                      inputs={'cases.txt': sha(casefile)},
                      outputs={n: sha(out / n) for n in ('original.565', 'native.565', 'sanitized.565', 'comparisons.json')})
    except Exception as error:
        report['error'] = str(error)
    finally:
        run('original-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
        report['original_manifest_verified_before_after'] = True
        report['sources_stable'] = all(sha(ROOT / p) == digest for p, digest in sources.items())
        report['success'] = report['success'] and report['sources_stable']
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(out / 'report.json', flush=True)
    if not report['success']:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
