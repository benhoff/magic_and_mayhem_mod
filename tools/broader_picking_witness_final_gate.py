"""Conjoin exact-record picking while preserving both failed attempts and prior gates."""
import broader_picking_pose_final_gate as previous
from broader_player_picking_witness_matrix import specs
from broader_picking_witness_budget import P,FAILED,PILOT,review
from broader_picking_pose_target import POLICY
from broader_player_acceptance import checked
from rendering_pilot import require
same_context=previous.same_context
from broader_runtime_caller_final_gate import caller_binding
_clone=previous.previous._clone


def exact_witness_ledger(rows,values,old,failed,zero,font,reference,schema,picking,index,witness):
    require(len(rows)==191 and sum(v['reserved_games']for v in rows.values())==239,'Complete191/239ledger required')
    require(witness['case_id']==FAILED and witness['state']=='failed'and witness['reserved_games']==1
            and witness['acquisition_sha256']and rows.get(FAILED)==witness
            and all(v['spec']['case']['id']!=FAILED for v in values),'Failed live-latest witness row changed or promoted')
    previous.exact_pose_ledger({k:v for k,v in rows.items()if k!=FAILED},values,old,failed,zero,font,reference,schema,picking,index)

_FADING=_clone(previous._FADING,specs=specs)
_CALLER=_clone(previous._CALLER,original_supplement=_FADING)
_PRIOR=_clone(previous._PRIOR,_CALLER=_CALLER)


def prior_context(reader,actual,previous_id):
    palette=previous.previous.previous
    originals=[s for s in palette.parent.original_specs(reader)if s['case']['id']==previous_id]
    require(len(originals)==1,'Missing unique original finite context')
    if previous_id in actual:return same_context(actual[previous_id],originals[0])
    catalog=reader.json(P+'picking-witness-source-catalog-01.json')
    names=['picking-pose-source-catalog-01.json','picking-target-source-catalog-01.json','palette-schema-source-catalog-01.json',
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


def supplement(reader,root,rows,values,plan):
    q=review(reader,plan);witness=checked(reader,q['failed_witness_custody']);witness_row=witness['request_row_preserved_exactly']
    pose_review=checked(reader,plan['picking_pose_review']);index=checked(reader,pose_review['failed_index_custody']);index_row=index['request_row_preserved_exactly']
    old_review=checked(reader,plan['picking_target_review']);picking=checked(reader,old_review['failed_picking_custody']);picking_row=picking['request_row_preserved_exactly']
    negatives=[witness,index,picking];negative_rows=[v['request_row_preserved_exactly']for v in negatives]
    require(all(rows.get(v['case_id'])==v and all(x['spec']['case']['id']!=v['case_id']for x in values)for v in negative_rows)
            and all(v['actual_original_starts']==1 and v['case_accepted']is False and v['normal_quit_credit']is False for v in negatives),
            'Failed picking games changed or received positive/normal credit')
    projected={k:v for k,v in rows.items()if k not in {r['case_id']for r in negative_rows}}
    result=_PRIOR(reader,root,projected,values,plan)
    require(len(rows)==191 and sum(v['reserved_games']for v in rows.values())==239,'Complete actual-swap191/239ledger required')
    pilot_name=P+'picking-witness-single-pilot-01.json';pilot=checked(reader,dict(record=pilot_name,sha256=reader.digest(pilot_name)))
    actual=next(v for v in values if v['spec']['case']['id']==PILOT)
    require(pilot['qualified_current_contexts']==22 and pilot['conservative_charges_after']==163
            and pilot['acquisition']==actual['acquisition']and pilot['validation']==actual['validation'],'Missing qualified actual-swap picking pilot')
    picks=[v for v in values if v['spec']['family']=='picking']
    require(len(picks)==2 and {v['acquired']['configuration']['software_flag']for v in picks}=={False,True},
            'Both exact-ANI picking profiles required')
    proofs=[]
    for value in picks:
        a=value['acquired'];binding=a['provider_adaptation']['picking_pose_target']
        require(binding['policy']==POLICY and binding['actor_wire_version']==2
                and binding['launches']==[dict(flag='MNM_SCENE_ACTORS',value='1',existing_compiled_observer=True)]
                and binding['same_generation_nonce_thread_required']and binding['exact_record_and_body_CRC_required']
                and binding['Native_runtime_sources_changed']is False and binding['original_memory_writes']is False
                and binding['hit_feedback_search']is False and binding['quiet_source_samples']==binding['exact_displayed_witnesses']==3
                and binding['readiness_seconds']==8 and binding['normal_retirement_required'],
                'Exact ANI source or unchanged witness contract missing')
        comparison=checked(reader,value['value']['comparison']);pose=comparison['picking_pose_observation']
        require(pose['verified']and pose['complete']and pose['policy']==POLICY
                and pose['allocation_generations_proved']is False,'Passive actor source lost or promoted')
        windows=a['proof']['picking_windows'];cells=[w for w in windows if w['kind']=='cell']
        require(len(windows)==48 and len(cells)==24 and all(w['target']['target_policy']==POLICY
                and w['target']['footprint']==1 and w['target']['actor_source']['generation']==w['target']['source_generation']
                and len(w['target']['animation_record_sha256'])==64 for w in cells),
                'Actual cell windows not bound to exact same-generation ANI records')
        collection=a['provider_adaptation']['picking_presented_witness_collection']
        require(collection['policy']=='all-presented-exact-joins-v1'and collection['viewport_instances']==1
                and collection['original_ready_predicate_unchanged']and collection['physical_actions_unchanged']
                and collection['controller_eight_second_loop_unchanged']and collection['only_actual_swapped_paints']
                and collection['paint_event_deadline_us']==8000000 and collection['history_row_limit']==4096
                and collection['diagnostic_row_limit']==16384 and len(collection['observations'])<=16384
                and comparison['proof']['all_witness_paints_within_eight_seconds'],'Missing unchanged bounded actual-swap witness policy')
        collected=collection['windows'];require([v['name']for v in collected]==[v['name']for v in windows],'Missing actual collection windows')
        for w,c in zip(windows,collected):
            require(c['released_us']==int(w['released']['host_us'])and c['deadline_us']==c['released_us']+8000000
                    and {v['generation']for v in w['witnesses']}<=set(c['witness_generations']),
                    'Recorded witness was not returned by its bounded actual-swap collection window')
        proofs.append(dict(case=value['spec']['case']['id'],validation=value['validation'],source=pose))
    cn=P+'picking-witness-final-reader-controls-01.json';controls=checked(reader,dict(record=cn,sha256=reader.digest(cn)))
    require(controls['original_game_runs']==0 and controls['current_acceptance_granted']is False
            and controls['complete_ledger_refusals_verified']and controls['prior_predicate_projection_verified']
            and all(reader.digest(n)==h for n,h in controls['sources'].items()),'Exact-ANI final controls missing')
    result['prior_palette_schema_projection']=dict(rows=188,charges=236,
        excluded_only=[v['case_id']for v in negative_rows],all90_actual_positive_cases_preserved=True)
    result['picking_presented_witness_continuation']=dict(verified=True,review=plan['picking_witness_review'],
        failed_foot_target=old_review['failed_picking_custody'],failed_inverse_index=pose_review['failed_index_custody'],
        failed_live_latest_collection=q['failed_witness_custody'],qualified_pilot=dict(record=pilot_name,sha256=reader.digest(pilot_name)),
        actual_pose_source_proofs=proofs,final_reader_controls=dict(record=cn,sha256=reader.digest(cn)),original_game_runs=0,
        positive_contexts=90,positive_starts=98,total_rows=191,total_charges=239,combined_with_WP10=247,
        guarded_contexts=14,reservations_refunded=False,historical_failures_retired=False,
        universal_target_or_original_ray_recovery_proved=False)
    return result
