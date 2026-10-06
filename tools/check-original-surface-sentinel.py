#!/usr/bin/env python3
"""Replay retained original cursor-recovery traces offline, without Wine or media."""
import argparse
import base64
import gzip
import hashlib
import json
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT = ROOT / 'tests/fixtures/surfaces/original-sentinel-corpus.json'
BUILD_HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
LOST = 0x887601C2
PATH = 'bitmaps\\cursors.bmp'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def require(ok, message):
    if not ok:
        raise ValueError(message)


def bitmap_trace(c, asset, sentinel):
    """Recover submitted buffer bytes from input, never from captured outputs."""
    events = []

    def ev(kind, **fields):
        events.append(dict(kind=kind, **fields))

    if sentinel:
        ev('cursor_reload', surface=3)
    ev('bitmap_reload', surface=3, path=PATH)
    ev('open', path=PATH, mode='rb', present=asset is not None)
    if asset is None:
        return events, 0
    cursor = 0
    allocated = []

    def read(count):
        nonlocal cursor
        require(0 <= count <= 2 * 1024 * 1024, 'Unsupported read extent')
        data = asset[cursor:cursor+count]
        cursor += len(data)
        ev('read', requested=count, returned=len(data))
        return data

    def alloc(count):
        require(0 < count <= 2 * 1024 * 1024, 'Unsupported allocation extent')
        success = not c['allocation_failure']
        ev('alloc', bytes=count, success=success)
        if success:
            allocated.append(count)
        return success

    ok = False
    file_header = read(14)
    if len(file_header) == 14 and file_header[:2] == b'BM':
        header = read(40)
        if len(header) == 40 and alloc(1064):
            bits = struct.unpack_from('<H', header, 14)[0]
            count = {1: 2, 4: 16, 8: 256}.get(bits, 0) * 4
            palette = read(count)
            if len(palette) == count:
                size, offset = struct.unpack_from('<I', file_header, 2)[0], struct.unpack_from('<I', file_header, 10)[0]
                size = (size - offset) & 0xffffffff
                # Second-allocation failure is not captured/supported.
                require(not c['allocation_failure'], 'Uncaptured second allocation failure')
                if alloc(size):
                    pixels = read(size)
                    ok = len(pixels) == size
    ev('close')
    if ok and c['global_surface']:
        ev('get_dc', surface=3, result=c['get_dc'])
        width, height = struct.unpack_from('<II', header, 4)
        ev('dib', width=width, height=height, bits=bits, usage=0 if bits in [1, 4, 8] else 1,
           rop=0xcc0020, info_sha256=hashlib.sha256(header).hexdigest(),
           palette_sha256=hashlib.sha256(palette).hexdigest(),
           pixels_sha256=hashlib.sha256(pixels).hexdigest(), result=c['dib_result'])
        ev('release_dc', surface=3, result=c['release_dc'])
    for count in allocated:
        ev('free', bytes=count)
    return events, 0 if sentinel else int(ok)


def expected(c, asset):
    entry = c['entry']
    if entry in [0x4A2F90, 0x58D1A0]:
        events, result = bitmap_trace(c, asset, entry == 0x4A2F90)
        return dict(id=c['id'], events=events, draws_consumed=0, return_value=result)
    require(entry in [0x58BAC0, 0x58BC10, 0x58C4A0, 0x58C8A0], 'Unknown wrapper')
    fill = entry in [0x58BAC0, 0x58BC10]
    full = entry == 0x58BC10
    fast = bool(c['fast']) and not fill
    keyed = entry == 0x58C8A0
    flags = 0x1000400 if fill else ((1 if fast else 0x8000) if keyed else 0) | (0 if c['no_wait'] else 0x10 if fast else 0x1000000)
    events = []
    used = 0

    def ev(kind, **fields):
        events.append(dict(kind=kind, **fields))

    def draw():
        nonlocal used
        require(used < len(c['draw_results']), 'Finite draw schedule exhausted')
        h = c['draw_results'][used]
        used += 1
        ev('draw', surface=2, fast=fast, flags=flags, result=h,
           color=(c['color'] & 65535) if fill else 0)
        return h

    def recover(surface):
        h = c['source_restore' if surface == 1 else 'destination_restore']
        ev('restore', surface=surface, result=h)
        if h == 0:
            if c['key_enabled']:
                key = 0x5678 if surface == 1 else 0xe123
                ev('key', surface=surface, flags=8, range=[key, key], result=c['key_result'])
            mode = c['source_reload' if surface == 1 else 'destination_reload']
            require(mode in [0, 1, 2], 'Unknown reload mode')
            if mode == 1:
                ev('callback', surface=surface, argument=0)
            elif mode == 2:
                nested, _ = bitmap_trace(c, asset, True)
                events.extend(nested)
        return h

    while True:
        h = draw()
        if h or full:
            ev('report', result=h)
        if fill:
            if h == LOST:
                restored = recover(2)
                if full:
                    ev('report', result=restored)
                    ev('report', result=draw())
            break
        if h == 0:
            break
        if h == LOST:
            recover(1)
            recover(2)
    return dict(id=c['id'], events=events, draws_consumed=used)


def compare(payload):
    require(payload['schema'] == 1, 'Unsupported payload schema')
    assets = {n: None if b is None else base64.b64decode(b, validate=True) for n, b in payload['assets'].items()}
    require(len(assets) <= 32 and all(b is None or len(b) <= 2*1024*1024 for b in assets.values()), 'Asset limits')
    rows = payload['rows']
    require(0 < len(rows) <= 10000, 'Case limits')
    instructions = set()
    for id, row in enumerate(rows):
        c, out = row['input'], row['original']
        require(c['id'] == out['id'] == id, 'Case identity differs')
        require(len(c['draw_results']) <= 16, 'Schedule limits')
        model = expected(c, assets[c['asset']])
        require({k: out[k] for k in model} == model, f'Original trace differs: case {id}')
        require(0 < out['instructions'] <= 12000, 'Instruction budget')
        require(c['entry'] in out['executed_entries'], 'Missing original entry execution')
        instructions.update(out['executed_entries'])
    return dict(cases=len(rows), assets=len(assets), matched=len(rows),
                events=sum(len(row['original']['events']) for row in rows),
                executed_entries=sorted(instructions))


def load(catalog=DEFAULT):
    meta = json.loads(catalog.read_text())
    require(meta['schema'] == 1 and meta['original_sha256'] == BUILD_HASH, 'Unsupported provenance')
    require(meta['oracle'] == 'unchanged_original_x86_scripted_crt_com_gdi', 'Unsupported oracle')
    require(meta['path'] == 'original-sentinel.json.gz', 'Unsafe corpus path')
    corpus = catalog.parent / meta['path']
    require(sha(corpus) == meta['sha256'], 'Compressed fixture differs')
    with gzip.open(corpus, 'rb') as f:
        raw = f.read(32*1024*1024 + 1)
    require(len(raw) <= 32*1024*1024, 'Expanded fixture limit')
    require(hashlib.sha256(raw).hexdigest() == meta['raw_sha256'], 'Raw fixture differs')
    payload = json.loads(raw)
    require(len(payload['rows']) == meta['cases'] and len(payload['assets']) == meta['assets'], 'Catalog counts differ')
    return payload, meta


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--catalog', type=Path, default=DEFAULT)
    p.add_argument('--report', type=Path)
    a = p.parse_args()
    payload, meta = load(a.catalog)
    result = compare(payload)
    sources = {n: sha(ROOT/n) for n in ['tools/check-original-surface-sentinel.py',
               'tests/test-original-surface-sentinel.py',
               'tests/surface-sentinel-reference.py', 'tools/capture-original-surface-sentinel.py',
               'tests/surface-sentinel-requirements.txt']}
    sources.update({str(x.relative_to(ROOT)): sha(x) for x in [a.catalog, a.catalog.parent/meta['path']]})
    result.update(schema=1, success=True, original_sha256=BUILD_HASH, sources=sources,
                  scope='Input-derived offline BMP/CRT endpoint and four-wrapper ordering comparison against retained unchanged original x86 captures. DIB submitted buffers only; no real GDI rasterization, original CRT, native asset reload or live equivalence.')
    if a.report:
        with a.report.open('x') as f:
            json.dump(result, f, indent=2)
            f.write('\n')
    print(json.dumps({k: result[k] for k in ['success', 'cases', 'assets', 'events']}))


if __name__ == '__main__':
    main()
