#!/usr/bin/env python3
"""Fresh-process campaign checkpoints and independent native campaign wire checks."""
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

binary = Path(sys.argv[1]).resolve()

def run(*args, success=True):
    result = subprocess.run([str(binary), *map(str, args)], capture_output=True, text=True)
    assert (result.returncode == 0) == success, (args, result.stdout, result.stderr)

with tempfile.TemporaryDirectory(prefix='mnm-campaign-') as tmp:
    root = Path(tmp)
    initial, active, restored, direct = [root / name for name in ('initial.mnc', 'active.mnc', 'restored.mnc', 'direct.mnc')]
    run('create', initial)
    original = initial.read_bytes()
    run('create', initial, success=False)
    assert initial.read_bytes() == original
    run('advance', initial, active, 3, 'new')
    run('advance', active, restored, 9, 'new')
    run('advance', initial, direct, 12, 'new')
    assert restored.read_bytes() == direct.read_bytes(), 'fresh-process continuation diverged'
    before = active.read_bytes()
    run('advance', initial, active, 12, 'new', success=False)
    assert active.read_bytes() == before, 'overwrite refusal changed old file'
    run('advance', initial, active, 12, 'overwrite')
    assert active.read_bytes() == direct.read_bytes()
    run('create', root / 'missing' / 'new.mnc', success=False)
    corrupt = root / 'corrupt.mnc'
    damaged = bytearray(original)
    damaged[-1] ^= 1
    corrupt.write_bytes(damaged)
    run('advance', corrupt, root / 'refused.mnc', 1, 'new', success=False)
    assert not (root / 'refused.mnc').exists()
    # Independent little-endian walk: find the nested campaign block in the
    # enclosing snapshot, then assert all fixture fields and the exact endpoint.
    data = original
    start = data.index(b'MNMCAMP\0')
    assert struct.unpack_from('<I', data, start - 4)[0] > 12
    length = struct.unpack_from('<I', data, start - 4)[0]
    data = data[start:start + length]
    pos = 8
    def word(fmt='<I'):
        global pos
        value = struct.unpack_from(fmt, data, pos)[0]
        pos += struct.calcsize(fmt)
        return value
    def string():
        global pos
        size = word()
        value = data[pos:pos + size].decode('ascii')
        pos += size
        return value
    def strings():
        return [string() for _ in range(word())]
    assert word() == 1
    assert (string(), string()) == ('synthetic-campaign', 'fixture-content-v1')
    assert word() == 3 and word('<Q') == 0
    assert (string(), string(), string(), word('<B')) == ('player', 'first', 'entry', 0)
    assert word() == 2
    assert string() == 'player' and word('<Q') == 2**64 - 1
    assert [word('<i') for _ in range(6)] == [-2**31, -1, 0, 1, 42, 2**31 - 1]
    assert strings() == ['summon', 'heal'] and strings() == ['token', 'token']
    assert string() == 'other' and word('<Q') == 0
    assert [word('<i') for _ in range(6)] == [0] * 6
    assert strings() == [] and strings() == []
    assert word() == 2 and string() == 'first' and word() == 2
    assert (string(), word('<B'), string(), word('<B')) == ('entry', 1, 'other', 0)
    assert (string(), word('<B'), word('<B')) == ('exit', 0, 0)
    assert string() == 'second' and word() == 1
    assert (string(), word('<B'), word('<B')) == ('entry', 0, 0)
    assert word('<B') == 0 and pos == len(data)
    assert not list(root.glob('.*.??????')), 'temporary publication files leaked'
print('Native campaign fresh-process, wire, overwrite and corrupt-input checks passed.')
