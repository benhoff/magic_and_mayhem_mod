#!/usr/bin/env python3
"""Read-only export of pinned Cure delivery, resource and cleanup contracts."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
ENTRIES = [0x411e50, 0x493f20, 0x4943f0, 0x494ab0, 0x4970e0,
           0x507f00, 0x516150, 0x522720]


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    report = dict(success=False, scope='Static pinned instruction/dispatch evidence only; no execution or lifecycle equivalence.',
                  source_executable_sha256=HASH, sources={
                      'tools/export-cure-lifecycle.py': sha(Path(__file__).read_bytes()),
                      'research/runtime/coverage/binary-nocd.json': sha((ROOT/'research/runtime/coverage/binary-nocd.json').read_bytes())})
    def manifest(name):
        result = subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], capture_output=True, text=True)
        (args.output/name).write_text(result.stdout + result.stderr)
        result.check_returncode()
    manifest('original-before.log')
    try:
        executable = ROOT/'original/Arcane_Nocd/Chaos.exe'
        raw = executable.read_bytes()
        if sha(raw) != HASH:
            raise ValueError('Unsupported original build')
        pe = struct.unpack_from('<I', raw, 60)[0]
        sections = pe + 24 + struct.unpack_from('<H', raw, pe+20)[0]
        def read(address, size):
            for i in range(struct.unpack_from('<H', raw, pe+6)[0]):
                rva, length, offset = struct.unpack_from('<3I', raw, sections+i*40+12)
                if 0x400000+rva <= address and address+size <= 0x400000+rva+length:
                    return raw[offset+address-0x400000-rva:offset+address-0x400000-rva+size]
            raise ValueError('Unbacked address')
        inventory = json.loads((ROOT/'research/runtime/coverage/binary-nocd.json').read_text())
        functions = {f['entry']: f for f in inventory['functions']}
        output = subprocess.check_output(['objdump', '-d', '-Mintel', str(executable)], text=True)
        lines = []
        for line in output.splitlines():
            match = re.match(r'\s*([0-9a-f]+):\s', line)
            if match:
                lines.append((int(match[1], 16), line))
        report['functions'] = {}
        for entry in ENTRIES:
            function = functions[entry]
            assert len(function['ranges']) == 1
            start, end = function['ranges'][0]
            body = read(start, end-start)
            assert sha(body) == function['sha256']
            text = '\n'.join(line for address, line in lines if start <= address < end) + '\n'
            path = args.output/f'{entry:08x}.asm'
            path.write_text(text)
            report['functions'][hex(entry)] = dict(start=start, end=end, sha256=sha(body), entry_bytes=body[:16].hex(), assembly_sha256=sha(text.encode()))
        report['update_dispatch'] = list(struct.unpack('<89I', read(0x499e24, 89*4)))
        report['setup_dispatch'] = list(struct.unpack('<89I', read(0x496f04, 89*4)))
        report['input_unchanged'] = executable.read_bytes() == raw
        report['success'] = report['input_unchanged']
    finally:
        manifest('original-after.log')
        report['original_manifest_verified_before_after'] = True
        report['sources_unchanged'] = all(sha((ROOT/path).read_bytes()) == value for path, value in report['sources'].items())
        with (args.output/'report.json').open('x') as out:
            json.dump(report, out, indent=2)
            out.write('\n')
    if not report['success'] or not report['sources_unchanged']:
        raise ValueError('Unstable static export')
    print(args.output)


if __name__ == '__main__':
    main()
