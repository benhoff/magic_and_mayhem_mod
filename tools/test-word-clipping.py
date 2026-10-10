#!/usr/bin/env python3
"""Compare bounded clipped direct-word pixels with both original backends and GPU drawing."""
import argparse
from collections import Counter
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def source_paths():
    paths = {'tools/test-word-clipping.py', 'tools/test-word-sprites.py',
             'tests/word-sprite-reference.cpp', 'tests/word-clipping-gpu.cpp',
             'tests/word-sprite-clipping/CMakeLists.txt',
             'protocols/include/mnm/render_stream_v3.h'}
    for directory in ('renderer', 'assets'):
        for path in (ROOT / directory).rglob('*'):
            if path.is_file() and (path.suffix in ('.c', '.cpp', '.h', '.hpp', '.cmake') or path.name == 'CMakeLists.txt'):
                paths.add(str(path.relative_to(ROOT)))
    return sorted(paths)


def clipped(frame, width, height, stride, ax, ay, before):
    result = bytearray(before)
    fw, fh, ox, oy = struct.unpack_from('<IIii', frame, 4)
    for y in range(fh):
        control, pixel = struct.unpack_from('<II', frame, 40 + y * 8)
        x, opaque = 0, False
        while x < fw:
            length = frame[control]
            control += 1
            if opaque:
                for i in range(length):
                    dx, dy = ax - ox + x + i, ay - oy + y
                    if 0 <= dx < width and 0 <= dy < height:
                        at = (dy * stride + dx) * 2
                        result[at:at + 2] = frame[pixel + i * 2:pixel + i * 2 + 2]
                pixel += length * 2
            x += length
            opaque = not opaque
    return bytes(result)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    args = parser.parse_args()
    declaration = json.loads(args.claims.read_text())
    sources = {name: sha(ROOT / name) for name in source_paths()}
    assert declaration['claims'], 'Prospective behavior claims required'
    for name, digest in declaration['sources'].items():
        assert sources.get(name) == digest, name
    parent = ROOT / 'working/tests/word-clipping'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(run, flush=True)
    report = {'schema': 1, 'success': False, 'sources': sources,
              'claims': declaration['claims'], 'original_sha256': BUILD,
              'experiment': str(run.relative_to(ROOT)),
              'scope': 'Synthetic positive direct-word contiguous frames; viewport origin zero, default clip globals; exact edges, partial edges/corners, hidden sprites, signed origins, padded canvas and two destination alignments. Pixel equivalence only; workspace, ABI, auxiliary planes, empty/malformed/indexed inputs, nonzero clip globals and live replacement excluded.',
              'original_work_bypassed': False, 'live_replacement': False}
    executable = ROOT / 'working/game-nocd/Chaos.exe'

    def command(argv, name, env=None):
        result = subprocess.run([str(arg) for arg in argv], capture_output=True,
                                text=True, timeout=300, env=env)
        log = run / (name + '.log')
        log.write_text(result.stdout + result.stderr)
        report.setdefault('commands', []).append({'argv': [str(a) for a in argv],
                                                 'returncode': result.returncode,
                                                 'log': str(log.relative_to(ROOT)),
                                                 'log_sha256': sha(log)})
        result.check_returncode()
        return result.stdout

    try:
        command([ROOT / 'tools/original-manifest.sh', 'verify'], 'manifest-before')
        assert sha(executable) == BUILD
        build = run / 'build'
        command(['cmake', '-S', ROOT / 'tests/word-sprite-clipping', '-B', build,
                 '-DCMAKE_BUILD_TYPE=Debug'], 'configure')
        command(['cmake', '--build', build, '--target', 'word-clipping-gpu', '--parallel', '4'], 'build')
        compiled_sources = set()
        for dependency_file in build.rglob('*.o.d'):
            for path in dependency_file.read_text().replace('\\\n', ' ').split():
                if path.startswith(str(ROOT) + '/'):
                    compiled_sources.add(str(Path(path).resolve().relative_to(ROOT)))
        report['compiled_project_sources'] = sorted(compiled_sources)
        assert compiled_sources <= sources.keys(), sorted(compiled_sources - sources.keys())
        obj, reference = run / 'word-raster.o', run / 'word-reference'
        command(['gcc', '-m32', '-O2', '-std=c11', '-Wall', '-Wextra', '-Werror',
                 '-c', ROOT / 'renderer/sprites/word_raster.c', '-o', obj], 'reference-object')
        command(['g++', '-m32', '-O2', '-std=c++17', '-fno-pie', '-no-pie',
                 '-Wall', '-Wextra', '-Werror', ROOT / 'tests/word-sprite-reference.cpp',
                 obj, '-o', reference], 'reference-build')
        spec = importlib.util.spec_from_file_location('fixtures', ROOT / 'tools/test-word-sprites.py')
        fixtures = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(fixtures)
        cases, manifest = [], []
        for seed in range(64):
            fw, fh = [1, 2, 3, 4, 7, 16, 129, 255][seed % 8], [1, 2, 3, 5][seed // 8 % 4]
            frame = fixtures.fixture(fw, fh, seed)
            width, height, stride = fw + 5, fh + 5, fw + 8
            placements = {'interior': (1, 1), 'exact-left': (0, 1), 'exact-top': (1, 0),
                          'exact-right': (width - fw, 1), 'exact-bottom': (1, height - fh),
                          'left': (-1, 1), 'top': (1, -1), 'right': (width - fw + 1, 1),
                          'bottom': (1, height - fh + 1), 'top-left': (-1, -1),
                          'bottom-right': (width - fw + 1, height - fh + 1),
                          'hidden-left': (-fw, 1), 'hidden-top': (1, -fh),
                          'hidden-right': (width, 1), 'hidden-bottom': (1, height)}
            before = b''.join(struct.pack('<H', (i * 7919 + seed * 257) & 65535)
                              for i in range(stride * height))
            for branch, (left, top) in placements.items():
                ax, ay = left - 3, top + 2  # fixture origins are (-3, 2)
                expected = clipped(frame, width, height, stride, ax, ay, before)
                sample = run / f'{seed}-{branch}.bin'
                sample.write_bytes(fixtures.sample(frame, width, height, stride, ax, ay,
                                                   0x197086, before, before, (seed % 2) * 2))
                original = []
                for backend in (0x197086, 0x196cb8):
                    raw = bytearray(sample.read_bytes())
                    struct.pack_into('<I', raw, 60, backend)
                    path = run / f'{seed}-{branch}-{backend:x}.bin'
                    path.write_bytes(raw)
                    output = run / f'{path.stem}-original.bin'
                    result = subprocess.run([str(reference), str(executable), str(path), str(output), 'original'],
                                            capture_output=True, timeout=5)
                    item = {'backend': hex(0x400000 + backend), 'returncode': result.returncode}
                    if result.returncode == 0:
                        rendered = output.read_bytes()
                        item.update(result=struct.unpack_from('<I', rendered)[0],
                                    pixels_match=rendered[68:] == expected,
                                    output_sha256=sha(output), workspace_sha256=hashlib.sha256(rendered[4:68]).hexdigest())
                    else:
                        item['error'] = result.stderr.decode(errors='replace')
                    original.append(item)
                gpu = run / f'{seed}-{branch}-gpu.bin'
                manifest.append({'input': str(sample), 'output': str(gpu)})
                cases.append({'seed': seed, 'branch': branch, 'canvas_pixels': stride * height,
                              'sample': str(sample.relative_to(ROOT)), 'sample_sha256': sha(sample),
                              'expected_sha256': hashlib.sha256(expected).hexdigest(), 'original': original,
                              'gpu_output': str(gpu.relative_to(ROOT))})
        manifest_path = run / 'gpu-inputs.json'
        manifest_path.write_text(json.dumps(manifest, indent=2) + '\n')
        report.update(cases=cases, branches=dict(Counter(c['branch'] for c in cases)),
                      original_matches=sum(o.get('pixels_match', False) for c in cases for o in c['original']))
        report['driver'] = json.loads(command(['xvfb-run', '-a', build / 'word-clipping-gpu', manifest_path],
                                             'gpu', dict(os.environ, QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')))
        for case in cases:
            output = ROOT / case['gpu_output']
            case['gpu_sha256'] = sha(output)
            case['gpu_match'] = case['gpu_sha256'] == case['expected_sha256']
        report.update(cases=cases, branches=dict(Counter(c['branch'] for c in cases)),
                      gpu_matches=sum(c['gpu_match'] for c in cases),
                      original_matches=sum(o.get('pixels_match', False) for c in cases for o in c['original']),
                      original_failures=[{'seed': c['seed'], 'branch': c['branch'], **o}
                                         for c in cases for o in c['original'] if not o.get('pixels_match', False)],
                      compared_pixels=sum(c['canvas_pixels'] for c in cases),
                      reference_sha256=sha(reference), gpu_binary_sha256=sha(build / 'word-clipping-gpu'))
        report['success'] = report['gpu_matches'] == len(cases) and report['original_matches'] == len(cases) * 2
    except Exception as error:
        report['error'] = str(error)
    finally:
        try:
            command([ROOT / 'tools/original-manifest.sh', 'verify'], 'manifest-after')
            assert sha(executable) == BUILD
        except Exception as error:
            report.update(success=False, manifest_error=str(error))
        report['changed_sources'] = [p for p, digest in sources.items() if sha(ROOT / p) != digest]
        report['sources_stable'] = not report['changed_sources']
        report['success'] = report['success'] and report['sources_stable']
        with (run / 'report.json').open('x') as target:
            json.dump(report, target, indent=2)
            target.write('\n')
        print(json.dumps({k: report.get(k) for k in ('success', 'gpu_matches', 'original_matches', 'error', 'sources_stable')}), flush=True)
        print(run / 'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
