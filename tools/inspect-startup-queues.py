#!/usr/bin/env python3
"""Decode bounded raw startup queue diagnostics; addresses are process-local."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct

ADMITTED = frozenset((-2, 0, 1, 2, 3, 4, 5, 6, 22, 31, 33))


def signed(value):
    return value if value < 0x80000000 else value - 0x100000000


def decode(raw, startup_replay=False, sample_limit=16):
    if len(raw) < 64 or raw[:8] != b'MNMSTQ01':
        raise ValueError('Invalid startup queue magic/header')
    h = struct.unpack_from('<14I', raw, 8)
    version, header, size, sequence, sample, status, view, count, capacity, queue, base, stride, rows, reserved = h
    if version != 1 or header != 64 or size != len(raw) or stride != 36 or reserved:
        raise ValueError('Invalid startup queue envelope')
    if not 1 <= sequence <= 256 or sample_limit not in (16,32) or sample > sample_limit or status > 3 or rows > 12320 or size != 64 + rows * 36:
        raise ValueError('Invalid startup queue bounds')
    if status == 0 and (rows != count or count > capacity):
        raise ValueError('Incomplete startup draw rows')
    if status and rows:
        raise ValueError('Refused queue cannot claim raw rows')
    records = list(struct.iter_unpack('<9I', raw[64:]))
    counts = Counter(signed(row[6]) for row in records)
    admitted = ADMITTED | {16, 17, 20} if startup_replay else ADMITTED
    unsupported = [{'ordinal': i, 'kind': signed(row[6]), 'kind_hex': f'0x{row[6]:08x}',
                    'x': signed(row[2]), 'y': signed(row[3]), 'frame_pointer': f'0x{row[1]:08x}'}
                   for i, row in enumerate(records) if signed(row[6]) not in admitted]
    return {'queue': sequence, 'sample': sample, 'status': status, 'view': view, 'draw_count': count,
            'capacity': capacity, 'rows_captured': rows, 'kind_counts': dict(sorted(counts.items())),
            'unsupported_draws': unsupported, 'raw_rows': records,
            'default_policy_unsupported_draws': [{'ordinal': i, 'kind': signed(row[6])} for i, row in enumerate(records) if signed(row[6]) not in ADMITTED]}


def collect(directory, limit, startup_replay=False, sample_limit=16):
    if not 1 <= limit <= 256:
        raise ValueError('Invalid startup queue prefix length')
    paths = sorted(directory.glob('startup-queue-*.bin'))
    if len(paths) != limit:
        raise ValueError(f'Startup queue capture missing entries: expected {limit}, found {len(paths)}')
    queues, samples, kinds, default_kinds = [], set(), Counter(), Counter()
    for sequence, path in enumerate(paths, 1):
        raw = path.read_bytes()
        row = decode(raw, startup_replay, sample_limit)
        if row['queue'] != sequence or path.name != f'startup-queue-{sequence:04}.bin':
            raise ValueError('Startup queue capture has a gap/reordered sequence')
        if row['sample'] and row['sample'] in samples:
            raise ValueError('Duplicate startup queue sample correlation')
        if row['sample']:
            samples.add(row['sample'])
        kinds.update(item['kind'] for item in row['unsupported_draws'])
        default_kinds.update(item['kind'] for item in row['default_policy_unsupported_draws'])
        row.pop('raw_rows')
        row.update(path=str(path.name), sha256=hashlib.sha256(raw).hexdigest())
        queues.append(row)
    return {'requested_queues': limit, 'captured_queues': len(queues), 'contiguous_from_first_queue': True,
            'all_raw_rows_captured': all(q['status'] == 0 for q in queues), 'queues': queues,
            'unsupported_kind_counts': dict(sorted(kinds.items())),
            'default_policy_unsupported_kind_counts': dict(sorted(default_kinds.items())), 'startup_replay': startup_replay, 'original_pixels_used_as_native_inputs': False,
            'scope': 'Every consumer entry in the bounded startup prefix, before scene sampling. Unsupported means refused by the effective World observer, not absent from the original dispatcher.'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('directory', type=Path)
    parser.add_argument('--queues', type=int, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--startup-replay', action='store_true')
    args = parser.parse_args()
    value = collect(args.directory, args.queues, args.startup_replay)
    with args.output.open('x') as out:
        json.dump(value, out, indent=2)
        out.write('\n')


if __name__ == '__main__':
    main()
