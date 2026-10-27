#!/usr/bin/env python3
"""Pin selected original colour-mode stores; static review, no live reachability proof."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
from coverage_evidence import write_report

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT/path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    args.report = args.report.absolute()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    declaration = load('colour_mode_claims', 'tools/draft-coverage-claims.py').declaration(
        ROOT, register, ['RS.colour-mode-selection'], [])
    for name in ('tools/export-menu-support.py', 'tools/original-manifest.sh'):
        declaration['sources'][name] = hashlib.sha256((ROOT/name).read_bytes()).hexdigest()
    report = dict(schema=1, success=False, static_only=True, sources=declaration['sources'],
                  claims=declaration['claims'], original_sha256=BUILD, anchors=[], disassembly=[],
                  live_reachability_proved=False, original_native_comparison=False)
    before = False
    try:
        subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], cwd=ROOT, check=True)
        before = True
        original = ROOT/'original/Arcane_Nocd/Chaos.exe'
        data = original.read_bytes()
        if hashlib.sha256(data).hexdigest() != BUILD:
            raise ValueError('Unsupported original build')
        pe = load('colour_mode_pe', 'tools/export-menu-support.py')
        for address, hex_bytes, meaning in (
                (0x547972, 'c7 06 02 00 00 00', 'Shape initializer stores mode2'),
                (0x5485e1, 'c7 07 02 00 00 00', 'Selected first constructor stores mode2'),
                (0x5486b2, '89 1f 89 5f 04 89 5f 08', 'Selected second constructor stores EBX=2 into mode/width/height'),
                (0x5489f0, 'c1 e0 02 99 f7 3d 28 14 5e 00', 'Selected update shifts remaining and performs signed division by global5e1428'),
                (0x548a0d, '89 4e 1c', 'Selected update writes chosen mode to object+1c'),
                (0x54ab6a, '83 c1 1c e8 0e d2 ff ff', 'Draw wrapper passes shape+1c to547d80')):
            expected = bytes.fromhex(hex_bytes)
            at = pe.image_offset(data, address, len(expected))
            if data[at:at+len(expected)] != expected:
                raise ValueError('Unexpected original bytes at '+hex(address))
            report['anchors'].append(dict(address=address, expected=expected.hex(), meaning=meaning))
        for start, end in ((0x547950, 0x547990), (0x548560, 0x548600),
                           (0x548650, 0x5486e7), (0x548860, 0x548a18), (0x54ab60, 0x54ab75)):
            command = ['objdump', '-d', '-Mintel', '--start-address='+hex(start), '--stop-address='+hex(end), str(original)]
            text = subprocess.check_output(command, cwd=ROOT)
            path = args.report.with_name(args.report.stem+'-'+hex(start)[2:]+'.log')
            write_report(ROOT, path, text)
            report['disassembly'].append(dict(start=start, end=end, command=command,
                record=str(path.relative_to(ROOT)), sha256=hashlib.sha256(text).hexdigest()))
        report.update(success=True,
            findings=['Selected shape constructors and initializer start at mode2.',
                      'A gated update writes min(3,4-IDIV((remaining<<2),global5e1428)) to object+1c.',
                      'The selected draw wrapper forwards shape+1c unchanged to original547d80.'],
            boundaries=['Static branches do not prove natural modes2/3 reach drawing.',
                        'Whole update, external providers, overflow/division faults and object lifetimes remain unrecovered.'])
    except Exception as error:
        report['error'] = repr(error)
    finally:
        try:
            subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], cwd=ROOT, check=True)
            report['original_manifest_verified_before_after'] = before
        except Exception as error:
            report.update(success=False, manifest_error=repr(error))
        report['sources_stable'] = all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in declaration['sources'].items())
        report['success'] = report['success'] and report['sources_stable']
        write_report(ROOT, args.report, (json.dumps(report, indent=2)+'\n').encode())
        print(args.report)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
