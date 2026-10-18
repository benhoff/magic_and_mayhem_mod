#!/usr/bin/env python3
"""Read-only discovery of the pinned regional danger producers."""
from importlib.machinery import SourceFileLoader
from pathlib import Path
import json
import sys
import struct
import subprocess

module = SourceFileLoader('regional_export', str(Path(__file__).with_name('export-regional-navigation.py'))).load_module()
module.RANGES = {
    'danger-consumer': (0x523070, 0x5231df),
    'owner-driver': (0x415c40, 0x415d00),
    'task-interpreter-context': (0x464ec0, 0x465100),
    'hostile-producer': (0x41a060, 0x41af10),
    'deduction-producer': (0x41d270, 0x41dc20),
    'danger-filter': (0x41f6a0, 0x4202e0),
    'deduction-filter': (0x4202e0, 0x420e20),
    'base-filter': (0x420e20, 0x421110),
    'region-support': (0x529e40, 0x52a100),
    'actor-power': (0x51e500, 0x51e600),
    'memory-aging': (0x520620, 0x520720),
    'actor-classifier': (0x411e50, 0x411f10),
    'movement-classifier': (0x51fef0, 0x51ff70),
    'center-region': (0x542810, 0x542b00),
    'memory-value': (0x4297f0, 0x429850),
    'memory-find': (0x42ab50, 0x42aba0),
    'memory-range': (0x42e3e0, 0x42e440),
    'effect-range': (0x419920, 0x419ac0),
    'region-set-insert': (0x42abb0, 0x42ac30),
    'region-set-erase': (0x42fde0, 0x42fe80),
    'region-support-full': (0x529e40, 0x52a0e0),
    'clear-danger': (0x542b00, 0x542b31),
    'publish-danger': (0x542b40, 0x542b99),
    'clear-deduction': (0x542ba0, 0x542bd1),
    'publish-deduction': (0x542be0, 0x542c39),
    'clear-base': (0x542c40, 0x542c65),
    'publish-base': (0x542c70, 0x542c99),
    'world-frame': (0x46afc0, 0x46b900),
}
if __name__ == '__main__':
    at = sys.argv.index('--output') + 1
    sys.argv[at] = str(Path(sys.argv[at]).resolve())
    module.main()
    output = Path(sys.argv[at]) / 'report.json'
    report = json.loads(output.read_text())
    exe = module.ROOT / 'original/Arcane_Nocd/Chaos.exe'
    data = exe.read_bytes()
    if module.sha(exe) != module.HASH:
        raise ValueError('Unsupported executable')
    header = struct.unpack_from('<I', data, 0x3c)[0]
    count = struct.unpack_from('<H', data, header + 6)[0]
    sections = header + 24 + struct.unpack_from('<H', data, header + 20)[0]
    def read_at(va, fmt):
        for i in range(count):
            size, rva, length, offset = struct.unpack_from('<IIII', data, sections + 40*i + 8)
            if rva <= va-0x400000 < rva+length:
                return struct.unpack_from(fmt, data, offset+va-0x400000-rva)[0]
        raise ValueError('Unbacked original address')
    report['support_constants'] = {hex(va): read_at(va, '<d') for va in [0x5c72a0, 0x5c72a8]}
    subprocess.run([str(module.ROOT / 'tools/original-manifest.sh'), 'verify'], check=True)
    report['sources']['tools/export-regional-danger.py'] = module.sha(Path(__file__))
    output.write_text(json.dumps(report, indent=2) + '\n')
