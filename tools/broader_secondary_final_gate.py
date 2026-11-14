"""Conjoin the retained negative and staged replacement without rewriting any row."""
import broader_discovery_final_gate as previous
from broader_player_secondary_matrix import specs
from broader_secondary_budget import P,FAILED,PILOT,review,validate_rows
from broader_player_acceptance import checked
from broader_current_interaction_final_gate import reviewed_interaction
from rendering_pilot import require
same_context=previous.same_context
guarded_contexts=previous.guarded_contexts
caller_binding=previous.caller_binding
_clone=previous._clone


def rebound(new):
    require(new.get(FAILED)==1 and PILOT not in new,'Changed prospective interaction identity')
    result=dict(new);del result[FAILED];result[PILOT]=1
    return result


def collection_reservations(rows,old,new):
    return previous.collection_reservations(rows,old,rebound(new))


def strict_reservations(rows,old,new):
    return previous.strict_reservations(rows,old,rebound(new))


def discovery_reservations(rows,old,new):
    return previous.validate_rows(rows,old,rebound(new))


# Each inherited predicate keeps its exact code object. Only dependency bindings
# select the current catalog and compose one prospective single-start identity.
_FADING=_clone(previous._FADING,specs=specs)
_CALLER=_clone(previous._CALLER,original_supplement=_FADING)
_PRIOR=_clone(previous._PRIOR,_CALLER=_CALLER)
_WITNESS=_clone(previous._WITNESS,_PRIOR=_PRIOR)
_REBUILD=_clone(previous._REBUILD,_WITNESS=_WITNESS)
_UNCOVERED=_clone(previous._UNCOVERED,_REBUILD=_REBUILD)
_PAN=_clone(previous._PAN,_UNCOVERED=_UNCOVERED)
_COLLECTION_PROJECT=_clone(previous._COLLECTION_PROJECT,validate_rows=collection_reservations)
_COLLECTION=_clone(previous._COLLECTION,_PAN=_PAN,project_rows=_COLLECTION_PROJECT)
_STRICT_PROJECT=_clone(previous._STRICT_PROJECT,validate_rows=strict_reservations)
_STRICT=_clone(previous._STRICT,_COLLECTION=_COLLECTION,project_rows=_STRICT_PROJECT)
_DISCOVERY_PROJECT=_clone(previous.project_rows,validate_rows=discovery_reservations)
_DISCOVERY=_clone(previous.supplement,_STRICT=_STRICT,project_rows=_DISCOVERY_PROJECT)
_lineage_source=previous._lineage_source.replace('discovery-source-catalog-01.json','secondary-source-catalog-01.json').replace("names=['strict-binding-source-catalog-01.json'","names=['discovery-source-catalog-01.json','strict-binding-source-catalog-01.json'")
require(_lineage_source!=previous._lineage_source,'Missing explicit secondary context lineage')
_namespace=dict(previous.prior_context.__globals__)
exec(compile(_lineage_source,__file__,'exec'),_namespace)
prior_context=_namespace['prior_context']


def project_rows(rows,old,new):
    require(len(rows)==198 and validate_rows(list(rows.values()),old,new)==247,'Complete198/247 ledger required')
    require(rows[FAILED]['state']=='failed' and rows[FAILED]['reserved_games']==1
            and rows[FAILED]['acquisition_sha256'],'Actual runtime negative changed/promoted')
    result={k:v for k,v in rows.items() if k!=FAILED}
    require(len(result)==197 and sum(v['reserved_games'] for v in result.values())==246,
            'Changed197/246 prior projection')
    return result


def supplement(reader,root,rows,values,plan):
    q=review(reader,plan)
    projected=project_rows(rows,q['old_requests'],q['new_case_reservations'])
    require(len(values)==90 and len({v['spec']['case']['id']for v in values})==90
            and all(v['spec']['case']['id']!=FAILED for v in values),'All90 actual positive contexts required')
    retained=reviewed_interaction(reader,values)
    result=_DISCOVERY(reader,root,projected,values,plan)
    pn=P+'secondary44-single-pilot-01.json';pilot=checked(reader,dict(record=pn,sha256=reader.digest(pn)))
    require(pilot['success'] and pilot['case']==PILOT and pilot['request_rows_after']==154
            and pilot['conservative_charges_after']==203 and pilot['qualified_current_contexts']==46,
            'Missing fully qualified actual runtime replacement pilot')
    actual=next(v for v in values if v['spec']['case']['id']==PILOT)
    require(actual['validation']==pilot['validation'] and actual['acquisition']==pilot['acquisition']
            and actual['runtime_variant_current']['verified'],'Replacement pilot/runtime identity differs')
    variant_values=[v for v in values if 'scene_runtime_variant' in reader.json(v['spec']['plan'])]
    require(len(variant_values)==3 and all(v['spec']['family']=='interaction'
            and v['runtime_variant_current']['verified'] for v in variant_values),
            'All three new interaction contexts need fresh staged runtime verification')
    cn=P+'secondary-final-reader-controls-02.json';control=checked(reader,dict(record=cn,sha256=reader.digest(cn)))
    require(control['success'] and control['current_acceptance_granted'] is False
            and control['complete_ledger_refusals_verified'] and control['prior_predicate_code_objects_unchanged']
            and all(reader.digest(n)==h for n,h in control['sources'].items()),'Secondary final controls changed')
    result['retained_current_interaction_analysis']=dict(verified=True,conjunction=retained,
        additional_game_starts=0,additional_reservations=0,historical_failures_retired=False)
    result['secondary_runtime_continuation']=dict(verified=True,review=plan['secondary_caller_review'],
        failed_actual_runtime=q['direct_refusal_diagnosis'],negative_custody=q['negative_custody'],
        qualified_pilot=dict(record=pn,sha256=reader.digest(pn)),controls=dict(record=cn,sha256=reader.digest(cn)),
        variant_cases=[dict(case=v['spec']['case']['id'],verification=v['runtime_variant'])for v in variant_values],
        prior_projection=dict(rows=197,charges=246,excluded_only=[FAILED],actual_ledger_rows_renamed=False),
        prospective_reservation_identity=dict(previous=FAILED,current=PILOT),positive_contexts=90,
        positive_starts=98,total_rows=198,total_charges=247,combined_with_WP10=255,
        historical_failures_retired=False,reservations_refunded=False,source_waivers=[])
    return result
