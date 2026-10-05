#!/usr/bin/env python3
"""Independent v6 bytes and terrain/vertical continuation in fresh processes."""
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('ani', ROOT/'tests/test-native-ani-motion.py')
ani = importlib.util.module_from_spec(spec);spec.loader.exec_module(ani)
fixture = ani.segment.old.fixture
digest = ani.segment.old.digest


def wrap(payload):
    return b'MNMNWLD\0'+struct.pack('<IIQ', 6, len(payload), digest(payload))+payload


def fine(fields):
    return struct.pack('<13i3I2i', *fields)+b'\0'  # optional ANI cursor absent


def oracle(path, raw, tick, index, current=None, history=None, segment_ticks=0, pending=False):
    def blob(value): return struct.pack('<I', len(value))+value
    data = struct.pack('<5I', tick, tick, tick % 20, tick % 90, 53 if tick else 0)
    data += blob(str(path.resolve()).encode())+blob(b'')+blob(b'')
    data += b'\1'+struct.pack('<3iQ', 12, 12, 5, digest(raw))+b'\0'
    data += struct.pack('<I', 8)
    data += struct.pack('<IBIIIiiiBBI', 1, 1, 0, 0, 0, 1+index, 1, 1, 0, 0, 0)+b'\1'
    data += struct.pack('<I3i3iBIII', 0 if pending else 3 if index == 4 else 2,
                        1, 1, 1, 1 if pending else 5, 1, 1, 0, 300, index, 0 if pending else 4)
    if not pending:
        for x in range(2, 6): data += struct.pack('<6iI', x, 1, 1, 2, 0, 0, 720)
    data += b'\1'+bytes([current is not None])
    if current: data += fine(current)
    data += b'\1'+struct.pack('<I', segment_ticks)+bytes([history is not None])
    if history: data += struct.pack('<3i', 2, 0, 0)+fine(history)+struct.pack('<3i', index, 1, 1)
    data += b'\1'  # terrain driver selection
    data += struct.pack('<IB', 1, 0)*7+struct.pack('<I', int(pending))
    if pending: data += struct.pack('<IIIBB3i', 3, 0, 1, 0, 1, 5, 1, 1)
    return wrap(data)


def main():
    binary = str(Path(sys.argv[1]).resolve())
    def run(*args, good=True):
        p = subprocess.run([binary, *map(str, args)], capture_output=True, text=True)
        assert p.returncode == (0 if good else 1), (args, p.stdout, p.stderr)
        assert 'AddressSanitizer' not in p.stderr and 'runtime error:' not in p.stderr, p.stderr
        return p.stdout
    with tempfile.TemporaryDirectory(prefix='mnm-terrain-motion-') as folder:
        root = Path(folder);path = root/'terrace';raw = fixture.terrain_fixture();path.write_bytes(raw)
        asset = root/'motion.ani';asset.write_bytes(ani.ani())
        def move(output, tick, resource=path, start=(1,1,1), target=(5,1,1), animated=False):
            prefix = ['move-terrain-ani', resource, asset, 8] if animated else ['move-terrain', resource]
            return run(*prefix, output, *start, *target, tick)
        initial, half, boundary, middle, whole = [root/name for name in ['initial','half','boundary','middle','whole']]
        move(initial, 0);assert initial.read_bytes() == oracle(path, raw, 0, 0, pending=True)
        first = [720,720,16,4,0,60,60,0,42,32,17,-10,0,1,1,0,0,0]
        move(half, 2);assert half.read_bytes() == oracle(path, raw, 2, 0, first, segment_ticks=1)
        history = [720,720,16,4,0,240,240,0,72,32,21,-40,0,4,4,0,0,0]
        move(boundary, 5);assert boundary.read_bytes() == oracle(path, raw, 5, 1, history=history)
        current = [720,720,20,4,0,108,108,0,82,32,22,-58,0,5,5,0,0,0]
        move(middle, 6);assert middle.read_bytes() == oracle(path, raw, 6, 1, current, history, 1)
        move(whole, 24)
        python_save = root/'python';python_save.write_bytes(oracle(path, raw, 6, 1, current, history, 1))
        python_out = root/'python-out';run('resume', python_save, python_out, 18);assert python_out.read_bytes() == whole.read_bytes()
        trace = [json.loads(row) for row in run('trace', initial, 24).splitlines()]
        assert [trace[i]['entities'][0]['fine'][2] for i in range(7)] == [16,16,17,18,19,20,22]
        assert trace[14]['entities'][0]['position'] == [5,1,1]
        cases = [('terrace', path, (1,1,1), (5,1,1)), ('turn', path, (1,1,1), (5,3,1)),
                 ('seam', path, (0,1,1), (11,1,1))]
        slope = root/'slope';slope.write_bytes(fixture.terrain_fixture('slope'))
        cases += [('up-slope', slope, (1,1,1), (5,1,2)), ('down-slope', slope, (5,1,2), (1,1,1))]
        vertical = root/'vertical';vertical.write_bytes(fixture.terrain_fixture('vertical'))
        cases += [('up', vertical, (1,1,2), (1,1,3)), ('down', vertical, (1,1,3), (1,1,2)),
                  ('up-continuous', vertical, (1,1,2), (1,1,4)), ('down-continuous', vertical, (1,1,4), (1,1,2)),
                  ('category-four', vertical, (1,1,2), (5,1,2))]
        longmap = root/'long';longmap.write_bytes(fixture.fixture(width=4, height=40, layers=5))
        cases += [('prefix', longmap, (1,1,1), (1,18,1))]
        sizes = struct.unpack_from('<7I', raw, 64);scalar_at = 92+sum(sizes[:5])
        slow = bytearray(raw);struct.pack_into('<i', slow, scalar_at+0x10, 64)
        slowmap = root/'slow';slowmap.write_bytes(slow)
        cases += [('fractional', slowmap, (1,1,1), (5,1,1))]
        restarts = 0
        for animated in (False, True):
            for name, resource, start, target in cases:
                tag = name+str(animated);end = root/(tag+'-end');move(end, 90, resource, start, target, animated)
                row = json.loads(run('trace', end, 0))['entities'][0]
                assert row['position'] == list(target) and row['action'] == 3, (name, row)
                begin = root/(tag+'-begin');move(begin, 0, resource, start, target, animated)
                full = [json.loads(line) for line in run('trace', begin, 90).splitlines()]
                for tick in [1,2,3,5,6,8,10,13,40,53,54]:
                    saved = root/(tag+f'-{tick}');resumed = root/(tag+f'-resumed-{tick}')
                    move(saved, tick, resource, start, target, animated);run('resume', saved, resumed, 90-tick)
                    assert resumed.read_bytes() == end.read_bytes(), (name, animated, tick)
                    actual = [json.loads(line) for line in run('trace', saved, 90-tick).splitlines()]
                    assert actual == full[tick:], (name, animated, tick)
                    restarts += 1
        # Failed resource/restoration checks must not publish a checkpoint.
        refused = root/'refused'
        data = bytearray(middle.read_bytes()[24:]);tail = 7*5+4
        mode_at = len(data)-tail-1;history_at = mode_at-97;fine_at = history_at-6-73
        for at, value in [(fine_at+8,21), (fine_at+12,5), (fine_at+32,83),
                          (fine_at+52,6), (history_at+85,0)]:
            changed = bytearray(data);struct.pack_into('<i', changed, at, value)
            bad = root/'bad';bad.write_bytes(wrap(changed))
            run('resume', bad, refused, 1, good=False);assert not refused.exists()
        changed = bytearray(data);changed[mode_at] = 2
        bad = root/'bad-flag';bad.write_bytes(wrap(changed));run('inspect-json', bad, good=False)
        for name, offset, value in [('special-height', 92+sum(sizes[:6])+8, 2),
                                    ('unsupported-height', 92+sum(sizes[:3])+2*0x164+0x94, 17),
                                    ('reverse', 92+sum(sizes[:4])+0x722, 1)]:
            changed = bytearray(raw)
            if name == 'reverse': changed[offset] = value
            else: struct.pack_into('<i', changed, offset, value)
            resource = root/name;resource.write_bytes(changed)
            run('move-terrain', resource, refused, 2 if name == 'unsupported-height' else 1,1,1,5,1,1,2, good=False);assert not refused.exists()
        owned = root/'owned';ani_save = root/'ani-save';ani_end = root/'ani-end'
        move(ani_save, 6, animated=True);move(ani_end, 24, animated=True);asset.unlink()
        run('resume', ani_save, owned, 18);assert owned.read_bytes() == ani_end.read_bytes()
        path.write_bytes(raw[:-1]+bytes([raw[-1]^1]));run('resume', middle, refused, 1, good=False);assert not refused.exists()
        path.unlink();run('resume', middle, refused, 1, good=False);assert not refused.exists()
        if len(sys.argv)>2:
            path.write_bytes(raw)
            checked = subprocess.run([sys.argv[2], str(path)], capture_output=True, text=True)
            assert checked.returncode == 0, (checked.stdout, checked.stderr)
    print(f'Independent v6 bytes, {restarts} fresh-process restarts/traces, terrain/slopes/vertical/category-four/ANI, malformed/resource refusal passed')


if __name__ == '__main__': main()
