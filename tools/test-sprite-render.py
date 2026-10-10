#!/usr/bin/env python3
"""Installed SPR -> AssetFile -> owned OpenGL mask/RGB565 -> original oracle.

No game launch or binary patches. Generated PNG/pixel assets stay in working/.
Requires the built preview, pinned MMSprite checkout, i386 reference prerequisites,
and xvfb-run. See research/runtime/native-sprite-rendering.md.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import tempfile

REPO = Path(__file__).resolve().parent.parent


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--preview', type=Path, default=REPO / 'working/build/sprite-render/mnm-sprite-preview')
    parser.add_argument('--claims', type=Path, help='Prospective source/scope declaration for this execution')
    args = parser.parse_args()
    preview = args.preview.resolve()
    source_paths = ['renderer/blit.cpp', 'renderer/blit.hpp', 'renderer/sprites/sprite.cpp',
                    'renderer/sprites/sprite.hpp', 'renderer/sprites/preview_main.cpp',
                    'assets/sprite_loader.cpp', 'assets/sprite_loader.hpp',
                    'tests/sprite-render-test.cpp', 'tools/test-sprite-render.py',
                    'tools/compare-mmsprite-binary.py', 'tests/sprite-binary-reference.cpp']
    sources = {path: sha((REPO / path).read_bytes()) for path in source_paths}
    declaration = json.loads(args.claims.read_text()) if args.claims else {'claims': [], 'sources': {}}
    for path, expected in declaration['sources'].items():
        target = (REPO / path).resolve()
        if Path(path).is_absolute() or not target.is_relative_to(REPO) or target.relative_to(REPO).parts[0] in {'original', 'working', '.git'}:
            raise ValueError('Unsafe prospective source: ' + path)
        if sha(target.read_bytes()) != expected:
            raise ValueError('Prospective source changed: ' + path)
        sources[path] = expected
    executable_hash = sha(preview.read_bytes())
    parent = REPO / 'working/tests/sprite-render'
    parent.mkdir(parents=True, exist_ok=True)
    evidence = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(f'Native sprite rendering evidence: {evidence}', flush=True)

    def verify(phase):
        result = subprocess.run([str(REPO / 'tools/original-manifest.sh'), 'verify'], capture_output=True, text=True, timeout=120)
        (evidence / f'original-{phase}.log').write_text(result.stdout + result.stderr)
        print(result.stdout, end='', flush=True)
        result.check_returncode()

    verify('before')
    try:
        reference = subprocess.run(['python3', str(REPO / 'tools/compare-mmsprite-binary.py')],
                                   capture_output=True, text=True, timeout=240, cwd=REPO)
        (evidence / 'reference.log').write_text(reference.stdout + reference.stderr)
        reference.check_returncode()
        match = re.search(r'^Sprite binary evidence: (.+)$', reference.stdout, re.M)
        if not match:
            raise ValueError('Reference runner omitted evidence path')
        reference_dir = Path(match[1])
        reference_bytes = (reference_dir / 'report.json').read_bytes()
        baseline = json.loads(reference_bytes)
        root = Path(baseline['root'])
        inputs = {file['path']: file['sha256'] for file in baseline['files']}
        for path, expected in inputs.items():
            if sha((root / path).read_bytes()) != expected:
                raise ValueError(f'Installed input changed: {path}')
        entries = []
        for i, case in enumerate(baseline['cases']):
            mixed = ''.join(c.upper() if n % 2 else c.lower() for n, c in enumerate(case['path'])).replace('/', '\\')
            entries.append({'path': ('c:\\MagicMayhem\\' if i % 2 else '') + mixed, 'frame': case['frame'],
                            'output': str(evidence / f'frame-{i:03}.565'), 'preview': str(evidence / f'frame-{i:03}.png')})
        manifest = evidence / 'manifest.json'
        manifest.write_text(json.dumps(entries, indent=2) + '\n')
        command = ['xvfb-run', '-a', 'env', 'QT_QPA_PLATFORM=xcb', 'LIBGL_ALWAYS_SOFTWARE=1',
                   str(preview), '--root', str(root), '--prefix', 'C:/MagicMayhem', '--manifest', str(manifest)]
        result = subprocess.run(command, capture_output=True, text=True, timeout=120, cwd=REPO)
        (evidence / 'preview.json').write_text(result.stdout)
        (evidence / 'preview.stderr').write_text(result.stderr)
        result.check_returncode()
        rendered = json.loads(result.stdout)
        if not rendered['rendered'] or len(rendered['files']) != len(entries):
            raise ValueError('Preview omitted frames')
        cases = []
        for i, (entry, case, actual) in enumerate(zip(entries, baseline['cases'], rendered['files'])):
            if actual['path'] != entry['path'] or actual['frame'] != entry['frame']:
                raise ValueError('Preview reordered or changed requests')
            if (actual['width'], actual['height']) != (case['width'] + 2, case['height'] + 2):
                raise ValueError('Preview dimensions differ')
            if actual['anchor_x'] - actual['origin_x'] != 1 or actual['anchor_y'] - actual['origin_y'] != 1:
                raise ValueError('Preview origin placement differs')
            native = Path(entry['output']).read_bytes()
            original = (reference_dir / f'case-{i:03}.565').read_bytes()
            if sha(original) != case['original_565_sha256'] or native != original or sha(native) != actual['output_sha256']:
                raise ValueError(f'Native OpenGL/original draw mismatch: {case["path"]}:{case["frame"]}')
            rgba = bytearray()
            for (pixel,) in struct.iter_unpack('<H', original):
                # Independent Surface2/DC measurements establish bit replication
                # for all 32/64/32 channel levels (surface-dib-ddraw.md). The old
                # floor-scaled policy is a distinct, superseded expectation.
                red, green, blue = pixel >> 11, (pixel >> 5) & 63, pixel & 31
                rgba.extend(((red << 3) | (red >> 2), (green << 2) | (green >> 4),
                             (blue << 3) | (blue >> 2), 255))
            if sha(rgba) != actual['presentation_rgba_sha256']:
                raise ValueError('OpenGL RGB565 presentation differs from CPU expansion')
            png = Path(entry['preview']).read_bytes()
            if not png.startswith(b'\x89PNG\r\n\x1a\n'):
                raise ValueError('Missing PNG preview')
            if actual['empty'] != case['empty'] or actual['draw_copies'] != (0 if case['empty'] else 1):
                raise ValueError('Empty/no-op or persistent-copy count differs')
            cases.append({'path': case['path'], 'frame': case['frame'], 'source_sha256': inputs[case['path']],
                          'storage': 'rgb565' if case['palette_index'] == -1 else 'indexed8',
                          'palette_index': case['palette_index'], 'empty': case['empty'],
                          'origin_x': actual['origin_x'], 'origin_y': actual['origin_y'],
                          'original_565_sha256': case['original_565_sha256'], 'native_565_sha256': sha(native),
                          'native_matches_original': True, 'presentation_rgba_sha256': sha(rgba),
                          'presentation_matches_cpu': True, 'preview_png_sha256': sha(png)})
        for path, expected in inputs.items():
            if sha((root / path).read_bytes()) != expected:
                raise ValueError(f'Installed input changed after rendering: {path}')
        if sha(preview.read_bytes()) != executable_hash or any(sha((REPO / path).read_bytes()) != expected for path, expected in sources.items()):
            raise ValueError('Preview executable or experiment sources changed during run')
        report = {'success': True, 'sources_stable': True, 'claims': declaration['claims'],
                  'validation': 'offline native loading and OpenGL rendering against isolated original x86 draws',
                  'evidence_directory': str(evidence.relative_to(REPO)),
                  'reference_directory': str(reference_dir.relative_to(REPO)),
                  'reference_report_sha256': sha(reference_bytes), 'reference_executable_sha256': baseline['executable_sha256'],
                  'preview_executable_sha256': executable_hash, 'source_sha256': sources, 'inputs_unchanged': True,
                  'original_palette_builder_executed': False, 'live_game_validated': False,
                  'indexed_conversion_policy': 'embedded RGB >> 3/2/3 into unshaded RGB565',
                  'presentation_policy': 'RGB565 5/6/5 bit replication; independently measured Surface2/DC expansion',
                  'driver': {key: rendered[key] for key in ('vendor', 'renderer', 'version')},
                  'summary': {'frames': len(cases), 'indexed_frames': sum(c['storage'] == 'indexed8' for c in cases),
                              'direct_frames': sum(c['storage'] == 'rgb565' for c in cases),
                              'empty_frames': sum(c['empty'] for c in cases),
                              'original_draw_matches': len(cases), 'presentation_matches': len(cases),
                              'uploads': rendered['uploads'], 'copies': rendered['copies'], 'remaining_surfaces': rendered['surfaces']},
                  'cases': cases}
        (evidence / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps(report['summary'], indent=2), flush=True)
    finally:
        verify('after')


if __name__ == '__main__':
    main()
