#!/usr/bin/env python3
"""Evaluate a pinned, unmodified MMSprite reader; no native/game replacement."""
import argparse
from collections import Counter
import hashlib
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile

REPO = Path(__file__).resolve().parents[1]
REVISION = '7d9bd05a2fb203d81bca11b808dc08b31fbd8c41'
URL = 'https://github.com/sapphire-bt/MMSprite.git'
LOCK = REPO / 'research/formats/mmsprite-source-lock.json'
SAMPLES = {
    'Creatures/RedCap.spr', 'Realms/Celtic/Forest/Terrain.spr',
    'Sprites/LOGO.spr', 'Sprites/timer.spr', 'Sprites/Buttons.spr',
    'Sprites/effects3.spr', 'Sprites/body text 640.sft',
}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def source_inventory(source):
    revision = subprocess.check_output(
        ['git', '-C', str(source), 'rev-parse', 'HEAD'], text=True).strip()
    if revision != REVISION:
        raise ValueError(f'Expected MMSprite {REVISION}, got {revision}')
    names = subprocess.check_output(
        ['git', '-C', str(source), 'ls-tree', '-r', '--name-only', 'HEAD'], text=True).splitlines()
    hashes = {}
    for name in names:
        expected = subprocess.check_output(['git', '-C', str(source), 'show', f'HEAD:{name}'])
        actual = (source / name).read_bytes()
        if actual != expected:
            raise ValueError(f'Modified upstream file: {name}')
        hashes[name] = digest(actual)
    return hashes


def worker(source, root):
    sys.dont_write_bytecode = True
    sys.path.insert(0, str(source / 'py'))
    from mm_files import FontFile, SpriteFile
    files = []
    selected_buckets = set()
    for path in sorted(root.rglob('*')):
        if path.suffix.lower() not in ('.spr', '.sft') or not path.is_file():
            continue
        if path.is_symlink():
            raise ValueError(f'Symlink input: {path}')
        data = path.read_bytes()
        relative = path.relative_to(root).as_posix()
        entry = {'path': relative, 'size': len(data), 'sha256': digest(data)}
        files.append(entry)
        try:
            stream = io.BytesIO(data)
            sprite = (SpriteFile if path.suffix.lower() == '.spr' else FontFile)(stream)
            bucket = (path.suffix.lower(), sprite.version, sprite.palette_count)
            sample = relative in SAMPLES or bucket not in selected_buckets
            selected_buckets.add(bucket)
            entry.update(version=sprite.version, palette_count=sprite.palette_count,
                         frame_count=sprite.frame_count, parsed=True,
                         synthetic_palette=sprite.palette_count == 0)
            frames = []
            entry['frames'] = frames
            for i, frame in enumerate(sprite.frames):
                frames.append({'index': i, 'offset': frame.offset, 'size': frame.size,
                               'width': frame.width, 'height': frame.height,
                               'centre_x': frame.centre_x, 'centre_y': frame.centre_y,
                               'name': frame.name, 'palette_index': frame.palette_index,
                               'extent_in_file': 0 <= frame.offset <= frame.offset + frame.size <= len(data),
                               'empty': not frame.has_valid_dimensions,
                               'palette_out_of_range': not 0 <= frame.palette_index < len(sprite.palettes)})
            entry['sampled'] = sample
            if sample and frames:
                indices = {0, len(frames) // 2, len(frames) - 1}
                indices.update(next((f['index'] for f in frames if f[key]), -1)
                               for key in ('empty', 'palette_out_of_range'))
                for i in sorted(indices - {-1}):
                    info = frames[i]
                    frame = sprite.frames[i]
                    try:
                        # Limits are evaluator policy, not upstream decoder behavior.
                        if frame.width * frame.height > 4_194_304:
                            raise ValueError('Sample exceeds evaluator pixel budget')
                        rows = frame.get_pixel_data(stream, sprite.palettes)
                        rgba = bytes(value for row in rows for value in row)
                        info['decode'] = {'status': 'decoded', 'rgba_sha256': digest(rgba),
                                          'bytes': len(rgba), 'rectangular': len(rows) == frame.height and
                                          all(len(row) == frame.width * 4 for row in rows),
                                          'transparent_pixels': sum(value == 0 for row in rows for value in row[3::4])}
                    except Exception as exc:
                        info['decode'] = {'status': 'error', 'type': type(exc).__name__, 'message': str(exc)}
        except Exception as exc:
            entry.update(parsed=False, error_type=type(exc).__name__, error=str(exc))
        if digest(path.read_bytes()) != entry['sha256']:
            raise ValueError(f'Input changed: {path}')
    print(json.dumps(files))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=REPO / 'working/MMSprite')
    parser.add_argument('--root', type=Path, default=REPO / 'working/game-clean')
    parser.add_argument('--worker', action='store_true', help=argparse.SUPPRESS)
    args = parser.parse_args()
    source, root = args.source.resolve(), args.root.resolve()
    if args.worker:
        worker(source, root)
        return
    if not root.is_dir():
        raise ValueError(f'Missing installed assets: {root}')
    hashes = source_inventory(source)
    lock = json.loads(LOCK.read_text())
    if lock['revision'] != REVISION or lock['tracked_file_sha256'] != hashes:
        raise ValueError('Upstream source does not match the committed source lock')
    parent = REPO / 'working/tests/mmsprite'
    parent.mkdir(parents=True, exist_ok=True)
    evidence = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(f'MMSprite evidence: {evidence}', flush=True)

    def verify(phase):
        completed = subprocess.run([str(REPO / 'tools/original-manifest.sh'), 'verify'],
                                   capture_output=True, text=True)
        (evidence / f'original-{phase}.log').write_text(completed.stdout + completed.stderr)
        print(completed.stdout, end='', flush=True)
        completed.check_returncode()

    verify('before')
    try:
        # Run the external reader in a bounded subprocess; do not patch/vendor it.
        completed = subprocess.run([sys.executable, str(Path(__file__).resolve()), '--worker',
                                    '--source', str(source), '--root', str(root)],
                                   capture_output=True, text=True, timeout=120)
        (evidence / 'reader.stderr').write_text(completed.stderr)
        completed.check_returncode()
        files = json.loads(completed.stdout)
        if not files:
            raise ValueError('No SPR/SFT input files')
        actual_names = {p.relative_to(root).as_posix() for p in root.rglob('*')
                        if p.is_file() and p.suffix.lower() in ('.spr', '.sft')}
        missing_samples = sorted(SAMPLES - {item['path'] for item in files})
        unchanged = all(digest((root / item['path']).read_bytes()) == item['sha256'] for item in files)
        unchanged = unchanged and actual_names == {item['path'] for item in files}
        if source_inventory(source) != hashes or not unchanged:
            raise ValueError('Source or installed inputs changed')
        samples = [f['decode'] for item in files for f in item.get('frames', []) if 'decode' in f]
        metadata = [frame for item in files for frame in item.get('frames', [])]
        report = {'upstream_url': URL, 'upstream_revision': REVISION, 'upstream_file_sha256': hashes,
                  'evaluator_sha256': digest(Path(__file__).read_bytes()), 'root': str(root),
                  'inputs_unchanged': unchanged, 'original_game_output_compared': False,
                  'native_decoder_implemented': False, 'live_replacement': False,
                  'summary': {'files': len(files), 'parsed': sum(f['parsed'] for f in files),
                              'input_bytes': sum(f['size'] for f in files),
                              'frames': sum(f.get('frame_count', 0) for f in files),
                              'empty_frames': sum(f['empty'] for f in metadata),
                              'raw_palette_indices_out_of_range': sum(f['palette_out_of_range'] for f in metadata),
                              'frame_extents_out_of_file': sum(not f['extent_in_file'] for f in metadata),
                              'sampled_files': sum(f.get('sampled', False) for f in files),
                              'sampled_frames': len(samples),
                              'decoded_frames': sum(f['status'] == 'decoded' for f in samples),
                              'decode_errors': dict(Counter(f['type'] for f in samples if f['status'] == 'error')),
                              'nonrectangular_frames': sum(f['status'] == 'decoded' and not f['rectangular'] for f in samples)},
                  'missing_representative_paths': missing_samples,
                  'files': files}
        (evidence / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        compact = dict(report)
        compact['files'] = [dict(item, frames=[frame for frame in item.get('frames', []) if 'decode' in frame])
                            for item in files]
        (evidence / 'summary.json').write_text(json.dumps(compact, indent=2) + '\n')
        print(json.dumps(report['summary'], indent=2), flush=True)
    finally:
        verify('after')


if __name__ == '__main__':
    main()
