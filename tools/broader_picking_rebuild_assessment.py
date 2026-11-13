"""Require both exact source/paint joins in addition to the full picking proof."""
from broader_picking_witness_assessment import with_actor_frames as witness_assessment
from broader_picking_rebuild_join import POLICY,select,TRACE_LIMIT,INPUT_LIMIT
from rendering_pilot import require

def rebuild_joins(acquired,observed):
    policy=acquired['provider_adaptation']['picking_rebuild_source_join']
    require(policy['policy']==POLICY and policy['original_rebuild_bytecode_unchanged']is True
            and policy['original_deadline_seconds']==30 and policy['physical_actions_unchanged']is True
            and policy['same_generation_nonce_thread_required']is True and policy['only_actual_swapped_paints']is True
            and policy['history_row_limit']==4096 and policy['diagnostic_row_limit']==TRACE_LIMIT,
            'Rebuild observer policy differs')
    joins=policy['joins'];rebuilds=acquired['proof']['display_rebuilds']
    require(len(joins)==len(rebuilds)==2 and len(policy['observations'])<=TRACE_LIMIT,'Both rebuild joins required')
    paints={int(observed['paints'][i][0]['paint']):observed['paints'][i][0]for i in observed['swaps']}
    for join,rebuild in zip(joins,rebuilds):
        before=rebuild['original_before'];after=rebuild['original_after'];paint=rebuild['confirmed_paint']
        require(join['paint']==paint==paints.get(int(paint['paint'])) and join['original_after']==after
            and join['original_before_ordinal']==before['ordinal']
            and join['low']==(rebuild['after'][0]==640),'Rebuild joined observations differ')
        require(select([paint],[after],before,rebuild['after'],join['old_world'],join['low'],
                       join['selected_monotonic'],join['deadline'])is not None,'Invalid rebuild source/paint join')
    return True

def with_actor_frames(frames):
    original=witness_assessment(frames)
    def assess(reader,acquired,compared,observed,**kwargs):
        proof=original(reader,acquired,compared,observed,**kwargs)
        proof['same_generation_rebuild_source_joins']=rebuild_joins(acquired,observed)
        return proof
    return assess
