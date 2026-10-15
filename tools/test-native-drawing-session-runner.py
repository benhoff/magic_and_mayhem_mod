#!/usr/bin/env python3
"""Exercise the native owned-input session CLI with independent expected planes."""
import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def digest(data):
    return hashlib.sha256(data).hexdigest()


def record(sequence, kind, width=5, height=3, colour=0x1234, payload=b''):
    f = [0]*24
    f[0:4] = [96+len(payload), sequence, kind, 7]
    f[5:7] = [width, height]
    f[18] = len(payload)
    if kind == 5:
        f[12:15] = [width, height, colour]
    if kind in (11, 12):
        f[14] = 1
    return struct.pack('<24I', *f)+payload


def stream(records):
    return b'MNMPRO01'+struct.pack('<14I', 1, 64, 0x40209ca7, 65536, 1, *([0]*9))+b''.join(records)


def frame(sequence=2, queues=0, width=5, height=3):
    return dict(sequence=sequence, queues=queues, canvas=7, width=width, height=height)


def bitmap():
    # Owned two-by-two green RGB source, bottom-up, padded rows.
    pixels = (b'\x00\xff\x00'*2+b'\x00\x00')*2
    return (struct.pack('<2sIHHI', b'BM', 54+len(pixels), 0, 0, 54)
            +struct.pack('<IiiHHIIiiII', 40, 2, 2, 1, 24, 0, len(pixels), 0, 0, 0, 0)+pixels)


def world_raster(sequence):
    # Owned black/red SPR literals: row offset table and literal RLE.
    packet = bytearray(55)
    for offset, value in ((0, 55), (4, 3), (8, 1), (12, 1), (16, 0xffffffff), (40, 48), (44, 51)):
        struct.pack_into('<I', packet, offset, value)
    packet[48:51] = b'\x00\x02\x01'
    struct.pack_into('<H', packet, 53, 0xf800)
    raster = bytearray(record(sequence, 9, payload=packet))
    for index, value in ((8, 2), (12, 5), (13, 3), (15, 8), (19, 55)):
        struct.pack_into('<I', raster, index*4, value)
    return bytes(raster)


def check(binary, run):
    env = dict(os.environ, QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1')
    fixtures = {}
    segments = []
    expected = []
    def segment(label, records, frames, planes):
        i = len(segments)+1
        name = f'segment-{i}.bin'
        data = stream(records)
        fixtures[name] = data
        segments.append(dict(segment=i, stage=label, producer=dict(path=name, sha256=digest(data)), frames=frames))
        expected.extend(planes)
    # Multiple frames within a retained segment, then five fresh generations.
    segment('menu', [record(1, 1), record(2, 5), record(3, 5, colour=0xabcd)],
            [frame(), frame(3)], [struct.pack('<15H', *([c]*15)) for c in (0x1234, 0xabcd)])
    segment('loading', [record(1, 1, 6, 4), record(2, 5, 6, 4, 0xf800)],
            [frame(width=6, height=4)], [struct.pack('<24H', *([0xf800]*24))])
    world_plane = [0x1234]*15
    world_plane[6] = 0  # First literal is opaque black, then red at x=2,y=1.
    world_plane[7] = 0xf800
    segment('world', [record(1, 1), record(2, 5), record(3, 11), world_raster(4), record(5, 12)],
            [frame(5, 1)], [struct.pack('<15H', *world_plane)])
    # Frame-less segment cancels an active World queue explicitly.
    segment('world', [record(1, 1), record(2, 5), record(3, 11)], [], [])
    segment('results', [record(1, 1), record(2, 5), record(3, 20, payload=b'owned.bmp\0')],
            [frame(3)], [struct.pack('<15H', *([0x07e0]*2+[0x1234]*3+[0x07e0]*2+[0x1234]*8))])
    segment('menu', [record(1, 1), record(2, 5, colour=0x001f)],
            [frame()], [struct.pack('<15H', *([0x001f]*15))])
    fixtures['owned.bmp'] = bitmap()
    raw_sprite_frame = bytearray(world_raster(4)[96:])
    struct.pack_into('<I', raw_sprite_frame, 28, 0xffffffff)
    fixtures['owned.spr'] = struct.pack('<7I', 0x00525053, 28+55, 4, 1, 0, 0, 0)+raw_sprite_frame
    fixtures['unused.dat'] = b'owned unused binding'
    declaration = dict(schema=1, session='18446744073709551615',
                       assets={n: digest(fixtures[n]) for n in ('owned.bmp', 'owned.spr', 'unused.dat')}, segments=segments)
    observed = []
    def execute(name, config=None, change=None, existing=False, snapshots=False):
        source = run/name/'input'
        source.mkdir(parents=True)
        for filename, data in fixtures.items():
            (source/filename).write_bytes(data)
        config = copy.deepcopy(config or declaration)
        if change:
            change(config, source)
        (source/'drawing-session.json').write_text(json.dumps(config))
        output = run/name/'output'
        if existing:
            output.mkdir()
            (output/'sentinel').write_bytes(b'preserve')
        result = subprocess.run(['xvfb-run', '-a', str(binary), str(source), str(output)]+(['--source-snapshots'] if snapshots else []),
                                env=env, capture_output=True, timeout=30)
        (run/(name+'.log')).write_bytes(result.stdout+result.stderr)
        value = json.loads(result.stdout)
        if name == 'complete-native-snapshots':
            assert result.returncode==0 and value['success'] and value['native_source_snapshots'],value
            assert value['remaining_surfaces']==0 and value['terminal_state']=='source_snapshots_complete'
            assert not value['physical_presentation'] and not value['original_oracles_read']
            assert len(value['frames'])==len(expected)
            for f,plane in zip(value['frames'],expected):
                assert (output/f['path']).read_bytes()==plane and f['sha256']==digest(plane)
        elif name == 'same-prefix-source-snapshots':
            assert result.returncode==0 and value['success'] and value['native_source_snapshots'],value
            assert value['remaining_surfaces']==0 and len(value['frames'])==2
            assert [f['canvas'] for f in value['frames']]==[7,8]
            assert [f['sequence'] for f in value['frames']]==[4,4]
            for f,colour in zip(value['frames'],(0x1234,0xabcd)):
                assert (output/f['path']).read_bytes()==struct.pack('<15H',*([colour]*15))
        elif name == 'complete-native-session':
            assert result.returncode == 0 and value['success'], value
            assert value['remaining_surfaces'] == 0 and value['terminal_state'] == 'finished'
            assert value['segments'] == 6 and value['frames_published'] == 6 and value['records'] == 18
            assert value['session'] == declaration['session']
            assert not value['original_oracles_read'] and not value['physical_presentation']
            identities = []
            for s in segments:
                identities.extend((s['segment'], s['stage'], b) for b in s['frames'])
            for i, (f, plane, identity) in enumerate(zip(value['frames'], expected, identities), 1):
                assert f['presentation'] == i and f['session'] == declaration['session']
                assert (f['segment'], f['stage']) == identity[:2]
                assert all(f[k] == v for k, v in identity[2].items())
                assert (output/f['path']).read_bytes() == plane
                assert f['sha256'] == digest(plane)
            assert json.loads((output/'session-report.json').read_text()) == value
        else:
            assert result.returncode != 0 and not value['success'] and value.get('error'), (name, value)
            if output.exists() and not existing:
                assert value['terminal_state'] == 'failed' and value['remaining_surfaces'] == 0, value
                assert not list(output.glob('frame-*.565')), name
            if existing:
                assert (output/'sentinel').read_bytes() == b'preserve'
                assert len(list(output.iterdir())) == 1
        observed.append(dict(case=name, success=True, report=value))
    execute('complete-native-session')
    execute('complete-native-snapshots',snapshots=True)
    def same_prefix(config,source):
        records=[record(1,1),record(2,5),record(3,1),record(4,5,colour=0xabcd)]
        for i in (2,3):
            r=bytearray(records[i]);struct.pack_into('<I',r,12,8);records[i]=bytes(r)
        data=stream(records);(source/'sources.bin').write_bytes(data)
        config['segments']=[dict(segment=1,stage='scene',producer=dict(path='sources.bin',sha256=digest(data)),frames=[frame(4),dict(frame(4),canvas=8)])]
    execute('same-prefix-source-snapshots',change=same_prefix,snapshots=True)
    def mutate(name, fn):
        execute(name, change=lambda c, s: fn(c))
    mutate('segment-gap', lambda c: c['segments'][0].update(segment=2))
    mutate('unknown-stage', lambda c: c['segments'][0].update(stage='battle'))
    mutate('noncanonical-session', lambda c: c.update(session='01'))
    mutate('overflow-session', lambda c: c.update(session='18446744073709551616'))
    mutate('numeric-session', lambda c: c.update(session=1))
    mutate('boolean-identity', lambda c: c['segments'][0].update(segment=True))
    mutate('fractional-boundary', lambda c: c['segments'][0]['frames'][0].update(sequence=2.5))
    mutate('missing-terminal-frame', lambda c: c['segments'][-1].update(frames=[]))
    mutate('unpublished-tail', lambda c: c['segments'][0]['frames'].pop())
    mutate('duplicate-frame', lambda c: c['segments'][0]['frames'].append(frame(3)))
    mutate('out-of-range-frame', lambda c: c['segments'][0]['frames'][-1].update(sequence=4))
    mutate('oversized-extent', lambda c: c['segments'][0]['frames'][0].update(width=2049))
    mutate('unknown-oracle-field', lambda c: c.update(oracle='original.565'))
    mutate('changed-producer-hash', lambda c: c['segments'][0]['producer'].update(sha256='0'*64))
    mutate('changed-unused-asset', lambda c: c['assets'].update({'unused.dat': '0'*64}))
    mutate('unbound-sprite-catalogue', lambda c: c['assets'].pop('owned.spr'))
    mutate('producer-traversal', lambda c: c['segments'][0]['producer'].update(path='../outside.bin'))
    mutate('asset-traversal', lambda c: c['assets'].update({'../outside.bmp': digest(bitmap())}))
    def escape(c, source):
        outside = source.parent/'outside.bin'
        outside.write_bytes(fixtures['segment-1.bin'])
        (source/'escape.bin').symlink_to(outside)
        c['segments'][0]['producer']['path'] = 'escape.bin'
    execute('producer-symlink-escape', change=escape)
    def catalogue_escape(c, source):
        outside = source.parent/'outside.spr'
        outside.write_bytes(fixtures['owned.spr'])
        (source/'escape.spr').symlink_to(outside)
        c['assets']['escape.spr'] = digest(fixtures['owned.spr'])
    execute('catalogue-symlink-escape', change=catalogue_escape)
    # Runtime refusals must discard actual GL resources too; put the fault first.
    mutate('undefined-frame', lambda c: (c['segments'].pop(0), c['segments'].insert(0, dict(
        segment=1, stage='menu', producer=segments[0]['producer'], frames=[frame(1), frame(3)]))))
    mutate('wrong-frame-canvas', lambda c: c['segments'][0]['frames'][0].update(canvas=8))
    mutate('wrong-frame-extent', lambda c: c['segments'][0]['frames'][0].update(width=4))
    mutate('wrong-completed-queue', lambda c: [f.update(queues=1) for f in c['segments'][0]['frames']])
    mutate('unbound-source', lambda c: (c['assets'].pop('owned.bmp'), c.update(segments=[dict(
        segment=1, stage='results', producer=segments[4]['producer'], frames=[frame(3)])])))
    def changed_records(c, source, records):
        data = stream(records)
        (source/'segment-1.bin').write_bytes(data)
        c['segments'] = [dict(segment=1, stage='world', producer=dict(path='segment-1.bin', sha256=digest(data)), frames=[frame(len(records))])]
    execute('source-sequence-gap', change=lambda c, s: changed_records(c, s, [record(1, 1), record(3, 5)]))
    execute('unfinished-world-publication', change=lambda c, s: changed_records(c, s, [record(1, 1), record(2, 5), record(3, 11)]))
    # Structurally closed minimal raster packet, with a forbidden observed EAX.
    packet = bytearray(40)
    struct.pack_into('<I', packet, 0, 40)
    raster = bytearray(record(4, 9, payload=packet))
    struct.pack_into('<I', raster, 19*4, 40)
    struct.pack_into('<I', raster, 21*4, 7)
    execute('observed-raster-result', change=lambda c, s: changed_records(c, s,
        [record(1, 1), record(2, 5), record(3, 11), bytes(raster), record(5, 12)]))
    execute('existing-output-refusal', existing=True)
    return dict(success=True, cases=observed, complete_native_frames=len(expected),
                compared_native_pixels=sum(len(p)//2 for p in expected), remaining_surfaces=0,
                scope='Owned synthetic whole native planes and CLI refusal/cleanup; no original equivalence or physical presentation.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--artifacts', type=Path)
    args = parser.parse_args()
    parent = ROOT/'working/tests/draw03-session'
    parent.mkdir(parents=True, exist_ok=True)
    run = args.artifacts or Path(tempfile.mkdtemp(prefix='runner-', dir=parent))
    run.mkdir(parents=True, exist_ok=True)
    print(json.dumps(check(args.binary.resolve(), run)))


if __name__ == '__main__':
    main()
