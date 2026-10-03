#!/usr/bin/env python3
"""Export a hash-checked DirectDraw drawing inventory and direct call sites."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile

REPO = Path(__file__).resolve().parent.parent
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
# End addresses are exclusive. These are selected windows, not a complete renderer.
RANGES = {
    'fullscreen_setup': (0x58a9b0, 0x58aa90),
    'windowed_setup': (0x58aa90, 0x58acf0),
    'surface_creation': (0x58ad90, 0x58b270),
    'monitor_api_loading': (0x58b270, 0x58b370),
    'surface_lock': (0x58b660, 0x58b816),
    'opaque_surface_copy': (0x58bf40, 0x58c13e),
    'source_key_surface_copy': (0x58c140, 0x58c35e),
}
SITES = {
    0x58b69d: 'Lock +0x64', 0x58b735: 'Lock +0x64 retry',
    0x58b806: 'Unlock +0x80 conditional before returning pixels',
    0x58c03a: 'Blt +0x14, WAIT', 0x58c05a: 'BltFast +0x1c, WAIT',
    0x58c239: 'Blt +0x14, KEYSRC', 0x58c252: 'Blt +0x14, KEYSRC | WAIT',
    0x58c276: 'BltFast +0x1c, SRCCOLORKEY, optional WAIT',
}


def export(executable):
    if hashlib.sha256(executable.read_bytes()).hexdigest() != HASH:
        raise ValueError('Unsupported executable hash; refusing build-specific export')
    result = subprocess.run(['objdump', '-d', '-Mintel', str(executable)],
                            check=True, capture_output=True, text=True)
    lines = result.stdout.splitlines()
    instructions = []
    for line in lines:
        match = re.match(r'\s*([0-9a-f]+):\s', line)
        if match:
            instructions.append((int(match[1], 16), line))
    parent = REPO / 'working/decompiled'
    parent.mkdir(parents=True, exist_ok=True)
    output = Path(tempfile.mkdtemp(prefix='render-support-', dir=parent))
    artifacts, callers = {}, {}
    for name, (start, end) in RANGES.items():
        path = output / (name + '.asm')
        path.write_text('\n'.join(line for va, line in instructions if start <= va < end) + '\n')
        artifacts[path.name] = hashlib.sha256(path.read_bytes()).hexdigest()
        callers[name] = []
        for index, (va, line) in enumerate(instructions):
            call = re.search(r'\bcall\s+0x([0-9a-f]+)\b', line)
            if call and int(call[1], 16) == start:
                callers[name].append({'call_va': f'0x{va:08x}',
                                      'context': [s for _, s in instructions[max(0, index-6):index+2]]})
    path = output / 'direct-callers.json'
    path.write_text(json.dumps(callers, indent=2) + '\n')
    artifacts[path.name] = hashlib.sha256(path.read_bytes()).hexdigest()
    result = subprocess.run(['objdump', '-s', '--start-address=0x5f19d8', '--stop-address=0x5f1a64', str(executable)],
                            check=True, capture_output=True, text=True)
    path = output/'monitor-api-strings.txt'; path.write_text(result.stdout)
    artifacts[path.name] = hashlib.sha256(path.read_bytes()).hexdigest()
    site_lines = {f'0x{va:08x}': {'operation': name, 'instruction': next(s for at, s in instructions if at == va)}
                  for va, name in SITES.items()}
    if hashlib.sha256(executable.read_bytes()).hexdigest() != HASH:
        raise ValueError('Input changed during export')
    (output / 'manifest.json').write_text(json.dumps({
        'source': str(executable.resolve()), 'source_sha256': HASH, 'image_base': '0x00400000',
        'ranges': RANGES, 'sites': site_lines, 'artifacts': artifacts,
        'direct_caller_counts': {k: len(v) for k, v in callers.items()},
        'scope': 'Selected static wrappers; direct calls only, no inferred function boundaries or runtime frequencies',
        'input_unchanged': True,
    }, indent=2) + '\n')
    return output


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('executable', nargs='?', type=Path, default=REPO/'working/game-nocd/Chaos.exe')
    print(f'Evidence directory: {export(parser.parse_args().executable)}')
