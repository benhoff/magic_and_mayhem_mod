"""Strict live caller census and independent, retained copy-pixel accounting."""
import hashlib
import json
import struct
from collections import Counter
from pathlib import Path

CAPACITY = 8192


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def census(path):
    raw = Path(path).read_bytes()
    if len(raw) != 128 + CAPACITY * 128:
        raise ValueError('GPU copy census extent')
    h = struct.unpack_from('<32I', raw)
    if raw[:8] != b'MNMGCN01' or h[2:6] != (1, 128, 128, CAPACITY) or h[6] != h[8] or h[6] > CAPACITY or h[7] or h[9] or h[20] != 1 or not h[18] or not h[19] or any(h[21:]):
        raise ValueError('GPU copy census incomplete, overflowed or unclosed')
    rows = [struct.unpack_from('<32I', raw, 128 + i * 128) for i in range(h[6])]
    for i, row in enumerate(rows):
        if row[0] != i + 1 or row[2] != h[18] or row[15] & ~15 or row[16] > 256 or row[31] > 1:
            raise ValueError('GPU copy census row identity')
        rva = row[1] - h[18]
        if row[3] != (rva if 0 <= rva < h[19] else 0xffffffff):
            raise ValueError('GPU copy caller range')
        if not row[18] and row[16] and (not row[7] or row[8] < row[7] or not row[4] or not row[5] or not row[31]):
            raise ValueError('GPU copy emission/authority identity')
    counts = [sum(bool(r[15] & flag) for r in rows) for flag in (2, 1, 4, 8)]
    counts += [sum(not r[16] and not r[18] for r in rows), sum(bool(r[17] & 0x80000000) for r in rows),
               sum(bool(r[17] & 0x80000000) and bool(r[16]) for r in rows), sum(bool(r[18] & 0x80000000) for r in rows)]
    if counts != list(h[10:18]) or any(raw[128 + h[6] * 128:]):
        raise ValueError('GPU copy census counter or unused-tail disagreement')
    return h, rows


def check_pixels(directory, row):
    """Recompute the retained pixels in a separate Python process/implementation."""
    blobs = {}
    for name, filename in row['files'].items():
        if name not in ('source', 'before', 'actual', 'expected') or Path(filename).name != filename:
            raise ValueError('Unsafe copy pixel identity')
        data = (directory / filename).read_bytes()
        key = 'actual_sha256' if name == 'actual' else name + '_sha256'
        if hashlib.sha256(data).hexdigest() != row[key]:
            raise ValueError('Changed retained copy pixels')
        if len(data) % 4:
            raise ValueError('Copy pixel word extent')
        blobs[name] = list(struct.unpack('<' + 'I' * (len(data) // 4), data))
    if set(blobs) != {'source', 'before', 'actual', 'expected'}:
        raise ValueError('Incomplete retained copy pixels')
    sw, sh, dw, dh = [row[k] for k in ('source_width', 'source_height', 'target_width', 'target_height')]
    if any(not 1 <= v <= 2048 for v in (sw, sh, dw, dh)) or len(blobs['source']) != sw * sh or any(len(blobs[k]) != dw * dh for k in ('before', 'actual', 'expected')):
        raise ValueError('Copy pixel dimensions')
    w = row['words']; expected = blobs['before'][:]
    if len(w) != 13 or any(type(v) is not int or not 0 <= v <= 0xffffffff for v in w) or not (0 <= w[2] < w[4] <= sw and 0 <= w[3] < w[5] <= sh and w[6] + w[4] - w[2] <= dw and w[7] + w[5] - w[3] <= dh) or w[8] > 1 or any(w[10:]):
        raise ValueError('Copy witness geometry')
    for y in range(w[3], w[5]):
        for x in range(w[2], w[4]):
            pixel = blobs['source'][y * sw + x]
            if not w[8] or pixel != w[9]:
                expected[(w[7] + y - w[3]) * dw + w[6] + x - w[2]] = pixel
    if expected != blobs['expected'] or expected != blobs['actual']:
        raise ValueError('Independent retained copy destination mismatch')
    return dw * dh


def validate(experiment, directory):
    experiment, directory = Path(experiment), Path(directory)
    sensor = experiment / 'lock-capture/gpu-copy-census-v1.bin'
    h, calls = census(sensor)
    report = json.loads((directory / 'report.json').read_text())
    if report.get('success') is not True or not report['ended'] or report['truncated'] or report['error'] or not report['copies_checked'] or report['original_pixels_used_as_native_inputs']:
        raise ValueError('Incomplete live GPU copy comparison')
    records = [json.loads(line) for line in (directory / 'copies.jsonl').read_text().splitlines()]
    if len(records) != report['copies_checked'] or len({r['sequence'] for r in records}) != len(records) or any(r['mismatches'] or r['actual_sha256'] != r['expected_sha256'] or len(r['words']) != 13 for r in records):
        raise ValueError('Copy diagnostic record identity/mismatch')
    if report['diagnostic_native_readbacks'] != len(records) * 3 or sum(r['pixels'] for r in records) != report['pixels_compared']:
        raise ValueError('Copy diagnostic counters')
    diagnostic_bytes = sum((r['source_width'] * r['source_height'] + 2 * r['target_width'] * r['target_height']) * 4 for r in records)
    if diagnostic_bytes != report['diagnostic_readback_bytes'] or diagnostic_bytes > 32 * 1024**3 or len(records) > CAPACITY:
        raise ValueError('Copy diagnostic byte/capacity accounting')
    retained = [r for r in records if 'files' in r]
    retained_pixels = sum(check_pixels(directory, r) for r in retained)
    if len(retained) != report['retained_pixel_cases'] or not retained:
        raise ValueError('Missing retained independent copy witnesses')
    checked = {r['sequence']: r for r in records}
    matched = []; missing = []
    for row in calls:
        if row[18] or not row[16]:
            continue
        first = row[7] + bool(row[15] & 1)  # self begins with an opaque snapshot COPY
        needed = range(row[7], row[8] + 1)
        if any(seq not in checked for seq in needed):
            missing.append(row[0]);continue
        pieces = [checked[seq] for seq in range(first, row[8] + 1)]
        if len(pieces) != row[16] or any(r['words'][0] != (row[6] if row[15] & 1 else row[4]) or r['words'][1] != row[5] for r in pieces):
            raise ValueError('Caller/copy command identity mismatch')
        if row[15] & 1:
            snapshot = checked[row[7]]['words']
            if snapshot[:10] != [row[4], row[6], 0, 0, row[19], row[20], 0, 0, 0, 0]:
                raise ValueError('Aliased source was not frozen once')
        matched.append(row)
    if not matched or missing:
        raise ValueError('GPU calls lack native output comparison: ' + repr(missing[:16]))
    callers = Counter((row[3], row[15]) for row in matched)
    return dict(verified=True, calls=len(calls), matched_calls=len(matched), clipped_calls=h[10], self_calls=h[11],
                keyed_calls=h[12], bootstrap_calls=h[13], empty_calls=h[14], failed_results=h[15], partial_errors=h[16],
                admission_failures=h[17], census_sha256=sha(sensor), diagnostics=report,
                diagnostic_records_sha256=sha(directory / 'copies.jsonl'), retained_python_checks=len(retained),
                retained_python_pixels=retained_pixels,
                callers=[dict(caller_rva=None if rva == 0xffffffff else hex(rva), flags=flags, calls=count) for (rva, flags), count in sorted(callers.items())],
                scope='Actual caller/sequence-bound native copy destinations, exact CPU transform and separate retained Python replay. No original World, physical display or complete renderer equivalence.')
