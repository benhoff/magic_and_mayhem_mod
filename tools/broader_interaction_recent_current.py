"""Rejoin refreshed target rows and physical actions from the stopped acquisition."""
from pathlib import Path
from broader_interaction_recent_sources import POLICY
from rendering_interaction import decode
from rendering_pilot import require

def verify_recent(reader,phase,acquisition):
    require(phase['family']=='interaction'and phase['recent_interaction_sources']['policy']==POLICY,'Wrong recent source-target phase')
    require(acquisition['success']and acquisition['sources_stable']and acquisition['actual_provider_verified'],'Incomplete current target acquisition')
    binding=acquisition['provider_adaptation']['recent_interaction_sources']
    require(binding['policy']==POLICY and binding['maximum_age_frames']==2 and binding['selected_opaque_pixel_formula_unchanged']
        and binding['prefix_dependencies_decoded_and_hashed']and binding['materialized_only_requested_World']
        and binding['action_schedule_changed']is False and binding['old_15second_readiness_deadline_preserved']
        and binding['hit_feedback_search']is False and binding['Native_runtime_sources_changed']is False,'Changed recent target execution policy')
    artifacts=[v for v in acquisition['artifacts']if Path(v['path']).name=='frame-input.bin'];require(len(artifacts)==1,'Missing unique target frame stream')
    rows=decode(reader.read_bytes(artifacts[0]['record']),allow_pending=False)['rows'];facts=[];actions=acquisition['actions']
    for index,target in enumerate(actions):
        if target['action']not in('source-sprite-target','source-sprite-focus'):continue
        check=target['recent_source_check'];require(check['policy']==POLICY and check['source_row']==target['frame_input_row']
            and check['hit_feedback_used']is False and check['unchanged_fields']==['history','position','camera','mode'],'Missing source-only refreshed target')
        require(type(check['source_row'])is int and type(check['checked_row'])is int and 1<=check['source_row']<=check['checked_row']<=len(rows),'Invalid source target rows')
        before=rows[check['source_row']-1];after=rows[check['checked_row']-1]
        require(before['kind']==after['kind']==1 and before['frame']['generation']==target['source_generation']
            and before['frame']['nonce']==after['frame']['nonce']and before['thread']==after['thread']
            and after['frame']['generation']==check['checked_generation']and before['history'][:2]==target['anchor'],'Source target identity changed')
        age=after['frame']['generation']-before['frame']['generation']
        require(type(check['age_frames'])is int and age==check['age_frames']and 0<=age<=2
            and all(before[k]==after[k]for k in check['unchanged_fields']),'Stale/moved source target')
        following=[]
        for action in actions[index+1:]:
            if action['action']in('source-sprite-target','source-sprite-focus'):break
            if action['action']=='pointer':following.append(action);break
        require(len(following)==1 and following[0]['logical']==target['point'],'Missing exact physical target action')
        lag=int(following[0]['world'])-target['source_generation']
        require(-8<=lag<=6,'Source target stale before actual physical pointer')
        facts.append(dict(source_row=before['ordinal'],checked_row=after['ordinal'],generation=target['source_generation'],age_frames=age,pointer_World_lag=lag))
    require(len(facts)==12,'Complete twelve target/focus schedule required')
    return dict(verified=True,policy=POLICY,targets=facts,actual_original_rows_rejoined=True,source_formula_independently_checked_by_original_assessment=True)
