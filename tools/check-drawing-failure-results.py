#!/usr/bin/env python3
"""Compare native recovered caller/recovery traces using only owned fault schedules."""
import argparse,importlib.util,json,subprocess,tempfile
from pathlib import Path
from drawing_api_session import sha
from drawing_api_results import compare_fault_traces
ROOT=Path(__file__).resolve().parents[1]
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',type=Path,required=True);p.add_argument('--claims',type=Path);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    s=importlib.util.spec_from_file_location('retry',ROOT/'tools/check-original-surface-retry.py');r=importlib.util.module_from_spec(s);s.loader.exec_module(r);rows,meta=r.load()
    names=['tools/check-drawing-failure-results.py','tools/drawing_api_results.py','tools/drawing_api_session.py','tools/check-original-surface-retry.py','tests/surface-retry-model.cpp','reconstruction/rendering/surface_retry.hpp','tests/fixtures/surfaces/original-retry-corpus.json','tests/fixtures/surfaces/'+meta['path'],'tests/test-drawing-api-results.py']
    claims=json.loads(a.claims.read_text()) if a.claims else None;sources={n:sha((ROOT/n).read_bytes()) for n in names};sources.update(claims['sources'] if claims else {})
    if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective fault sources changed')
    parent=ROOT/'working/tests/drawing-failure-results';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));binary=run/'native-fault-replay'
    subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror','-pedantic',str(ROOT/'tests/surface-retry-model.cpp'),'-o',str(binary)],check=True)
    inputs=run/'owned-inputs.txt';inputs.write_text(''.join(' '.join(str(v) for v in [*(c[k] for k in ['id','entry','fast','no_wait','key_enabled','reload_bits','source_restore','destination_restore','key_result','color']),len(c['draw_results']),*c['draw_results']])+'\n' for c in (row['input'] for row in rows)))
    results={}
    for name,budget in [('complete',None),('bounded',2)]:
        cmd=[str(binary),str(inputs)]+([str(budget)] if budget is not None else []);actual=subprocess.run(cmd,capture_output=True,text=True,check=True,timeout=60);(run/(name+'.json')).write_text(actual.stdout);results[name]=compare_fault_traces(json.loads(actual.stdout),rows,budget)
    for budget in ('0','65537'):
        out=subprocess.run([str(binary),str(inputs),budget],capture_output=True,timeout=60)
        if out.returncode==0:raise ValueError('Invalid fault draw budget accepted')
    if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Fault comparison source drift')
    report={'schema':1,'success':True,'sources':sources,'sources_stable':True,'original_sha256':meta['original_sha256'],'reference_corpus_sha256':meta['sha256'],'original_reference_kind':meta['oracle'],'complete':results['complete'],'bounded':results['bounded'],'invalid_budgets_refused':2,'original_oracles_used_as_native_inputs':False,'original_driver_results_in_fault_schedule_are_owned_inputs':True,'real_driver_executed':False,'live_game_recovery_executed':False,'complete_operation_failure_coverage':False,'artifacts':str(run.relative_to(ROOT)),'native_binary_sha256':sha(binary.read_bytes()),'scope':'Recovered native wrapper scheduling versus4480independently retained unchanged-original/scripted COM fault traces. All expected original events stay outside native inputs. Draw/Restore/key driver outcomes are controlled fault inputs. Native bounded refusal preserves original prefixes; no true driver loss, restored pixels/palettes, whole-game recovery or completed global operation gates.'}
    if claims:report['claims']=claims['claims']
    a.report.parent.mkdir(parents=True,exist_ok=True)
    with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
    print(json.dumps({'success':True,'cases':len(rows),'budget_exhaustions':results['bounded']['budget_exhaustions']}))
if __name__=='__main__':main()
