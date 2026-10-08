#!/usr/bin/env python3
"""Decode bounded startup diagnostics or export pinned canvas lifecycle disassembly."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
NAMES = ['create', 'release', 'lock', 'bind', 'clip', 'fill_full', 'fill_rect',
         'copy', 'copy_keyed', 'jpeg', 'sprite_dispatch', 'indexed_raster', 'first_world', 'overflow']
RANGES = {
    'surface_setup': (0x4e40d0, 0x4e4263),
    'frame_copy_window': (0x46b970, 0x46ba24),
    'loading_copy': (0x4dc5e0, 0x4dc6b4),
    'loading_entry': (0x539b80, 0x539d82),
    'world_bind_draw': (0x4fd42e, 0x4fd5af),
    'battle_clear': (0x46ae39, 0x46ae70),
    'bind_clip': (0x57dc20, 0x57dcf0),
    'create': (0x58ad90, 0x58b263),
    'release': (0x58b3e0, 0x58b4b8),
    'lock': (0x58b660, 0x58b815),
    'fill_full': (0x58bc10, 0x58bd08),
    'fill_rect': (0x58bac0, 0x58bb87),
    'jpeg': (0x58d320, 0x58d36d),
    'queue': (0x5002a0, 0x5002c0),
}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def decode(raw):
    if len(raw) < 320 or raw[:8] != b'MNMCST01' or (len(raw)-160) % 160:
        raise ValueError('Invalid/truncated canvas startup envelope')
    header = struct.unpack_from('<40I', raw)
    if header[2:7] != (1, 160, 160, 16384, 12) or any(header[7:]):
        raise ValueError('Unsupported canvas startup version/bounds')
    rows = list(struct.iter_unpack('<40I', raw[160:]))
    if len(rows) > 16384 or rows[-1][1:3] != (12, 0):
        raise ValueError('Trace capacity exhausted or first World missing')
    stack = []
    for i, r in enumerate(rows):
        if r[0] != i+1 or r[1] > 12 or r[2] > 1 or any(r[33:]):
            raise ValueError('Invalid event order/tag/phase/reserved words')
        if r[1] == 12:
            if i != len(rows)-1 or stack:
                raise ValueError('World boundary inside unfinished wrapper trace')
        elif r[2] == 0:
            stack.append((r[1], r[3], r[5], r[6], r[7:13]))
        elif not stack or stack.pop() != (r[1], r[3], r[5], r[6], r[7:13]):
            raise ValueError('Unpaired or interleaved wrapper calls')
    return rows


def analyze(report_path):
    capture = json.loads(report_path.read_text())
    if not capture['success'] or not capture['sources_stable'] or capture['source_executable_sha256'] != HASH:
        raise ValueError('Capture is unsuccessful or from another build')
    trace = Path(capture['experiment']) / capture['canvas_startup']['path']
    if sha(trace) != capture['canvas_startup']['sha256']:
        raise ValueError('Startup trace hash changed')
    rows = decode(trace.read_bytes())
    first = rows[-1]
    locks = [r for r in rows if r[1:3] == (2, 1) and r[4] == first[13]]
    objects = {r[5] for r in locks}
    tokens = {}
    def token(p):
        if not p:
            return 0
        if p not in tokens:
            tokens[p] = len(tokens)+1
        return tokens[p]
    events = []
    for r in rows:
        events.append({'sequence': r[0], 'operation': NAMES[r[1]], 'phase': ['entry','return'][r[2]],
            'caller_return_va': hex(r[3]), 'result_eax': hex(r[4]), 'ecx': hex(r[5]), 'edx': hex(r[6]),
            'args': [hex(v) for v in r[7:13]], 'bound_pointer_token': token(r[13]),
            'bound_stride_words': r[14], 'bound_height': r[15], 'clip': list(r[16:20]),
            'alternate_canvas': hex(r[20]), 'surface_fields_04_through_18': [hex(v) for v in r[21:27]],
            'sample': {'pointer_token': token(r[27]), 'width': r[28], 'height': r[29],
                       'stride_words': r[30], 'nonzero_pixels': None if r[31] == 0xffffffff else r[31],
                       'fnv1a': hex(r[32]) if r[31] != 0xffffffff else None}})
    related = [e for r,e in zip(rows,events) if
        r[5] in objects and r[1] in (0,1,2,5,6,7,8,9) or
        r[1] == 3 and r[5] == first[13] or r[1] == 12]
    return {'success': True, 'scope': 'Selected wrappers through first World queue; pointer tokens are process-local, no exhaustive writer census or native input/replacement',
        'sources': {str(Path(__file__).relative_to(ROOT)): sha(Path(__file__))},
        'source_executable_sha256': HASH, 'capture_report': str(report_path), 'capture_report_sha256': sha(report_path),
        'trace_sha256': sha(trace), 'records': len(rows), 'calls': dict(Counter(NAMES[r[1]] for r in rows if r[2] == 0)),
        'first_world': events[-1], 'world_surface_objects': [hex(p) for p in sorted(objects)],
        'same_pointer_lock_returns_before_world': len(locks), 'world_surface_timeline': related,
        'events': events, 'original_pixels_used_as_native_inputs': False, 'live_replacement': False}


def export(executable, output):
    subprocess.run([str(ROOT/'tools/original-manifest.sh'), 'verify'], check=True)
    try:
        if sha(executable) != HASH:
            raise ValueError('Unsupported executable hash')
        asm = subprocess.run(['objdump','-d','-Mintel',str(executable)], check=True, capture_output=True, text=True).stdout
        ins = [(int(m[1],16),line) for line in asm.splitlines() if (m := re.match(r'\s*([0-9a-f]+):\s',line))]
        selected = {name: [line for va,line in ins if a <= va < b] for name,(a,b) in RANGES.items()}
        callers = {name: [{'site': hex(va), 'instruction': line} for va,line in ins if
                   re.search(r'\bcall\s+0x'+format(a,'x')+r'\b',line)] for name,(a,_) in RANGES.items()}
        result = {'success': True, 'scope': 'Selected static windows/direct callers only; not exhaustive ownership or indirect writer recovery',
            'source_executable_sha256': HASH, 'sources': {str(Path(__file__).relative_to(ROOT)): sha(Path(__file__))},
            'ranges': RANGES, 'disassembly': selected, 'direct_callers': callers,
            'canvas_pointer_mentions': [line for _,line in ins if '0x658174' in line]}
        if sha(executable) != HASH:
            raise ValueError('Executable changed during export')
        with output.open('x') as f:
            json.dump(result,f,indent=2);f.write('\n')
    finally:
        subprocess.run([str(ROOT/'tools/original-manifest.sh'), 'verify'], check=True)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    g = p.add_mutually_exclusive_group(required=True)
    g.add_argument('--capture-report',type=Path)
    g.add_argument('--executable',type=Path)
    p.add_argument('--output',type=Path,required=True)
    args = p.parse_args()
    if args.executable:
        export(args.executable,args.output)
    else:
        with args.output.open('x') as f:
            json.dump(analyze(args.capture_report),f,indent=2);f.write('\n')
    print(args.output)


if __name__ == '__main__':
    main()
