"""Conjoin source-only object targeting while preserving all five picking negatives."""
import inspect,dis
from types import CodeType
import broader_picking_rebuild_final_gate as previous
import broader_palette_schema_final_gate as palette
from broader_picking_target_final_gate import guarded_contexts
from broader_player_picking_uncovered_matrix import specs
from broader_picking_uncovered_budget import P,FAILED,PILOT,review
from broader_picking_uncovered_target import POLICY
from broader_runtime_caller_final_gate import caller_binding
from broader_player_acceptance import checked
from rendering_pilot import require
same_context=previous.same_context
_clone=previous._clone

def exact_uncovered_ledger(rows,values,old,failed,zero,font,reference,schema,picking,index,witness,rebuild,uncovered):
    require(len(rows)==193 and sum(v['reserved_games']for v in rows.values())==241,'Complete193/241ledger required')
    require(uncovered['case_id']==FAILED and uncovered['state']=='failed'and uncovered['reserved_games']==1
            and uncovered['acquisition_sha256']and rows.get(FAILED)==uncovered
            and all(v['spec']['case']['id']!=FAILED for v in values),'Failed overlapping-body row changed or promoted')
    previous.exact_rebuild_ledger({k:v for k,v in rows.items()if k!=FAILED},values,old,failed,zero,font,reference,schema,picking,index,witness,rebuild)

_FADING=_clone(previous._FADING,specs=specs)
_CALLER=_clone(previous._CALLER,original_supplement=_FADING)
_PRIOR=_clone(previous._PRIOR,_CALLER=_CALLER)
_WITNESS=_clone(previous._WITNESS,_PRIOR=_PRIOR)

def adapted_rebuild_source():
    original=inspect.getsource(previous.supplement);source=original
    changes={"pn=P+'picking-rebuild-single-pilot-01.json'":"pn=P+'picking-uncovered-single-pilot-01.json'",
             "pilot['conservative_charges_after']==165":"pilot['conservative_charges_after']==166"}
    for old,new in changes.items():
        require(source.count(old)==1,'Unexpected prior pilot receipt predicate');source=source.replace(old,new)
    restored=source
    for old,new in changes.items():restored=restored.replace(new,old)
    require(restored==original,'Non-pilot prior predicate changed')
    return source

_namespace=dict(previous.supplement.__globals__,_WITNESS=_WITNESS,PILOT=PILOT)
exec(compile(adapted_rebuild_source(),__file__,'exec'),_namespace)
_REBUILD=_namespace['supplement']
def prior_rebuild_predicate_review():
    changes=dict(pilot_record_literals=0,after_pilot_charge_literals=0)
    def compare(old,new):
        for key in ['co_names','co_varnames','co_freevars','co_cellvars','co_argcount','co_posonlyargcount','co_kwonlyargcount','co_flags']:
            require(getattr(old,key)==getattr(new,key),'Prior function identity changed')
        left=list(dis.get_instructions(old));right=list(dis.get_instructions(new))
        require(len(left)==len(right),'Prior instruction count changed')
        for a,b in zip(left,right):
            if(a.opname,a.arg)==(b.opname,b.arg):continue
            require(a.opname==b.opname=='LOAD_SMALL_INT'and a.arg==165 and b.arg==166,
                    'Non-receipt prior instruction changed')
            changes['after_pilot_charge_literals']+=1
        require(len(old.co_consts)==len(new.co_consts),'Prior constants changed')
        for a,b in zip(old.co_consts,new.co_consts):
            if isinstance(a,CodeType):
                require(isinstance(b,CodeType),'Prior nested code changed');compare(a,b)
            elif a!=b:
                if(a,b)==('picking-rebuild-single-pilot-01.json','picking-uncovered-single-pilot-01.json'):
                    changes['pilot_record_literals']+=1
                else:
                    require(type(a)is type(b)is int and(a,b)==(165,166),'Non-receipt prior constant changed')
                    changes['after_pilot_charge_literals']+=1
    compare(previous.supplement.__code__,_REBUILD.__code__)
    require(changes==dict(pilot_record_literals=1,after_pilot_charge_literals=1),'Missing exact receipt changes')
    return changes

prior_rebuild_predicate_review()

def prior_context(reader,actual,previous_id):
    originals=[s for s in palette.parent.original_specs(reader)if s['case']['id']==previous_id]
    require(len(originals)==1,'Missing unique original finite context')
    if previous_id in actual:return same_context(actual[previous_id],originals[0])
    catalog=reader.json(P+'picking-uncovered-source-catalog-01.json')
    names=['picking-rebuild-source-catalog-01.json','picking-witness-source-catalog-01.json','picking-pose-source-catalog-01.json',
           'picking-target-source-catalog-01.json','palette-schema-source-catalog-01.json','font-reference-source-catalog-01.json',
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
    require(len(matches)==1 and matches[0]['case_id']in actual,'Missing unique current context')
    value=actual[matches[0]['case_id']]
    require(value['spec']['case']['id']==matches[0]['case_id']and value['spec']['family']==matches[0]['family']
            and value['spec']['role']==matches[0]['role'],'Context lineage changed')
    return same_context(value,originals[0])

def uncovered_proof(reader,value):
    a=value['acquired'];policy=a['provider_adaptation']['picking_uncovered_body_target']
    require(a['configuration']['software_flag']is True and policy['policy']==POLICY and policy['source_actor_slot']==0
        and policy['only_captured_creature_opacity_masks']and policy['all_other_body_masks_excluded_without_depth_assumptions']
        and policy['original_hit_feedback_used']is False and policy['Native_or_Original_destination_pixels_used']is False
        and policy['cell_ANI_policy_unchanged']and policy['physical_route_unchanged']
        and policy['original_quiet_deadline_seconds']==policy['original_witness_deadline_seconds']==8
        and 0<=policy['unavailable_source_samples']<=policy['unavailable_sample_limit']==16384
        and policy['source_sprite_cache_limit']==64,'Source-only object target policy differs')
    comparison=checked(reader,value['value']['comparison']);proof=comparison['proof']
    require(proof['source_only_body_mask_exclusion_replayed']and proof['decoded_reference_binding_derivation_verified']
        and proof['same_generation_rebuild_source_joins']and proof['all_witness_paints_within_eight_seconds'],
        'Complete object-source/rebuild/witness replay missing')
    objects=[w for w in a['proof']['picking_windows']if w['kind']=='object']
    require(len(objects)==24 and all(w['target']['target_policy']==POLICY
        and w['target']['source_actor_generation']==w['target']['source_generation']
        and w['target']['hit_feedback_used']is False and w['target']['original_hit_order_assumed']is False for w in objects),
        'Physical object windows lack source-only body mask selection')
    return dict(verified=True,policy=POLICY,validation=value['validation'],comparison=value['value']['comparison'])

def supplement(reader,root,rows,values,plan):
    q=review(reader,plan);negative=checked(reader,q['failed_uncovered_custody']);row=negative['request_row_preserved_exactly']
    require(rows.get(FAILED)==row and all(v['spec']['case']['id']!=FAILED for v in values)
        and negative['actual_original_starts']==1 and negative['case_accepted']is False
        and negative['normal_quit_credit']is False,'Overlapping-body failure received positive/normal credit')
    projected={k:v for k,v in rows.items()if k!=FAILED};result=_REBUILD(reader,root,projected,values,plan)
    require(len(rows)==193 and sum(v['reserved_games']for v in rows.values())==241,'Complete source-only193/241ledger required')
    actual=next(v for v in values if v['spec']['case']['id']==PILOT);proof=uncovered_proof(reader,actual)
    cn=P+'picking-uncovered-final-reader-controls-01.json';controls=checked(reader,dict(record=cn,sha256=reader.digest(cn)))
    require(controls['original_game_runs']==0 and controls['current_acceptance_granted']is False
        and controls['complete_ledger_refusals_verified']and controls['prior_predicate_projection_verified']
        and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Source-only final controls missing')
    result['prior_rebuild_projection']=dict(rows=192,charges=240,excluded_only=[FAILED],all90_actual_positive_cases_preserved=True,
        explicit_pilot_receipt_changes=dict(selected_pilot=PILOT,after_pilot_charges=166,other_predicates_bytecode_unchanged=True))
    result['prior_palette_schema_projection']['excluded_only'].append(FAILED)
    result['picking_uncovered_body_continuation']=dict(verified=True,review=plan['picking_uncovered_review'],
        failed_overlapping_body=q['failed_uncovered_custody'],qualified_pilot=dict(record=P+'picking-uncovered-single-pilot-01.json',sha256=reader.digest(P+'picking-uncovered-single-pilot-01.json')),
        source_only_body_proof=proof,retained_default_picking_case=previous.previous.PILOT,
        final_reader_controls=dict(record=cn,sha256=reader.digest(cn)),positive_contexts=90,positive_starts=98,
        total_rows=193,total_charges=241,combined_with_WP10=249,guarded_contexts=14,
        reservations_refunded=False,historical_failures_retired=False,original_game_runs=0)
    return result
