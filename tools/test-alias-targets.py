#!/usr/bin/env python3
"""Compare recovered alias targets, compact callbacks and order cursors."""
import importlib.util
import json
import os
from pathlib import Path
import sys
import tempfile
from coverage_evidence import read_json, write_report
from owner_planner_comparison import ROOT, SUITES, main


def run():
    if '--output' not in sys.argv:
        raise ValueError('Require --output')
    out=Path(sys.argv[sys.argv.index('--output')+1]).resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):
        raise ValueError('Require new task-owned working/tests output')
    result=ROOT/'research/runtime/alias-targets-results-20261023'/(out.name+'.json')
    if result.exists():
        raise ValueError('Require new immutable result identity')
    register=read_json(ROOT/'research/runtime/coverage/register.json',root=ROOT)
    bid='AI.borrowed-alias-target-bodies'
    behavior=next(b for b in register['behaviors'] if b['id']==bid)
    declaration_path=result.with_name(out.name+'-inputs.json')
    if declaration_path.exists():raise ValueError('Immutable input identity already exists')
    spec=importlib.util.spec_from_file_location('alias_target_declarations',ROOT/'tools/draft-coverage-claims.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    declaration=module.declaration(ROOT,register,[bid],['alias-targets-20261023'])
    declaration['source_versions']={p:(ROOT/p).read_text() for p in behavior['implementation']+behavior['tests']}
    bound=write_report(ROOT,declaration_path,(json.dumps(declaration,indent=2)+'\n').encode())
    out.parent.mkdir(parents=True,exist_ok=True)
    # Bounded single-consumer interchange required by the existing runner.
    interchange=out.parent/('claims-'+out.name+'.json')
    with interchange.open('x') as f:json.dump(declaration,f)
    sys.argv.extend(['--claims',str(interchange)])
    inspection=read_json(ROOT/'research/runtime/alias-targets-results-20261023/inspection-1.json',root=ROOT)
    ranges=[(0x51e890,0x51ec59),(0x5060a0,0x50617a),(0x505f70,0x506080),(0x524130,0x524382),(0x40e650,0x40e672),(0x4132e0,0x413302),(0x526977,0x5269a5),(0x5292c6,0x5292f0),(0x523e0f,0x523e34)]
    for c in inspection['tables']:
        for key in ['invoke','deleting','destruct']:
            rows=inspection['instructions'][hex(c[key])]
            ranges.append((c[key],rows[-1]['address']+rows[-1]['size']))
    endpoints=[0x5269a5,0x5292f0,0x523e34,0x52c9d0,0x512220,0x597850,0x597870,0x5979b0,0x44d970,0x4f5230,0x4f4900,0x5093e0,0x5070e0,0x509230,
               0x54dad0,0x54dce0,0x51f5e0,0x506d30,0x51a8f0,0x50f850,0x51aa30,0x51a880,0x5178c0,
               0x520e70,0x50ab30,0x4f2dd0,0x4970b0,0x50eb60,0x50aa30,0x4943f0,0x512340,0x52d0c0,0x520ad0]
    SUITES['alias-targets']=dict(ranges=ranges,endpoints=endpoints,
        scope=behavior['scope'],
        suite='alias-targets',sources=['tests/alias-targets-test.cpp','reconstruction/creature-ai/owner_knowledge.cpp'],wrapper='tools/test-alias-targets.py')
    try:
        with tempfile.TemporaryDirectory(prefix='alias-targets-compiler-',dir=ROOT/'working') as temporary:
            old=os.environ.get('TMPDIR');os.environ['TMPDIR']=temporary
            try:main('alias-targets')
            finally:
                if old is None:os.environ.pop('TMPDIR',None)
                else:os.environ['TMPDIR']=old
    finally:
        if (out/'report.json').exists():
            report=json.loads((out/'report.json').read_text())
            report['original_output_used_as_native_input']=False
            report['prospective_inputs']={'record':str(declaration_path.relative_to(ROOT)),'sha256':bound}
            report['boundaries']='Explicit lower service bodies; exact cursor projections only; Linux original fault cases and native refusal/throw policies do not establish Windows SEH/unwind equivalence. No World/live replacement.'
            write_report(ROOT,result,(json.dumps(report,indent=2)+'\n').encode())
        if out.exists():(out/'.retain').write_text('Alias target bounded binaries, diagnostics and failures; durable result in SQLite.\n')

if __name__=='__main__':
    run()
