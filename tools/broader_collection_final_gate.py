"""Add exact setup-failure retention while preserving prior acceptance predicates."""
import broader_picking_pan_final_gate as previous
from broader_player_collection_matrix import specs
from broader_collection_budget import P,FAILED,PILOT,review,validate_rows
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
_PAN=_clone(previous.supplement,_UNCOVERED=_UNCOVERED)
_namespace=dict(previous.prior_context.__globals__)
_lineage_source=previous._lineage_source.replace('picking-pan-source-catalog-01.json','collection-source-catalog-01.json').replace("names=['picking-uncovered-source-catalog-01.json'","names=['picking-pan-source-catalog-01.json','picking-uncovered-source-catalog-01.json'")
require(_lineage_source!=previous._lineage_source,'Missing collection context lineage binding')
exec(compile(_lineage_source,__file__,'exec'),_namespace)
prior_context=_namespace['prior_context']

def project_rows(rows,old,new):
    require(len(rows)==195 and validate_rows(list(rows.values()),old,new)==244,'Complete195/244ledger required')
    require(FAILED in rows and rows[FAILED]['state']=='failed'and rows[FAILED]['reserved_games']==2 and rows[FAILED]['acquisition_sha256']is None,'Zero-execution setup negative changed/promoted')
    result={k:v for k,v in rows.items()if k!=FAILED}
    require(len(result)==194 and sum(v['reserved_games']for v in result.values())==242,'Changed194/242 prior projection')
    return result

def supplement(reader,root,rows,values,plan):
    q=review(reader,plan);projected=project_rows(rows,q['old_requests'],q['new_case_reservations'])
    require(len(values)==90 and len({v['spec']['case']['id']for v in values})==90 and all(v['spec']['case']['id']!=FAILED for v in values),'All90positive contexts must survive projection')
    result=_PAN(reader,root,projected,values,plan)
    pilot=checked(reader,dict(record=P+'collection41-single-pilot-01.json',sha256=reader.digest(P+'collection41-single-pilot-01.json')))
    require(pilot['success']and pilot['case']==PILOT and pilot['request_rows_after']==129 and pilot['conservative_charges_after']==171 and pilot['qualified_current_contexts']==24,'Missing fully qualified collection replacement pilot')
    actual=next(v for v in values if v['spec']['case']['id']==PILOT);require(actual['validation']==pilot['validation']and actual['acquisition']==pilot['acquisition'],'Replacement pilot mismatch')
    controls=checked(reader,dict(record=P+'collection-final-reader-controls-01.json',sha256=reader.digest(P+'collection-final-reader-controls-01.json')))
    require(controls['success']and controls['current_acceptance_granted']is False and controls['complete_ledger_refusals_verified']and controls['prior_predicates_bytecode_unchanged']and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Current collection final controls missing')
    result['prior_pan_projection']=dict(rows=194,charges=242,excluded_only=[FAILED],all90_actual_positive_cases_preserved=True,all_prior_acceptance_predicates_bytecode_unchanged=True)
    result['collection_map_setup_continuation']=dict(verified=True,review=plan['collection_map_review'],setup_negative=q['setup_diagnosis'],qualified_pilot=dict(record=P+'collection41-single-pilot-01.json',sha256=reader.digest(P+'collection41-single-pilot-01.json')),final_reader_controls=dict(record=P+'collection-final-reader-controls-01.json',sha256=reader.digest(P+'collection-final-reader-controls-01.json')),positive_contexts=90,positive_starts=98,total_rows=195,total_charges=244,combined_with_WP10=252,zero_execution_setup_reserved_charges=2,reservations_refunded=False,historical_failures_retired=False,original_game_runs=0)
    return result
