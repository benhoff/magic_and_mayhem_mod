"""Prepare bounded source-only native JPEG pixels; never consumes game outputs."""
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def decoder():
    out = ROOT/'working/build/jpeg-pixel-cache'
    subprocess.run(['cmake', '-S', ROOT/'assets', '-B', out, '-DBUILD_TESTING=OFF', '-DCMAKE_BUILD_TYPE=Release'], check=True)
    subprocess.run(['cmake', '--build', out, '--target', 'mnm-jpeg-word-cache', '-j2'], check=True)
    return out/'mnm-jpeg-word-cache'


def stage(root, executable=None):
    game = root/'game'
    files = sorted(p for p in game.rglob('*') if p.is_file() and p.suffix.lower() in ('.jpg', '.jpeg'))
    if not files or len(files)>4096:
        raise ValueError('Native JPEG cache requires a bounded installed JPEG corpus')
    codec = executable or decoder()
    entries, inputs, size = [], {}, 32
    with tempfile.TemporaryDirectory(prefix='jpeg-pixels-', dir=root) as temp:
        for i, p in enumerate(files):
            if p.is_symlink() or not p.resolve().is_relative_to(game.resolve()):
                raise ValueError('JPEG input escapes owned game')
            name = p.relative_to(game).as_posix()
            key = name.encode('ascii')
            if len(key)>239 or name.lower() in inputs:
                raise ValueError('Ambiguous or oversized JPEG path')
            before = sha(p)
            rgb = Path(temp)/f'{i}.rgb'
            metadata = json.loads(subprocess.check_output([codec, game, name, rgb], text=True))
            w, h = metadata['width'], metadata['height']
            if not (0<w<=2048 and 0<h<=2048) or rgb.stat().st_size!=w*h*3 or sha(rgb)!=metadata['rgb_sha256'] or sha(p)!=before:
                raise ValueError('Native JPEG decode/source changed')
            record = struct.pack('<5I', 20+len(key)+w*h*3, len(key), w, h, w*h*3)+key+rgb.read_bytes()
            size += len(record)
            if size>1024*1024*1024:
                raise ValueError('JPEG cache exceeds bounded protocol size')
            entries.append(record)
            inputs[name.lower()] = dict(path=name, source_sha256=before, **metadata)
    target = root/'jpeg-pixels-v1.bin'
    with target.open('xb') as f:
        f.write(b'MNMJPIX1'+struct.pack('<6I', 1, 32, len(entries), size, 0, 0))
        for entry in entries:
            f.write(entry)
    manifest = dict(schema=1, cache_path=str(target), cache_sha256=sha(target), bytes=size,
                    source_only=True, decoded_images=len(entries), inputs=inputs, decoder_sha256=sha(codec))
    with (root/'jpeg-pixels-manifest.json').open('x') as f:
        json.dump(manifest, f, indent=2)
        f.write('\n')
    return manifest


def authorize(root, metadata, env):
    for name in ('MNM_JPEG_PIXELS', 'MNM_JPEG_ROOT'):
        env.pop(name, None)
    manifest = metadata.get('jpeg_pixel_cache')
    if manifest is None:
        return
    target = root/'jpeg-pixels-v1.bin'
    if target.is_symlink() or not target.resolve().is_relative_to(root.resolve()) or sha(target)!=manifest.get('cache_sha256'):
        raise ValueError('Native JPEG cache hash/path mismatch')
    for entry in manifest['inputs'].values():
        name = Path(entry['path'])
        p = root/'game'/name
        if name.is_absolute() or '..' in name.parts or p.is_symlink() or not p.resolve().is_relative_to((root/'game').resolve()) or sha(p)!=entry['source_sha256']:
            raise ValueError('Native JPEG source hash/path mismatch')
    env['MNM_JPEG_PIXELS'] = 'Z:'+str(target.resolve()).replace('/', '\\')
    env['MNM_JPEG_ROOT'] = 'Z:'+str((root/'game').resolve()).replace('/', '\\')
