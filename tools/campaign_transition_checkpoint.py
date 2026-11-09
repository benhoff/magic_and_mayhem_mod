"""Original victory-ready Save/Load input; never a map-transition outcome."""
import hashlib,math
from campaign_checkpoint import decode,compare,FIELDS
from campaign_gameplay import creatures
from rendering_progression import require


def retired_goal(value):
    require(isinstance(value,dict) and all(type(value.get(k)) is int for k in ('type','owner','active')) and value.get('type')==24 and value.get('owner')==1
        and value.get('active')==0 and type(value.get('health')) is int and value['health']<=0
        and type(value.get('slot')) is int and 0<=value['slot']<256
        and all(type(value.get(k)) is int and 0<=value[k]<bound for k,bound in (('x',80),('y',80),('z',24))),
        'Transition input requires an observed retired enemy Camulos pose')
    return dict(value)


def transition_state(saved,loaded,goal):
    goal=retired_goal(goal);storage=compare(saved,loaded)
    require(storage['verified'],'Original transition Save/Load actor fields differ')
    heroes=[a for a in saved['actors'] if a['active'] and a['type']==0 and a['owner']==0 and 0<a['health']<0x80000000]
    require(len(heroes)==1 and not any(a['active'] and a['type']==24 and a['owner']==1 for a in saved['actors']),
        'Transition checkpoint requires a living Wizard and no living Camulos')
    require(all(type(heroes[0][k]) is int and 0<=heroes[0][k]<bound for k,bound in (('x',80),('y',80),('z',24))),'Invalid saved Wizard coordinates')
    require(math.hypot(heroes[0]['x']-goal['x'],heroes[0]['y']-goal['y'])<=12,
        'Transition checkpoint is not near the observed retired Boss')
    return heroes[0],storage,goal


def validate_transition_capture(flow,route,data,save_bytes,gameplay_rows,seed_snapshot):
    from campaign_storage import validate_storage_flow,validate_mini_dispatch
    validate_storage_flow(flow);validate_mini_dispatch(flow)
    require(route.get('success') is route.get('checkpoint_ready') is True
        and route.get('simulation_writes') is route.get('semantic_outcome_accepted') is False
        and route.get('checkpoint_domain')=='transition-ready','Invalid transition checkpoint stop')
    seed=route.get('seed_reference',{});ready=[s for s in flow['states'] if s.get('event')=='checkpoint-seed-loaded']
    require(len(ready)==1 and ready[0].get('sha256')==seed.get('save_sha256')
        and ready[0].get('name')==seed.get('save_name'),'Transition seed Load identity differs')
    groups=decode(data);require([g['screen'] for g in groups]==[12,13,12],'Require seed Load, new original Save and new Load')
    require(compare(seed_snapshot,groups[0])['verified'],'Initial original seed actor fields differ')
    goal=retired_goal(route.get('retired_boss'));sequence=goal.get('sequence')
    require(type(sequence) is int and sequence>0,'Missing original retirement observation sequence')
    rows=[r for r in gameplay_rows if r[0]==sequence];observed=creatures(rows).get(goal['slot'])
    require(observed is not None and all(observed[k]==goal[k] for k in ('sequence',*FIELDS)),
        'Portal proposal does not bind the exact original retired actor observation')
    hero,storage,goal=transition_state(groups[1],groups[2],goal)
    require(save_bytes and hashlib.sha256(save_bytes).hexdigest()==flow.get('saved_sha256'),
        'Transition save bytes differ from the original writer')
    return dict(verified=True,domain='transition-ready',save_sha256=flow['saved_sha256'],save_bytes=len(save_bytes),
        wizard=hero,storage=storage,saved_snapshot=groups[1],portal_goal=goal,diagnostic_only=True,
        fresh_campaign_progression_accepted=False,scope='Original synchronous Save/Load active actor fields and an exact earlier original retired-Boss ground proposal. No whole-state/RNG/AI equivalence, movement delivery, natural victory or map progression is inferred.')
