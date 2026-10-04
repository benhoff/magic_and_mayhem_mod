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
    'voice_busy': (0x49c530, 0x49c5a6),
    'voice_volume': (0x4ff640, 0x4ff6ff),
    'manager_voice_paths': (0x56f000, 0x56fe76),
    'positional_voice_update': (0x571460, 0x5715a6),
    'positional_controls': (0x5715b0, 0x571920),
    'voice_scheduler': (0x571ab0, 0x571f7a),
    'voice_pan': (0x5722b0, 0x57231d),
    'manager_initialize': (0x56df60, 0x56e5cc),
    'manager_shutdown': (0x56e640, 0x56e7f1),
    'primary_setup': (0x56fe80, 0x56ffa7),
    'static_wav_upload': (0x570140, 0x570365),
    'voice_duplicate': (0x572180, 0x5722a7),
    'wav_format': (0x58ff10, 0x58ffa5),
    'wav_duration': (0x58ffb0, 0x5900c6),
    'wav_data_size': (0x5900d0, 0x59013a),
    'wav_data_read': (0x590140, 0x5901ef),
}
SITES = {
    0x49c54c: 'Secondary.GetStatus +0x24; errors count as busy',
    0x4ff679: 'Secondary.SetVolume +0x3c; caches written before call',
    0x56f291: 'Secondary.Play +0x30; reserved 0/0, loop boolean',
    0x5714d2: 'Secondary.GetStatus +0x24; failed query attempts Stop',
    0x5714e7: 'Secondary.Stop +0x48',
    0x571519: 'Secondary.SetCurrentPosition +0x34; zero after successful Stop',
    0x571557: 'Secondary.SetVolume +0x3c; positional update',
    0x571596: 'Secondary.SetPan +0x40; positional update',
    0x571cb8: 'Secondary.GetStatus +0x24; scheduler retirement',
    0x571ccd: 'Secondary.Stop +0x48; scheduler retirement',
    0x571cdf: 'Secondary.SetCurrentPosition +0x34; scheduler retirement',
    0x572237: 'Duplicate.Play +0x30; reserved 0/0, loop boolean',
    0x5722d1: 'Secondary.SetPan +0x40; duplicate propagation wrapper',
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
    # Slot offsets alone do not identify an interface. Keep every indirect call
    # in the selected audio region, including register calls, for manual review.
    audited = []
    for index, (address, line) in enumerate(instructions):
        selected = 0x56df60 <= address < 0x572320 or 0x49c530 <= address < 0x49c5a6 or 0x4ff640 <= address < 0x4ff6ff
        call = re.search(r'\bcall\s+(.+)', line)
        if selected and call and not re.match(r'0x[0-9a-f]+\b', call[1]):
            slot = re.search(r'\[[a-z]{3}\+0x([0-9a-f]+)\]', call[1])
            audited.append({'address': f'0x{address:08x}', 'instruction': line,
                'slot_offset': int(slot[1], 16) if slot else None,
                'context': [s for _, s in instructions[max(0,index-8):index+3]]})
    audit = {'ranges': [['0x0056df60','0x00572320'],['0x0049c530','0x0049c5a6'],['0x004ff640','0x004ff6ff']],
             'indirect_calls': audited, 'set_frequency_slot_candidates': [c['address'] for c in audited if c['slot_offset']==0x44],
             'get_position_slot_candidates': [c['address'] for c in audited if c['slot_offset']==0x10],
             'scope': 'Selected static region only; register targets retained, not automatically resolved; no whole-program absence proof'}
    (root / 'voice-call-audit.json').write_text(json.dumps(audit, indent=2)+'\n')
    artifacts['voice-call-audit.json'] = hashlib.sha256((root / 'voice-call-audit.json').read_bytes()).hexdigest()
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
