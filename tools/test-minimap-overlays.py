#!/usr/bin/env python3
"""Compare owned minimap overlays and caller-visible results with pinned PE32."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
EXPECTED = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SOURCES = ['renderer/minimap/minimap.hpp', 'renderer/minimap/overlays.hpp',
           'renderer/minimap/overlays.cpp', 'renderer/minimap/overlays/CMakeLists.txt',
           'tests/minimap-overlays-fixture.hpp', 'tests/minimap-overlays-native.cpp',
           'tests/minimap-overlays-original.cpp', 'tests/sprite-binary-reference.cpp',
           'tools/test-minimap-overlays.py', 'tools/original-manifest.sh']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def corpus():
    cases = []
    def add(kind, w, h, cx, cy, view, fmt, x, y, index=0, emphasized=0,
            flash=0, fog=0, pattern=0, viewport=(0, 0), extents=None):
        rw, rh = extents or (w, h)
        cases.append((kind, w, h, cx, cy, rw, rh, view, fmt, x, y, index,
                      emphasized, flash, fog, pattern, *viewport, len(cases) + 1))
    for w, h in [(3, 2), (4, 3), (8, 5), (16, 24), (31, 17), (64, 32)]:
        centres = [(0, 0), (w - 1, h - 1)]
        for cx, cy in centres:
            for view in range(4):
                for fmt in range(2):
                    for emphasized, flash in [(0, 0), (0, 1), (1, 0), (1, 1)]:
                        for index in range(9):
                            # Negative/far coordinates exercise cell-marker
                            # modulo; creature input uses normalized cells.
                            x = [-8 * w - 1, -1, 0, 1, w - 1, w, 8 * w + 1, w // 2, w * 3][index]
                            y = [-8 * h - 1, h, 0, -1, h - 1, 1, 8 * h + 1, h // 2, h * 3][index]
                            add(0, w, h, cx, cy, view, fmt, x, y, index, emphasized, flash,
                                extents=(w - (index % 2), h - (index % 2)))
                    for fog in range(2):
                        for pattern in range(4):
                            add(1, w, h, cx, cy, view, fmt, w // 2, h // 2,
                                fog=fog, pattern=pattern)
    # Complete small-grid source/centre projection with ordinary/ring markers.
    for cx in range(3):
        for cy in range(2):
            for x in range(3):
                for y in range(2):
                    for view in range(4):
                        for ring in range(2):
                            add(0, 3, 2, cx, cy, view, 0, x, y, emphasized=ring, flash=1)
    for w, h in [(16, 16), (16, 24), (31, 17), (32, 64), (64, 32), (128, 127)]:
        for view in range(4):
            for fmt in range(2):
                for viewport in [(0, 0), (127, 63), (128, 64), (255, 127), (640, 256)]:
                    add(2, w, h, 0, 0, view, fmt, 0, 0, viewport=viewport)
    return cases


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
    parent = ROOT / 'working/tests/minimap-overlays'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    report = {'success': False, 'sources': sources, 'claims': declaration['claims'],
              'scope': 'Four-view owned RGB565/RGB555 cell and creature markers plus selected camera-corner outlines. Independent source-only private PE32 comparison includes cell EAX destination offsets and creature dimension/camera origin results. Outline terrain is a byte-checked private no-op adapter; outer border and subsequent marker list are excluded. No live hook, simulation visibility/corner-vector generation, driver or drawing bypass claim.',
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
        run('configure', ['cmake', '-S', ROOT / 'renderer/minimap/overlays', '-B', build, '-DCMAKE_BUILD_TYPE=Debug'])
        run('build', ['cmake', '--build', build, '-j4'])
        run('native-refusals', ['ctest', '--test-dir', build, '--output-on-failure'])
        original, sanitized = out / 'original-reference', out / 'native-sanitized'
        run('original-build', ['g++', '-m32', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror', '-no-pie', ROOT / 'tests/minimap-overlays-original.cpp', '-o', original])
        run('sanitized-build', ['c++', '-std=c++17', '-O1', '-g', '-Wall', '-Wextra', '-Werror', '-fsanitize=address,undefined', '-fno-omit-frame-pointer', ROOT / 'tests/minimap-overlays-native.cpp', ROOT / 'renderer/minimap/overlays.cpp', '-o', sanitized])
        run('sanitized-refusals', [sanitized, '--selftest'])
        run('original', [original, pe, casefile, out / 'original.words'])
        run('native', [build / 'minimap-overlays-native', casefile, out / 'native.words'])
        run('sanitized', [sanitized, casefile, out / 'sanitized.words'])
        comparisons, words = [], 0
        with (out / 'original.words').open('rb') as originalOut, (out / 'native.words').open('rb') as nativeOut, (out / 'sanitized.words').open('rb') as checkedOut:
            for i, case in enumerate(cases):
                kind, w, h, _, _, rw, rh = case[:7]
                extent = max(w, h, rw, rh)
                count = (2 * extent + 71) * (extent + 64)
                length = count * 2 + 12
                a, b, c = originalOut.read(length), nativeOut.read(length), checkedOut.read(length)
                equal = len(a) == length and a == b == c
                comparisons.append({'case': i + 1, 'kind': kind, 'equal': equal,
                                    'pixels_equal': len(a) == length and a[:-12] == b[:-12] == c[:-12],
                                    'result_equal': len(a) == length and a[-12:] == b[-12:] == c[-12:]})
                words += count
            if originalOut.read(1) or nativeOut.read(1) or checkedOut.read(1):
                raise ValueError('Trailing overlay fixture output')
        (out / 'comparisons.json').write_text(json.dumps(comparisons, indent=2) + '\n')
        report.update(success=all(c['equal'] for c in comparisons), cases=len(cases),
                      equal=sum(c['equal'] for c in comparisons), words_compared=words,
                      cases_by_kind={str(k): sum(c[0] == k for c in cases) for k in range(3)},
                      results_compared=len(cases), orientations=4, formats=2, atomic_refusals=16,
                      original_source_storage_unchanged=True, original_object_mutations_checked=True,
                      sanitizer_passed=True, source_executable_sha256=EXPECTED,
                      original_entry_anchors={'0x5532f0': '83ec28535556'},
                      inputs={'cases.txt': sha(casefile)},
                      outputs={n: sha(out / n) for n in ('original.words', 'native.words', 'sanitized.words', 'comparisons.json')})
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
