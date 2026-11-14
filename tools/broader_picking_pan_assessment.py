"""Retain the original captured-source pan assertion and check all bounded live acks."""
from broader_picking_uncovered_assessment import with_actor_frames as uncovered_assessment
from broader_picking_pan_ack import POLICY,PAN_ACTIONS,acknowledged
from rendering_interaction import decode
from rendering_menu_acceptance import artifact
from rendering_pilot import require

def pan_receipts(reader,acquired):
    policy=acquired['provider_adaptation']['picking_source_pan_ack']
    require(policy['policy']==POLICY and policy['physical_key_identities_and_order_unchanged']
        and policy['hold_release_policy_changed']is True and policy['minimum_hold_seconds']==.22
        and policy['maximum_hold_plus_settle_seconds']==1.02 and policy['settlement_uses_remaining_existing_budget']
        and policy['source_generation_nonce_thread_required']and policy['changed_camera_same_view_required']
        and policy['original_five_World_thirty_second_progress_wait_unchanged']
        and policy['original_pan_analysis_predicate_unchanged']and policy['journey_instances']==1,
        'Physical pan acknowledgement policy differs')
    attempts=policy['attempts'];expected=[p+'-'+m+'-pan'for p in ('high-initial','low','high-returned')for m in ('sharp','integer')]
    require([v['action']for v in attempts]==expected,'All six acknowledged pan actions required')
    rows=decode(artifact(reader,acquired,'frame-input.bin'))['rows']
    for attempt in attempts:
        before=attempt['before'];after=attempt['acknowledged_after'];pressed=attempt['pressed_monotonic'];ack=attempt['acknowledged_monotonic'];released=attempt['released_monotonic'];deadline=attempt['deadline_monotonic']
        require(after is not None and 0<before['ordinal']<=len(rows)and 0<after['ordinal']<=len(rows)and before==rows[before['ordinal']-1]and after==rows[after['ordinal']-1]
            and attempt['physical_release_accepted']is True and acknowledged(before,after),'Pan acknowledgement lacks exact Original source rows')
        require(attempt['minimum_hold_seconds']==.22 and attempt['hold_plus_settle_budget_seconds']==1.02
            and abs(deadline-pressed-1.02)<1e-7 and pressed+.22<=ack<=released<=deadline,
            'Pan exceeded original hold-plus-settle budget')
        markers={v['phase']:v for v in acquired['camera_actions']if v['action']==attempt['action']}
        require(set(markers)=={'begin','released','settled'}and int(markers['settled']['world'])>=int(markers['released']['world'])+5,
            'Original post-release five-World progress missing')
    return True

def with_actor_frames(frames):
    original=uncovered_assessment(frames)
    def assess(reader,acquired,compared,observed,**kwargs):
        proof=original(reader,acquired,compared,observed,**kwargs)
        proof['source_pan_acknowledgements_verified']=pan_receipts(reader,acquired)
        proof['original_captured_source_pan_predicate_unchanged']=True
        return proof
    return assess
