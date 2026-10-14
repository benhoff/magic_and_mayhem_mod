#!/usr/bin/env python3
"""Compare bounded recovered dynamic descriptors with the pinned original parser."""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile
from pathlib import Path
from coverage_claims import behavior_contract,required_sources
ROOT=Path(__file__).resolve().parents[1]
BIDS=['CP.grimoire-dynamic-fields','NP.grimoire-dynamic-guards']
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True);args=parser.parse_args()
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());behaviors=[next(b for b in r['behaviors'] if b['id']==bid) for bid in BIDS];b=behaviors[0]
    claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{x['id']:x for x in r['builds']}),scenarios={}) for b in behaviors]
    paths=set().union(*(required_sources(b) for b in behaviors))
    def fingerprints():return {p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(paths)}
    sources=fingerprints();decl=json.loads(args.claims.read_text())
    if decl['claims']!=claims or decl['sources']!=sources:raise ValueError('Prospective dynamic descriptor declaration mismatch')
    parent=ROOT/'working/tests/grimoire-dynamic';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    (out/'.retain').write_text('Private original dynamic parser/native model comparison; preserve raw evidence,not live gameplay.\n');(out/'claims.json').write_bytes(args.claims.read_bytes())
    def run(command,name,timeout=120):
        p=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=timeout);(out/(name+'.log')).write_text(p.stdout+p.stderr);p.check_returncode();return p.stdout
    run([ROOT/'tools/original-manifest.sh','verify'],'original-before')
    try:
        e=module('dynamic_image','tools/export-menu-support.py');exe=ROOT/'original/Arcane_Nocd/Chaos.exe';data=e.pinned_image(exe)
        def prefix(va):return data[e.image_offset(data,va,8):e.image_offset(data,va,8)+8]
        def array(raw):return ','.join(hex(x) for x in raw)
        header='struct Dependency {unsigned address;unsigned char bytes[8];};\nstatic const Dependency dependencies[]={\n'
        for va in [0x597850,0x597870,0x59c73c,0x59c6e7,0x59c5a4,0x505bf0,0x505c50]:header+='    {0x%x,{%s}},\n'%(va,array(prefix(va)))
        header+='};\nstatic const unsigned char parser_bytes[]={'+array(prefix(0x4c8950))+'};\nstatic const unsigned char constructor_bytes[]={'+array(prefix(0x4dd970))+'};\n'
        (out/'grimoire_dynamic_bytes.h').write_text(header)
        for va,text in [(0x5e2070,b'HTH\0'),(0x5e206c,b'LRC\0')]:
            at=e.image_offset(data,va,len(text))
            if data[at:at+len(text)]!=text:raise ValueError('Original combat property identity changed')
        run(['g++','-m32','-fno-pie','-no-pie','-fno-stack-protector','-std=c++17','-O2','-Wall','-Wextra','-Werror','-I'+str(out),ROOT/'tests/grimoire-dynamic-reference.cpp',ROOT/'tests/grimoire-dynamic-abi.S','-o',out/'reference'],'compile')
        result=json.loads(run([out/'reference',exe],'comparison',30))
        if not result['success'] or (result['cases'],result['omitted'],result['native_guards'],result['constructor_cases'],result['legacy_unknown_cases'])!=(468,18,10,1,1):raise ValueError('Incomplete original dynamic descriptor comparison')
        if fingerprints()!=sources or hashlib.sha256(exe.read_bytes()).hexdigest()!=e.HASH:raise ValueError('Execution inputs changed')
        result.update(source_stable=True,source_sha256=e.HASH,sources=sources,execution_claims=claims,scope=b['scope'],original_parser_constructor_unmodified=True,original_work_bypassed=False,raw_directory=str(out),artifacts={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in out.iterdir() if p.is_file()})
        (out/'report.json').write_text(json.dumps(result,indent=2)+'\n');print('468 original dynamic descriptor comparisons passed',flush=True)
    finally:run([ROOT/'tools/original-manifest.sh','verify'],'original-after')
if __name__=='__main__':main()
