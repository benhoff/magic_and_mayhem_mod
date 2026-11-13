"""Preserve every prior predicate while requiring six bounded source-acknowledged pans."""
import inspect,dis
from types import CodeType
import broader_picking_uncovered_final_gate as previous
from broader_player_picking_pan_matrix import specs
from broader_picking_pan_budget import P,FAILED,PILOT,review
from broader_picking_pan_ack import POLICY
from broader_runtime_caller_final_gate import caller_binding
from broader_player_acceptance import checked
from rendering_pilot import require
same_context=previous.same_context
guarded_contexts=previous.guarded_contexts
_clone=previous._clone
_FADING=_clone(previous._FADING,specs=specs)
_CALLER=_clone(previous._CALLER,original_supplement=_FADING)
_PRIOR=_clone(previous._PRIOR,_CALLER=_CALLER)
_WITNESS=_clone(previous._WITNESS,_PRIOR=_PRIOR)

def exact_pan_ledger(rows,values,old,failed,zero,font,reference,schema,picking,index,witness,rebuild,uncovered,pan):
    require(len(rows)==194 and sum(v['reserved_games']for v in rows.values())==242,'Complete194/242ledger required')
    require(pan['case_id']==FAILED and pan['state']=='acquired'and pan['reserved_games']==1
        and pan['acquisition_sha256']and rows.get(FAILED)==pan
        and all(v['spec']['case']['id']!=FAILED for v in values),'Nominal pan-negative row changed or promoted')
    previous.exact_uncovered_ledger({k:v for k,v in rows.items()if k!=FAILED},values,old,failed,zero,font,reference,schema,picking,index,witness,rebuild,uncovered)

def replace_exact(source,changes):
    original=source
    for old,new,count in changes:
        require(source.count(old)==count,'Unexpected prior successful-pilot receipt source');source=source.replace(old,new)
    restored=source
    for old,new,_ in reversed(changes):restored=restored.replace(new,old)
    require(restored==original,'Non-pilot prior source predicate changed')
    return source

def rebuilt_source():
    return replace_exact(previous.adapted_rebuild_source(),[
        ("pn=P+'picking-uncovered-single-pilot-01.json'","pn=P+'picking-pan-single-pilot-01.json'",1),
        ("pilot['conservative_charges_after']==166","pilot['conservative_charges_after']==167",1)])

def uncovered_source():
    return replace_exact(inspect.getsource(previous.supplement),[
        ('picking-uncovered-single-pilot-01.json','picking-pan-single-pilot-01.json',2),
        ('after_pilot_charges=166','after_pilot_charges=167',1)])

_rebuild_namespace=dict(previous._REBUILD.__globals__,_WITNESS=_WITNESS,PILOT=PILOT)
exec(compile(rebuilt_source(),__file__,'exec'),_rebuild_namespace)
_REBUILD=_rebuild_namespace['supplement']
_uncovered_namespace=dict(previous.supplement.__globals__,_REBUILD=_REBUILD,PILOT=PILOT)
exec(compile(uncovered_source(),__file__,'exec'),_uncovered_namespace)
_UNCOVERED=_uncovered_namespace['supplement']

def receipt_literal_review(old,new):
    changes=dict(pilot_record_literals=0,after_pilot_charge_literals=0)
    def compare(a,b):
        for key in ['co_names','co_varnames','co_freevars','co_cellvars','co_argcount','co_posonlyargcount','co_kwonlyargcount','co_flags']:
            require(getattr(a,key)==getattr(b,key),'Prior function identity changed')
        left=list(dis.get_instructions(a));right=list(dis.get_instructions(b));require(len(left)==len(right),'Prior instruction count changed')
        for x,y in zip(left,right):
            if(x.opname,x.arg)==(y.opname,y.arg):continue
            require(x.opname==y.opname=='LOAD_SMALL_INT'and(x.arg,y.arg)==(166,167),'Non-receipt prior instruction changed')
            changes['after_pilot_charge_literals']+=1
        require(len(a.co_consts)==len(b.co_consts),'Prior constants changed')
        for x,y in zip(a.co_consts,b.co_consts):
            if isinstance(x,CodeType):require(isinstance(y,CodeType),'Prior nested code changed');compare(x,y)
            elif x!=y:
                if(x,y)==('picking-uncovered-single-pilot-01.json','picking-pan-single-pilot-01.json'):changes['pilot_record_literals']+=1
                else:
                    require(type(x)is type(y)is int and(x,y)==(166,167),'Non-receipt prior constant changed')
                    changes['after_pilot_charge_literals']+=1
    compare(old.__code__,new.__code__)
    require(changes==dict(pilot_record_literals=1,after_pilot_charge_literals=1),'Missing exact successful-pilot receipt changes')
    return changes

receipt_literal_review(previous._REBUILD,_REBUILD)
receipt_literal_review(previous.supplement,_UNCOVERED)
_lineage_namespace=dict(previous.prior_context.__globals__)
_lineage_source=inspect.getsource(previous.prior_context).replace('picking-uncovered-source-catalog-01.json','picking-pan-source-catalog-01.json').replace("names=['picking-rebuild-source-catalog-01.json'","names=['picking-uncovered-source-catalog-01.json','picking-rebuild-source-catalog-01.json'")
exec(compile(_lineage_source,__file__,'exec'),_lineage_namespace)
prior_context=_lineage_namespace['prior_context']

def pan_proof(reader,value):
    a=value['acquired'];policy=a['provider_adaptation']['picking_source_pan_ack'];comparison=checked(reader,value['value']['comparison'])
    require(policy['policy']==POLICY and policy['physical_key_identities_and_order_unchanged']
        and policy['hold_release_policy_changed']is True and policy['minimum_hold_seconds']==.22
        and policy['maximum_hold_plus_settle_seconds']==1.02 and policy['settlement_uses_remaining_existing_budget']
        and policy['source_generation_nonce_thread_required']and policy['changed_camera_same_view_required']
        and policy['original_five_World_thirty_second_progress_wait_unchanged']and policy['original_pan_analysis_predicate_unchanged']
        and policy['journey_instances']==1 and len(policy['attempts'])==6
        and comparison['proof']['source_pan_acknowledgements_verified']
        and comparison['proof']['original_captured_source_pan_predicate_unchanged'],'Missing bounded acknowledged physical pan proof')
    return dict(verified=True,policy=POLICY,validation=value['validation'],comparison=value['value']['comparison'],pan_actions=6)

def supplement(reader,root,rows,values,plan):
    q=review(reader,plan);negative=checked(reader,q['failed_pan_custody']);row=negative['request_row_preserved_exactly']
    require(rows.get(FAILED)==row and row['state']=='acquired'and all(v['spec']['case']['id']!=FAILED for v in values)
        and negative['actual_acquisition_success']and negative['actual_normal_Quit_preserved']
        and negative['comparison_success']is False and negative['case_accepted']is False
        and negative['normal_quit_credit']is False,'Nominal pan failure changed or received whole-case credit')
    result=_UNCOVERED(reader,root,{k:v for k,v in rows.items()if k!=FAILED},values,plan)
    require(len(rows)==194 and sum(v['reserved_games']for v in rows.values())==242,'Complete pan194/242ledger required')
    actual=next(v for v in values if v['spec']['case']['id']==PILOT);proof=pan_proof(reader,actual)
    cn=P+'picking-pan-final-reader-controls-01.json';controls=checked(reader,dict(record=cn,sha256=reader.digest(cn)))
    require(controls['original_game_runs']==0 and controls['current_acceptance_granted']is False
        and controls['complete_ledger_refusals_verified']and controls['prior_predicate_projection_verified']
        and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Pan final controls missing')
    result['prior_uncovered_projection']=dict(rows=193,charges=241,excluded_only=[FAILED],all90_actual_positive_cases_preserved=True,
        explicit_successful_pilot_receipt_changes=dict(selected_pilot=PILOT,after_pilot_charges=167,other_predicates_unchanged=True))
    result['prior_palette_schema_projection']['excluded_only'].append(FAILED)
    result['picking_acknowledged_pan_continuation']=dict(verified=True,review=plan['picking_pan_review'],failed_nominal_pan=q['failed_pan_custody'],
        qualified_pilot=dict(record=P+'picking-pan-single-pilot-01.json',sha256=reader.digest(P+'picking-pan-single-pilot-01.json')),
        source_pan_proof=proof,retained_default_picking_case=previous.previous.previous.PILOT,
        final_reader_controls=dict(record=cn,sha256=reader.digest(cn)),positive_contexts=90,positive_starts=98,total_rows=194,
        total_charges=242,combined_with_WP10=250,guarded_contexts=14,reservations_refunded=False,historical_failures_retired=False,original_game_runs=0)
    return result
