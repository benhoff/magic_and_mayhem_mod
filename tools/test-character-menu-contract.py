#!/usr/bin/env python3
"""Execute original Character edit/OK/Cancel with private price-refresh dependencies."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
from coverage_claims import behavior_contract

ROOT = Path(__file__).resolve().parents[1]
STUBS = {'selected':0x5908e0, 'fade':0x557510, 'refresh':0x4b5f90}
SOURCES = ['tools/test-character-menu-contract.py','tests/character-menu-contract-reference.cpp',
           'tests/sprite-binary-reference.cpp','tools/export-menu-support.py','tools/coverage_claims.py']


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path)
    args = parser.parse_args()
    parent = ROOT/'working/tests/character-menu-contract';parent.mkdir(parents=True,exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def run(command,name,timeout=180):
        value = subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=timeout)
        (out/(name+'.log')).write_text(value.stdout+value.stderr);value.check_returncode();return value
    run([ROOT/'tools/original-manifest.sh','verify'],'original-before')
    try:
        sources = {p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in SOURCES}
        claims = None
        if args.claims:
            claims = json.loads(args.claims.read_text())
            register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
            behavior = next(b for b in register['behaviors'] if b['id']=='CP.character-actions')
            builds = {b['id']:b for b in register['builds']}
            expected = [{'behavior':behavior['id'],'contract_sha256':behavior_contract(behavior,builds),'scenarios':{}}]
            if claims['claims']!=expected or claims['sources']!=sources:
                raise ValueError('Prospective Character declaration differs from current scope/sources')
            (out/'prospective-claims.json').write_bytes(args.claims.read_bytes())
        spec = importlib.util.spec_from_file_location('character_image',ROOT/'tools/export-menu-support.py')
        exporter = importlib.util.module_from_spec(spec);spec.loader.exec_module(exporter)
        exe = ROOT/'original/Arcane_Nocd/Chaos.exe';data = exporter.pinned_image(exe)
        def at(va,n):
            offset=exporter.image_offset(data,va,n);return data[offset:offset+n]
        # Complete callback return and dispatch data; the fixture maps the whole image.
        if at(0x4b596e,3)!=bytes.fromhex('c20400'):
            raise ValueError('Character callback return boundary differs')
        header = '// Whole-file hash-pinned original dependencies; private mapping only.\n'
        for name,va in STUBS.items():
            header+=f'static const unsigned {name}_address=0x{va:x};\nstatic const unsigned char {name}_bytes[]={{'+','.join(hex(b) for b in at(va,8))+'};\n'
        (out/'character_menu_fixture_bytes.h').write_text(header)
        run(['g++','-m32','-fno-pie','-no-pie','-std=c++17','-O2','-Wall','-Wextra','-Werror',
             '-I'+str(out),ROOT/'tests/character-menu-contract-reference.cpp','-o',out/'reference'],'compile')
        result=json.loads(run([out/'reference',exe],'reference',20).stdout)
        if not result.get('success') or tuple(result[k] for k in ['edit_cases','accept_cases','cancel_cases','invalid_cases'])!=(288,10,6,25):
            raise ValueError('Incomplete Character action matrix')
        if hashlib.sha256(exe.read_bytes()).hexdigest()!=exporter.HASH:
            raise ValueError('Original input changed')
        if sources!={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in SOURCES}:
            raise ValueError('Character comparison sources changed during execution')
        result.update(source_sha256=exporter.HASH,real_game_launched=False,live_validated=False,
                      scope='Original Character callback0x4b5470:288 six-row +/- action cases,10 OK cases including negative and greater-than-opening experience,6 Cancel cases,25 ignored actions. Two wizard identities,baseline/max/budget bounds,draft-only edits,selected wizard stat/experience commit or rollback. Original callback unchanged; wizard selection,fade and price/UI refresh privately stubbed. No price formula/refund schedule equivalence,live control admission,persistence on disk,full ABI or native integration.',
                      anchors={'0x4b5470':at(0x4b5470,16).hex()},sources=sources,
                      artifacts={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in out.iterdir() if p.is_file()})
        if claims:result['execution_claims']=claims['claims']
        (out/'report.json').write_text(json.dumps(result,indent=2)+'\n')
        print('329 original Character edit/OK/Cancel cases passed',flush=True)
    finally:
        run([ROOT/'tools/original-manifest.sh','verify'],'original-after')


if __name__=='__main__':
    main()
