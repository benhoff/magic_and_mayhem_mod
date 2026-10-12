#!/usr/bin/env python3
"""Authored native skirmish, independent wire fields and fresh-process continuation."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('combat_test', ROOT / 'tests/test-native-combat.py')
wire = importlib.util.module_from_spec(spec)
spec.loader.exec_module(wire)


def campaign_offsets(raw):
    offset = 44
    size, = struct.unpack_from('<I', raw, offset)
    offset += 4 + size
    size, = struct.unpack_from('<I', raw, offset)
    return offset, offset + 4, size


def replace_campaign(raw, campaign):
    prefix, start, size = campaign_offsets(raw)
    payload = raw[24:prefix] + struct.pack('<I', len(campaign)) + campaign + raw[start + size:]
    version, = struct.unpack_from('<I', raw, 8)
    return raw[:8] + struct.pack('<IIQ', version, len(payload), wire.digest(payload)) + payload


def declaration(path):
    result = json.loads(path.read_text()) if path else {'claims': [], 'sources': {}}
    assert result['sources'] == {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in result['sources']}
    return result


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('binary', type=Path)
    p.add_argument('--output', type=Path)
    p.add_argument('--claims', type=Path)
    p.add_argument('--contract-binary', type=Path)
    p.add_argument('--health-contract-binary', type=Path)
    args = p.parse_args()
    claims = declaration(args.claims)
    temporary = tempfile.TemporaryDirectory(prefix='mnm-skirmish-') if not args.output else None
    out = Path(temporary.name) if temporary else args.output.resolve()
    out.mkdir(parents=True, exist_ok=True)
    assert not any(out.iterdir()), 'Refusing occupied output'

    def run(*arguments, good=True):
        r = subprocess.run([str(args.binary.resolve()), *map(str, arguments)], capture_output=True, text=True, timeout=30)
        assert r.returncode == (0 if good else 1), (arguments, r.stdout, r.stderr)
        assert 'ERROR: AddressSanitizer' not in r.stderr and 'runtime error:' not in r.stderr
        return [json.loads(row) for row in r.stdout.splitlines()]

    contract = None
    if args.contract_binary:
        r = subprocess.run([str(args.contract_binary.resolve())], capture_output=True, text=True, timeout=30)
        (out / 'contract.stdout').write_text(r.stdout)
        (out / 'contract.stderr').write_text(r.stderr)
        assert not r.returncode and 'runtime error:' not in r.stderr and 'ERROR: AddressSanitizer' not in r.stderr, r.stderr
        contract = hashlib.sha256(args.contract_binary.read_bytes()).hexdigest()
    health_contract = None
    if args.health_contract_binary:
        r = subprocess.run([str(args.health_contract_binary.resolve())], capture_output=True, text=True, timeout=30)
        (out / 'health-contract.stdout').write_text(r.stdout)
        (out / 'health-contract.stderr').write_text(r.stderr)
        assert not r.returncode and 'runtime error:' not in r.stderr and 'ERROR: AddressSanitizer' not in r.stderr, r.stderr
        health_contract = hashlib.sha256(args.health_contract_binary.read_bytes()).hexdigest()
    initial = out / 'initial.mnms'
    first = run('create', initial)[0]
    assert first['healthRules'] == 0x4e53323348454131
    assert first['tick'] == 0 and [u['health'] for u in first['units']] == [48, 48]
    raw = initial.read_bytes()
    _, start, size = campaign_offsets(raw)
    words = [1, 20, 20, 3, 8, 4, 2, 4, 10, 48, 24, 6, 8, 6, 8, 8, 3]
    rule_hash = wire.digest(struct.pack('<17I', *words))
    expected = b'MNMARM01' + struct.pack('<Q3I', rule_hash, 0, 0, 1)
    for slot in range(2):
        expected += struct.pack('<12I', slot, 1, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    expected += struct.pack('<I6I2I', 2, 0, 1, 0, 1, 1, 0, 0, 0)
    assert raw[start:start + size] == expected
    independent = out / 'independent.mnms'
    independent.write_bytes(replace_campaign(raw, expected))
    assert run('inspect', independent) == [first]
    script = out / 'victory.txt'
    script.write_text('0 summon 0 2 5\n3 bolt 0 1\n6 bolt 0 1\n9 bolt 0 1\n12 bolt 0 1\n')
    final = out / 'victory.mnms'
    trace = run('resume', initial, final, 100, script)
    assert trace[-1]['outcome'] == 1 and trace[-1]['units'][1]['health'] == 0
    (out / 'victory.json').write_text(json.dumps(trace, indent=2) + '\n')
    # Independent phase oracle for every player cast: admission keeps full
    # windup; launch occurs two ticks later; impact four ticks after launch.
    casting = [row['tick'] for row in trace for e in row['events'] if e['actor'] == 0 and e['kind'] == 0]
    launched = [row['tick'] for row in trace for e in row['events'] if e['actor'] == 0 and e['kind'] == 2]
    impacts = [row['tick'] for row in trace for e in row['events'] if e['actor'] == 0 and e['kind'] == 3]
    assert casting == [1, 4, 7, 10, 13]
    assert launched == [6, 9, 12, 15] and impacts == [10, 13, 16, 19]
    assert trace[0]['wizards'][0]['mana'] == 12 and trace[2]['wizards'][0]['mana'] == 13
    assert len(trace[2]['units']) == 4
    continuations = 0
    for cut in range(1, len(trace) + 1):
        partial, resumed = out / f'cut-{cut}.mnms', out / f'resumed-{cut}.mnms'
        before = run('resume', initial, partial, cut, script)
        after = run('resume', partial, resumed, 100, script)
        assert before + after == trace and resumed.read_bytes() == final.read_bytes(), cut
        continuations += 1
    defeat = out / 'defeat.mnms'
    idle = run('resume', initial, defeat, 200)
    assert idle[-1]['outcome'] == 2 and idle[-1]['units'][0]['health'] == 0
    frozen = out / 'terminal.mnms'
    assert run('resume', defeat, frozen, 10) == [] and frozen.read_bytes() == defeat.read_bytes()
    refusals = 0
    corruptions = [expected + b'\0', expected[:27], b'BADARM01' + expected[8:]]
    for offset, value in [(8, 0), (16, 1), (28 + 8, 21), (28 + 16, 99), (28 + 20, 1), (124, 33), (136, 1)]:
        bad = bytearray(expected)
        struct.pack_into('<I', bad, offset, value)
        corruptions.append(bad)
    for i, bad in enumerate(corruptions):
        path = out / f'bad-{i}.mnms'
        path.write_bytes(replace_campaign(raw, bad))
        run('inspect', path, good=False)
        refusals += 1
    _, system_start, system_size = wire.system_offsets(raw)
    for identity in [0, 99]:
        system = bytearray(raw[system_start:system_start + system_size])
        assert system[:8] == b'MNMCOM03'
        struct.pack_into('<Q', system, 16, identity)
        path = out / f'bad-health-rules-{identity}.mnms'
        path.write_bytes(wire.replace_system(raw, system))
        run('inspect', path, good=False)
        refusals += 1
    bad_script = out / 'bad-script.txt'
    for content in ['0 summon 1 5 2\n', '0 move 0 1 3\n', '0 bolt 0 0\n', '0 summon 0 0 0\n', '0 stop 31\n', '0 bogus 0\n']:
        bad_script.write_text(content)
        target = out / f'refused-{refusals}.mnms'
        run('resume', initial, target, 1, bad_script, good=False)
        assert not target.exists()
        refusals += 1
    assert claims['sources'] == {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in claims['sources']}
    report = dict(all_match=True, health_backend_identity=first['healthRules'], health_contract_passed=bool(args.health_contract_binary), health_contract_binary_sha256=health_contract, contract_passed=bool(args.contract_binary), claims=claims['claims'], source_sha256=claims['sources'], binary_sha256=hashlib.sha256(args.binary.read_bytes()).hexdigest(), contract_binary_sha256=contract, fresh_process_continuations=continuations, refusals=refusals, victory_tick=trace[-1]['tick'], defeat_tick=idle[-1]['tick'], original_comparison=False, wine_required=False)
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report))


if __name__ == '__main__':
    main()
