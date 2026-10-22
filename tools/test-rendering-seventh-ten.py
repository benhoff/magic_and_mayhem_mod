#!/usr/bin/env python3
"""Execute ten bounded offline rendering contracts with immutable SQLite evidence."""
import argparse
import ast
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
from coverage_evidence import write_report, evidence_digest

ROOT=Path(__file__).resolve().parents[1]
BEHAVIORS=['NR.drawing-two-stage-source-chain','NR.drawing-cursor-mutex-prefix-binding','NR.drawing-cursor-map-composition','NR.drawing-source-frame-binding','NR.drawing-source-update-intervals','NR.drawing-egl-source-domain-accounting','NR.drawing-source-physical-interval-close','NR.drawing-copy-api-worker-association','NR.drawing-worker-prefix-analysis','NR.drawing-source-worker-egl-binding']
SCENARIO='NR.seventh-ten.offline-identities-20261022'
TESTS=['tests/test-drawing-'+n+'.py' for n in ['two-stage-source','cursor-mutex-binding','cursor-map-composition','source-frame-binding','source-windows','egl-source-domains','source-interval','copy-bindings','copy-return-execution','source-worker-egl','source-completion','worker-producer-points','engine-windows','api-physical','egl-physical']]+['tests/test-rendering-seventh-ten.py']


def sha(path):return evidence_digest(Path(path),root=ROOT)


def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path)
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report',type=Path,required=True)
    args=parser.parse_args()
    report_path=args.report if args.report.is_absolute() else ROOT/args.report
    if report_path.exists():parser.error('Refusing evidence overwrite')
    parent=ROOT/'working/tests/rendering-seventh-ten';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    (out/'.retain').write_text('Seventh rendering batch raw controls and failed attempts. Generated evidence is in SQLite.\n')
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    declaration=module('seventh_claims','tools/draft-coverage-claims.py').declaration(ROOT,register,BEHAVIORS,[SCENARIO])
    todo=set(declaration['sources'])|set(TESTS)|{'tools/test-rendering-seventh-ten.py','tools/coverage_evidence.py','tools/coverage_claims.py','tools/draft-coverage-claims.py','tools/drawing_observed_inputs.py'}
    visited=set()
    while todo:
        name=todo.pop()
        if name in visited:continue
        visited.add(name);path=ROOT/name;declaration['sources'][name]=sha(path)
        if path.suffix=='.py':
            for node in ast.walk(ast.parse(path.read_text())):
                if isinstance(node,ast.Constant) and isinstance(node.value,str) and node.value.endswith('.py'):
                    for candidate in (ROOT/node.value,path.parent/node.value,ROOT/'tools'/node.value,ROOT/'tests'/node.value):
                        if candidate.is_file() and candidate.resolve().is_relative_to(ROOT):todo.add(candidate.resolve().relative_to(ROOT).as_posix())
                names=([alias.name for alias in node.names] if isinstance(node,ast.Import) else
                  [node.module]+[node.module+'.'+alias.name for alias in node.names] if isinstance(node,ast.ImportFrom) and node.module else [])
                for name in names:
                    relative=Path(*name.split('.'))
                    for base in (ROOT/'tools',ROOT/'protocols/python'):
                        for candidate in (base/relative.with_suffix('.py'),base/relative/'__init__.py'):
                            if candidate.is_file():todo.add(candidate.relative_to(ROOT).as_posix())
        if path.suffix in {'.c','.cpp','.h','.hpp','.S','.inc'}:
            for include in re.findall(r'^\s*#\s*include\s*"([^"]+)"',path.read_text(),re.M):
                child=(path.parent/include).resolve()
                if child.is_file() and child.is_relative_to(ROOT):todo.add(child.relative_to(ROOT).as_posix())
    report=dict(schema=1,success=False,sources=declaration['sources'],claims=declaration['claims'],commands=[],tests_run=0,
        scope=__doc__,original_game_executed=False,original_artifacts_consumed=False,original_comparison_claimed=False,live_replacement=False,
        reviewed_head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),artifacts=out.relative_to(ROOT).as_posix())
    env=dict(os.environ,TMPDIR=str(out));env['PYTHONDONTWRITEBYTECODE']='1'
    try:
        for path in TESTS:
            print(path,flush=True)
            command=[sys.executable,'-B',str(ROOT/path)]
            execution=subprocess.run(command,cwd=ROOT,env=env,capture_output=True,text=True,timeout=120)
            log=execution.stdout+execution.stderr
            (out/(Path(path).stem+'.log')).write_text(log)
            report['commands'].append(dict(argv=command,actual_exit=execution.returncode,expected_exit=0,log=log,log_sha256=hashlib.sha256(log.encode()).hexdigest()))
            if execution.returncode:raise ValueError(path+': '+log)
            match=re.search(r'Ran (\d+) tests?',log)
            if not match:raise ValueError('Missing executed-test count: '+path)
            report['tests_run']+=int(match[1])
        report['sources_stable']=all(sha(ROOT/name)==digest for name,digest in report['sources'].items())
        if not report['sources_stable']:raise ValueError('Source drift during execution')
        current=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        fresh=module('seventh_final_claims','tools/draft-coverage-claims.py').declaration(ROOT,current,BEHAVIORS,[SCENARIO])
        if fresh['claims']!=declaration['claims']:raise ValueError('Contract drift during execution')
        report['success']=True
    except Exception as error:report['error']=repr(error)
    finally:
        report['source_inputs']=len(report['sources'])
        digest=write_report(ROOT,report_path,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],report=str(report_path),sha256=digest,tests_run=report['tests_run'],source_inputs=report['source_inputs'],error=report.get('error'))),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
