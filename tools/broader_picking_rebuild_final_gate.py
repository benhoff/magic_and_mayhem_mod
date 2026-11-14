"""Preserve all four picking negatives and conjoin the exact software rebuild joins."""
import broader_picking_witness_final_gate as previous
from broader_player_picking_rebuild_matrix import specs
from broader_picking_rebuild_budget import P,FAILED,PILOT,review
from broader_player_acceptance import checked
from broader_runtime_caller_final_gate import caller_binding
from broader_picking_rebuild_join import POLICY
from rendering_pilot import require
same_context=previous.same_context
_clone=previous._clone

def exact_rebuild_ledger(rows,values,old,failed,zero,font,reference,schema,picking,index,witness,rebuild):
    require(len(rows)==192 and sum(v['reserved_games']for v in rows.values())==240,'Complete192/240ledger required')
    require(rebuild['case_id']==FAILED and rebuild['state']=='failed'and rebuild['reserved_games']==1
            and rebuild['acquisition_sha256']and rows.get(FAILED)==rebuild
            and all(v['spec']['case']['id']!=FAILED for v in values),'Failed software rebuild row changed or promoted')
    previous.exact_witness_ledger({k:v for k,v in rows.items()if k!=FAILED},values,old,failed,zero,font,reference,schema,picking,index,witness)

_FADING=_clone(previous._FADING,specs=specs)
_CALLER=_clone(previous._CALLER,original_supplement=_FADING)
_PRIOR=_clone(previous._PRIOR,_CALLER=_CALLER)
_WITNESS=_clone(previous.supplement,_PRIOR=_PRIOR)

def prior_context(reader,actual,previous_id):
    palette=previous.previous.previous.previous
    originals=[s for s in palette.parent.original_specs(reader)if s['case']['id']==previous_id]
    require(len(originals)==1,'Missing unique original finite context')
    if previous_id in actual:return same_context(actual[previous_id],originals[0])
    catalog=reader.json(P+'picking-rebuild-source-catalog-01.json')
    names=['picking-witness-source-catalog-01.json','picking-pose-source-catalog-01.json','picking-target-source-catalog-01.json','palette-schema-source-catalog-01.json',
           'font-reference-source-catalog-01.json','prerequisite-source-catalog-01.json',
           'loader-source-catalog-01.json','capacity-source-catalog-01.json']
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

def software_join(value,reader):
    a=value['acquired'];policy=a['provider_adaptation']['picking_rebuild_source_join']
    require(policy['policy']==POLICY and policy['original_rebuild_bytecode_unchanged']
        and policy['original_deadline_seconds']==30 and policy['physical_actions_unchanged']
        and policy['same_generation_nonce_thread_required']and policy['only_actual_swapped_paints']
        and policy['history_row_limit']==4096 and policy['diagnostic_row_limit']==2048
        and len(policy['observations'])<=2048 and len(policy['joins'])==2
        and len(a['proof']['display_rebuilds'])==2,'Missing bounded software rebuild join policy')
    comparison=checked(reader,value['value']['comparison'])
    require(comparison['proof']['same_generation_rebuild_source_joins']is True,'Software rebuild exact joins not independently checked')
    for join,rebuild in zip(policy['joins'],a['proof']['display_rebuilds']):
        before=rebuild['original_before'];after=rebuild['original_after'];paint=rebuild['confirmed_paint']
        require(join['paint']==paint and join['original_after']==after and join['original_before_ordinal']==before['ordinal']
            and int(paint['world'])==after['frame']['generation']>before['frame']['generation']
            and after['thread']==before['thread']and after['frame']['nonce']==before['frame']['nonce']
            and after['high_resolution']==int(rebuild['after'][0]==800)
            and join['selected_monotonic']<join['deadline'],'Software rebuild join differs')
    return dict(verified=True,policy=POLICY,validation=value['validation'],comparison=value['value']['comparison'])

def supplement(reader,root,rows,values,plan):
    q=review(reader,plan);negative=checked(reader,q['failed_rebuild_custody']);row=negative['request_row_preserved_exactly']
    require(rows.get(FAILED)==row and all(v['spec']['case']['id']!=FAILED for v in values)
        and negative['actual_original_starts']==1 and negative['case_accepted']is False
        and negative['normal_quit_credit']is False,'Failed software rebuild received positive/normal credit')
    projected={k:v for k,v in rows.items()if k!=FAILED}
    result=_WITNESS(reader,root,projected,values,plan)
    require(len(rows)==192 and sum(v['reserved_games']for v in rows.values())==240,'Complete software-join192/240ledger required')
    pn=P+'picking-rebuild-single-pilot-01.json';pilot=checked(reader,dict(record=pn,sha256=reader.digest(pn)))
    actual=next(v for v in values if v['spec']['case']['id']==PILOT)
    require(pilot['qualified_current_contexts']==23 and pilot['conservative_charges_after']==165
        and pilot['acquisition']==actual['acquisition']and pilot['validation']==actual['validation'],'Missing qualified software rebuild pilot')
    join=software_join(actual,reader)
    require(actual['acquired']['configuration']['software_flag']is True,'Software rebuild observer credited to another profile')
    cn=P+'picking-rebuild-final-reader-controls-01.json';controls=checked(reader,dict(record=cn,sha256=reader.digest(cn)))
    require(controls['original_game_runs']==0 and controls['current_acceptance_granted']is False
        and controls['complete_ledger_refusals_verified']and controls['prior_predicate_projection_verified']
        and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Rebuild final controls missing')
    result['prior_witness_projection']=dict(rows=191,charges=239,excluded_only=[FAILED],all90_actual_positive_cases_preserved=True)
    result['prior_palette_schema_projection']['excluded_only'].append(FAILED)
    result['picking_rebuild_source_join_continuation']=dict(verified=True,review=plan['picking_rebuild_review'],
        failed_software_rebuild=q['failed_rebuild_custody'],qualified_pilot=dict(record=pn,sha256=reader.digest(pn)),
        software_source_join=join,retained_default_picking_case=previous.PILOT,final_reader_controls=dict(record=cn,sha256=reader.digest(cn)),
        positive_contexts=90,positive_starts=98,total_rows=192,total_charges=240,combined_with_WP10=248,
        guarded_contexts=14,reservations_refunded=False,historical_failures_retired=False,original_game_runs=0)
    return result
