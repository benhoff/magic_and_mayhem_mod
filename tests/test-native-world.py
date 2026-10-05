#!/usr/bin/env python3
"""Independent native-v1 wire oracle, malformed input and process continuation."""
import pathlib
import struct
import subprocess
import sys
import tempfile


def digest(payload):
    value = 14695981039346656037
    for byte in payload:
        value = ((value ^ byte) * 1099511628211) & ((1 << 64) - 1)
    return value


def pack(payload, version=1):
    return b'MNMNWLD\0' + struct.pack('<IIQ', version, len(payload), digest(payload)) + payload


def oracle(sequence=0xffffffff, tick=42, phase20=19, phase90=89, budget=7, pending=True, cleaned=False):
    def blob(data):
        return struct.pack('<I', len(data)) + data
    def entity(family, kind, owner, x, y, target, data, clean=False):
        return (struct.pack('<IIIiiiB', family, kind, owner, x, y, 0, clean)
                + (b'\1' + struct.pack('<II', *target) if target else b'\0') + blob(data))
    payload = struct.pack('<IIIII', sequence, tick, phase20, phase90, budget)
    payload += blob(b'fixture/map') + blob(bytes([4, 0, 5])) + blob(bytes([6, 7]))
    payload += struct.pack('<I', 4)
    payload += struct.pack('<IB', 1, 1) + entity(0, 17, 2, -7, 12, None, bytes([0, 255, 9]), cleaned)
    payload += struct.pack('<IB', 1, 1) + entity(1, 8, 0, 0, 0, (0, 1), bytes([1, 2, 3]))
    payload += struct.pack('<IB', 1, 0) * 2
    payload += struct.pack('<I', int(pending))
    if pending:
        payload += struct.pack('<IIIB', 1, 0, 1, 0)
    return pack(payload)


def main():
    executable = str(pathlib.Path(sys.argv[1]).resolve())
    def run(*args, good=True):
        result = subprocess.run([executable, *map(str, args)], capture_output=True, text=True)
        assert result.returncode == (0 if good else 1), (args, result.stdout, result.stderr)
        assert 'ERROR: AddressSanitizer' not in result.stderr and 'runtime error:' not in result.stderr, result.stderr
        return result
    with tempfile.TemporaryDirectory(prefix='mnm-world-oracle-') as directory:
        root = pathlib.Path(directory)
        initial = root / 'initial.mnw'
        run('create', initial)
        assert initial.read_bytes() == oracle()
        # Refusal must leave bytes unchanged, and zero ticks retain pending commands.
        run('create', initial, good=False)
        assert initial.read_bytes() == oracle()
        zero = root / 'zero.mnw'
        run('resume', initial, zero, 0)
        assert zero.read_bytes() == initial.read_bytes()
        whole = root / 'whole.mnw'
        first = root / 'first.mnw'
        split = root / 'split.mnw'
        run('resume', initial, whole, 200)
        run('resume', initial, first, 73)
        run('resume', first, split, 127)
        assert whole.read_bytes() == split.read_bytes() == oracle(199, 242, 19, 19, 53, False, True)
        # Independently encoded data loads without a native encode/decode round trip.
        independent = root / 'python.mnw'
        independent.write_bytes(oracle())
        run('inspect', independent)
        # Check every byte mutation (header and payload) and every incomplete prefix.
        invalid = root / 'invalid.mnw'
        reference = oracle()
        for offset in range(len(reference)):
            changed = bytearray(reference)
            changed[offset] ^= 1
            invalid.write_bytes(changed)
            run('inspect', invalid, good=False)
            invalid.write_bytes(reference[:offset])
            run('inspect', invalid, good=False)
        # Recomputed digests exercise validation beyond the corruption checksum.
        payload = bytearray(reference[24:])
        bad_phase = bytearray(payload)
        struct.pack_into('<I', bad_phase, 12, 90)
        for data in (pack(payload, 2), pack(payload + b'\0'), pack(bad_phase)):
            invalid.write_bytes(data)
            run('inspect', invalid, good=False)
        # Find the target field via a separate cursor, then make it dangling.
        offset = 20
        for _ in range(3):
            length, = struct.unpack_from('<I', payload, offset)
            offset += 4 + length
        offset += 4  # slot count
        offset += 5 + 25 + 1 + 4 + 3  # first slot + entity + absent target + state
        target_slot = offset + 5 + 25 + 1  # second slot + entity + presence
        dangling = bytearray(payload)
        struct.pack_into('<I', dangling, target_slot, 99)
        invalid.write_bytes(pack(dangling))
        run('inspect', invalid, good=False)
        output = root / 'refused.mnw'
        run('resume', invalid, output, 1, good=False)
        assert not output.exists()
        assert not list(root.glob('.*'))
    print('independent bytes, malformed data, refusal and fresh-process continuation passed')


if __name__ == '__main__':
    main()
