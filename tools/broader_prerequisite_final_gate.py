"""Extend the full runtime/caller gate with one preserved capacity failure."""
import inspect
from broader_runtime_final_gate import supplement as fading_supplement
from broader_runtime_caller_final_gate import caller_binding,supplement as caller_supplement
from broader_caller_loader_budget import review,P,FAILED,PILOT
from broader_prerequisite_budget import review as prerequisite_review,PILOT as FONT_PILOT
from broader_player_acceptance import checked
from broader_player_runtime_caller_matrix import specs as original_specs
from rendering_pilot import require

def exact_prerequisite_ledger(rows,values,old,failed,zero,font):
    require(len(rows)==186 and sum(v['reserved_games']for v in rows.values())==234
            and len(values)==90 and sum(v['spec']['case']['reserved_original_games']for v in values)==98,
            'Complete186/234ledger and90/98positive matrix required')
    fixed={v['case_id']:v for v in old};positive={v['spec']['case']['id']for v in values}
    require(len(fixed)==93 and sum(v['reserved_games']for v in old)==133
            and all(rows.get(k)==v for k,v in fixed.items()),'Historical93/133requests changed')
    require(failed['case_id']==FAILED and failed['state']=='failed'and failed['reserved_games']==1
            and rows.get(FAILED)==failed and FAILED not in fixed,'Exact new capacity failure changed')
    require(len(positive)==90 and positive.isdisjoint(fixed)and FAILED not in positive
            and zero['case_id']=='bp30-world-caller-default800-01'and zero['state']=='failed'and zero['reserved_games']==1 and zero['acquisition_sha256']is None and rows.get(zero['case_id'])==zero and zero['case_id']not in positive and zero['case_id']not in fixed and font['case_id']=='bp31-world-font-default800-01'and font['state']=='failed'and font['reserved_games']==1 and font['acquisition_sha256']and rows.get(font['case_id'])==font and font['case_id']not in positive and font['case_id']not in fixed and set(rows)==set(fixed)|positive|{FAILED,zero['case_id'],font['case_id']},'Failed,historical,duplicate or extra context received positive credit')

def _adapt_fading_gate():
    original=inspect.getsource(fading_supplement)
    old='total_rows=183,total_charges=231,combined_with_WP10=239'
    new='total_rows=186,total_charges=234,combined_with_WP10=242'
    require(original.count(old)==1,'Unexpected fading gate metadata')
    source=original.replace(old,new);require(source.replace(new,old)==original,'Unexpected fading gate change')
    namespace=dict(fading_supplement.__globals__)
    def exact_ledger(rows,values,q):
        failures=[v for v in rows.values()if v['case_id']==FAILED]
        require(len(failures)==1,'Missing exact failed capacity row')
        exact_prerequisite_ledger(rows,values,q['old_requests'],failures[0],rows['bp30-world-caller-default800-01'],rows['bp31-world-font-default800-01'])
    namespace['exact_ledger']=exact_ledger
    exec(compile(source,__file__,'exec'),namespace)
    return namespace['supplement']

_FADING=_adapt_fading_gate()
_CALLER_GLOBALS=dict(caller_supplement.__globals__);_CALLER_GLOBALS['original_supplement']=_FADING
exec(compile(inspect.getsource(caller_supplement),__file__,'exec'),_CALLER_GLOBALS)
_CALLER=_CALLER_GLOBALS['supplement']

def supplement(reader,root,rows,values,plan):
    controls_name=P+'prerequisite-final-reader-controls-02.json'
    controls=checked(reader,dict(record=controls_name,sha256=reader.digest(controls_name)))
    require(controls['tests']==95 and controls['original_game_runs']==0 and controls['current_acceptance_granted']is False and controls['sources_stable']and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Current complete loader final-reader controls missing')
    prerequisites=prerequisite_review(reader,plan);font_negative=checked(reader,prerequisites['failed_font_custody'])
    capacity=review(reader,plan)
    negative=checked(reader,capacity['failed_case_custody'])
    prior=checked(reader,capacity['parent_global_plan'])
    old=checked(reader,prior['runtime_continuation_review']);zero=checked(reader,capacity['failed_loader_custody'])
    exact_prerequisite_ledger(rows,values,old['old_requests'],negative['request_row_preserved_exactly'],zero['request_row_preserved_exactly'],font_negative['request_row_preserved_exactly'])
    result=_CALLER(reader,root,rows,values,plan)
    pilot_name=P+'loader-caller-single-pilot-01.json';pilot=checked(reader,dict(record=pilot_name,sha256=reader.digest(pilot_name)))
    actual=next(v for v in values if v['spec']['case']['id']==PILOT)
    require(pilot['qualified_current_contexts']==11 and pilot['acquisition']==actual['acquisition']
            and pilot['validation']==actual['validation']and pilot['conservative_charges_after']==146,
            'Capacity replacement lacks its actual qualified pilot')
    caller_values=[v for v in values if v['spec']['family']=='caller'and v['spec']['case']['id'].startswith('bp31-')]
    require(len(caller_values)==3,'All three remaining caller contexts required')
    for value in caller_values:
        receipt=value['acquired']['provider_adaptation']['caller_target_observation']
        require(receipt['policy']=='live-qt-target-properties-v1'and receipt['menu_bindings']==2 and receipt['click_bindings']==1
                and receipt['cached_observations']is False and receipt['game_actions_changed']is False
                and receipt['response_deadlines_changed']is False and receipt['native_code_or_limits_changed']is False
                and receipt['full_screenshots_unchanged']and receipt['physical_click_body_AST_verified'],
                'Caller target observation changed beyond reviewed lookup')
    result['caller_capacity_continuation']=dict(verified=True,review=plan['caller_loader_review'],final_reader_controls=dict(record=controls_name,sha256=reader.digest(controls_name)),
        failed_case=capacity['failed_case_custody'],zero_game_loader_refusal=capacity['failed_loader_custody'],real_recursive_loader_controls=capacity['recursive_loader_controls'],qualified_replacement=dict(record=pilot_name,sha256=reader.digest(pilot_name)),
        exact_prior_rows_preserved=105,prior_charges_preserved=145,ten_current_contexts_reused_without_source_waiver=True,
        remaining_positive_contexts=80,remaining_positive_starts=88,total_rows=186,total_charges=234,combined_with_WP10=242,
        target_observation_controls=capacity['target_observation_controls'],original_game_runs=0,
        historical_failure_retired=False,reservations_refunded=False,automatic_retries=False)
    font_pilot_name=P+'prerequisite-font-single-pilot-02.json';font_pilot=checked(reader,dict(record=font_pilot_name,sha256=reader.digest(font_pilot_name)));font_actual=next(v for v in values if v['spec']['case']['id']==FONT_PILOT)
    require(font_pilot['qualified_current_contexts']==14 and font_pilot['conservative_charges_after']==150 and font_pilot['acquisition']==font_actual['acquisition']and font_pilot['validation']==font_actual['validation'],'Fresh font prerequisite replacement pilot missing')
    binding=checked(reader,font_pilot['positive_pilot_binding']);require(binding['validation']==font_actual['validation']and binding['payload_bytes_unchanged'],'Font expansion pilot binding changed')
    result['font_prerequisite_continuation']=dict(verified=True,review=plan['font_prerequisite_review'],failed_zero_game_case=prerequisites['failed_font_custody'],current_control_refresh=prerequisites['control_refresh'],replacement_pilot=dict(record=font_pilot_name,sha256=reader.digest(font_pilot_name)),retained_current_contexts=13,remaining_positive_contexts=77,remaining_positive_starts=85,total_rows=186,total_charges=234,combined_with_WP10=242,old_hashes_refreshed=False,Native_source_waivers=[],reservations_refunded=False,historical_failure_retired=False)
    return result

def same_context(value,original):
    actual=value['spec'];require(actual['role']==original['role']and actual['family']==original['family']
        and all(actual['case'][k]==original['case'][k]for k in ['profile','provider','size','reserved_original_games']),
        'Renamed finite context changed provider,extent,profile,role or start domain')
    if original['role']=='strict':require(actual['case']['stress_seconds']==original['case']['stress_seconds'],'Strict context duration changed')
    return value

def prior_context(reader,actual,previous_id):
    """Resolve a declared context key; report paths and payload hashes stay exact."""
    originals=[s for s in original_specs(reader)if s['case']['id']==previous_id]
    require(len(originals)==1,'Missing/ambiguous original finite context')
    if previous_id in actual:return same_context(actual[previous_id],originals[0])
    catalog=reader.json(P+'prerequisite-source-catalog-01.json');parents=[reader.json(P+n)for n in ['loader-source-catalog-01.json','capacity-source-catalog-01.json']]
    previous_bindings={v['case_id']:v for parent in parents for v in parent['case_bindings']}
    def original_id(binding):
        prior=binding.get('previous_case_id',binding['case_id']);seen=set()
        while prior in previous_bindings and prior not in seen:
            seen.add(prior);ancestor=previous_bindings[prior];next_id=ancestor.get('previous_case_id',prior)
            if next_id==prior:break
            prior=next_id
        return prior
    matches=[v for v in catalog['case_bindings']if original_id(v)==previous_id]
    require(len(matches)==1 and matches[0]['case_id']in actual,'Missing/ambiguous finite context lineage')
    value=actual[matches[0]['case_id']]
    require(value['spec']['case']['id']==matches[0]['case_id']and value['spec']['family']==matches[0]['family']
            and value['spec']['role']==matches[0]['role'],'Context lineage changed family or role')
    return same_context(value,originals[0])
