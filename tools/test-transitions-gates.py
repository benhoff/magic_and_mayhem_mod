#!/usr/bin/env python3
"""Retain exact current actual-data transitions-gate adversarial controls."""
import argparse
import importlib.util
import io
import json
from pathlib import Path
import platform
import sqlite3
import sys
import unittest
import numpy
from coverage_claims import behavior_contract,required_sources,scenario_contract
from coverage_evidence import Reader,write_report
from transitions_gate import BID,SID,PREFIX,local_python_sources
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
    require(a.report.startswith(PREFIX)and'..'not in Path(a.report).parts and not(ROOT/a.report).exists(),'Fresh owned controls report required')
    with Reader(ROOT)as r:
        reg=r.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors']if v['id']==BID);s=next(v for v in reg['scenarios']if v['id']==SID)
        names=required_sources(b);require(local_python_sources()<=names,'Unpinned local control dependency')
        before={n:r.digest(n)for n in sorted(names)}
    operator=Path(__file__).read_bytes();op=a.report.removesuffix('.json')+'-runner.bin'
    op_ref=dict(record=op,sha256=write_report(ROOT,op,operator))
    spec=importlib.util.spec_from_file_location('transitions_gate_actual_controls',ROOT/'tests/test-transitions-gates.py')
    test=importlib.util.module_from_spec(spec);spec.loader.exec_module(test)
    log=io.StringIO();result=unittest.TextTestRunner(stream=log,verbosity=2).run(unittest.defaultTestLoader.loadTestsFromModule(test))
    log_name=a.report.removesuffix('.json')+'-log.bin';log_ref=dict(record=log_name,sha256=write_report(ROOT,log_name,log.getvalue().encode()))
    with Reader(ROOT)as r:
        stable=all(r.digest(n)==h for n,h in before.items())and local_python_sources()<=names and Path(__file__).read_bytes()==operator
    q=dict(schema=1,kind='transitions_gate_actual_data_controls',success=result.wasSuccessful()and stable and not result.skipped,
        tests=result.testsRun,failures=len(result.failures),errors=len(result.errors),skipped=len(result.skipped),sources=before,sources_stable=stable,
        claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={SID:scenario_contract(s)})],
        runner=op_ref,log=log_ref,development_fixture_overlay=False,original_game_runs=0,new_native_executions=0,new_pixel_comparisons=0,
        python=platform.python_version(),sqlite=sqlite3.sqlite_version,numpy=numpy.__version__)
    sha=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode())
    print(json.dumps(dict(success=q['success'],record=a.report,sha256=sha,tests=q['tests'],failures=q['failures'],errors=q['errors'],sources_stable=stable)),flush=True)
    if not q['success']:print(log.getvalue(),flush=True)
    return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
