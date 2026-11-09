#!/usr/bin/env python3
"""Queue only the last domain after a zero-game environment-pool refusal."""
import argparse,importlib.util,json,sqlite3,subprocess
from pathlib import Path
from coverage_evidence import Reader,write_report
from natural_copy_applicability import execution_review
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1];P='research/runtime/rendering-copy-natural-results-20261108/'


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
    require(a.report.startswith(P) and not (ROOT/a.report).exists(),'Fresh final-domain plan required')
    spec=importlib.util.spec_from_file_location('copy_final_collect',ROOT/'tools/run-rendering-copy-branches.py')
    collector=importlib.util.module_from_spec(spec);spec.loader.exec_module(collector)
    with Reader(ROOT) as reader:
        failure=P+'software800-01-acquisition.json';old=reader.json(failure)
        require(not old['success'] and old['original_game_runs']==old['charged_original_games']==old['admitted_original_sessions']==0
                and old['error']=='Both environment slots are busy or held; no additional clone was created', 'Exact zero-game pool refusal required')
        plan=reader.json(old['plan']['record']);require(reader.digest(old['plan']['record'])==old['plan']['sha256'],'Prior plan changed')
        require(all(reader.digest(n)==h for n,h in plan['sources'].items()),'Current frozen native/collector source changed')
        positives=[]
        for name,suffix in [('default640-10','-02'),('default800-02',''),('software640-01','')]:
            ref=P+name+'-branch-review'+suffix+'.json';proof=reader.json(ref)
            require(proof['success'] and proof['natural_stretch_calls']>0 and proof['mismatching_pixels']==0
                    and all(reader.digest(n)==h for n,h in proof['analysis_sources'].items()),'Positive domain analysis changed')
            acq=reader.json(proof['acquisition']['record'])
            require(reader.digest(proof['acquisition']['record'])==proof['acquisition']['sha256']
                    and execution_review(reader,acq)==proof['execution_source_review'],'Positive execution phase changed')
            positives.append(dict(record=ref,sha256=reader.digest(ref)))
        with sqlite3.connect(ROOT/'working/copy-natural-attempts.sqlite3') as db:
            rows=[list(r) for r in db.execute('SELECT name,charged,success FROM attempts ORDER BY name')]
        require(len(rows)==13 and sum(r[1] for r in rows)==10 and sum(r[2]==1 for r in rows)==3
                and {r[0] for r in rows if r[1] and not r[2]}==set(plan['reviewed_failures']), 'All original charges must remain counted')
        plan.update(kind='natural_stretch_queued_final_domain_plan',
            source_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
            sources={n:reader.digest(n) for n in sorted(collector.source_paths(plan))},configurations=[dict(software=True,size=[800,600])],
            final_domain=dict(zero_game_refusal=dict(record=failure,sha256=reader.digest(failure)),
                accepted_positive_analyses=positives,prior_attempts=rows,additional_requests=1,additional_original_starts=1,
                wait_for_exclusive_idle_pool_slot=True,maximum_queue_seconds=600),
            purpose='Only software800 after a retained zero-game pool refusal. Preserve the other three exact positive domains, all seven charged failures and three zero-game managed refusals. Do not touch foreign active loans or the unresolved old held slot; queue for exclusive idle admission. No matrix restart or new fixture.')
        plan['bounds'].update(maximum_requests=14,maximum_charged_original_games=11)
        source=a.report.removesuffix('.json')+'-collector.bin'
        plan['collector_source']=dict(record=source,sha256=write_report(ROOT,source,(ROOT/'tools/run-rendering-copy-branches.py').read_bytes()))
    print(write_report(ROOT,a.report,(json.dumps(plan,indent=2)+'\n').encode()),flush=True)


if __name__=='__main__':main()
