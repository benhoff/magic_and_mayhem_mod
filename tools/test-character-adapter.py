#!/usr/bin/env python3
"""Scope-bound private original Character adapter and synthetic Qt transaction tests."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True);args=parser.parse_args()
    parent=ROOT/'working/tests/character-adapter';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def run(command,name,timeout=180,**kw):
        value=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=timeout,**kw)
        (out/(name+'.log')).write_text(value.stdout+value.stderr);value.check_returncode();return value
    run([ROOT/'tools/original-manifest.sh','verify'],'original-before')
    try:
        declaration=json.loads(args.claims.read_text());register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        b=next(b for b in register['behaviors'] if b['id']=='NP.character-menu-dispatch');builds={b['id']:b for b in register['builds']}
        scenario=next(s for s in register['scenarios'] if s['id']=='character-adapter-private-20261012')
        paths=required_sources(b)|set(scenario['tests']);sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(paths)}
        claims=[{'behavior':b['id'],'contract_sha256':behavior_contract(b,builds),'scenarios':{scenario['id']:scenario_contract(scenario)}}]
        if declaration['claims']!=claims or declaration['sources']!=sources:raise ValueError('Declaration differs from current adapter scope/sources')
        (out/'claims.json').write_bytes(args.claims.read_bytes())
        spec=importlib.util.spec_from_file_location('adapter_image',ROOT/'tools/export-menu-support.py')
        exporter=importlib.util.module_from_spec(spec);spec.loader.exec_module(exporter)
        exe=ROOT/'original/Arcane_Nocd/Chaos.exe';data=exporter.pinned_image(exe);at=exporter.image_offset(data,0x557510,8)
        (out/'character_adapter_fixture.h').write_text('static const unsigned char fade_bytes[]={'+','.join(hex(v) for v in data[at:at+8])+'};\n')
        run(['g++','-m32','-fno-pie','-no-pie','-std=c++17','-O2','-Wall','-Wextra','-Werror','-I'+str(out),ROOT/'tests/character-adapter-reference.cpp','-o',out/'reference'],'compile-private')
        private=json.loads(run([out/'reference',exe],'private',30).stdout)
        if not private.get('success') or private['guard_cases']!=16:raise ValueError('Incomplete private adapter guards')
        run(['python3',ROOT/'tools/build-menu-observer.py'],'build-observer')
        run(['cmake','-S',ROOT/'apps/qt-shell','-B',ROOT/'working/build/qt-shell'],'configure')
        run(['cmake','--build',ROOT/'working/build/qt-shell','--target','mnm-qt-shell','menu-character-bridge-test','character-screen-test','menu-host-lobby-bridge-test','-j4'],'build-qt',600)
        run(['ctest','--test-dir',ROOT/'working/build/qt-shell','-R','^(qt-menu-character|qt-character-screen|qt-menu-host-lobby-bridge)$','--output-on-failure','--output-junit',out/'qt.xml'],'ctest',60,env=dict(os.environ,QT_QPA_PLATFORM='offscreen'))
        if sources!={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(paths)}:raise ValueError('Adapter sources changed')
        if hashlib.sha256(exe.read_bytes()).hexdigest()!=exporter.HASH:raise ValueError('Original input changed')
        report=dict(success=True,source_sha256=exporter.HASH,private=private,qt_tests=3,execution_claims=claims,sources=sources,
                    live_validated=False,original_work_bypassed=False,scope=b['scope'],
                    artifacts={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in out.iterdir() if p.is_file()})
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Private Character adapter guards/original callbacks and3 Qt tests passed',flush=True)
    finally:run([ROOT/'tools/original-manifest.sh','verify'],'original-after')
if __name__=='__main__':main()
