#!/usr/bin/env python3
"""Hash-pinned isolated i386 motion action and route-consumption comparison."""
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
# Every private-memory redirection requires the whole-image hash and entry bytes.
ANCHORS = {0x5104b0: '83ec38', 0x512460: '83ec40', 0x5070e0: '8b542404',
           0x464ec0: '515355568b', 0x464d70: '53568bf133', 0x51a8f0: '568bf1578b', 0x50f850: '83', 0x51aa30: '83'}

def compare():
    source = ROOT / 'working/game-nocd/Chaos.exe'
    data = source.read_bytes()
    if hashlib.sha256(data).hexdigest() != HASH:
        raise ValueError('Unsupported original image')
    pe, = struct.unpack_from('<I', data, 0x3c)
    count, = struct.unpack_from('<H', data, pe+6)
    opt_size, = struct.unpack_from('<H', data, pe+20)
    def at(va, n):
        for i in range(count):
            rva, size, raw = struct.unpack_from('<III', data, pe+24+opt_size+i*40+12)
            if 0x400000+rva <= va and va+n <= 0x400000+rva+size:
                return data[raw+va-0x400000-rva:raw+va-0x400000-rva+n]
        raise ValueError('Address not file backed')
    for va, anchor in ANCHORS.items():
        if at(va, len(anchor)//2).hex() != anchor:
            raise ValueError(f'Entry anchor mismatch {va:x}: {at(va, 12).hex()}')
    parent = ROOT/'working/tests'; parent.mkdir(exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='original-creature-motion-', dir=parent))
    (out/'reference.exe').write_bytes(data)
    sources = ['reconstruction/motion/creature_motion.cpp', 'tests/creature-motion-test.cpp']
    command = [os.environ.get('CXX', 'g++'), '-m32', '-std=c++17', '-Wall', '-Wextra', '-Werror',
               '-DMNM_MOTION_REFERENCE', '-I'+str(ROOT/'reconstruction/motion'),
               '-I'+str(ROOT/'reconstruction/pathfinding'), *[str(ROOT/p) for p in sources], '-o', str(out/'compare')]
    subprocess.run(command, check=True)
    result = subprocess.run([str(out/'compare'), str(out/'reference.exe')], capture_output=True, text=True, timeout=60)
    (out/'comparison.log').write_text(result.stdout+result.stderr)
    report = {'source_sha256': HASH, 'anchors': {hex(k):v for k,v in ANCHORS.items()},
              'sources': {p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sources},
              'compile': command, 'returncode': result.returncode, 'output': result.stdout+result.stderr,
              'scope': 'Synthetic object; original action with animation event and completion stubs; original consumption with environment callbacks stubbed'}
    (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(out);print(result.stdout+result.stderr, end='')
    if source.read_bytes()!=data: raise ValueError('Input changed')
    if result.returncode: raise RuntimeError('Original comparison failed')
    return out

def main():
    verify = [str(ROOT/'tools/original-manifest.sh'), 'verify']
    subprocess.run(verify, check=True)
    try:
        out = compare()
    finally:
        subprocess.run(verify, check=True)
    report_path = out/'report.json'
    report = json.loads(report_path.read_text())
    report['original_manifest_verified_before_after'] = True
    report['input_unchanged'] = True
    report['comparison_sha256'] = hashlib.sha256((out/'compare').read_bytes()).hexdigest()
    report['comparison_log_sha256'] = hashlib.sha256((out/'comparison.log').read_bytes()).hexdigest()
    report_path.write_text(json.dumps(report, indent=2)+'\n')

if __name__ == '__main__':
    main()
