"""Preserve prior final predicates and separately account for the picking failure."""
import inspect
import types
from collections import Counter
import broader_palette_schema_final_gate as previous
from broader_player_acceptance import checked
from broader_picking_target_budget import P,FAILED as PICKING_FAILED,PILOT,review
from broader_player_picking_repair_matrix import specs
from rendering_pilot import require
same_context=previous.same_context


def exact_picking_target_ledger(rows,values,old,failed,zero,font,reference,schema,picking):
    require(len(rows)==189 and sum(v['reserved_games']for v in rows.values())==237,
            'Complete189/237 ledger required')
    require(picking['case_id']==PICKING_FAILED and picking['state']=='failed'
            and picking['reserved_games']==1 and picking['acquisition_sha256']
            and rows.get(PICKING_FAILED)==picking
            and all(v['spec']['case']['id']!=PICKING_FAILED for v in values),
            'Exact failed picking row changed or received positive credit')
    projected={k:v for k,v in rows.items()if k!=PICKING_FAILED}
    previous.exact_palette_schema_ledger(projected,values,old,failed,zero,font,reference,schema)


def guarded_contexts(declared,values,repaired):
    families={'attachment','font','palette','picking'}
    expected={s['case']['id']for s in declared if s['family']in families}
    require(Counter(s['family']for s in declared if s['family']in families)
            ==dict(attachment=4,font=4,palette=4,picking=2),
            'Changed finite canonical-controller context denominator')
    actual=[v['case']for v in repaired]
    require(len(actual)==len(set(actual))==14 and set(actual)==expected
            and {v['spec']['case']['id']for v in values if v['spec']['family']in families}==expected,
            'Missing,duplicate or foreign canonical-controller proof')
    return True


def _clone(function,**bindings):
    namespace=dict(function.__globals__);namespace.update(bindings)
    return types.FunctionType(function.__code__,namespace,function.__name__,function.__defaults__,function.__closure__)


def _fading_projection():
    # The immutable prior reader asks for16 guarded contexts, while its declared
    #90-case catalog contains4attachment+4font+4palette+2picking=14. Recheck the
    # exact declared set, not a guessed count or two invented execution cases.
    original=inspect.getsource(previous.parent.fading_supplement)
    old="require(len(repaired)==16,'All16guarded contexts required')"
    new='guarded_contexts(specs(reader),values,repaired)'
    require(original.count(old)==1,'Unexpected inherited canonical-context assertion')
    source=original.replace(old,new)
    old_totals='total_rows=183,total_charges=231,combined_with_WP10=239'
    prior_totals='total_rows=188,total_charges=236,combined_with_WP10=244'
    require(source.count(old_totals)==1,'Unexpected inherited ledger metadata')
    source=source.replace(old_totals,prior_totals)
    require(source.replace(new,old).replace(prior_totals,old_totals)==original,
            'Unexpected inherited final predicate change')
    namespace=dict(previous.parent.fading_supplement.__globals__)
    def exact_ledger(rows,values,q):
        previous.exact_palette_schema_ledger(rows,values,q['old_requests'],rows[previous.FAILED],
            rows['bp30-world-caller-default800-01'],rows['bp31-world-font-default800-01'],
            rows[previous.REFERENCE_FAILED],rows[previous.SCHEMA_FAILED])
    namespace.update(exact_ledger=exact_ledger,guarded_contexts=guarded_contexts,specs=specs)
    exec(compile(source,__file__,'exec'),namespace)
    return namespace['supplement']

_FADING=_fading_projection()
_CALLER=_clone(previous._CALLER,original_supplement=_FADING)
_PRIOR=_clone(previous.supplement,_CALLER=_CALLER)


def prior_context(reader,actual,previous_id):
    originals=[s for s in previous.parent.original_specs(reader)if s['case']['id']==previous_id]
    require(len(originals)==1,'Missing unique original finite context')
    if previous_id in actual:return previous.same_context(actual[previous_id],originals[0])
    catalog=reader.json(P+'picking-target-source-catalog-01.json')
    names=['palette-schema-source-catalog-01.json','font-reference-source-catalog-01.json',
           'prerequisite-source-catalog-01.json','loader-source-catalog-01.json','capacity-source-catalog-01.json']
    parents={b['case_id']:b for n in names for b in reader.json(P+n)['case_bindings']}
    def original_id(binding):
        current=binding.get('previous_case_id',binding['case_id']);seen=set()
        while current in parents:
            require(current not in seen,'Cyclic finite context lineage');seen.add(current)
            next_id=parents[current].get('previous_case_id',current)
            if next_id==current:break
            current=next_id
        return current
    matches=[b for b in catalog['case_bindings']if original_id(b)==previous_id]
    require(len(matches)==1 and matches[0]['case_id']in actual,'Missing unique current finite context')
    value=actual[matches[0]['case_id']]
    require(value['spec']['case']['id']==matches[0]['case_id']and value['spec']['family']==matches[0]['family']
            and value['spec']['role']==matches[0]['role'],'Finite context lineage changed')
    return previous.same_context(value,originals[0])


def supplement(reader,root,rows,values,plan):
    q=review(reader,plan);negative=checked(reader,q['failed_picking_custody']);picking=negative['request_row_preserved_exactly']
    require(rows.get(PICKING_FAILED)==picking and negative['actual_original_starts']==1
            and negative['case_accepted']is False and negative['normal_quit_credit']is False,
            'Failed picking game lost or promoted')
    # Run every inherited188/236 predicate over the exact prior projection.
    # All90 actual positive cases remain present. The new failed row is checked
    # independently before and after that projection.
    projected={k:v for k,v in rows.items()if k!=PICKING_FAILED}
    result=_PRIOR(reader,root,projected,values,plan)
    require(len(rows)==189 and sum(v['reserved_games']for v in rows.values())==237,
            'Complete picking continuation ledger required')
    pilot_name=P+'picking-target-single-pilot-01.json';pilot=checked(reader,dict(record=pilot_name,sha256=reader.digest(pilot_name)))
    actual=next(v for v in values if v['spec']['case']['id']==PILOT)
    require(pilot['qualified_current_contexts']==22 and pilot['conservative_charges_after']==161
            and pilot['acquisition']==actual['acquisition']and pilot['validation']==actual['validation'],
            'Missing qualified picking replacement pilot')
    picks=[v for v in values if v['spec']['family']=='picking']
    require(len(picks)==2 and {v['acquired']['configuration']['software_flag']for v in picks}=={False,True},
            'Both finite picking profiles required')
    for value in picks:
        acquired=value['acquired'];binding=acquired['provider_adaptation']['picking_animation_target']
        require(binding['policy']=='inverse-unique-ANI-body-placement-v1'
                and binding['review']==q['animation_placement_review']and binding['hit_feedback_search']is False
                and binding['quiet_source_samples']==binding['exact_displayed_witnesses']==3
                and binding['readiness_seconds']==8 and binding['normal_retirement_required'],
                'Picking target policy or witness contract changed')
        windows=acquired['proof']['picking_windows']
        require(len(windows)==48 and all(w['target']['target_policy']==binding['policy']
                and w['target']['footprint']==1 for w in windows if w['kind']=='cell'),
                'Actual picking cell target not bound to reviewed source policy')
    controls_name=P+'picking-target-final-reader-controls-01.json'
    controls=checked(reader,dict(record=controls_name,sha256=reader.digest(controls_name)))
    require(controls['tests']==214 and controls['fresh_tests']==100 and controls['retained_tests']==114
            and controls['original_game_runs']==0 and controls['current_acceptance_granted']is False
            and controls['complete_ledger_refusals_verified']and controls['guarded_context_set_verified']
            and all(reader.digest(n)==h for n,h in controls['sources'].items()),
            'Current picking final-reader controls missing')
    result['prior_palette_schema_projection']=dict(rows=188,charges=236,excluded_only=PICKING_FAILED,
        all_90_actual_positive_cases_preserved=True,scope='Prior predicates only;full current ledger is189/237 below.')
    result['picking_target_continuation']=dict(verified=True,review=plan['picking_target_review'],
        failed_case=q['failed_picking_custody'],qualified_pilot=dict(record=pilot_name,sha256=reader.digest(pilot_name)),
        final_reader_controls=dict(record=controls_name,sha256=reader.digest(controls_name)),
        original_game_runs=0,positive_contexts=90,positive_starts=98,total_rows=189,total_charges=237,
        combined_with_WP10=245,guarded_contexts=14,reservations_refunded=False,historical_failure_retired=False,
        universal_target_or_original_ray_recovery_proved=False)
    return result
