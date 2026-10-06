#!/usr/bin/env python3
"""Verify portable surface fixtures against independent captured driver pixels."""
import argparse
import gzip
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import tempfile

ROOT = Path(__file__).resolve().parents[1]
DEFAULT = ROOT / 'tests/fixtures/surfaces/corpus.json'
BUILD_HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
spec = importlib.util.spec_from_file_location('surface_replay', ROOT / 'tools/replay-render-capture.py')
replay = importlib.util.module_from_spec(spec)
spec.loader.exec_module(replay)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def local(root, name):
    path = Path(name)
    if path.is_absolute() or '..' in path.parts or not path.parts:
        raise ValueError('Fixture paths must stay within the corpus directory')
    target = (root / path).resolve()
    if not target.is_relative_to(root.resolve()):
        raise ValueError('Fixture path escapes corpus directory')
    return target


def read_case(root, case):
    packed = local(root, case['capture']['path']).read_bytes()
    if sha(packed) != case['capture']['sha256']:
        raise ValueError('Compressed capture hash mismatch')
    with gzip.GzipFile(fileobj=io.BytesIO(packed)) as stream:
        data = stream.read(replay.MAX_CAPTURE + 1)
    if len(data) > replay.MAX_CAPTURE or sha(data) != case['capture']['raw_sha256']:
        raise ValueError('Raw capture size/hash mismatch')
    capture = replay.decode(data)
    proof = local(root, case['provenance']['path']).read_bytes()
    if sha(proof) != case['provenance']['sha256']:
        raise ValueError('Capture provenance hash mismatch')
    manifest = json.loads(proof)
    if (manifest.get('origin') != 'directdraw_opengl_presentation' or
            manifest.get('source_sha256') != BUILD_HASH or
            case['oracle'] != 'original_driver_post_call_lock' or
            case['original_sha256'] != BUILD_HASH):
        raise ValueError('Fixture lacks the admitted original-game capture provenance')
    for key in ('staged_sha256', 'dll_sha256'):
        value = manifest.get(key, '')
        if len(value) != 64 or any(c not in '0123456789abcdef' for c in value):
            raise ValueError('Missing staged executable or capture DLL identity')
    if (case['caller_return_va'] != capture['caller_return_va'] or
            case['operation'] != capture['operation'] or case['bits'] != capture['bits'] or
            case['expected_sha256'] != sha(capture['after'])):
        raise ValueError('Fixture metadata disagrees with captured operation/output')
    return data, capture


def check(corpus_path, backend='cpu', executable=None, headless=False):
    corpus = json.loads(corpus_path.read_text())
    if corpus.get('schema') != 1 or not corpus.get('cases'):
        raise ValueError('Unsupported or empty fixture corpus')
    matrix_path = ROOT / 'research/runtime/native-surface-operation-matrix.json'
    matrix = json.loads(matrix_path.read_text())
    operations = {row['id']: row for row in matrix['operations']}
    seen, covered, rows = set(), set(), []
    for case in corpus['cases']:
        if case['id'] in seen:
            raise ValueError('Duplicate fixture ID')
        seen.add(case['id'])
        if not case['matrix_ids'] or any(key not in operations for key in case['matrix_ids']):
            raise ValueError('Unregistered fixture operation')
        # Version 1 captures establish only bounded opaque RGB565 BltFast copies.
        if set(case['matrix_ids']) - {'SURF.copy.opaque', 'SURF.format.rgb565'}:
            raise ValueError('Capture cannot establish the claimed operation semantics')
        data, capture = read_case(corpus_path.parent, case)
        if (capture['caller_return_va'] != '0x0058c5be' or
                capture['operation'] != 'BltFast' or capture['bits'] != 16 or
                capture['masks'] != (0xf800, 0x07e0, 0x001f) or capture['key'] is not None):
            raise ValueError('Fixture lies outside the admitted opaque RGB565 scope')
        cpu = replay.replay(capture)
        row = {'id': case['id'], 'matrix_ids': case['matrix_ids'],
               'capture_sha256': sha(data), 'expected_sha256': sha(capture['after']),
               'cpu_comparison': replay.compare(capture, cpu),
               'changed_pixels': sum(capture['before'][at:at+2] != capture['after'][at:at+2]
                                     for at in range(0, len(capture['after']), 2))}
        if backend == 'opengl':
            parent = ROOT / 'working/tests/surface-fixtures'
            parent.mkdir(parents=True, exist_ok=True)
            output = Path(tempfile.mkdtemp(prefix=case['id']+'-', dir=parent))
            path = output / 'capture.bin'
            path.write_bytes(data)
            native, metadata = replay.render_gl(path, output, executable, headless)
            if metadata.get('capture_sha256') != sha(data):
                raise ValueError('OpenGL consumed a different capture')
            row['artifacts'] = str(output.relative_to(ROOT))
            row['opengl_comparison'] = replay.compare(capture, native)
            row['opengl_vs_cpu'] = {'matching': native == cpu}
            rgb = replay.ppm(capture, native).split(b'\n', 3)[3]
            rgba = b''.join(rgb[at:at+3] + b'\xff' for at in range(0, len(rgb), 3))
            row['presentation_comparison'] = {
                'matching': sha(rgba) == metadata.get('presentation_rgba_sha256'),
                'reference_rgba_sha256': sha(rgba)}
            row['opengl'] = metadata
        row['success'] = row['cpu_comparison']['matching'] and (
            backend == 'cpu' or row['opengl_comparison']['matching'] and
            row['opengl_vs_cpu']['matching'] and row['presentation_comparison']['matching'])
        rows.append(row)
        covered.update(case['matrix_ids'])
    required = {key for key, row in operations.items() if row['requirement'] == 'required'}
    sources = ['tools/check-surface-fixtures.py', 'tools/replay-render-capture.py',
               'tests/test-surface-fixtures.py',
               'research/runtime/native-surface-operation-matrix.json',
               'renderer/blit.cpp', 'renderer/blit.hpp', 'renderer/capture.cpp',
               'renderer/capture.hpp', 'renderer/replay_main.cpp']
    for case in corpus['cases']:
        sources.extend(str(local(corpus_path.parent, case[field]['path']).relative_to(ROOT))
                       for field in ('capture', 'provenance'))
    sources.append(str(corpus_path.resolve().relative_to(ROOT)))
    return {'schema': 1, 'success': all(row['success'] for row in rows), 'backend': backend,
            'original_sha256': BUILD_HASH, 'cases': rows,
            'scope': 'Five retained game-driver RGB565 opaque BltFast captures at return PC 0x0058c5be; offline comparison only.',
            'covered_matrix_ids': sorted(covered), 'missing_required_matrix_ids': sorted(required-covered),
            'milestone_complete': required <= covered,
            'sources': {path: sha((ROOT/path).read_bytes()) for path in sorted(set(sources))}}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--corpus', type=Path, default=DEFAULT)
    parser.add_argument('--backend', choices=('cpu', 'opengl'), default='cpu')
    parser.add_argument('--gl-executable', type=Path)
    parser.add_argument('--headless', action='store_true')
    parser.add_argument('--report', type=Path, required=True, help='New JSON report; refuses overwrite')
    args = parser.parse_args()
    if args.report.exists():
        parser.exit(2, 'Refusing to overwrite report\n')
    try:
        result = check(args.corpus.resolve(), args.backend, args.gl_executable, args.headless)
        args.report.parent.mkdir(parents=True, exist_ok=True)
        with args.report.open('x') as out:
            json.dump(result, out, indent=2)
            out.write('\n')
        print(json.dumps({key: result[key] for key in ('success', 'backend', 'covered_matrix_ids',
                                                     'missing_required_matrix_ids', 'milestone_complete')}))
        return 0 if result['success'] else 1
    except (ValueError, KeyError, TypeError, OSError, EOFError) as exc:
        parser.exit(2, str(exc) + '\n')


if __name__ == '__main__':
    raise SystemExit(main())
