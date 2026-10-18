#!/usr/bin/env python3
"""Read-only pinned lower creature resource and ownership exports."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RANGES = {'resource-change': (0x507f00,0x507f9c), 'resource-set': (0x507fa0,0x508036), 'associated-actor': (0x466870,0x4668b0), 'owner-transfer': (0x51ec80,0x51eded), 'contribution-account': (0x416d80,0x416ea0)}


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        parser.error('Refusing existing evidence output')
    args.output.mkdir(parents=True)
    manifest = [str(ROOT / 'tools/original-manifest.sh'), 'verify']
    subprocess.run(manifest, cwd=ROOT, check=True, capture_output=True)
    report = {'success': False, 'source_executable_sha256': HASH,
              'source_sha256': {'tools/export-creature-resources.py': sha(Path(__file__).read_bytes())},
              'scope': 'Static disassembly only; no execution or semantic equivalence.',
              'commands': [], 'bodies': {}, 'artifacts': {}}
    try:
        executable = ROOT / 'original/Arcane_Nocd/Chaos.exe'
        raw = executable.read_bytes()
        if sha(raw) != HASH:
            raise ValueError('Unsupported original image')
        pe = struct.unpack_from('<I', raw, 60)[0]
        sections = pe + 24 + struct.unpack_from('<H', raw, pe + 20)[0]
        def image_bytes(va, size):
            for i in range(struct.unpack_from('<H', raw, pe + 6)[0]):
                rva, n, offset = struct.unpack_from('<3I', raw, sections + i * 40 + 12)
                if 0x400000 + rva <= va and va + size <= 0x400000 + rva + n:
                    return raw[offset + va - 0x400000 - rva:offset + va - 0x400000 - rva + size]
            raise ValueError('Unbacked original range')
        for name, (start, end) in RANGES.items():
            body = image_bytes(start, end - start)
            command = ['objdump', '-d', '-M', 'intel', f'--start-address={start}',
                       f'--stop-address={end}', str(executable)]
            report['commands'].append(command)
            result = subprocess.run(command, check=True, capture_output=True, timeout=30)
            artifact = args.output / (name + '.asm')
            artifact.write_bytes(result.stdout)
            report['bodies'][name] = {'start': start, 'end': end,
                                      'sha256': sha(body), 'anchor': body[:16].hex()}
            report['artifacts'][artifact.name] = sha(artifact.read_bytes())
        report['entry_anchors'] = {hex(v['start']):v['anchor'] for v in report['bodies'].values()}
        report['input_unchanged'] = executable.read_bytes() == raw
        if not report['input_unchanged']:
            raise ValueError('Original changed')
        report['success'] = True
    finally:
        subprocess.run(manifest, cwd=ROOT, check=True, capture_output=True)
        report['original_manifest_verified_before_after'] = True
        (args.output / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({'success': report['success'], 'bodies': report['bodies']}))


if __name__ == '__main__':
    main()
