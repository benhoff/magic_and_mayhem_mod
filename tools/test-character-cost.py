#!/usr/bin/env python3
"""Compare native Character costs with the complete original calculator/math helpers."""
import argparse
import configparser
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
from coverage_claims import behavior_contract
ROOT=Path(__file__).resolve().parents[1]
SOURCES=['tools/test-character-cost.py','tests/character-cost-reference.cpp','tests/sprite-binary-reference.cpp',
         'tools/export-menu-support.py','tools/coverage_claims.py','reconstruction/campaign/character_cost.hpp',
         'reconstruction/campaign/character_cost.cpp','reconstruction/campaign/CMakeLists.txt']

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path);args=parser.parse_args()
    parent=ROOT/'working/tests/character-cost';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def run(command,name,timeout=180):
        value=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=timeout)
        (out/(name+'.log')).write_text(value.stdout+value.stderr);value.check_returncode();return value
    run([ROOT/'tools/original-manifest.sh','verify'],'original-before')
    try:
        sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in SOURCES};claims=None
        if args.claims:
            claims=json.loads(args.claims.read_text());register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
            builds={b['id']:b for b in register['builds']}
            expected=[{'behavior':b['id'],'contract_sha256':behavior_contract(b,builds),'scenarios':{}}
                      for ident in ['CP.character-pricing','NP.character-price-admission']
                      for b in register['behaviors'] if b['id']==ident]
            if claims['claims']!=expected or claims['sources']!=sources:raise ValueError('Prospective costs declaration differs')
            (out/'claims.json').write_bytes(args.claims.read_bytes())
        spec=importlib.util.spec_from_file_location('cost_image',ROOT/'tools/export-menu-support.py')
        exporter=importlib.util.module_from_spec(spec);spec.loader.exec_module(exporter)
        exe=ROOT/'original/Arcane_Nocd/Chaos.exe';data=exporter.pinned_image(exe)
        config=ROOT/'working/game-nocd/Interface/CharacterScreen/screen (Character Screen).cfg';raw=config.read_bytes()
        cfg=configparser.ConfigParser(interpolation=None,inline_comment_prefixes=(';',));cfg.read_string(raw.decode('latin-1'))
        coefficients=[]
        for name in ['Mana','Health','Control','Talisman']:
            row=cfg['STEPS_COSTS'];coefficients.append([int(row[name+'_IncrementAmount']),*[float(row[name+'_'+key]) for key in ['InitialCost','LinearComponent','ExponentialComponent']]])
        header='static const std::array<mnm::recovered::campaign::CharacterCostCoefficients,4> installed{{'
        header+=','.join('{'+str(row[0])+','+','.join(str(v)+'f' for v in row[1:])+'}' for row in coefficients)+'}};\n'
        (out/'character_cost_fixture.h').write_text(header)
        run(['g++','-m32','-fno-pie','-no-pie','-std=c++17','-O2','-Wall','-Wextra','-Werror','-I'+str(out),
             ROOT/'tests/character-cost-reference.cpp',ROOT/'reconstruction/campaign/character_cost.cpp','-o',out/'reference'],'compile')
        result=json.loads(run([out/'reference',exe],'reference',30).stdout)
        if not result.get('success') or (result['calculator_cases'],result['price_comparisons'],result['refusals'])!=(1643,19716,12):
            raise ValueError('Incomplete cost comparison')
        if sources!={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in SOURCES}:raise ValueError('Sources changed')
        if hashlib.sha256(exe.read_bytes()).hexdigest()!=exporter.HASH or config.read_bytes()!=raw:raise ValueError('Input changed')
        def anchor(va):
            at=exporter.image_offset(data,va,16);return data[at:at+16].hex()
        result.update(source_sha256=exporter.HASH,installed_config_sha256=hashlib.sha256(raw).hexdigest(),coefficients=coefficients,
                      scope='1643 original six-row calculator executions/19716 costs using installed coefficients, positive fractional synthetic coefficients, zero/constant and power-of-two cases; 12 intentional native input/output refusals. Calculator and original x87 pow/truncation helpers unchanged; no private code stubs. Original runtime math mode configured explicitly. No live Character control admission, complete math error/overflow ABI, original input failure equivalence, save persistence or replacement.',
                      sources=sources,anchors={hex(a):anchor(a) for a in [0x4b5d10,0x59bcc0,0x59bee0]},real_game_launched=False,
                      artifacts={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in out.iterdir() if p.is_file()})
        if claims:result['execution_claims']=claims['claims']
        (out/'report.json').write_text(json.dumps(result,indent=2)+'\n');print('19716 original Character costs and 12 native refusals passed',flush=True)
    finally:run([ROOT/'tools/original-manifest.sh','verify'],'original-after')
if __name__=='__main__':main()
