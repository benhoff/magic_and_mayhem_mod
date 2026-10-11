#!/usr/bin/env python3
"""Retain a pinned original-active startup prefix through its composed cursor canvas."""
import argparse
from collections import Counter
import gzip
import json
import lzma
from pathlib import Path
import struct

from drawing_corpus import ROOT, BUILD, local, sha, load_tool

EVIDENCE = 'NR.world-producer-handoff.original-gpu-current-20261008'
BOUNDARY_SEQUENCE = 20458
ORACLES = (908, 915, 916, 918, 919, 920)


def frame_key(raw, indexed):
    return sha(bytes([bool(indexed)]) + raw[:28] + bytes(4) + raw[32:])


def retain(destination, *, sequence=BOUNDARY_SEQUENCE, oracles=ORACLES, scope=None, frames=None, historical_root=ROOT):
    from rendering_coverage import evidence_record
    register = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
    ev = next(e for e in register['evidence'] if e['id'] == EVIDENCE)
    report = evidence_record(ROOT, ev, {b['id']: b for b in register['builds']})
    if report['original_pixels_used_as_native_inputs'] or report['original_oracles_read_by_native']:
        raise ValueError('Historical native inputs were oracle-dependent')
    if destination.exists() or not destination.resolve().is_relative_to((ROOT / 'tests/fixtures').resolve()):
        raise ValueError('Choose a new directory beneath tests/fixtures')
    inputs = report['inputs']
    stream_path = next(p for p in inputs if p.endswith('/capture/canvas-producers.bin'))
    capture = Path(stream_path).parent
    game = capture.parent / 'game'

    def pinned(name):
        data = local(historical_root, name).read_bytes()
        if inputs.get(name) != sha(data):
            raise ValueError('Historical input changed: ' + name)
        return data

    raw = pinned(stream_path)
    inspector = load_tool('inspect-canvas-producers')
    _, all_records = inspector.decode(raw)
    records = all_records[:sequence]
    end = 64 + sum(r[0] for r, _ in records)
    prefix = raw[:end]
    _, decoded = inspector.decode(prefix, complete=False)
    if decoded != records or records[-1][0][2] != 10 or records[-1][0][14] not in oracles:
        raise ValueError('Declared startup boundary changed')
    prepared = []

    def pack(label, data, historical=None, encoding='gzip'):
        packed = gzip.compress(data, mtime=0) if encoding == 'gzip' else lzma.compress(data)
        path = destination / (label + ('.gz' if encoding == 'gzip' else '.xz'))
        entry = {'path': path.relative_to(ROOT).as_posix(), 'encoding': encoding,
                 'sha256': sha(packed), 'raw_sha256': sha(data)}
        if historical:
            entry['historical_path'] = historical
        prepared.append((path, packed))
        return entry

    input_entry = pack('startup-input', prefix) if len(prefix) <= 32 * 1024 * 1024 else {
        'raw_sha256': sha(prefix), 'chunks': [pack(f'input-{i//(24*1024*1024):03}', prefix[i:i+24*1024*1024], encoding='xz') for i in range(0,len(prefix),24*1024*1024)]}
    queues = sum(r[2] == 12 for r, _ in records)
    catalog = {'schema': 2 if frames else 1, 'build': ev['build'], 'source_sha256': BUILD,
               'retention_sources': {n: sha((ROOT / n).read_bytes()) for n in ('tools/retain-drawing-scene.py', 'tools/drawing_corpus.py', 'tools/inspect-canvas-producers.py', 'tools/rendering_coverage.py')},
               'historical_report_sha256': ev['sha256'], 'original_inputs_verified': True,
               'evidence': EVIDENCE, 'scope': scope or 'Quick Battle map2 first complete 800x600 pre-display-copy canvas: contiguous native startup history, World queue1, HUD, minimap/markers and cursor. Physical presentation and original consumer/driver side effects excluded.',
               'boundary': {'sequence': sequence, 'queues': queues, 'canvas': records[-1][0][3], 'width': records[-1][0][5], 'height': records[-1][0][6]},
               'historical_stream': {'path': stream_path, 'sha256': sha(raw), 'prefix_bytes': end},
               'input': input_entry, 'assets': [], 'checkpoints': [],
               'producer_counts': dict(sorted(Counter(str(r[2]) for r, _ in records).items())),
               'remaining': ['Physical display copy/Flip and driver/error semantics', 'Original World consumer non-pixel state and readbacks beyond admitted native composition', 'General HUD/tooltips, picking/hotspots and source-state generation', 'Camera actions, active battles, alternate dispatch and unbounded sessions']}
    if frames:
        catalog['frames'] = frames
    encoded = {p[:-1].decode('ascii') for r, p in records if r[2] in (7, 20)}
    candidates = {Path(p).relative_to(game).as_posix().casefold(): p for p in inputs if Path(p).is_relative_to(game)}
    selected = {}
    for name in sorted(encoded):
        historical = candidates[name.replace('\\', '/').casefold()]
        selected[name] = historical
    # Keep actual independently decoded SPR assets, not sprites synthesized
    # from destination pixels or substituted unshaded approximations.
    needed = set()
    active = False
    for r, p in records:
        if r[2] == 11:
            active = True
        elif r[2] == 12:
            active = False
        elif active and r[2] == 9:
            needed.add(frame_key(p[:r[19]], r[17]))
    for asset in sorted(report['asset_sources'], key=lambda a: a['path']):
        if not asset['path'].lower().endswith('.spr'):
            continue
        historical = (game / asset['path']).as_posix()
        data = pinned(historical)
        if len(data) < 24 or struct.unpack_from('<I', data, 8)[0] != 4:
            continue
        count, palettes = struct.unpack_from('<II', data, 12)
        table = 24 + palettes * 768
        base = table + count * 4
        keys = set()
        for i in range(count):
            at = base + struct.unpack_from('<I', data, table + i * 4)[0]
            size = struct.unpack_from('<I', data, at)[0]
            keys.add(frame_key(data[at:at+size], palettes != 0))
        if keys & needed:
            selected[asset['path']] = historical
            needed -= keys
        if not needed:
            break
    if needed:
        raise ValueError('Unmapped World source frames')
    for ordinal, (name, historical) in enumerate(sorted(selected.items()), 1):
        entry = pack(f'asset-{ordinal:03}', pinned(historical), historical)
        entry['name'] = name
        catalog['assets'].append(entry)
    native = {c['oracle']: c for c in report['native']['checkpoints']}
    comparisons = {c['oracle']: c for c in report['checkpoints']}
    for r, _ in records:
        if r[2] != 10 or r[14] not in oracles:
            continue
        historical = (capture / f'producer-oracle-{r[14]:04}.565').as_posix()
        original = pinned(historical)
        n = native[r[14]]
        if not comparisons[r[14]]['equal'] or n['sha256'] != sha(original) or n['sequence'] != r[1] or n['canvas'] != r[3]:
            raise ValueError('Original checkpoint identity mismatch')
        catalog['checkpoints'].append({'ordinal': r[14], 'sequence': r[1], 'canvas': r[3],
                                       'width': r[5], 'height': r[6], 'oracle': pack(f'oracle-{r[14]:04}', original, historical)})
    destination.mkdir()
    for path, data in prepared:
        with path.open('xb') as out:
            out.write(data)
    with (destination / 'scene.json').open('x') as out:
        json.dump(catalog, out, indent=2)
        out.write('\n')
    return catalog


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    result = retain(args.output.resolve())
    print(json.dumps({'assets': len(result['assets']), 'checkpoints': len(result['checkpoints']), 'boundary': result['boundary']}))


if __name__ == '__main__':
    main()
