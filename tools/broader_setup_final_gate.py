"""Retain the additional zero-start setup row beside all unchanged final predicates."""
from pathlib import Path
from coverage_evidence import Reader
from broader_player_acceptance import checked
from broader_setup_continuation_budget import review
from rendering_pilot import require

def supplement(reader,root,rows,values,plan):
    q=review(reader,plan);closed=q['closed_setup_requests'][0];require(rows[closed['case_id']]==closed,'Closed setup row changed')
    require(len(rows)==162 and sum(v['reserved_games']for v in rows.values())==202
            and len(values)==90 and sum(v['spec']['case']['reserved_original_games']for v in values)==98,
            'Complete162/202 ledger and90/98 positive matrix required')
    ref=q['current_controls'];controls=checked(reader,ref)
    require(controls['tests']==33 and controls['files']==3 and controls['original_game_runs']==controls['new_native_executions']==0
            and controls['sources_stable']and controls['Native_sources_unchanged']
            and set(controls['affected_families'])=={'attachment','font','palette','picking'}
            and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Current four-family setup controls missing')
    failure=checked(reader,q['closed_setup_certificate']);require(not(Path(root)/closed['report']).exists(),'Failed setup acquisition was invented')
    from broader_player_copy_matrix import P
    hn=P+'B06-hotspot-analysis-controls-01.json';hc=checked(reader,dict(record=hn,sha256=reader.digest(hn)))
    require(hc['tests']==93 and hc['files']==2 and hc['original_game_runs']==hc['new_native_executions']==0
            and hc['sources_stable']and all(reader.digest(n)==h for n,h in hc['sources'].items()),'Retained hotspot analysis controls missing')
    hotspot=[v for v in values if v['spec']['case']['id']in('bp27-world-hotspot-default640-01','bp27-world-hotspot-default800-01')]
    require(len(hotspot)==2 and all(v.get('analysis_revalidation')for v in hotspot),'Both exact retained hotspot analyses required')
    negative=P.replace('broader-player-acceptance-results-20261110/','cursor-hotspot-results-20261108/bp27/world-hotspot/')+'bp27-world-hotspot-default640-01-comparison-01.json'
    raw=reader.json(negative);require(raw['success']is False and raw['comparison']['success']and raw['comparison']['mismatching_pixels']==0
            and raw['error']=='Original cursor draw did not precede the actual readiness paint','Original partial-paint analysis negative changed')
    repaired=[]
    for v in values:
        if v['spec']['family']not in controls['affected_families']:continue
        a=v['acquired'];g=a['provider_adaptation']['canonical_plan_guard_adaptation']
        require(g['phase']==v['spec']['plan']and g['changed_guards']==1 and g['all_other_AST_nodes_unchanged']
                and g['actions_deadlines_budgets_Native_code_unchanged']
                and reader.digest(g['original_source']['record'])==g['original_source']['sha256']==g['source_sha256']
                and reader.digest(g['adapted_source']['record'])==g['adapted_source']['sha256']==g['adapted_source_sha256'],
                'Actual canonical-plan adapter/source proof missing')
        repaired.append(dict(case=v['spec']['case']['id'],validation=v['validation'],acquisition=v['acquisition'],guard=g))
    require(len(repaired)==16,'All four guarded families/providers/extents required')
    return dict(verified=True,review=plan['setup_continuation_review'],certificate=q['closed_setup_certificate'],closed_request=closed,closed_actual_Original_starts=0,closed_reservation_preserved=1,old_rows_preserved=71,old_charges_preserved=103,positive_contexts=90,positive_actual_starts=98,total_rows=162,total_charges=202,combined_with_WP10=210,controls=ref,actual_guarded_cases=repaired,retained_hotspot_controls=dict(record=hn,sha256=reader.digest(hn)),retained_hotspot_analyses=[v['analysis_revalidation']for v in hotspot],original_partial_paint_negative=dict(record=negative,sha256=reader.digest(negative)),reservations_refunded=False,historical_failure_retired=False,Native_source_waivers=[],scope='One failed pre-launch plan-prefix request remains an exact charged historical negative. Fresh attachment declarations and the same remaining positive domains complete the unchanged90/98 matrix; only the extra closed setup charge changes final accounting. Each of the16affected current cases retains exact original/adapted controller source bytes and one guard-only AST change,with all existing actions,deadlines,capture/provider andNative predicates required.')
