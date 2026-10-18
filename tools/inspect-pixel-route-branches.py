#!/usr/bin/env python3
"""Overapproximate direct x86 edges into the five unobserved producer bodies.

Every-byte candidates are not decoded instructions or original game callers.
Keep indirect/computed/external execution unresolved even when this scan is empty.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location(
    'entry_boundaries', ROOT / 'tools/inspect-pixel-route-entry-boundaries.py')
pe = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pe)
# Shared scanner bounds now match the existing recovered camera3 range.
WINDOWS = dict(pe.WINDOWS)


def branches(code, base):
    """32-bit code-segment candidates, including overlapping interpretations.

    Near rel16 with operand-size override clears upper EIP and cannot reach
    these >64KiB windows. Scanning its unprefixed interpretation deliberately
    overapproximates instead of discarding possibly prefixed data. Far immediate
    offsets retain the selector: its runtime base is not assumed to be zero.
    """
    for at, op in enumerate(code):
        size, kind, operand = 0, None, 1
        if op in (0xe8, 0xe9):
            size, kind = 5, 'call_rel32' if op == 0xe8 else 'jmp_rel32'
        elif op == 0xeb or 0x70 <= op <= 0x7f or 0xe0 <= op <= 0xe3:
            size, kind = 2, 'jmp_rel8' if op == 0xeb else 'jcc_rel8' if op < 0x80 else 'loop_rel8'
        elif op == 0x0f and at+1 < len(code) and 0x80 <= code[at+1] <= 0x8f:
            size, kind, operand = 6, 'jcc_rel32', 2
        elif op in (0x9a, 0xea):
            size, kind = 7, 'call_far32' if op == 0x9a else 'jmp_far32'
        if not size or at+size > len(code):
            continue
        row = dict(site=base+at, kind=kind, bytes=code[at:at+size].hex())
        if kind.endswith('far32'):
            row.update(target=struct.unpack_from('<I', code, at+1)[0],
                       selector=struct.unpack_from('<H', code, at+5)[0],
                       runtime_linear_target_known=False)
        else:
            displacement = int.from_bytes(code[at+operand:at+size], 'little', signed=True)
            row['target'] = (base+at+size+displacement) & 0xffffffff
        yield row


def scan(code, base, windows):
    result = {route: dict(start=lo, end=hi, internal_candidates=0,
                          external_candidates=[]) for route, (lo, hi) in windows.items()}
    for candidate in branches(code, base):
        for route, (lo, hi) in windows.items():
            if lo <= candidate['target'] < hi:
                if lo <= candidate['site'] < hi:
                    result[route]['internal_candidates'] += 1
                else:
                    result[route]['external_candidates'].append(candidate)
    return result


# Independently reviewed ordinary instruction alignment. This is not a proof
# that adversarial or dynamically constructed overlapping entry is impossible.
OVERLAPS = {
    0x54b5d5: (0x54b5d4, '03eb40', 'ADD EBP,EBX; INC EAX'),
    0x54b854: (0x54b851, '894424708b8424f4000000', 'MOV [ESP+70],EAX; MOV EAX,[ESP+f4]'),
    0x58d908: (0x58d906, '83e1e089542420', 'AND ECX,-32; MOV [ESP+20],EDX'),
}


def inspect(data):
    if hashlib.sha256(data).hexdigest() != pe.BUILD:
        raise ValueError('Unsupported original executable')
    image = pe.PE32(data)
    rows = {route: dict(start=lo, end=hi, internal_candidates=0,
                        external_candidates=[]) for route, (lo, hi) in WINDOWS.items()}
    for section in image.sections:
        if not section['executable']:
            continue
        code = data[section['offset']:section['offset']+section['size']]
        for route, result in scan(code, image.base+section['rva'], WINDOWS).items():
            rows[route]['internal_candidates'] += result['internal_candidates']
            for candidate in result['external_candidates']:
                candidate['review'] = 'unassigned_byte_candidate'
                if candidate['site'] in OVERLAPS:
                    anchor, expected, instruction = OVERLAPS[candidate['site']]
                    if image.read(anchor-image.base, len(bytes.fromhex(expected))) != bytes.fromhex(expected):
                        raise ValueError('Reviewed instruction bytes changed')
                    candidate.update(review='operand_overlap_in_reviewed_instruction_alignment',
                                     instruction_start=anchor, instruction_bytes=expected,
                                     instruction=instruction)
                rows[route]['external_candidates'].append(candidate)
    constant = 0x5c6020
    raw = image.read(constant-image.base, 8)
    consumers = []
    for site in (0x479f2e, 0x47a026):
        expected = b'\xdc\x05' + struct.pack('<I', constant)
        if image.read(site-image.base, 6) != expected:
            raise ValueError('Reviewed FADD instruction changed')
        consumers.append(dict(site=site, bytes=expected.hex(),
                              instruction='FADD QWORD PTR [0x5c6020]'))
    # Recheck stored VAs/RVAs/file offsets and PE roots against the corrected
    # extent too; the old shorter-window report cannot cover its missing tail.
    roots = pe.inspect_module(data, image, WINDOWS)
    return dict(routes=rows,
                stored_byte_candidates=roots['stored_byte_candidates'],
                boundary_candidates=roots['boundary_candidates'],
                data_candidate=dict(site=0x5c6022, dword=struct.unpack_from('<I', raw, 2)[0],
                                    aligned_storage=constant, bytes=raw.hex(),
                                    double=struct.unpack('<d', raw)[0], consumers=consumers,
                                    review='interior_bytes_of_arithmetic_double_constant'),
                camera_window_correction=dict(previous_end=0x553640, end=0x5536bc,
                                              terminal_ret=0x5536bb,
                                              recovered_range='minimap-outline-view3-5532f0'),
                global_unreachable_proved=False, original_game_executed=False,
                limits=['Byte candidates require instruction review; even a reviewed overlap cannot exclude dynamically chosen overlapping execution.',
                        'Indirect branches, computed/encoded pointers, runtime code changes, foreign/system modules and other builds remain unresolved.',
                        'Far offsets retain unknown segment selectors; no general protected-mode segment-base closure.',
                        'No route credit, dispatch modification, native replacement or denominator reduction.'])


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mode', choices=('controls', 'image'), required=True)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    declaration = json.loads(args.claims.read_text())
    if any(sha(ROOT/p) != h for p, h in declaration['sources'].items()):
        parser.error('Prospective source changed')
    report = dict(success=False, kind='static' if args.mode == 'image' else 'native_integration',
                  claims=declaration['claims'], sources=declaration['sources'],
                  original_game_executed=False)
    try:
        if args.mode == 'controls':
            runs = []
            for test in ('test-pixel-route-branches.py', 'test-pixel-entry-boundaries.py'):
                result = subprocess.run([sys.executable, '-B', ROOT/'tests'/test],
                                        capture_output=True, text=True, timeout=60)
                runs.append(dict(test=test, returncode=result.returncode,
                                 output=result.stdout+result.stderr))
            report.update(runs=runs, success=all(r['returncode'] == 0 for r in runs))
        else:
            image = ROOT/'original/Arcane_Nocd/Chaos.exe'
            subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
            try:
                report.update(inspect(image.read_bytes()), original_sha256=sha(image), success=True)
            finally:
                subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
                report['original_verified_before_after'] = True
    except Exception as error:
        report['error'] = str(error)
    finally:
        report['sources_stable'] = all(sha(ROOT/p) == h for p, h in declaration['sources'].items())
        report['success'] &= report['sources_stable']
        args.report.parent.mkdir(parents=True, exist_ok=True)
        with args.report.open('x') as stream:
            json.dump(report, stream, indent=2)
            stream.write('\n')
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
