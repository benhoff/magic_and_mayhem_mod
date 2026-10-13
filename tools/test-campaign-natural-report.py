#!/usr/bin/env python3
"""Private original natural report OK callback plus synthetic Qt return."""
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
    spec=importlib.util.spec_from_file_location(name,ROOT/path);value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True);args=parser.parse_args()
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(x for x in r['behaviors'] if x['id']=='NP.campaign-natural-report-dispatch');s=next(x for x in r['scenarios'] if x['id']=='campaign-natural-report-private-20261012')
    claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{x['id']:x for x in r['builds']}),scenarios={s['id']:scenario_contract(s)})]
    paths=required_sources(b)|set(s['tests'])
    def fingerprints():return {p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(paths)}
    sources=fingerprints();declaration=json.loads(args.claims.read_text())
    if declaration['claims']!=claims or declaration['sources']!=sources:raise ValueError('Prospective report declaration mismatch')
    parent=ROOT/'working/tests/campaign-natural-report';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    (out/'.retain').write_text('Private report delegation and synthetic Qt evidence; not genuine progressed gameplay.\n');(out/'claims.json').write_bytes(args.claims.read_bytes())
    def run(command,name,timeout=180,**kwargs):
        value=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=timeout,**kwargs);(out/(name+'.log')).write_text(value.stdout+value.stderr);value.check_returncode();return value.stdout
    run([ROOT/'tools/original-manifest.sh','verify'],'original-before')
    try:
        exporter=module('report_export','tools/export-menu-support.py');exe=ROOT/'original/Arcane_Nocd/Chaos.exe';data=exporter.pinned_image(exe)
        header='// Expected dependency bytes from the complete hash-pinned executable.\n'
        for name,va in {'cue':0x4a39a0,'focus':0x474b50}.items():
            at=exporter.image_offset(data,va,8);header+=f'static const unsigned {name}_address=0x{va:x};\nstatic const unsigned char {name}_bytes[]={{'+','.join(hex(x) for x in data[at:at+8])+'};\n'
        (out/'campaign_natural_report_bytes.h').write_text(header)
        run(['g++','-m32','-fno-pie','-no-pie','-std=c++17','-O2','-Wall','-Wextra','-Werror','-I'+str(out),ROOT/'tests/campaign-natural-report-reference.cpp','-o',out/'reference'],'compile')
        private=json.loads(run([out/'reference',exe],'private',30))
        if not private['success'] or (private['outcomes'],private['guards'])!=(2,13):raise ValueError('Incomplete original report guards')
        run(['python3',ROOT/'tools/build-menu-observer.py'],'pe32-build')
        build=ROOT/'working/build/qt-shell';run(['cmake','-S',ROOT/'apps/qt-shell','-B',build,'-DBUILD_TESTING=ON','-DCMAKE_BUILD_TYPE=Release'],'configure',120)
        targets=['mnm-qt-shell','menu-campaign-natural-report-test','menu-campaign-defeat-test','menu-region-campaign-test','menu-character-bridge-test','campaign-menu-fixture']
        run(['cmake','--build',build,'--target',*targets,'--parallel','4'],'qt-build',600)
        closure=module('region_compiler','tools/test-native-campaign.py').linked_compiler_sources(build,ROOT)
        if not closure<=set(sources):raise ValueError('Undeclared compiler inputs: '+repr(sorted(closure-set(sources))))
        qt=json.loads(run([build/'menu-campaign-natural-report-test'],'qt-region',30,env=dict(os.environ,QT_QPA_PLATFORM='offscreen')))
        if not qt['success'] or qt['cases']!=8:raise ValueError('Incomplete Qt report journey')
        run(['ctest','--test-dir',build,'-R','^(qt-menu-campaign-defeat|qt-menu-region-campaign|qt-menu-character|qt-campaign-menu-fixture)$','--output-on-failure'],'regression',120)
        if fingerprints()!=sources or hashlib.sha256(exe.read_bytes()).hexdigest()!=exporter.HASH:raise ValueError('Execution inputs changed')
        report=dict(success=True,private=private,qt=qt,pe32_builds=1,qt_regressions=4,source_stable=True,source_sha256=exporter.HASH,sources=sources,execution_claims=claims,scope=b['scope'],live_validated=False,original_work_bypassed=False,artifacts={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in out.iterdir() if p.is_file()})
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Private original report delegation and Qt campaign return passed',flush=True)
    finally:run([ROOT/'tools/original-manifest.sh','verify'],'original-after')
if __name__=='__main__':main()
