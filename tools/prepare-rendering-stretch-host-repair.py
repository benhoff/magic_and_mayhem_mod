#!/usr/bin/env python3
"""One host-close repair plus the two never-started software domains."""
import argparse,importlib.util,json,sqlite3,subprocess
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_pilot import require
from natural_copy_applicability import execution_review
from rendering_build_binding import native_runtime_source_paths

ROOT=Path(__file__).resolve().parents[1];P='research/runtime/rendering-copy-natural-results-20261108/'


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
    require(a.report.startswith(P) and not (ROOT/a.report).exists(),'Fresh owned host-repair plan required')
    spec=importlib.util.spec_from_file_location('copy_host_collect',ROOT/'tools/run-rendering-copy-branches.py')
    collector=importlib.util.module_from_spec(spec);spec.loader.exec_module(collector)
    with Reader(ROOT) as reader:
        failed_name=P+'default800-01-acquisition.json';failed=reader.json(failed_name)
        require(not failed['success'] and failed['original_game_runs']==1 and failed['charged_original_games']==1
                and failed['error'].startswith('Fatal X11 BadWindow/X_SendEvent') and failed['prefix_pool']['returned']
                and failed['retirement']['state']=='completed' and failed['sources_match_plan_at_recovery'],
                'Exact retained external host-close crash required')
        plan=reader.json(failed['plan']['record'])
        require(reader.digest(failed['plan']['record'])==failed['plan']['sha256'],'Failed source plan changed')
        native=set(plan['compiler_sources'])|native_runtime_source_paths(ROOT)
        require(all(n in plan['sources'] and reader.digest(n)==plan['sources'][n] for n in native),'Native implementation changed')
        proof_name=P+'default640-10-branch-review-02.json';proof=reader.json(proof_name)
        require(proof['success'] and proof['natural_stretch_calls']==84 and proof['mismatching_pixels']==0
                and all(reader.digest(n)==h for n,h in proof['analysis_sources'].items()),'Current retained scout analysis required')
        scout=reader.json(proof['acquisition']['record'])
        require(reader.digest(proof['acquisition']['record'])==proof['acquisition']['sha256']
                and execution_review(reader,scout)==proof['execution_source_review'], 'Scout applicability changed')
        with sqlite3.connect(ROOT/'working/copy-natural-attempts.sqlite3') as db:
            rows=[list(r) for r in db.execute('SELECT name,charged,success FROM attempts ORDER BY name')]
        failures=[*plan['reviewed_failures'],failed_name]
        require(len(rows)==10 and sum(r[1] for r in rows)==8 and sum(r[2]==1 for r in rows)==1
                and {r[0] for r in rows if r[1] and not r[2]}==set(failures),'Original attempts must remain charged')
        require(collector.sha(ROOT/plan['player_binary']['path'])==plan['player_binary']['sha256']
                ==reader.digest(plan['player_binary']['record']),'Tested player changed')
        plan.update(kind='natural_stretch_host_close_repair_plan',
            source_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
            sources={n:reader.digest(n) for n in sorted(collector.source_paths(plan))},reviewed_failures=failures,
            repair_of=dict(record=failed_name,sha256=reader.digest(failed_name)),
            repair_reason='One exact host-close race repair. Wait for normal host exit first; trap only BadWindow/X_SendEvent for its exact retired window in the bounded close fallback. Real destroyed-window control passes. No native/observed Journey change. Recollect only High with lost external receipts; software domains never started. Preserve seven failed starts and two zero-game managed refusals, plus separate setup07.',
            expansion=dict(prior_attempts=rows,additional_requests=3,additional_original_starts=3,stop_on_failure=True,
                current_scout_analysis=dict(record=proof_name,sha256=reader.digest(proof_name))),
            purpose='Only default800 host-close repair and first software640/software800 observations. Existing Low scout is preserved by exact scoped AST review; no unrelated acceptance is reopened.')
        plan['bounds'].update(maximum_requests=13,maximum_charged_original_games=11)
        source=a.report.removesuffix('.json')+'-collector.bin'
        plan['collector_source']=dict(record=source,sha256=write_report(ROOT,source,(ROOT/'tools/run-rendering-copy-branches.py').read_bytes()))
    print(write_report(ROOT,a.report,(json.dumps(plan,indent=2)+'\n').encode()),flush=True)


if __name__=='__main__':main()
