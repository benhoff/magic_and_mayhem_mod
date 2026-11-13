"""Preserve the zero-game setup row and require its actual qualified replacement."""
import ast,inspect
import broader_secondary_final_gate as previous
from broader_derived_budget import P,FAILED,HOVER_FAILED,DEATH_FAILED,PILOT,review,validate_rows
from broader_player_derived_matrix import specs
from broader_player_acceptance import checked
from rendering_pilot import require
same_context=previous.same_context
guarded_contexts=previous.guarded_contexts
caller_binding=previous.caller_binding
_clone=previous._clone


def latest_reservations(new):
    require(new.get(FAILED)==1 and PILOT not in new,'Changed prospective startup replacement identity')
    result=dict(new);del result[FAILED];result[PILOT]=1;return result


def rebound(new):
    return latest_reservations(previous.rebound(new))


collection_reservations=_clone(previous.collection_reservations,rebound=rebound)
strict_reservations=_clone(previous.strict_reservations,rebound=rebound)
discovery_reservations=_clone(previous.discovery_reservations,rebound=rebound)
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
_DISCOVERY_PROJECT=_clone(previous._DISCOVERY_PROJECT,validate_rows=discovery_reservations)
_DISCOVERY=_clone(previous._DISCOVERY,_STRICT=_STRICT,project_rows=_DISCOVERY_PROJECT)


_PRIOR_SECONDARY_VALIDATE=_clone(previous.validate_rows,PILOT=PILOT)


def secondary_reservations(rows,old,new):
    return _PRIOR_SECONDARY_VALIDATE(rows,old,latest_reservations(new))


_SECONDARY_PROJECT=_clone(previous.project_rows,validate_rows=secondary_reservations)
PILOT_CHANGES={'secondary44-single-pilot-01.json':'derived48-single-pilot-01.json',154:157,203:206}


def selected_prior_source(source):
    tree=ast.parse(source);counts={}
    class Select(ast.NodeTransformer):
        def visit_Constant(self,node):
            if type(node.value)in(str,int)and node.value in PILOT_CHANGES:
                counts[node.value]=counts.get(node.value,0)+1;return ast.copy_location(ast.Constant(PILOT_CHANGES[node.value]),node)
            return node
    tree=Select().visit(tree);require(counts=={'secondary44-single-pilot-01.json':1,154:1,203:1},
                                     'Prior pilot receipt/count predicate changed')
    return ast.unparse(ast.fix_missing_locations(tree))+'\n'


# The formerly planned bp44 pilot failed before Original startup. Select the
# new same-context bp46 proof and its actual cumulative counts explicitly;
# never turn the failed bp44 row into a positive or rename any actual row.
_namespace=dict(previous.supplement.__globals__,_DISCOVERY=_DISCOVERY,project_rows=_SECONDARY_PROJECT,PILOT=PILOT)
exec(compile(selected_prior_source(inspect.getsource(previous.supplement)),__file__,'exec'),_namespace)
_SECONDARY=_namespace['supplement']
_lineage_source=previous._lineage_source.replace('secondary-source-catalog-01.json','derived-source-catalog-03.json').replace("names=['discovery-source-catalog-01.json'","names=['derived-source-catalog-02.json','derived-source-catalog-01.json','secondary-source-catalog-02.json','secondary-source-catalog-01.json','discovery-source-catalog-01.json'")
require(_lineage_source!=previous._lineage_source,'Missing derived context lineage')
_lineage_namespace=dict(previous.prior_context.__globals__);exec(compile(_lineage_source,__file__,'exec'),_lineage_namespace);prior_context=_lineage_namespace['prior_context']


def project_rows(rows,old,new):
    require(len(rows)==201 and validate_rows(list(rows.values()),old,new)==250,'Complete201/250ledger required')
    require(rows[FAILED]['state']=='failed'and rows[FAILED]['reserved_games']==1 and rows[FAILED]['acquisition_sha256'],
            'Zero-Original startup failure changed/promoted')
    require(rows[HOVER_FAILED]['state']=='failed'and rows[HOVER_FAILED]['reserved_games']==1 and rows[HOVER_FAILED]['acquisition_sha256'],
            'Actual hover failure changed/promoted')
    require(rows[DEATH_FAILED]['state']=='failed'and rows[DEATH_FAILED]['reserved_games']==1 and rows[DEATH_FAILED]['acquisition_sha256'],
            'Natural Quick death failure changed/promoted')
    result={k:v for k,v in rows.items()if k not in(FAILED,HOVER_FAILED,DEATH_FAILED)}
    require(len(result)==198 and sum(v['reserved_games']for v in result.values())==247,'Changed198/247prior projection')
    return result


def supplement(reader,root,rows,values,plan):
    q=review(reader,plan);projected=project_rows(rows,q['old_requests'],q['new_case_reservations'])
    require(len(values)==90 and len({v['spec']['case']['id']for v in values})==90
            and all(v['spec']['case']['id']not in(FAILED,HOVER_FAILED,DEATH_FAILED)for v in values),'All90actual positive contexts required')
    result=_SECONDARY(reader,root,projected,values,plan)
    pn=P+'derived48-single-pilot-01.json';pilot=checked(reader,dict(record=pn,sha256=reader.digest(pn)))
    require(pilot['success']and pilot['case']==PILOT and pilot['request_rows_after']==157
            and pilot['conservative_charges_after']==206 and pilot['qualified_current_contexts']==46,
            'Missing qualified derived-admission replacement pilot')
    actual=next(v for v in values if v['spec']['case']['id']==PILOT)
    require(actual['acquisition']==pilot['acquisition']and actual['validation']==pilot['validation']
            and actual['derived_admission_current']['verified'],'Derived pilot input/admission mismatch')
    selected=[v for v in values if 'derived_native_admission'in reader.json(v['spec']['plan'])]
    require(len(selected)==3 and all(v['spec']['family']=='interaction'and v['derived_admission_current']['verified']for v in selected),
            'Allthree new interaction contexts need actual derived startup admission')
    cn=P+'derived-final-reader-controls-03.json';control=checked(reader,dict(record=cn,sha256=reader.digest(cn)))
    require(control['success']and control['current_acceptance_granted']is False
            and control['complete_ledger_refusals_verified']and control['prior_other_predicates_preserved']
            and all(reader.digest(p)==h for p,h in control['sources'].items()),'Derived final-reader controls stale')
    require(all(v['public_pause_collection_current']['verified']for v in selected),'Actual public pause collection/current proof missing')
    relevance=checked(reader,q['terminal_relevance'])
    result['known_unsupported_native_controls']=[dict(review=q['terminal_relevance'],scope=relevance['explicit_known_unsupported_boundary'],required_row_excluded=False)]
    result['prior_secondary_projection']=dict(rows=198,charges=247,excluded_only=[FAILED,HOVER_FAILED,DEATH_FAILED],
        actual_ledger_rows_renamed=False,all90_actual_positive_contexts_preserved=True,
        qualified_pilot_selection=dict(previous_planned_but_failed=FAILED,current=PILOT,
            receipt=pn,actual_rows_after=157,actual_charges_after=206,previous_failed_case_granted_credit=False))
    result['derived_native_admission_continuation']=dict(verified=True,review=plan['derived_admission_review'],
        failed_zero_Original_setup=q['startup_diagnosis'],negative_custody=q['negative_custody'],
        failed_hover=q['hover_diagnosis'],hover_custody=q['hover_negative_custody'],recent_source_controls=q['recent_source_controls'],
        failed_death=q['death_diagnosis'],death_custody=q['death_negative_custody'],terminal_relevance=q['terminal_relevance'],public_pause_controls=q['public_pause_controls'],
        controls=dict(record=cn,sha256=reader.digest(cn)),qualified_pilot=dict(record=pn,sha256=reader.digest(pn)),
        actual_derived_cases=[dict(case=v['spec']['case']['id'],verification=v['derived_admission'])for v in selected],
        physical_and_actual_compiled_inputs_verified_separately=True,positive_contexts=90,positive_starts=98,
        total_rows=201,total_charges=250,combined_with_WP10=258,reservations_refunded=False,
        historical_failures_retired=False,source_waivers=[])
    return result
