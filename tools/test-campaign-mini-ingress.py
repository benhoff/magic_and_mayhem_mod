#!/usr/bin/env python3
"""Privately execute the unchanged original Escape/tutorial Mini branch."""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]

def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path)
    result=importlib.util.module_from_spec(spec);spec.loader.exec_module(result);return result

def main():
    parent=ROOT/'working/tests/campaign-mini-ingress';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior=next(b for b in register['behaviors'] if b['id']=='CP.gameplay-mini-tutorial-gate')
    claims=module('ingress_claims','tools/coverage_claims.py')
    declaration=dict(behavior=behavior['id'],contract_sha256=claims.behavior_contract(behavior,{b['id']:b for b in register['builds']}))
    paths=set(behavior['tests'])|set(behavior.get('validation_dependencies',[]))
    sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(paths)}
    report=dict(success=False,sources=sources,claims=[declaration],scope=behavior['scope'])
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        pe=module('ingress_pe','tools/export-menu-support.py')
        executable=ROOT/'original/Arcane_Nocd/Chaos.exe';data=pe.pinned_image(executable)
        report['source_sha256']=pe.HASH
        start,end=0x46c1af,0x46c1eb
        at=pe.image_offset(data,start,end-start)
        report['anchors']={hex(start):data[at:at+end-start].hex()}
        command=['g++','-m32','-std=c++17','-O2','-fno-pie','-no-pie','-MD','-MF',str(out/'reference.d'),str(ROOT/'tests/campaign-mini-ingress-reference.cpp'),'-o',str(out/'reference')]
        with (out/'build.log').open('w') as log:subprocess.run(command,check=True,stdout=log,stderr=subprocess.STDOUT)
        result=subprocess.run([str(out/'reference'),str(executable)],capture_output=True,text=True,check=True,timeout=10)
        report['original']=json.loads(result.stdout)
        report['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==v for p,v in sources.items())
        report['input_unchanged']=pe.pinned_image(executable)==data
        if not report['source_stable'] or not report['original']['success']:raise ValueError('Original reference/source check failed')
        report['success']=True
    finally:
        subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
        report['original_manifest_verified_before_after']=True
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        (out/'.retain').write_text('Original Escape/tutorial branch evidence; preserve reference and compiler provenance.\n')
    print('Original Escape/tutorial Mini branch: six cases passed')

if __name__=='__main__':main()
