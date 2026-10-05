#!/usr/bin/env python3
"""Frozen recovered routing + native movement checkpoint/process integration."""
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('movement_fixture', ROOT / 'tools/create-movement-fixture.py')
fixture = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixture)


def digest(data):
    value = 14695981039346656037
    for byte in data:
        value = ((value ^ byte) * 1099511628211) & ((1 << 64) - 1)
    return value


def payload(data):
    assert data[:8] == b'MNMNWLD\0'
    version, length, checksum = struct.unpack_from('<IIQ', data, 8)
    assert version == 2 and length == len(data) - 24 and checksum == digest(data[24:])
    return data[24:]


def oracle(map_path, map_bytes, tick, next_point, pending=False):
    """Independent complete v2 fixture bytes, including recovered route metadata."""
    def blob(data):
        return struct.pack('<I', len(data)) + data
    data = struct.pack('<5I', tick, tick, tick % 20, tick % 90, 53 if tick else 0)
    data += blob(str(map_path).encode()) + blob(b'') + blob(b'')
    data += b'\1' + struct.pack('<3iQ', 12, 12, 3, digest(map_bytes))
    data += struct.pack('<I', 8)
    position = 1 + next_point
    data += struct.pack('<IBIIIiiiBBI', 1, 1, 0, 0, 0, position, 1, 1, 0, 0, 0)
    data += b'\1'  # typed motion present
    action = 0 if pending else 3 if next_point == 4 else 2
    destination = 1 if pending else 5
    data += struct.pack('<I3i3iBIII', action, 1, 1, 1, destination, 1, 1, 0, 300, next_point, 0 if pending else 4)
    if not pending:
        for x in range(2, 6):
            data += struct.pack('<6iI', x, 1, 1, 2, 0, 0, 720)
    data += struct.pack('<IB', 1, 0) * 7
    data += struct.pack('<I', int(pending))
    if pending:
        data += struct.pack('<IIIBB3i', 3, 0, 1, 0, 1, 5, 1, 1)
    return b'MNMNWLD\0' + struct.pack('<IIQ', 2, len(data), digest(data)) + data


def main():
    binary = str(Path(sys.argv[1]).resolve())
    def run(*args, good=True):
        result = subprocess.run([binary, *map(str, args)], text=True, capture_output=True)
        assert result.returncode == (0 if good else 1), (args, result.stdout, result.stderr)
        assert 'ERROR: AddressSanitizer' not in result.stderr and 'runtime error:' not in result.stderr, result.stderr
        return result
    with tempfile.TemporaryDirectory(prefix='mnm-native-movement-') as directory:
        root = Path(directory)
        map_file = root / 'map.bin'
        raw_map = fixture.fixture()
        map_file.write_bytes(raw_map)
        initial, half, whole, resumed = [root / name for name in ['initial.mnw', 'half.mnw', 'whole.mnw', 'resumed.mnw']]
        run('move', map_file, initial, 1, 1, 1, 5, 1, 1, 0)
        assert initial.read_bytes() == oracle(map_file, raw_map, 0, 0, True)
        run('resume', initial, half, 2)
        assert half.read_bytes() == oracle(map_file, raw_map, 2, 1)
        run('resume', initial, whole, 8)
        run('resume', half, resumed, 6)
        assert whole.read_bytes() == resumed.read_bytes() == oracle(map_file, raw_map, 8, 4)
        trace = [json.loads(line) for line in run('trace', initial, 8).stdout.splitlines()]
        assert [row['entities'][0]['position'] for row in trace] == [[x, 1, 1] for x in [1, 1, 2, 3, 4, 5, 5, 5, 5]]
        resumed_trace = [json.loads(line) for line in run('trace', half, 6).stdout.splitlines()]
        assert trace[2:] == resumed_trace
        # Independently constructed mid-route snapshot rebinds and continues too.
        independent = root / 'python.mnw'
        independent.write_bytes(oracle(map_file, raw_map, 2, 1))
        python_resume = root / 'python-resumed.mnw'
        run('resume', independent, python_resume, 6)
        assert python_resume.read_bytes() == whole.read_bytes()
        # Real adapter route output caps at 16; crossing that boundary must replan
        # from the new position and remain identical after a process restart.
        long_map = root / 'long-map.bin'
        long_map.write_bytes(fixture.fixture(width=40, height=4))
        long_half, long_whole, long_resumed = [root / name for name in ['long-half.mnw', 'long-whole.mnw', 'long-resumed.mnw']]
        run('move', long_map, long_half, 1, 1, 1, 18, 1, 1, 8)
        run('move', long_map, long_whole, 1, 1, 1, 18, 1, 1, 20)
        final = json.loads(run('resume', long_half, long_resumed, 12).stdout)
        assert long_whole.read_bytes() == long_resumed.read_bytes()
        assert final['entities'][0]['position'] == [18, 1, 1] and final['entities'][0]['action'] == 3
        # Toroidal seam direction and zero-distance admission use recovered routing.
        seam = root / 'seam.mnw'
        final = json.loads(run('move', map_file, seam, 0, 1, 1, 11, 1, 1, 2).stdout)
        assert final['entities'][0]['position'] == [11, 1, 1] and final['entities'][0]['route'][0][3] == 6
        stationary = root / 'stationary.mnw'
        final = json.loads(run('move', map_file, stationary, 1, 1, 1, 1, 1, 1, 1).stdout)
        assert final['entities'][0]['position'] == [1, 1, 1] and final['entities'][0]['action'] == 3
        # Confirm static terrain invalidity (not the selected occupancy bypass).
        blocked_map = root / 'blocked.bin'
        blocked_map.write_bytes(fixture.fixture(True))
        blocked = root / 'blocked.mnw'
        result = json.loads(run('move', blocked_map, blocked, 1, 1, 1, 5, 1, 1, 8).stdout)
        assert result['entities'][0]['position'] == [1, 1, 1] and result['entities'][0]['action'] == 4
        # Changed/missing maps must fail before publishing output.
        refused = root / 'refused.mnw'
        changed = bytearray(raw_map)
        changed[-1] ^= 1
        map_file.write_bytes(changed)
        run('resume', half, refused, 1, good=False)
        assert not refused.exists()
        map_file.unlink()
        run('resume', half, refused, 1, good=False)
        assert not refused.exists()
        map_file.write_bytes(raw_map)
        # Size/boolean/version/checksum/route integrity, with recomputed digests.
        reference = half.read_bytes()
        bad = root / 'bad.mnw'
        for size in range(len(reference)):
            bad.write_bytes(reference[:size])
            run('inspect-json', bad, good=False)
        for offset in range(len(reference)):
            changed = bytearray(reference)
            changed[offset] ^= 1
            bad.write_bytes(changed)
            run('inspect-json', bad, good=False)
        data = bytearray(payload(reference))
        offset = 20
        for _ in range(3):
            count, = struct.unpack_from('<I', data, offset)
            offset += 4 + count
        binding = offset
        motion = binding + 21 + 4 + 5 + 24 + 1 + 1 + 4 + 1
        for at, value in [(motion, 99), (motion + 33, 99), (motion + 37, 17), (motion + 41, 63)]:
            changed = bytearray(data)
            struct.pack_into('<I', changed, at, value)
            bad.write_bytes(b'MNMNWLD\0' + struct.pack('<IIQ', 2, len(changed), digest(changed)) + changed)
            run('inspect-json', bad, good=False)
        # Metadata corruption can be structurally valid but fail recovered edge validation.
        changed = bytearray(data)
        struct.pack_into('<i', changed, motion + 41 + 12, 6)  # wrong direction for +X
        bad.write_bytes(b'MNMNWLD\0' + struct.pack('<IIQ', 2, len(changed), digest(changed)) + changed)
        run('inspect-json', bad)  # legal numeric field, wrong recovered meaning
        run('resume', bad, refused, 1, good=False)
        assert not refused.exists() and not list(root.glob('.*'))
    print('recovered routing, independent v2 bytes, moving restart traces and map refusal passed')


if __name__ == '__main__':
    main()
