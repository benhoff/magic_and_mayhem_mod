#!/usr/bin/env python3
"""Compare settled Realm selection model with unchanged hash-pinned original tick."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
from coverage_claims import behavior_contract

ROOT = Path(__file__).resolve().parents[1]
STUBS = {'input_poll':0x4a5070, 'hover_clear':0x4dc110, 'hover_text':0x550130,
         'cue':0x4a39a0, 'sample':0x56f000, 'kind':0x4bfb10, 'movement':0x54ead0,
         'draw':0x5512c0, 'draw_prepare':0x4a45b0, 'render':0x58bf40,
         'draw_finish':0x4a4380, 'push':0x557040, 'wizard':0x5908e0,
         'tutorial':0x572400, 'free_tutorial':0x597870}
SOURCES = ['tools/test-realm-selection-contract.py', 'tests/realm-selection-contract-reference.cpp',
           'tests/sprite-binary-reference.cpp', 'tools/export-menu-support.py',
           'reconstruction/campaign/realm_selection.hpp', 'reconstruction/campaign/realm_selection.cpp',
           'reconstruction/campaign/CMakeLists.txt', 'tools/coverage_claims.py']


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, help='Prospective scope/source declaration prepared before execution')
    args = parser.parse_args()
    parent = ROOT / 'working/tests/realm-selection-contract'; parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent)); print(out, flush=True)

    def run(command, name, timeout=180):
        result = subprocess.run(list(map(str, command)), capture_output=True, text=True, timeout=timeout)
        (out / (name + '.log')).write_text(result.stdout + result.stderr)
        result.check_returncode(); return result

    run([ROOT / 'tools/original-manifest.sh', 'verify'], 'original-before')
    try:
        source_hashes = {p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in SOURCES}
        claims = None
        if args.claims:
            claims = json.loads(args.claims.read_text())
            register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
            behavior = next(b for b in register['behaviors'] if b['id'] == 'CP.realm-region-selection')
            builds = {b['id']:b for b in register['builds']}
            expected = [{'behavior':behavior['id'], 'contract_sha256':behavior_contract(behavior,builds), 'scenarios':{}}]
            if claims['claims'] != expected or claims['sources'] != source_hashes:
                raise ValueError('Prospective Realm declaration does not match current scope/sources')
            (out/'prospective-claims.json').write_bytes(args.claims.read_bytes())
        spec = importlib.util.spec_from_file_location('realm_export', ROOT / 'tools/export-menu-support.py')
        exporter = importlib.util.module_from_spec(spec); spec.loader.exec_module(exporter)
        exe = ROOT / 'original/Arcane_Nocd/Chaos.exe'; data = exporter.pinned_image(exe)
        def at(va, n):
            offset = exporter.image_offset(data, va, n); return data[offset:offset+n]
        header = '// Complete executable hash checked; only private external dependencies patched.\n'
        for name, va in STUBS.items():
            header += f'static const unsigned {name}_address=0x{va:x};\n'
            header += f'static const unsigned char {name}_bytes[]={{' + ','.join(hex(b) for b in at(va,8)) + '};\n'
        (out / 'realm_selection_fixture_bytes.h').write_text(header)
        # The fixture maps the entire original image, including dispatch data and complete RETs.
        run(['g++','-m32','-fno-pie','-no-pie','-std=c++17','-O2','-Wall','-Wextra','-Werror',
             '-I'+str(out), ROOT/'tests/realm-selection-contract-reference.cpp',
             ROOT/'reconstruction/campaign/realm_selection.cpp','-o',out/'reference'], 'compile')
        result = json.loads(run([out/'reference',exe], 'reference', 30).stdout)
        if not result.get('success') or (result['cases'],result['adjacency_cases'],result['coordinate_cases']) != (15360,54,14):
            raise ValueError('Incomplete Realm selection matrix')
        if hashlib.sha256(exe.read_bytes()).hexdigest() != exporter.HASH:
            raise ValueError('Original input changed')
        if source_hashes != {p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in SOURCES}:
            raise ValueError('Comparison sources changed during execution')
        result.update(source_sha256=exporter.HASH, real_game_launched=False, live_validated=False,
                      scope='Settled Realm tick state0 hit-map region0/3 in low/high resolution, two players, ownership/owned adjacency, normal/alternate mode, occupancy0/1/3/4/5/9, press/release latch, four auxiliary requests and tutorial overlay; source regions1/4/9 against visible counts1..9, canvas edges/negative coordinates. Original tick/occupancy counter unchanged; resources, movement, drawing, audio, auxiliary push and tutorial cleanup privately stubbed. No live readiness, map initialization, tutorial generation, progression, full input/ABI or replacement.',
                      anchors={hex(va):at(va,16).hex() for va in [0x5517a0,0x54e690]},
                      sources=source_hashes,
                      artifacts={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in out.iterdir() if p.is_file()})
        if claims:
            result['execution_claims'] = claims['claims']
        (out/'report.json').write_text(json.dumps(result,indent=2)+'\n')
        print('15360 original Realm selections, 54 adjacency bounds and 14 canvas bounds passed', flush=True)
    finally:
        run([ROOT/'tools/original-manifest.sh','verify'], 'original-after')


if __name__ == '__main__':
    main()
