"""Retain the pre-game strict negative and every earlier acceptance predicate."""
import broader_collection_final_gate as previous
from broader_player_strict_binding_matrix import specs
from broader_strict_binding_budget import P,FAILED,PILOT,review,validate_rows
from broader_player_acceptance import checked
from rendering_pilot import require
same_context=previous.same_context
guarded_contexts=previous.guarded_contexts
caller_binding=previous.caller_binding
_clone=previous._clone
_FADING=_clone(previous._FADING,specs=specs)
_CALLER=_clone(previous._CALLER,original_supplement=_FADING)
_PRIOR=_clone(previous._PRIOR,_CALLER=_CALLER)
_WITNESS=_clone(previous._WITNESS,_PRIOR=_PRIOR)
_REBUILD=_clone(previous._REBUILD,_WITNESS=_WITNESS)
_UNCOVERED=_clone(previous._UNCOVERED,_REBUILD=_REBUILD)
_PAN=_clone(previous._PAN,_UNCOVERED=_UNCOVERED)

def prior_reservations(rows,old,new):
    # Prior prospective data names the zero-start strict ID. Its new successful
    # replacement has the identical single-start context; keep actual rows exact.
    require(FAILED in new and new[FAILED]==1 and PILOT not in new,'Changed strict prospective identity')
    rebound=dict(new);del rebound[FAILED];rebound[PILOT]=1
    return previous.validate_rows(rows,old,rebound)

_PROJECT=_clone(previous.project_rows,validate_rows=prior_reservations)
_COLLECTION=_clone(previous.supplement,_PAN=_PAN,project_rows=_PROJECT)
_namespace=dict(previous.prior_context.__globals__)
_lineage_source=previous._lineage_source.replace('collection-source-catalog-01.json','strict-binding-source-catalog-01.json').replace("names=['picking-pan-source-catalog-01.json'","names=['collection-source-catalog-01.json','picking-pan-source-catalog-01.json'")
require(_lineage_source!=previous._lineage_source,'Missing strict context lineage binding')
exec(compile(_lineage_source,__file__,'exec'),_namespace)
prior_context=_namespace['prior_context']

def project_rows(rows,old,new):
    require(len(rows)==196 and validate_rows(list(rows.values()),old,new)==245,'Complete196/245 ledger required')
    require(FAILED in rows and rows[FAILED]['state']=='failed'and rows[FAILED]['reserved_games']==1 and rows[FAILED]['acquisition_sha256']is None,'Strict setup negative changed/promoted')
    result={k:v for k,v in rows.items()if k!=FAILED}
    require(len(result)==195 and sum(v['reserved_games']for v in result.values())==244,'Changed195/244 prior projection')
    return result

def supplement(reader,root,rows,values,plan):
    q=review(reader,plan);projected=project_rows(rows,q['old_requests'],q['new_case_reservations'])
    require(len(values)==90 and len({v['spec']['case']['id']for v in values})==90 and all(v['spec']['case']['id']!=FAILED for v in values),'All90 positive contexts must survive projection')
    result=_COLLECTION(reader,root,projected,values,plan)
    pilot=checked(reader,dict(record=P+'strict-binding42-single-pilot-01.json',sha256=reader.digest(P+'strict-binding42-single-pilot-01.json')))
    require(pilot['success']and pilot['case']==PILOT and pilot['request_rows_after']==138 and pilot['conservative_charges_after']==187 and pilot['qualified_current_contexts']==32,'Missing qualified strict replacement pilot')
    actual=next(v for v in values if v['spec']['case']['id']==PILOT);require(actual['validation']==pilot['validation']and actual['acquisition']==pilot['acquisition'],'Strict replacement pilot mismatch')
    controls=checked(reader,dict(record=P+'strict-binding-final-reader-controls-01.json',sha256=reader.digest(P+'strict-binding-final-reader-controls-01.json')))
    require(controls['success']and controls['current_acceptance_granted']is False and controls['complete_ledger_refusals_verified']and controls['prior_predicates_bytecode_unchanged']and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Current strict final controls missing')
    result['prior_collection_projection']=dict(rows=195,charges=244,excluded_only=[FAILED],all90_actual_positive_cases_preserved=True,all_prior_acceptance_predicates_bytecode_unchanged=True,prior_prospective_single_start_identity_rebound=dict(previous=FAILED,current=PILOT,actual_ledger_rows_renamed=False))
    result['strict_declaration_setup_continuation']=dict(verified=True,review=plan['strict_binding_review'],setup_negative=q['setup_diagnosis'],qualified_pilot=dict(record=P+'strict-binding42-single-pilot-01.json',sha256=reader.digest(P+'strict-binding42-single-pilot-01.json')),final_reader_controls=dict(record=P+'strict-binding-final-reader-controls-01.json',sha256=reader.digest(P+'strict-binding-final-reader-controls-01.json')),positive_contexts=90,positive_starts=98,total_rows=196,total_charges=245,combined_with_WP10=253,zero_execution_setup_reserved_charges=1,reservations_refunded=False,historical_failures_retired=False,original_game_runs=0)
    return result
