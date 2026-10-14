#!/usr/bin/env python3
"""Private original Realm delegation and synthetic Qt wire/session checks."""
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

def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path)
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True);args=parser.parse_args()
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    b=next(x for x in r['behaviors'] if x['id']=='NP.realm-menu-dispatch')
    scenario=next(x for x in r['scenarios'] if x['id']=='realm-adapter-private-20261012')
    claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{x['id']:x for x in r['builds']}),scenarios={scenario['id']:scenario_contract(scenario)})]
    paths=required_sources(b)|set(scenario['tests'])
    def fingerprints():return {p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(paths)}
    sources=fingerprints();declaration=json.loads(args.claims.read_text())
    if declaration['claims']!=claims or declaration['sources']!=sources:raise ValueError('Prospective Realm declaration mismatch')
    parent=ROOT/'working/tests/realm-adapter';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    (out/'.retain').write_text('Private Realm adapter evidence; synthetic campaign inputs, not progressed live gameplay.\n')
    (out/'claims.json').write_bytes(args.claims.read_bytes())
    def run(command,name,timeout=180,**kwargs):
        p=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=timeout,**kwargs)
        (out/(name+'.log')).write_text(p.stdout+p.stderr);p.check_returncode();return p.stdout
    run([ROOT/'tools/original-manifest.sh','verify'],'original-before')
    try:
        exporter=module('realm_export','tools/export-menu-support.py');exe=ROOT/'original/Arcane_Nocd/Chaos.exe';data=exporter.pinned_image(exe)
        stubs=module('realm_reference','tools/test-realm-selection-contract.py').STUBS
        header='// Expected dependency bytes from the complete hash-pinned image.\n'
        for name,va in stubs.items():
            at=exporter.image_offset(data,va,8);header+=f'static const unsigned {name}_address=0x{va:x};\nstatic const unsigned char {name}_bytes[]={{'+','.join(hex(x) for x in data[at:at+8])+'};\n'
        (out/'realm_selection_fixture_bytes.h').write_text(header)
        run(['g++','-m32','-fno-pie','-no-pie','-std=c++17','-O2','-Wall','-Wextra','-Werror','-I'+str(out),ROOT/'tests/realm-adapter-reference.cpp',ROOT/'reconstruction/campaign/realm_selection.cpp','-o',out/'reference'],'compile')
        private=json.loads(run([out/'reference',exe],'private',30))
        if not private['success'] or (private['admissions'],private['actions'],private['guards'])!=(4,7,23):raise ValueError('Incomplete private Realm matrix')
        run(['python3',ROOT/'tools/build-menu-observer.py'],'pe32-build')
        build=ROOT/'working/build/qt-shell'
        run(['cmake','-S',ROOT/'apps/qt-shell','-B',build,'-DBUILD_TESTING=ON','-DCMAKE_BUILD_TYPE=Release'],'configure',120)
        run(['cmake','--build',build,'--target','mnm-qt-shell','menu-realm-bridge-test','menu-character-bridge-test','realm-viewer-test','campaign-menu-fixture','--parallel','4'],'qt-build',600)
        closure=module('realm_compiler','tools/test-native-campaign.py').linked_compiler_sources(build,ROOT)
        if not closure<=set(sources):raise ValueError('Undeclared compiler inputs: '+repr(sorted(closure-set(sources))))
        qt=json.loads(run([build/'menu-realm-bridge-test'],'qt-realm',30,env=dict(os.environ,QT_QPA_PLATFORM='offscreen')))
        if not qt['success'] or qt['cases']!=11:raise ValueError('Incomplete Qt Realm cases')
        run(['ctest','--test-dir',build,'-R','^(qt-menu-character|qt-realm-viewer|qt-campaign-menu-fixture)$','--output-on-failure'],'regression',120)
        if sources!=fingerprints() or hashlib.sha256(exe.read_bytes()).hexdigest()!=exporter.HASH:raise ValueError('Execution inputs changed')
        report=dict(success=True,private=private,qt=qt,pe32_builds=1,qt_regressions=3,source_stable=True,source_sha256=exporter.HASH,
                    sources=sources,execution_claims=claims,scope=b['scope'],live_validated=False,original_work_bypassed=False,
                    artifacts={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in out.iterdir() if p.is_file()})
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Private original Realm delegation and Qt wire/session passed',flush=True)
    finally:run([ROOT/'tools/original-manifest.sh','verify'],'original-after')

if __name__=='__main__':main()
