#!/usr/bin/env python3
"""Execute production UI observer headers on privately owned32bit fixture pages."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
from coverage_evidence import write_report
from rendering_ui_acceptance import decode,decode_keys,PREFIX
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
    require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'Fresh fixture report required')
    names=['tools/test-hud-acceptance-observer.py','tests/hud-acceptance-observer.cpp','runtime/menu/hud_acceptance.h','runtime/render/ui_key_observation.h','tools/rendering_ui_acceptance.py']
    sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in names}
    out=Path(tempfile.mkdtemp(prefix='mnm-hud-observer-'));result=dict(schema=1,kind='native_ui_observer_fixture',success=False,sources=sources,original_game_runs=0,original_artifacts_consumed=False)
    try:
        command=['g++','-m32','-O2','-fno-pie','-no-pie','-std=c++17','-Wall','-Wextra','-Werror',str(ROOT/'tests/hud-acceptance-observer.cpp'),'-o',str(out/'fixture')]
        built=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=60);require(built.returncode==0,'Fixture compile: '+built.stderr)
        run=subprocess.run([out/'fixture',out/'ui.bin',out/'keys.bin'],capture_output=True,text=True,timeout=10)
        require(run.returncode==0,'Fixture execution: '+str(run.returncode)+' '+run.stderr)
        ui=decode((out/'ui.bin').read_bytes());keys=decode_keys((out/'keys.bin').read_bytes());r=ui['rows'][0]
        require(len(ui['rows'])==5 and ui['calls']==2 and len(keys['rows'])==4 and (r['mode'],r['input_mode'],r['accepted_sequence'])==(1,2,42)
                and (r['wizard']['maximum_mana'],r['wizard']['mana'])==(20,15*256),'Production observation fields or callback count differs')
        result.update(success=True,command=command,build_stdout=built.stdout,build_stderr=built.stderr,execution_stdout=run.stdout,execution_stderr=run.stderr,
            decoded=ui,keys=keys,artifacts=[])
        for name in ('ui.bin','keys.bin'):
            raw=(out/name).read_bytes();record=a.report.removesuffix('.json')+'-'+name
            result['artifacts'].append(dict(record=record,sha256=write_report(ROOT,record,raw)))
        require(all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items()),'Fixture source drift')
    except Exception as e:result.update(success=False,error=str(e))
    result['scope']='Private Linux32 pages and callback/API stubs execute the production observer headers: disabled/mismatched prologue refusals, exactly-once callback forwarding, return/LastError preservation, distinct pause/input/render fields, integer maximum/fixed-point current mana and owned key transitions. No original body, live game, DLL ABI or complete FP-register fixture claim.'
    print(write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode()))
    return int(not result['success'])


if __name__=='__main__':raise SystemExit(main())
