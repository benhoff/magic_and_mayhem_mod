#!/usr/bin/env python3
"""Synthetic native melee encounter, independent subsystem bytes and process restart."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('fixture', ROOT / 'tools/create-movement-fixture.py')
fixture = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixture)


def digest(data):
    value = 14695981039346656037
    for byte in data:
        value = ((value ^ byte) * 1099511628211) & ((1 << 64) - 1)
    return value


def system_offsets(raw):
    assert raw[:8] == b'MNMNWLD\0'
    _, length, checksum = struct.unpack_from('<IIQ', raw, 8)
    assert length == len(raw) - 24 and checksum == digest(raw[24:])
    offset = 24 + 20
    for _ in range(2):
        size, = struct.unpack_from('<I', raw, offset)
        offset += 4 + size
    size, = struct.unpack_from('<I', raw, offset)
    return offset, offset + 4, size


def replace_system(raw, system):
    prefix, start, size = system_offsets(raw)
    payload = raw[24:prefix] + struct.pack('<I', len(system)) + system + raw[start + size:]
    version, = struct.unpack_from('<I', raw, 8)
    return raw[:8] + struct.pack('<IIQ', version, len(payload), digest(payload)) + payload


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary', type=Path)
    parser.add_argument('--output', type=Path)
    parser.add_argument('--claims', type=Path)
    parser.add_argument('--contract-binary', type=Path)
    args = parser.parse_args()
    temporary = tempfile.TemporaryDirectory(prefix='mnm-combat-') if not args.output else None
    out = Path(temporary.name) if temporary else args.output.resolve()
    out.mkdir(parents=True, exist_ok=True)
    if any(out.iterdir()):
        raise ValueError('Refusing occupied output directory')
    declaration = json.loads(args.claims.read_text()) if args.claims else {'claims': [], 'sources': {}}
    sources = {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in declaration['sources']}
    if sources != declaration['sources']:
        raise ValueError('Source declaration changed before execution')
    binary = str(args.binary.resolve())
    binary_hash = hashlib.sha256(Path(binary).read_bytes()).hexdigest()
    contract_hash = None
    if args.contract_binary:
        contract_hash = hashlib.sha256(args.contract_binary.read_bytes()).hexdigest()
        result = subprocess.run([str(args.contract_binary.resolve())], capture_output=True, text=True, timeout=30)
        (out / 'contract.stdout').write_text(result.stdout)
        (out / 'contract.stderr').write_text(result.stderr)
        if result.returncode or 'ERROR: AddressSanitizer' in result.stderr or 'runtime error:' in result.stderr:
            raise AssertionError((result.returncode, result.stdout, result.stderr))

    def run(*arguments, good=True):
        result = subprocess.run([binary, *map(str, arguments)], capture_output=True, text=True, timeout=30)
        if result.returncode != (0 if good else 1) or 'ERROR: AddressSanitizer' in result.stderr or 'runtime error:' in result.stderr:
            raise AssertionError((arguments, result.stdout, result.stderr))
        return [json.loads(row) for row in result.stdout.splitlines()]

    map_file, initial = out / 'synthetic.frozen', out / 'initial.mnms'
    raw_map = fixture.fixture()
    map_file.write_bytes(raw_map)
    first = run('create', map_file, initial)[0]
    assert first['tick'] == 0 and all(f['health'] == 20 and f['action'] == 'idle' for f in first['fighters'])
    raw = initial.read_bytes()
    _, start, size = system_offsets(raw)
    expected = b'MNMCOM01' + struct.pack('<2I', 12, 2)
    for slot, damage in [(0, 6), (1, 4)]:
        expected += struct.pack('<12I', slot, 1, 20, damage, 2, 3, 20, 0, 0, 0, 0, 0)
    assert raw[start:start + size] == expected
    independent = out / 'independent.mnms'
    independent.write_bytes(replace_system(raw, expected))
    assert run('inspect', independent) == [first]

    trace = run('trace', initial, 40)
    (out / 'encounter.json').write_text(json.dumps(trace, indent=2) + '\n')
    events = [(row['tick'], e) for row in trace for e in row['events']]
    assert sum(e['kind'] == 'acquired' for _, e in events) == 2
    assert any(e['kind'] == 'approach' for _, e in events)
    hits = [(tick, e) for tick, e in events if e['kind'] == 'hit']
    deaths = [(tick, e) for tick, e in events if e['kind'] == 'died']
    assert len(hits) == 7 and len(deaths) == 1
    assert hits[0][0] >= 3 and all(e['amount'] in (2, 4, 6) for _, e in hits)
    assert [f['health'] for f in trace[-1]['fighters']] == [8, 0]
    assert trace[-1]['fighters'][1]['cleaned'] and trace[-1]['fighters'][1]['action'] == 'dead'
    assert trace[-1]['fighters'][0]['target'] is None
    # Independent event oracle: windup is two admitted ticks; ordinary surviving
    # targets receive fixed damage, and slot-ordered lethal hits cancel retaliation.
    health = {0: 20, 1: 20}
    windup = {}
    for row in trace:
        for e in row['events']:
            actor, target = e['actor'][0], e['target'][0]
            if e['kind'] == 'windup':
                windup[actor] = row['tick']
            if e['kind'] == 'hit':
                assert row['tick'] == windup[actor] + 2 and health[actor] > 0
                expected_damage = min(6 if actor == 0 else 4, health[target])
                assert e['amount'] == expected_damage
                health[target] -= expected_damage
        assert [f['health'] for f in row['fighters']] == [health[0], health[1]]
    assert all(row['searchExpansions'] <= 53 for row in trace)

    whole = out / 'whole.mnms'
    run('resume', initial, whole, 40)
    continuations = 0
    phases = set()
    for split in range(25):
        half, resumed = out / f'half-{split}.mnms', out / f'resumed-{split}.mnms'
        run('resume', initial, half, split)
        resumed_rows = run('resume', half, resumed, 40 - split)
        assert resumed.read_bytes() == whole.read_bytes()
        assert resumed_rows == trace[split + 1:]
        phases.update(f['action'] for f in trace[split]['fighters'])
        continuations += 1
    assert {'idle', 'approaching', 'windingUp', 'recovering', 'dead'} <= phases

    # The recovered intra-cell motion driver and its reservations also continue
    # exactly through a native encounter. No installed creature assets are used.
    slow = bytearray(raw_map)
    sizes = struct.unpack_from('<7I', slow, 64)
    scalar = 92 + sum(sizes[:5])
    struct.pack_into('<i', slow, scalar + 16, 10)
    for i in range(48):
        struct.pack_into('<I', slow, scalar + 0xd8 + i * 4, 8)
    slow_map, fine_initial = out / 'slow.frozen', out / 'fine-initial.mnms'
    slow_map.write_bytes(slow)
    run('create-fine', slow_map, fine_initial)
    fine_trace = run('trace', fine_initial, 512)
    (out / 'fine-encounter.json').write_text(json.dumps(fine_trace, indent=2) + '\n')
    assert [f['health'] for f in fine_trace[-1]['fighters']] == [8, 0]
    fine_whole = out / 'fine-whole.mnms'
    run('resume', fine_initial, fine_whole, 512)
    for split in [1, 2, 5, 17, 40, 80, 100]:
        half, resumed = out / f'fine-half-{split}.mnms', out / f'fine-resumed-{split}.mnms'
        run('resume', fine_initial, half, split)
        rows = run('resume', half, resumed, 512 - split)
        assert fine_whole.read_bytes() == resumed.read_bytes() and rows == fine_trace[split + 1:]
        continuations += 1

    refusals = 0
    run('create', map_file, initial, good=False)
    assert initial.read_bytes() == raw
    refusals += 1
    invalid, destination = out / 'invalid.mnms', out / 'refused.mnms'
    for bad in [expected[:-1], expected + b'\0', b'UNKNOWN!' + expected[8:],
                expected[:12] + struct.pack('<I', 33) + expected[16:]]:
        invalid.write_bytes(replace_system(raw, bad))
        run('resume', invalid, destination, 1, good=False)
        assert not destination.exists()
        refusals += 1
    # Syntactically valid, checksummed corrupt semantic states must also refuse.
    for offset, value in [(16 + 4, 2), (16 + 24, 21), (16 + 28, 99), (16 + 32, 1)]:
        bad = bytearray(expected)
        struct.pack_into('<I', bad, offset, value)
        invalid.write_bytes(replace_system(raw, bad))
        run('resume', invalid, destination, 1, good=False)
        assert not destination.exists()
        refusals += 1
    corrupt = bytearray(raw);corrupt[-1] ^= 1;invalid.write_bytes(corrupt)
    run('resume', invalid, destination, 1, good=False)
    assert not destination.exists()
    refusals += 1
    changed = bytearray(raw_map);changed[-1] ^= 1;map_file.write_bytes(changed)
    run('resume', initial, destination, 1, good=False)
    assert not destination.exists()
    refusals += 1
    map_file.unlink()
    run('resume', initial, destination, 1, good=False)
    assert not destination.exists()
    refusals += 1
    map_file.write_bytes(raw_map)
    for n in ['-1', '4097', '2junk']:
        run('resume', initial, destination, n, good=False)
        assert not destination.exists()
        refusals += 1
    assert sources == {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in sources}
    assert binary_hash == hashlib.sha256(Path(binary).read_bytes()).hexdigest()
    if args.contract_binary:
        assert contract_hash == hashlib.sha256(args.contract_binary.read_bytes()).hexdigest()
    summary = {'all_match': True, 'scope': 'Synthetic headless native melee policy; no original combat comparison, installed-world or live replacement.',
               'claims': declaration['claims'], 'source_sha256': sources, 'binary_sha256': binary_hash,
               'input_sha256': {'synthetic.frozen': hashlib.sha256(raw_map).hexdigest(), 'slow.frozen': hashlib.sha256(slow).hexdigest()},
               'contract_passed': args.contract_binary is not None, 'contract_binary_sha256': contract_hash,
               'ticks': 40, 'hits': len(hits), 'deaths': len(deaths), 'fresh_process_continuations': continuations,
               'refusals': refusals, 'phases': sorted(phases), 'encounter_sha256': hashlib.sha256((out / 'encounter.json').read_bytes()).hexdigest(),
               'fine_encounter_sha256': hashlib.sha256((out / 'fine-encounter.json').read_bytes()).hexdigest()}
    (out / 'report.json').write_text(json.dumps(summary, indent=2) + '\n')
    print(json.dumps({k: v for k, v in summary.items() if k not in ('claims', 'source_sha256')}, indent=2))


if __name__ == '__main__':
    main()
