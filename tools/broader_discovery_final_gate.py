"""Preserve every earlier predicate and the zero-game discovery failure."""
import broader_strict_provenance_final_gate as previous
import broader_strict_binding_final_gate as strict_previous
from broader_player_discovery_matrix import specs
from broader_discovery_budget import P,FAILED,PILOT,review,validate_rows
from broader_player_acceptance import checked
from rendering_pilot import require
same_context=previous.same_context
guarded_contexts=previous.guarded_contexts
caller_binding=previous.caller_binding
_clone=strict_previous._clone
_FADING=_clone(previous._FADING,specs=specs)
_CALLER=_clone(previous._CALLER,original_supplement=_FADING)
_PRIOR=_clone(previous._PRIOR,_CALLER=_CALLER)
_WITNESS=_clone(previous._WITNESS,_PRIOR=_PRIOR)
_REBUILD=_clone(previous._REBUILD,_WITNESS=_WITNESS)
_UNCOVERED=_clone(previous._UNCOVERED,_REBUILD=_REBUILD)
_PAN=_clone(previous._PAN,_UNCOVERED=_UNCOVERED)

def prior_case_reservations(new):
    require(new.get(FAILED)==1 and PILOT not in new,'Changed prospective boundary identity')
    rebound=dict(new);del rebound[FAILED];rebound[PILOT]=1;return rebound

def collection_reservations(rows,old,new):
    return strict_previous.prior_reservations(rows,old,prior_case_reservations(new))

def strict_reservations(rows,old,new):
    return strict_previous.validate_rows(rows,old,prior_case_reservations(new))

_COLLECTION_PROJECT=_clone(previous._PROJECT,validate_rows=collection_reservations)
_COLLECTION=_clone(previous._COLLECTION,_PAN=_PAN,project_rows=_COLLECTION_PROJECT)
_STRICT_PROJECT=_clone(strict_previous.project_rows,validate_rows=strict_reservations)
_STRICT=_clone(previous.supplement,_COLLECTION=_COLLECTION,project_rows=_STRICT_PROJECT)
_namespace=dict(previous.prior_context.__globals__)
_lineage_source=strict_previous._lineage_source.replace('strict-binding-source-catalog-01.json','discovery-source-catalog-01.json').replace("names=['collection-source-catalog-01.json'","names=['strict-binding-source-catalog-01.json','collection-source-catalog-01.json'")
require(_lineage_source!=strict_previous._lineage_source,'Missing collection discovery lineage')
exec(compile(_lineage_source,__file__,'exec'),_namespace)
prior_context=_namespace['prior_context']

def project_rows(rows,old,new):
    require(len(rows)==197 and validate_rows(list(rows.values()),old,new)==246,'Complete197/246 ledger required')
    require(FAILED in rows and rows[FAILED]['state']=='failed'and rows[FAILED]['reserved_games']==1 and rows[FAILED]['acquisition_sha256'],'Discovery setup failure changed/promoted')
    result={k:v for k,v in rows.items()if k!=FAILED}
    require(len(result)==196 and sum(v['reserved_games']for v in result.values())==245,'Changed196/245 prior projection');return result

def supplement(reader,root,rows,values,plan):
    q=review(reader,plan);projected=project_rows(rows,q['old_requests'],q['new_case_reservations'])
    require(len(values)==90 and len({v['spec']['case']['id']for v in values})==90 and all(v['spec']['case']['id']!=FAILED for v in values),'All90 positive contexts required')
    result=_STRICT(reader,root,projected,values,plan)
    pn=P+'discovery43-single-pilot-01.json';pilot=checked(reader,dict(record=pn,sha256=reader.digest(pn)))
    require(pilot['success']and pilot['case']==PILOT and pilot['request_rows_after']==143 and pilot['conservative_charges_after']==192 and pilot['qualified_current_contexts']==36,'Missing fully qualified discovery replacement pilot')
    actual=next(v for v in values if v['spec']['case']['id']==PILOT);require(actual['validation']==pilot['validation']and actual['acquisition']==pilot['acquisition'],'Discovery replacement pilot mismatch')
    cn=P+'discovery-final-reader-controls-01.json';controls=checked(reader,dict(record=cn,sha256=reader.digest(cn)))
    require(controls['success']and controls['current_acceptance_granted']is False and controls['complete_ledger_refusals_verified']and controls['prior_predicates_bytecode_unchanged']and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Current discovery final controls missing')
    result['prior_strict_projection']=dict(rows=196,charges=245,excluded_only=[FAILED],all90_actual_positive_cases_preserved=True,all_prior_acceptance_predicates_bytecode_unchanged=True,prior_prospective_single_start_identity_rebound=dict(previous=FAILED,current=PILOT,actual_ledger_rows_renamed=False))
    result['collection_discovery_setup_continuation']=dict(verified=True,review=plan['discovery_binding_review'],setup_negative=q['setup_diagnosis'],qualified_pilot=dict(record=pn,sha256=reader.digest(pn)),final_reader_controls=dict(record=cn,sha256=reader.digest(cn)),positive_contexts=90,positive_starts=98,total_rows=197,total_charges=246,combined_with_WP10=254,zero_execution_setup_reserved_charges=1,reservations_refunded=False,historical_failures_retired=False,original_game_runs=0)
    return result
