#!/usr/bin/env python3
"""Hash-pinned isolated i386 motion segment initialization and continuity comparison."""
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
ANCHORS = {0x510e80:'83ec305355568bf1',0x516ee0:'55568bf133ed8b86',
           0x514360:'83ec385355568bd9',0x5093e0:'8b4424088b54240c',
           0x509230:'8b4424088b54240c',0x464cb0:'8b44240453568bf1',
           0x50db60:'55568bf133ed39ae',0x5070e0:'8b5424048b442408'}

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
    out = Path(tempfile.mkdtemp(prefix='original-segment-setup-', dir=parent))
    (out/'reference.exe').write_bytes(data)
    for name, start, end in [('motion_segment_setup', 0x510e80, 0x511c0c), ('motion_coordinate_snap', 0x5070e0, 0x507189)]:
        assembly = subprocess.run(['objdump', '-d', '-M', 'intel', f'--start-address={start}', f'--stop-address={end}', str(out/'reference.exe')], check=True, capture_output=True, text=True)
        (out/(name+'.asm')).write_text(assembly.stdout)
    sources = ['reconstruction/motion/segment_setup.cpp','reconstruction/pathfinding/route_scalar.cpp','reconstruction/pathfinding/route_request.cpp','reconstruction/pathfinding/route_search.cpp','tests/segment-setup-test.cpp']
    command = [os.environ.get('CXX', 'g++'), '-m32', '-std=c++17', '-Wall', '-Wextra', '-Werror',
               '-DMNM_SEGMENT_REFERENCE','-ffunction-sections','-Wl,--gc-sections', '-I'+str(ROOT/'reconstruction/motion'),
               '-I'+str(ROOT/'reconstruction/pathfinding'), *[str(ROOT/p) for p in sources], '-o', str(out/'compare')]
    subprocess.run(command, check=True)
    result = subprocess.run([str(out/'compare'), str(out/'reference.exe')], capture_output=True, text=True, timeout=60)
    (out/'comparison.log').write_text(result.stdout+result.stderr)
    report = {'source_sha256': HASH, 'anchors': {hex(k):v for k,v in ANCHORS.items()},
              'sources': {p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sources},
              'compile': command, 'returncode': result.returncode, 'output': result.stdout+result.stderr,
              'scope': 'Synthetic admitted category-zero/four planar, sloped and vertical setup plus ordinary terrain snap; original complete setup, scalar and direction/wrap helpers and coordinate snap; hazard, acceptance, occupancy, animation selection and action transition controlled; special generator class-two height helper excluded'}
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
