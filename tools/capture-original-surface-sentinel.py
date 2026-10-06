#!/usr/bin/env python3
"""Capture unchanged cursor reload code with bounded CRT/COM/GDI endpoints."""
import argparse
import base64
import gzip
import hashlib
import importlib.util
import itertools
import json
import struct
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LOST, FAIL = 0x887601C2, 0x80004005
SOURCE_FILES = ['tests/surface-sentinel-reference.py',
                'tools/capture-original-surface-sentinel.py',
                'tests/surface-sentinel-requirements.txt']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def bitmap(bits=8, gap=0):
    width, height = 8, 6
    stride = ((width * bits + 31) // 32) * 4
    pixels = bytes((i * 37 + 11) & 255 for i in range(stride * height))
    palette = bytes((i * 13 + 7) & 255 for i in range({1: 2, 4: 16, 8: 256}.get(bits, 0) * 4))
    offset = 54 + len(palette) + gap
    header = struct.pack('<IiiHHIIiiII', 40, width, height, 1, bits, 0, len(pixels), 0, 0, 0, 0)
    return struct.pack('<2sIHHI', b'BM', offset + len(pixels), 0, 0, offset) + header + palette + b'G' * gap + pixels


def cases(asset_names):
    defaults = dict(global_surface=1, source_reload=2, destination_reload=2,
                    fast=0, no_wait=0, key_enabled=1, source_restore=0,
                    destination_restore=0, key_result=0, get_dc=0,
                    release_dc=0, dib_result=6, allocation_failure=0,
                    color=0x12345678, draw_results=[LOST, 0], asset='indexed8')
    rows = []

    def add(**fields):
        rows.append(dict(defaults, **dict(fields, id=len(rows))))

    for entry, asset, present, dc in itertools.product(
            [0x4A2F90, 0x58D1A0], asset_names, [0, 1],
            [(0, 0, 6), (FAIL, 0, 6), (0, FAIL, 6), (0, 0, 0), (0, 0, 0xffffffff)]):
        add(entry=entry, asset=asset, global_surface=present,
            get_dc=dc[0], release_dc=dc[1], dib_result=dc[2])
    for entry in [0x4A2F90, 0x58D1A0]:
        add(entry=entry, allocation_failure=1)
    for entry in [0x58BAC0, 0x58BC10, 0x58C4A0, 0x58C8A0]:
        for fast, modes, restores, key, kr, draws in itertools.product(
                [0] if entry in [0x58BAC0, 0x58BC10] else [0, 1],
                [(2, 0), (0, 2), (2, 2), (2, 1), (1, 2)],
                [(0, 0), (FAIL, 0), (0, FAIL), (FAIL, FAIL)],
                [0, 1], [0, FAIL], [[0], [LOST, 0], [LOST, LOST, 0]]):
            add(entry=entry, fast=fast, source_reload=modes[0],
                destination_reload=modes[1], source_restore=restores[0],
                destination_restore=restores[1], key_enabled=key,
                key_result=kr, draw_results=draws)
        for asset in ['missing', 'installed-cursors', 'short-pixels']:
            add(entry=entry, asset=asset)
        if entry in [0x58C4A0, 0x58C8A0]:
            for fast in [0, 1]:
                add(entry=entry, fast=fast, no_wait=1)
    return rows


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--executable', type=Path, default=ROOT / 'working/game-nocd/Chaos.exe')
    p.add_argument('--cursor', type=Path, default=ROOT / 'working/game-nocd/Bitmaps/cursors.bmp')
    p.add_argument('--fixture-dir', type=Path, default=ROOT / 'tests/fixtures/surfaces')
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    corpus = a.fixture_dir / 'original-sentinel.json.gz'
    catalog = a.fixture_dir / 'original-sentinel-corpus.json'
    if any(x.exists() for x in [corpus, catalog, a.report]):
        p.error('Refusing retained evidence overwrite')
    spec = importlib.util.spec_from_file_location('reference', ROOT / SOURCE_FILES[0])
    ref = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(ref)
    if ref.unicorn.__version__ != '2.1.4':
        p.error('Capture requires pinned Unicorn 2.1.4')
    parent = ROOT / 'working/tests/surface-sentinel'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    sources = {name: sha(ROOT / name) for name in SOURCE_FILES}

    def verify(label):
        out = subprocess.run([str(ROOT / 'tools/original-manifest.sh'), 'verify'],
                             capture_output=True, text=True, timeout=120)
        (run / (label + '.log')).write_text(out.stdout + out.stderr)
        out.check_returncode()

    verify('original-before')
    try:
        image = a.executable.read_bytes()
        reference = ref.Reference(image)
        normal = bitmap()
        assets = dict(missing=None, **{'short-file': normal[:13], 'bad-signature': b'ZZ' + normal[2:],
                      'short-info': normal[:53], 'short-palette': normal[:100],
                      'short-pixels': normal[:-1], 'indexed1': bitmap(1),
                      'indexed4': bitmap(4), 'indexed8': normal, 'rgb24': bitmap(24),
                      'offset-gap': bitmap(8, 8), 'installed-cursors': a.cursor.read_bytes()})
        inputs = cases(list(assets))
        outputs = [reference.run(c, assets[c['asset']]) for c in inputs]
        # Budget refusal is a collector stop, never an original successful return.
        refused = False
        try:
            reference.run(inputs[0], assets[inputs[0]['asset']], budget=1)
        except ValueError:
            refused = True
        assert refused
        assert sha(a.executable) == ref.BUILD_HASH and sha(a.cursor) == hashlib.sha256(assets['installed-cursors']).hexdigest()
        assert all(sha(ROOT / n) == h for n, h in sources.items())
        payload = dict(schema=1, assets={n: None if b is None else base64.b64encode(b).decode() for n, b in assets.items()},
                       rows=[dict(input=c, original=o) for c, o in zip(inputs, outputs, strict=True)])
        raw = json.dumps(payload, separators=(',', ':')).encode()
        a.fixture_dir.mkdir(parents=True, exist_ok=True)
        corpus.write_bytes(gzip.compress(raw, mtime=0))
        scope = ('Unchanged pinned x86 sentinel/bitmap parser/DC loader and four recovery wrappers in Unicorn2.1.4; '
                 'scripted CRT file/allocation, COM, GetDC (always writes fixture HDC even on failure), GDI and error-report endpoints. '
                 'Exact ordered calls and submitted DIB buffer hashes; no original CRT/driver/GDI rasterization, native asset reload or live equivalence.')
        meta = dict(schema=1, original_sha256=ref.BUILD_HASH, path=corpus.name,
                    sha256=sha(corpus), raw_sha256=hashlib.sha256(raw).hexdigest(),
                    cases=len(inputs), assets=len(assets), scope=scope,
                    oracle='unchanged_original_x86_scripted_crt_com_gdi')
        catalog.write_text(json.dumps(meta, indent=2) + '\n')
        sources.update({str(x.relative_to(ROOT)): sha(x) for x in [corpus, catalog]})
        anchors = {}
        for start, end in ref.RANGES:
            for at, data in reference.sections:
                if at <= start < end <= at + len(data):
                    anchors[hex(start)] = dict(end=end, sha256=hashlib.sha256(data[start-at:end-at]).hexdigest())
                    break
        report = dict(schema=1, success=True, original_sha256=ref.BUILD_HASH,
                      sources=sources, cases=len(inputs), assets=len(assets),
                      installed_cursor_sha256=hashlib.sha256(assets['installed-cursors']).hexdigest(),
                      unchanged_ranges=anchors, executed_entries=sorted({e for o in outputs for e in o['executed_entries']}),
                      instruction_budget_refused=refused, emulator='Unicorn2.1.4',
                      original_instruction_bytes_patched=False, wine_executed=False,
                      real_driver_executed=False, scope=scope, artifacts=str(run.relative_to(ROOT)))
    finally:
        verify('original-after')
    report['original_manifest_verified_before_after'] = True
    with a.report.open('x') as f:
        json.dump(report, f, indent=2)
        f.write('\n')
    print(json.dumps(dict(success=True, cases=len(inputs), assets=len(assets))))


if __name__ == '__main__':
    main()
