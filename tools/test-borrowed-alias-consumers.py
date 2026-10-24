#!/usr/bin/env python3
"""Compare13 member callback classes and task refresh after snapshot expiry."""
import importlib.util
import json
import os
from pathlib import Path
import sys
import tempfile
from coverage_evidence import read_json, write_report
from owner_planner_comparison import ROOT, SUITES, main

BID='AI.owner-borrowed-alias-consumers'
SCENARIO='borrowed-alias-consumers-20261023'
DIRECTORY='research/runtime/borrowed-alias-consumers-results-20261023'


def run():
    if '--output' not in sys.argv:
        raise ValueError('Require --output')
    out=Path(sys.argv[sys.argv.index('--output')+1]).resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):
        raise ValueError('Require new task-owned working/tests output')
    register=read_json(ROOT/'research/runtime/coverage/register.json',root=ROOT)
    behavior=next(b for b in register['behaviors'] if b['id']==BID)
    inspection=read_json(ROOT/DIRECTORY/'inspection-3.json',root=ROOT)
    ranges=[(0x407920,0x407982),(0x4079f0,0x407a73),(0x407a80,0x407b03),
            (0x407c10,0x407de3),(0x407ed0,0x407f5b),(0x42f2e0,0x42f322)]
    for c in inspection['tables']:
        for key in ['invoke','delete','destruct']:
            rows=inspection['instructions'][hex(c[key])]
            ranges.append((c[key],rows[-1]['address']+rows[-1]['size']))
    SUITES['borrowed-alias']=dict(ranges=ranges,endpoints=[0x597850,0x597870,0x5979b0],
        scope=behavior['scope'],suite='borrowed-alias-consumers',
        sources=['tests/borrowed-alias-consumers-test.cpp','reconstruction/creature-ai/owner_knowledge.cpp'],
        wrapper='tools/test-borrowed-alias-consumers.py')
    declaration_path=ROOT/DIRECTORY/(out.name+'-inputs.json')
    result_path=ROOT/DIRECTORY/(out.name+'.json')
    if declaration_path.exists() or result_path.exists():
        raise ValueError('Immutable result identity already exists')
    spec=importlib.util.spec_from_file_location('alias_declarations',ROOT/'tools/draft-coverage-claims.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    declaration=module.declaration(ROOT,register,[BID],[SCENARIO])
    out.parent.mkdir(parents=True,exist_ok=True)
    # Single-consumer bounded interchange for the shared comparison runner.
    interchange=out.parent/('claims-'+out.name+'.json')
    with interchange.open('x') as f:
        json.dump(declaration,f)
    bound=write_report(ROOT,declaration_path,(json.dumps(declaration,indent=2)+'\n').encode())
    sys.argv.extend(['--claims',str(interchange)])
    try:
        with tempfile.TemporaryDirectory(prefix='alias-compiler-',dir=ROOT/'working') as temporary:
            previous=os.environ.get('TMPDIR')
            os.environ['TMPDIR']=temporary
            try:
                main('borrowed-alias')
            finally:
                if previous is None:
                    os.environ.pop('TMPDIR',None)
                else:
                    os.environ['TMPDIR']=previous
    finally:
        if (out/'report.json').exists():
            report=json.loads((out/'report.json').read_text())
            report['prospective_inputs']={'record':str(declaration_path.relative_to(ROOT)),'sha256':bound}
            report['original_output_used_as_native_input']=False
            report['boundaries']='Targets are explicit member-call recorders;actual actor51e890 and other lower targets are not implemented by this comparison. Freed callback invocation is refused only by native checked storage,never executed in original. Original missing-record refresh runs with expired snapshot PROT_NONE. Native record index is a host representation;original42f2e0 destroys original snapshot,real407c10 resolves the same key/tree. No full World/live replacement.'
            write_report(ROOT,result_path,(json.dumps(report,indent=2)+'\n').encode())
        if out.exists():
            (out/'.retain').write_text('Borrowed alias source/callback comparison binaries,diagnostics and failed attempts. Durable reports and declarations are SQLite references.\n')


if __name__=='__main__':
    run()
