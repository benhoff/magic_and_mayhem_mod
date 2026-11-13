"""Extend the full runtime/caller gate with one preserved capacity failure."""
import inspect
from broader_runtime_final_gate import supplement as fading_supplement
from broader_runtime_caller_final_gate import supplement as caller_supplement
from broader_caller_capacity_budget import review,P,FAILED,PILOT
from broader_player_acceptance import checked
from rendering_pilot import require

def exact_capacity_ledger(rows,values,old,failed):
    require(len(rows)==184 and sum(v['reserved_games']for v in rows.values())==232
            and len(values)==90 and sum(v['spec']['case']['reserved_original_games']for v in values)==98,
            'Complete184/232ledger and90/98positive matrix required')
    fixed={v['case_id']:v for v in old};positive={v['spec']['case']['id']for v in values}
    require(len(fixed)==93 and sum(v['reserved_games']for v in old)==133
            and all(rows.get(k)==v for k,v in fixed.items()),'Historical93/133requests changed')
    require(failed['case_id']==FAILED and failed['state']=='failed'and failed['reserved_games']==1
            and rows.get(FAILED)==failed and FAILED not in fixed,'Exact new capacity failure changed')
    require(len(positive)==90 and positive.isdisjoint(fixed)and FAILED not in positive
            and set(rows)==set(fixed)|positive|{FAILED},'Failed,historical,duplicate or extra context received positive credit')

def _adapt_fading_gate():
    original=inspect.getsource(fading_supplement)
    old='total_rows=183,total_charges=231,combined_with_WP10=239'
    new='total_rows=184,total_charges=232,combined_with_WP10=240'
    require(original.count(old)==1,'Unexpected fading gate metadata')
    source=original.replace(old,new);require(source.replace(new,old)==original,'Unexpected fading gate change')
    namespace=dict(fading_supplement.__globals__)
    def exact_ledger(rows,values,q):
        failures=[v for v in rows.values()if v['case_id']==FAILED]
        require(len(failures)==1,'Missing exact failed capacity row')
        exact_capacity_ledger(rows,values,q['old_requests'],failures[0])
    namespace['exact_ledger']=exact_ledger
    exec(compile(source,__file__,'exec'),namespace)
    return namespace['supplement']

_FADING=_adapt_fading_gate()
_CALLER_GLOBALS=dict(caller_supplement.__globals__);_CALLER_GLOBALS['original_supplement']=_FADING
exec(compile(inspect.getsource(caller_supplement),__file__,'exec'),_CALLER_GLOBALS)
_CALLER=_CALLER_GLOBALS['supplement']

def supplement(reader,root,rows,values,plan):
    capacity=review(reader,plan)
    negative=checked(reader,capacity['failed_case_custody'])
    prior=checked(reader,capacity['parent_global_plan'])
    old=checked(reader,prior['runtime_continuation_review'])
    exact_capacity_ledger(rows,values,old['old_requests'],negative['request_row_preserved_exactly'])
    result=_CALLER(reader,root,rows,values,plan)
    pilot_name=P+'capacity-caller-single-pilot-01.json';pilot=checked(reader,dict(record=pilot_name,sha256=reader.digest(pilot_name)))
    actual=next(v for v in values if v['spec']['case']['id']==PILOT)
    require(pilot['qualified_current_contexts']==11 and pilot['acquisition']==actual['acquisition']
            and pilot['validation']==actual['validation']and pilot['conservative_charges_after']==145,
            'Capacity replacement lacks its actual qualified pilot')
    caller_values=[v for v in values if v['spec']['family']=='caller'and v['spec']['case']['id'].startswith('bp30-')]
    require(len(caller_values)==3,'All three remaining caller contexts required')
    for value in caller_values:
        receipt=value['acquired']['provider_adaptation']['caller_target_observation']
        require(receipt['policy']=='live-qt-target-properties-v1'and receipt['menu_bindings']==2 and receipt['click_bindings']==1
                and receipt['cached_observations']is False and receipt['game_actions_changed']is False
                and receipt['response_deadlines_changed']is False and receipt['native_code_or_limits_changed']is False
                and receipt['full_screenshots_unchanged']and receipt['physical_click_body_AST_verified'],
                'Caller target observation changed beyond reviewed lookup')
    result['caller_capacity_continuation']=dict(verified=True,review=plan['caller_capacity_review'],
        failed_case=capacity['failed_case_custody'],qualified_replacement=dict(record=pilot_name,sha256=reader.digest(pilot_name)),
        exact_prior_rows_preserved=104,prior_charges_preserved=144,ten_current_contexts_reused_without_source_waiver=True,
        remaining_positive_contexts=80,remaining_positive_starts=88,total_rows=184,total_charges=232,combined_with_WP10=240,
        target_observation_controls=capacity['target_observation_controls'],original_game_runs=0,
        historical_failure_retired=False,reservations_refunded=False,automatic_retries=False)
    return result

def prior_context(reader,actual,previous_id):
    """Resolve a declared context's renamed key, never alias a report or payload."""
    if previous_id in actual:return actual[previous_id]
    catalog=reader.json(P+'capacity-source-catalog-01.json')
    matches=[v for v in catalog['case_bindings']if v.get('previous_case_id')==previous_id]
    require(len(matches)==1 and matches[0]['case_id']in actual,'Missing/ambiguous finite context lineage')
    value=actual[matches[0]['case_id']]
    require(value['spec']['case']['id']==matches[0]['case_id']and value['spec']['family']==matches[0]['family']
            and value['spec']['role']==matches[0]['role'],'Context lineage changed family or role')
    return value
