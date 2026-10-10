#!/usr/bin/env python3
"""Compare selected original word backend state and ABI with a bounded reconstruction."""
import argparse
from collections import Counter
import hashlib
import importlib.util
import json
from pathlib import Path
import random
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
ORIGINAL = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
SOURCES = ['reconstruction/rendering/word_backend_state.c', 'reconstruction/rendering/word_backend_state.h',
           'tests/word-backend-state-reference.cpp', 'tests/word-backend-probe.S',
           'tools/test-word-backend-state.py', 'tools/test-word-sprites.py',
           'renderer/sprites/word_raster.c', 'renderer/sprites/word_raster.h',
           'runtime/scene/word_workspace.h', 'runtime/scene/word_route.c',
           'runtime/scene/word_entry.S', 'runtime/shadow/win32_min.h',
           'tools/build-word-sprites.py', 'tools/build-shadow-bridge.py', 'tools/original-manifest.sh']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def pixels(frame, width, height, stride, left, top, before, clip_left, clip_top):
    result = bytearray(before)
    fw, fh = struct.unpack_from('<II', frame, 4)
    if not fw or not fh:
        return bytes(result)
    for y in range(fh):
        control, colour = struct.unpack_from('<II', frame, 40 + y * 8)
        x, opaque = 0, False
        while x < fw:
            n = frame[control]
            control += 1
            if opaque:
                for i in range(n):
                    dx, dy = left + x + i, top + y
                    if clip_left <= dx < width and clip_top <= dy < height:
                        at = (dy * stride + dx) * 2
                        result[at:at + 2] = frame[colour + i * 2:colour + i * 2 + 2]
                colour += n * 2
            x += n
            opaque = not opaque
    return bytes(result)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    args = parser.parse_args()
    declaration = json.loads(args.claims.read_text())
    sources = {name: sha(ROOT / name) for name in SOURCES}
    assert declaration['claims']
    assert all(sources.get(p) == h for p, h in declaration['sources'].items())
    parent = ROOT / 'working/tests/word-backend-state'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(run, flush=True)
    report = {'schema': 1, 'success': False, 'claims': declaration['claims'], 'sources': sources,
              'original_sha256': ORIGINAL, 'experiment': str(run.relative_to(ROOT)),
              'scope': 'Selected No-CD direct-word backend workspace, cdecl argument-slot mutation, seeded integer registers/stack, defined XOR-zero return flags and DF; masked x87 control/status/active values, XMM0..7 and MXCSR. Positive contiguous synthetic frames, selected nonzero clip globals, hidden/oversized/exact-edge cases and empty dimensions. Actual native workspace helper checked only under existing strict unclipped admission. Floating instruction/data pointers/opcode, unused x87 slots, undefined AF, unmasked exceptions, original malformed handling, multithreading and live hook ABI/replacement excluded.',
              'original_work_bypassed': False, 'live_replacement': False}

    def command(argv, name):
        result = subprocess.run([str(a) for a in argv], capture_output=True, text=True, timeout=180)
        log = run / (name + '.log')
        log.write_text(result.stdout + result.stderr)
        report.setdefault('commands', []).append({'argv': [str(a) for a in argv], 'returncode': result.returncode,
                                                 'log': str(log.relative_to(ROOT)), 'log_sha256': sha(log)})
        result.check_returncode()
        return result.stdout

    executable = ROOT / 'working/game-nocd/Chaos.exe'
    try:
        command([ROOT / 'tools/original-manifest.sh', 'verify'], 'manifest-before')
        assert sha(executable) == ORIGINAL
        assembly = command(['objdump', '-d', '-Mintel', '--start-address=0x596cb8', '--stop-address=0x5974d0', executable], 'original-assembly')
        report['disassembly_sha256'] = hashlib.sha256(assembly.encode()).hexdigest()
        objects = []
        for name in ['reconstruction/rendering/word_backend_state.c', 'renderer/sprites/word_raster.c']:
            obj = run / (Path(name).stem + '.o')
            command(['gcc', '-m32', '-O2', '-std=c11', '-Wall', '-Wextra', '-Werror', '-pedantic',
                     '-MMD', '-MF', obj.with_suffix('.d'), '-c', ROOT / name, '-o', obj], obj.stem)
            objects.append(obj)
        reference = run / 'word-state-reference'
        command(['g++', '-m32', '-O2', '-std=c++17', '-fno-pie', '-no-pie', '-Wall', '-Wextra', '-Werror',
                 '-MMD', '-MF', run / 'reference.d', '-c', ROOT / 'tests/word-backend-state-reference.cpp',
                 '-o', run / 'reference.o'], 'reference-object')
        command(['gcc', '-m32', '-c', ROOT / 'tests/word-backend-probe.S', '-o', run / 'probe.o'], 'probe-build')
        command(['gcc', '-m32', '-c', ROOT / 'runtime/scene/word_entry.S', '-o', run / 'entry.o'], 'entry-build')
        command(['g++', '-m32', '-fno-pie', '-no-pie', run / 'reference.o', run / 'probe.o',
                 run / 'entry.o', *objects, '-o', reference], 'reference-build')
        compiled = set()
        for dependency in run.glob('*.d'):
            for path in dependency.read_text().replace('\\\n', ' ').split(':', 1)[1].split():
                if path.startswith(str(ROOT) + '/'):
                    compiled.add(str(Path(path).resolve().relative_to(ROOT)))
        assert compiled <= sources.keys(), sorted(compiled - sources.keys())
        report['compiled_project_sources'] = sorted(compiled)
        spec = importlib.util.spec_from_file_location('fixtures', ROOT / 'tools/test-word-sprites.py')
        fixtures = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(fixtures)
        cases, paths = [], []

        def add(frame, fw, fh, seed, width, height, stride, clip_left, clip_top, branch, left, top):
            before = b''.join(struct.pack('<H', (i * 7919 + seed * 257) & 65535) for i in range(stride * height))
            expected = pixels(frame, width, height, stride, left, top, before, clip_left, clip_top)
            rng = random.Random(seed * 991 + len(cases))
            workspace = struct.pack('<16I', *(rng.randrange(2**32) for _ in range(16)))
            for backend in (0x197086, 0x196cb8):
                data = bytearray(fixtures.sample(frame, width, height, stride, left - 3, top + 2, backend,
                                                before, expected, (seed % 2) * 2))
                struct.pack_into('<II', data, 64, clip_left, clip_top)
                struct.pack_into('<I', data, 76, seed % 4)  # private probe seed; v1 live samples keep this reserved word zero
                data[80:144] = workspace
                path = run / f'case-{len(cases):05d}.bin'
                path.write_bytes(data)
                cases.append({'seed': seed, 'branch': branch, 'backend': hex(0x400000 + backend),
                              'clip_left': clip_left, 'clip_top': clip_top, 'alignment': (seed % 2) * 2,
                              'fp_flags_seed': seed % 4, 'sample': str(path.relative_to(ROOT)), 'sample_sha256': sha(path)})
                paths.append(str(path))

        for seed in range(64):
            fw, fh = [1, 2, 3, 4, 7, 16, 129, 255][seed % 8], [1, 2, 3, 5][seed // 8 % 4]
            frame = fixtures.fixture(fw, fh, seed)
            for cl, ct in ((0, 0), (2, 2)):
                width, height, stride = fw + 5, fh + 5, fw + 8
                placements = {'interior': (cl + 1, ct + 1), 'exact-left': (cl, ct + 1),
                              'exact-top': (cl + 1, ct), 'exact-right': (width - fw, ct + 1),
                              'exact-bottom': (cl + 1, height - fh), 'left': (cl - 1, ct + 1),
                              'top': (cl + 1, ct - 1), 'right': (width - fw + 1, ct + 1),
                              'bottom': (cl + 1, height - fh + 1), 'top-left': (cl - 1, ct - 1),
                              'top-right': (width - fw + 1, ct - 1),
                              'bottom-left': (cl - 1, height - fh + 1),
                              'bottom-right': (width - fw + 1, height - fh + 1),
                              'hidden-left': (cl - fw, ct + 1), 'hidden-top': (cl + 1, ct - fh),
                              'hidden-right': (width, ct + 1), 'hidden-bottom': (cl + 1, height)}
                for branch, (left, top) in placements.items():
                    add(frame, fw, fh, seed, width, height, stride, cl, ct, branch, left, top)
            if fw >= 7 and fh >= 2:
                add(frame, fw, fh, seed, fw // 2, fh - 1, fw // 2 + 3, 0, 0, 'oversized-both-axes', -1, -1)
        for seed in range(4):
            for fw, fh in ((0, 0), (0, 2), (3, 0)):
                frame = struct.pack('<IIIii8sIII', 40, fw, fh, -3, 2, b'empty\0\0\0', 0xffffffff, 0, 0)
                for cl, ct in ((0, 0), (2, 2)):
                    add(frame, fw, fh, seed, 8, 8, 10, cl, ct, 'empty-dimensions', -1, -1)
        manifest = run / 'inputs.txt'
        manifest.write_text('\n'.join(paths) + '\n')
        report['cases'] = cases
        output = run / 'state-results.bin'
        command([reference, executable, manifest, output], 'original-model')
        data = output.read_bytes()
        assert data[:8] == b'MNMWST01' and struct.unpack_from('<II', data, 8) == (1, 256)
        assert len(data) == 16 + len(cases) * 256
        counts = Counter()
        failures = []
        for index, case in enumerate(cases):
            raw = data[16 + index * 256:16 + (index + 1) * 256]
            row = struct.unpack('<64I', raw)
            case.update(check_mask=row[0], state_record_sha256=hashlib.sha256(raw).hexdigest(),
                        native_entry_check_mask=row[60],
                        native_workspace_admitted=bool(row[7]), native_workspace_mismatch_mask=row[52],
                        old_zero_workspace_bug=bool(row[53]))
            counts['model_original_matches'] += row[0] == 1023
            counts['native_entry_original_matches'] += row[60] == 1023
            counts['native_workspace_cases'] += row[7]
            counts['native_workspace_matches'] += bool(row[7]) and row[52] == 0
            counts['confirmed_old_zero_workspace_errors'] += row[53]
            for bit, name in enumerate(('workspace', 'arguments', 'registers', 'stack', 'return_flags',
                                        'x87_control_status', 'x87_values', 'xmm', 'mxcsr', 'pixels_guards_source')):
                counts[name + '_matches'] += bool(row[0] & (1 << bit))
            if row[0] != 1023 or row[52] or row[60] != 1023:
                failures.append({'case': index, **case, 'original_words': list(row[8:24]),
                                 'model_words': list(row[24:40]), 'first_workspace_mismatch': row[6],
                                 'flags_before': row[4], 'flags_after': row[5],
                                 'x87_status_before': row[54], 'x87_status_after': row[55]})
        report.update(counts=dict(counts), failures=failures, case_count=len(cases),
                      branches=dict(Counter(c['branch'] for c in cases)),
                      state_results_sha256=sha(output), reference_sha256=sha(reference))
        command(['python3', ROOT / 'tools/build-word-sprites.py'], 'pe32-adapter-build')
        report['pe32_adapter_manifest'] = json.loads((ROOT / 'working/build/word-sprites/manifest.json').read_text())
        report['success'] = not failures and counts['confirmed_old_zero_workspace_errors'] > 0
    except Exception as error:
        report['error'] = str(error)
    finally:
        try:
            command([ROOT / 'tools/original-manifest.sh', 'verify'], 'manifest-after')
            assert sha(executable) == ORIGINAL
        except Exception as error:
            report.update(success=False, manifest_error=str(error))
        report['changed_sources'] = [p for p, h in sources.items() if sha(ROOT / p) != h]
        report['sources_stable'] = not report['changed_sources']
        report['success'] = report['success'] and report['sources_stable']
        with (run / 'report.json').open('x') as out:
            json.dump(report, out, indent=2)
            out.write('\n')
        print(json.dumps({k: report.get(k) for k in ('success', 'case_count', 'counts', 'error')}), flush=True)
        print(run / 'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
