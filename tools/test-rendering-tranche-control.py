#!/usr/bin/env python3
"""Prospectively bind a bounded rendering control to reviewed sources and scope.

This wrapper adds provenance, not engine coverage. Child reports and failures
remain immutable; its success requires successful execution and stable inputs.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import time

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--behavior',action='append',required=True)
    parser.add_argument('--scenario',action='append',default=[])
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--child-report',type=Path,action='append',default=[])
    parser.add_argument('command',nargs=argparse.REMAINDER)
    args=parser.parse_args()
    command=args.command[1:] if args.command[:1]==['--'] else args.command
    if not command:parser.error('A child command is required')
    out=args.output.resolve();out.mkdir(parents=True,exist_ok=False)
    spec=importlib.util.spec_from_file_location('tranche_claims',ROOT/'tools/draft-coverage-claims.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    declaration=module.declaration(ROOT,json.loads((ROOT/'research/runtime/coverage/register.json').read_text()),args.behavior,args.scenario)
    # Reviewed compile/runner closure. Required behavior/scenario links are
    # supplied above; additional shared inputs prevent a narrow declaration
    # from hiding changes in compiled includes or imported runner helpers.
    paths=set(declaration['sources'])
    for directory in ('runtime','renderer','protocols','assets','audio','reconstruction','apps/qt-shell','tests','tools'):
        for p in (ROOT/directory).rglob('*'):
            if p.is_file() and p.suffix in ('.c','.h','.cpp','.hpp','.py','.json','.txt','.cmake','.sh') and '__pycache__' not in p.parts:
                paths.add(str(p.relative_to(ROOT)))
    declaration['sources']={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(paths)}
    claims=out/'claims.json';claims.write_text(json.dumps(declaration,indent=2)+'\n')
    command=[s.replace('{claims}',str(claims)).replace('{output}',str(out)) for s in command]
    report=dict(schema=1,success=False,claims=declaration['claims'],sources=declaration['sources'],command=command,child_reports=[],scope='Bounded tranche control only; no automatic implementation or original-equivalence promotion.',started_unix=time.time())
    try:
        with (out/'execution.log').open('x') as log:
            result=subprocess.run(command,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT)
        report['exit_code']=result.returncode
        candidates=list(args.child_report)
        for line in (out/'execution.log').read_text(errors='replace').splitlines():
            p=Path(line.split('report:',1)[-1].strip())
            if p.suffix=='.json' and p.is_file() and p.resolve().is_relative_to(ROOT/'working'):
                candidates.append(p)
        for p in dict.fromkeys(candidates):
            value=json.loads(p.read_text())
            report['child_reports'].append(dict(path=str(p.resolve().relative_to(ROOT)),sha256=hashlib.sha256(p.read_bytes()).hexdigest(),success=value.get('success')))
        report['success']=result.returncode==0 and all(c['success'] is not False for c in report['child_reports'])
    except Exception as error:report['error']=str(error)
    finally:
        report['sources_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in declaration['sources'].items())
        report['success'] &= report['sources_stable'];report['finished_unix']=time.time()
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(out/'report.json',flush=True)
    return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
