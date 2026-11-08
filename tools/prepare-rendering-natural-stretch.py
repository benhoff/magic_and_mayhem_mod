#!/usr/bin/env python3
"""Freeze only the three missing domains after the positive natural stretch scout."""
import argparse,importlib.util,json,sqlite3,subprocess
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-copy-natural-results-20261108/'


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
    require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'Fresh owned plan required')
    spec=importlib.util.spec_from_file_location('natural_collect',ROOT/'tools/run-rendering-copy-branches.py')
    collector=importlib.util.module_from_spec(spec);spec.loader.exec_module(collector)
    scout=PREFIX+'default640-10-acquisition.json';proof=PREFIX+'default640-10-branch-review.json'
    with Reader(ROOT) as reader:
        acquired=reader.json(scout);checked=reader.json(proof)
        require(acquired['success'] and checked['success'] and checked['natural_stretch_calls']>0
                and checked['mismatching_pixels']==0 and checked['closed_call_census']
                and checked['acquisition']==dict(record=scout,sha256=reader.digest(scout))
                and acquired['configuration']['software'] is False
                and acquired['configuration']['actual_size']==[640,480],'Positive default640 scout required')
        plan=reader.json(acquired['plan']['record'])
        require(reader.digest(acquired['plan']['record'])==acquired['plan']['sha256'],'Scout plan changed')
        require(all(reader.digest(n)==h for n,h in plan['sources'].items()),'Scout execution source changed')
        require(reader.digest(plan['player_binary']['record'])==collector.sha(ROOT/plan['player_binary']['path'])
                ==plan['player_binary']['sha256'],'Tested player differs')
        with sqlite3.connect(ROOT/'working/copy-natural-attempts.sqlite3') as db:
            rows=[list(r) for r in db.execute('SELECT name,charged,success FROM attempts ORDER BY name')]
        require(len(rows)==9 and sum(r[1] for r in rows)==7 and sum(r[2]==1 for r in rows)==1
                and [r[0] for r in rows if r[2]==1]==[scout]
                and {r[0] for r in rows if r[1] and not r[2]}==set(plan['reviewed_failures']),
                'Exact stopped scout budget required')
        plan.update(kind='natural_stretch_remaining_domain_plan',
            source_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
            sources={n:reader.digest(n) for n in sorted(collector.source_paths(plan))},
            configurations=[dict(software=False,size=[800,600]),dict(software=True,size=[640,480]),dict(software=True,size=[800,600])],
            expansion=dict(scout=dict(record=scout,sha256=reader.digest(scout)),
                proof=dict(record=proof,sha256=reader.digest(proof)),prior_attempts=rows,
                additional_requests=3,additional_original_starts=3,stop_on_failure=True),
            purpose='Complete only the remaining actual800/default and640/800/software domains with unchanged tested NCS1 player, native runtime, public navigation and readback bounds. No accepted unrelated journey is rerun; no alias or original-driver equivalence is inferred.')
        plan['bounds'].update(maximum_requests=12,maximum_charged_original_games=10)
        plan['collector_source']=dict(record=a.report.removesuffix('.json')+'-collector.bin',
            sha256=write_report(ROOT,a.report.removesuffix('.json')+'-collector.bin',(ROOT/'tools/run-rendering-copy-branches.py').read_bytes()))
    print(write_report(ROOT,a.report,(json.dumps(plan,indent=2)+'\n').encode()),flush=True)


if __name__=='__main__':main()
