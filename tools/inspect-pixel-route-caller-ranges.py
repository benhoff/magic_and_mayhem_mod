#!/usr/bin/env python3
"""Search all bytes of five unresolved original windows, retaining unknown flows."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
# Scoped code windows from the original-reference fixtures; not newly asserted
# function boundaries or exceptions to the binary inventory.
WINDOWS = {
    'pixel/pcx_words': (0x54b5f0, 0x54b7fb),
    'pixel/brighten': (0x58d770, 0x58d8b6),
    'pixel/row_copy': (0x58d3b0, 0x58d4a7),
    'raster/splice15': (0x57e1b0, 0x57e540),
    # Existing minimap-outline-view3-5532f0 recovered range, through RET.
    # 553640 was inside MOV and omitted the final 124 bytes of this body.
    'minimap/camera3': (0x5532f0, 0x5536bc),
}


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def inspect(data, inventory):
    if hashlib.sha256(data).hexdigest() != BUILD:
        raise ValueError('Unsupported original executable')
    pe = struct.unpack_from('<I', data, 60)[0]
    optional = pe + 24
    if struct.unpack_from('<H', data, optional)[0] != 0x10b:
        raise ValueError('Expected PE32')
    base = struct.unpack_from('<I', data, optional + 28)[0]
    section_table = optional + struct.unpack_from('<H', data, pe + 20)[0]
    sections = []
    for i in range(struct.unpack_from('<H', data, pe + 6)[0]):
        at = section_table + i * 40
        virtual, rva, length, raw = struct.unpack_from('<4I', data, at + 8)
        if raw + length > len(data):
            raise ValueError('Truncated PE section')
        sections.append(dict(name=data[at:at+8].rstrip(b'\0').decode('ascii'),
                             start=base+rva, end=base+rva+length, raw=raw,
                             executable=bool(struct.unpack_from('<I', data, at+36)[0] & 0x20000000)))
    def read(at, length):
        for section in sections:
            if section['start'] <= at and at + length <= section['end']:
                offset = section['raw'] + at - section['start']
                return data[offset:offset+length]
        raise ValueError('Unbacked selected window')
    rows = {route: dict(route=route, start=start, end=end,
                       window_sha256=hashlib.sha256(read(start, end-start)).hexdigest(),
                       external_rel32_byte_candidates=[], external_address_byte_candidates=[],
                       internal_relative_candidates=0, census_direct_external_flows=[])
            for route, (start, end) in WINDOWS.items()}
    for section in sections:
        code = data[section['raw']:section['raw']+section['end']-section['start']]
        for i in range(len(code)-3):
            site = section['start'] + i
            absolute = struct.unpack_from('<I', code, i)[0]
            for row in rows.values():
                if row['start'] <= absolute < row['end'] and not row['start'] <= site < row['end']:
                    row['external_address_byte_candidates'].append(dict(site=site, target=absolute, section=section['name']))
            if section['executable'] and i+5 <= len(code) and code[i] in (0xe8, 0xe9):
                target = (site+5+struct.unpack_from('<i', code, i+1)[0]) & 0xffffffff
                for row in rows.values():
                    if row['start'] <= target < row['end']:
                        if row['start'] <= site < row['end']:
                            row['internal_relative_candidates'] += 1
                        else:
                            row['external_rel32_byte_candidates'].append(dict(site=site, target=target, opcode=hex(code[i])))
    for flow in inventory['flows']:
        if flow['computed']:
            continue
        for row in rows.values():
            for target in flow['targets']:
                if row['start'] <= target < row['end'] and not row['start'] <= flow['site'] < row['end']:
                    row['census_direct_external_flows'].append(dict(site=flow['site'], owner=flow['owner'], kind=flow['kind'], target=target))
    for row in rows.values():
        row['discovered_entry'] = any(f['entry'] == row['start'] for f in inventory['functions'])
        row['classification'] = 'unresolved_indirect_or_external_callers'
    table = struct.unpack('<4I', read(0x555e08, 16))
    if table != (0x555c4e, 0x555c57, 0x555c4e, 0x555c60):
        raise ValueError('Original camera table changed')
    cameras = []
    for value, branch in enumerate(table):
        code = read(branch, 7)
        if code[:3] != bytes.fromhex('8bcbe8'):
            raise ValueError('Original camera branch changed')
        cameras.append(dict(case=value, branch=branch, target=branch+7+struct.unpack_from('<i', code, 3)[0]))
    rows['minimap/camera3']['camera_dispatch'] = cameras
    rows['minimap/camera3']['classification'] = 'unselected_by_reviewed_camera_dispatch_other_callers_unresolved'
    return dict(routes=list(rows.values()), camera_dispatch=cameras,
                executable_sections=[s for s in sections if s['executable']],
                unresolved_computed_flows=sum(f['computed'] for f in inventory['flows']),
                global_unreachable_proved=False,
                scope='Whole pinned image byte candidates targeting any offset of five scoped original windows; review hits as instructions separately. Zero hits cannot exclude computed addresses, fallthrough, external modules or other builds. Camera0/2/0/1 dispatch is exact, not a global camera3 dead-code proof.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    declaration = json.loads(args.claims.read_text())
    if any(sha(ROOT/p) != h for p, h in declaration['sources'].items()):
        parser.error('Prospective source changed')
    image = ROOT/'original/Arcane_Nocd/Chaos.exe'
    inventory = ROOT/'research/runtime/coverage/binary-nocd.json'
    report = dict(success=False, kind='static', sources=declaration['sources'], claims=declaration['claims'])
    subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
    try:
        report.update(inspect(image.read_bytes(), json.loads(inventory.read_text())))
        report.update(original_sha256=sha(image), inventory_sha256=sha(inventory), original_game_executed=False)
        report['success'] = True
    finally:
        subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
        report['sources_stable'] = all(sha(ROOT/p) == h for p, h in declaration['sources'].items())
        report['success'] &= report['sources_stable']
        args.report.parent.mkdir(parents=True, exist_ok=True)
        with args.report.open('x') as stream:
            json.dump(report, stream, indent=2); stream.write('\n')
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
