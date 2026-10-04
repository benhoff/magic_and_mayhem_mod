#!/usr/bin/env python3
"""Export pinned DirectSound creation/upload wrappers and their direct callers."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile

REPO = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RANGES = {
    'manager_initialize': (0x56df60, 0x56e5cc),
    'manager_shutdown': (0x56e640, 0x56e7f1),
    'primary_setup': (0x56fe80, 0x56ffa7),
    'static_wav_upload': (0x570140, 0x570365),
    'voice_duplicate': (0x572180, 0x5722a7),
    'wav_format': (0x58ff10, 0x58ffa5),
    'wav_data_size': (0x5900d0, 0x59013a),
    'wav_data_read': (0x590140, 0x5901ef),
}
SITES = {
    0x56e496: 'DirectSoundCreate: default GUID, no aggregation, manager +8 output',
    0x56e4d1: 'Device.SetCooperativeLevel +0x18: manager HWND, level 2',
    0x56fecf: 'Device.CreateSoundBuffer +0x0c: primary descriptor size 20 flags 0x81',
    0x56fefb: 'Device.GetCaps +0x10: size 96',
    0x56ff71: 'Primary.SetFormat +0x38',
    0x56ff8a: 'Primary.GetCaps +0x0c: size 20, HRESULT not checked',
    0x56ff9a: 'Device.Compact +0x1c',
    0x570289: 'Device.CreateSoundBuffer +0x0c: secondary descriptor size 20 flags 0xea',
    0x5702af: 'Secondary.Lock +0x2c: offset 0, full data bytes, flags 0, null second outputs',
    0x5702ca: 'WAV data read into first region',
    0x5702e1: 'Secondary.Unlock +0x4c: first pointer/requested bytes, null second region',
    0x572194: 'Device.DuplicateSoundBuffer +0x14',
}


def export(executable):
    if hashlib.sha256(executable.read_bytes()).hexdigest() != HASH:
        raise ValueError('Unsupported executable hash')
    disassembly = subprocess.run(['objdump', '-d', '-Mintel', str(executable)], check=True, text=True, capture_output=True).stdout
    instructions = []
    for line in disassembly.splitlines():
        match = re.match(r'\s*([0-9a-f]+):\s', line)
        if match:
            instructions.append((int(match[1], 16), line))
    parent = REPO / 'working/decompiled'
    parent.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix='audio-support-', dir=parent))
    artifacts, callers = {}, {}
    for name, (start, end) in RANGES.items():
        path = root / (name+'.asm')
        path.write_text('\n'.join(line for address, line in instructions if start<=address<end)+'\n')
        artifacts[path.name] = hashlib.sha256(path.read_bytes()).hexdigest()
        callers[name] = []
        for index, (address, line) in enumerate(instructions):
            match = re.search(r'\bcall\s+0x([0-9a-f]+)\b', line)
            if match and int(match[1], 16) == start:
                callers[name].append({'address': f'0x{address:08x}', 'context': [s for _, s in instructions[max(0, index-6):index+2]]})
    sites = {f'0x{address:08x}': {'operation': name, 'context': []} for address, name in SITES.items()}
    for index, (address, line) in enumerate(instructions):
        if address in SITES:
            sites[f'0x{address:08x}']['context'] = [s for _, s in instructions[max(0, index-8):index+3]]
    if any(not site['context'] for site in sites.values()):
        raise ValueError('Missing pinned instruction')
    (root / 'direct-callers.json').write_text(json.dumps(callers, indent=2)+'\n')
    artifacts['direct-callers.json'] = hashlib.sha256((root / 'direct-callers.json').read_bytes()).hexdigest()
    if hashlib.sha256(executable.read_bytes()).hexdigest() != HASH:
        raise ValueError('Input changed during export')
    (root / 'manifest.json').write_text(json.dumps({'source': str(executable.resolve()), 'source_sha256': HASH,
        'architecture': 'PE32 i386', 'image_base': '0x00400000', 'ranges': RANGES, 'sites': sites, 'artifacts': artifacts,
        'input_unchanged': True, 'scope': 'Selected static audio paths; runtime frequency/coverage unproven'}, indent=2)+'\n')
    return root


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('executable', nargs='?', type=Path, default=REPO / 'working/game-nocd/Chaos.exe')
    print(f'Audio evidence: {export(parser.parse_args().executable)}')
