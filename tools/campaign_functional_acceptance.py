"""Strict native functional lifecycle admission; FPS is diagnostic only."""
import math
import json
from pathlib import Path


def load_policy():
    path=Path(__file__).resolve().parents[1]/"tests/fixtures/native-functional-lifecycle.json"
    policy=json.loads(path.read_text())
    if (policy.get("schema")!=1 or policy.get("kind")!="native-functional-lifecycle-v1" or policy.get("fps_policy")!="diagnostic"
            or type(policy.get("minimum_worlds")) is not int or policy["minimum_worlds"]<64
            or type(policy.get("minimum_stress_seconds")) is not int or not 10<=policy["minimum_stress_seconds"]<=120):
        raise ValueError("Invalid functional lifecycle policy")
    return policy


STEPS = ['main','region','campaign-handoff','gameplay','campaign-menu','gameplay-resumed',
         'campaign-quit-menu','quit-no-resumed','campaign-quit-menu','campaign-defeat','quit-main']
ACTIONS = [[3,0],[18,65537],[18,0],[17,4],[17,3],[17,3],[6,21],[3,4]]


def validate_strict_functional(value):
    policy=load_policy()
    for key in ('success','sources_stable','original_game_executed','original_verified_after'):
        if value.get(key) is not True:raise ValueError('Missing functional provenance: '+key)
    if value.get('ordinary_campaign_accepted') is not False or value.get('fps_policy')!='diagnostic':
        raise ValueError('Functional scope must keep benchmark acceptance separate')
    seconds=value.get('stress_seconds')
    if type(seconds) is not int or not policy['minimum_stress_seconds']<=seconds<=120:raise ValueError('Missing bounded active stress')
    flow=value['flow']
    if flow.get('success') is not True or flow.get('normal_quit') is not True or flow.get('native_command_fallback') is not False:
        raise ValueError('Incomplete normal native lifecycle')
    if [s['step'] for s in flow['steps']]!=STEPS:raise ValueError('Missing lifecycle transition')
    if flow.get('native_command_frames',0)<6 or flow.get('native_painted_frames',0)<6:raise ValueError('Missing visible native output')
    phases=[]
    for cycle in (0,1):
        rows=[r for r in flow['gameplay_rates'] if r['cycle']==cycle]
        if not rows or any(type(r['seconds']) not in (int,float) or not math.isfinite(r['seconds']) or r['seconds']<=0 for r in rows):
            raise ValueError('Invalid stress observation interval')
        for r in rows:
            for key in ('native_frames','painted_frames','world_imports'):
                if type(r[key]) is not int or r[key]<0:raise ValueError('Invalid advancing counter')
        duration=sum(r['seconds'] for r in rows)
        counters={k:sum(r[k] for r in rows) for k in ('native_frames','painted_frames','world_imports')}
        if duration<seconds-2 or min(counters.values())<=0:raise ValueError('Incomplete advancing stress phase')
        phases.append(dict(cycle=cycle,seconds=duration,**counters))
    resident=value['resident'];counts=[int(resident[k]) for k in ('completed','imports','verified_returns')]
    if min(counts)<policy['minimum_worlds'] or len(set(counts))!=1 or resident.get('error') or resident.get('pending') is not False or resident.get('normal_quit_retired') is not True:
        raise ValueError('Unbalanced owned World retirement')
    if any(resident.get(k)!=0 for k in ('world_readbacks','native_readbacks','rgba_readbacks','world_pixel_reply_bytes')):
        raise ValueError('Unexpected ordinary pixel readback')
    engine=value['engine_journey']
    actions=engine.get('actions',[])
    if any(not isinstance(row,(list,tuple)) or len(row)!=2 or any(type(v) is not int for v in row) for row in actions):raise ValueError('Invalid original callback encoding')
    if [list(row) for row in actions]!=ACTIONS or engine.get('world_ticks',0)<3 or engine.get('world_resumes',0)<2 or engine.get('protocol_version')!=17:
        raise ValueError('Missing independent original callback/resume proof')
    readiness=value.get('mini_readiness',[])
    if len(readiness)!=3 or any(r.get('verified') is not True for r in readiness) or [r['action'] for r in readiness]!=[4,3,3]:
        raise ValueError('Missing original Mini readiness binding')
    inputs=value.get('physical_stress',[])
    if len(inputs)!=2 or any(r.get('verified') is not True or r.get('seconds',0)<seconds or r.get('actions',0)<20 for r in inputs):
        raise ValueError('Missing physical stress inputs')
    if value.get('native_compiler_inputs_checked',0)<1:raise ValueError('Missing actual shell compiler closure')
    process=value['original_process']
    if value.get('shell_exit')!=0 or process.get('original_exit_code')!=0 or process.get('cleanup_after_original_exit') is not True:
        raise ValueError('Missing normal original exit')
    admission=value['admission']
    if (admission.get('required_backing_mode')!=3 or admission.get('host_graphics_devices') is not False or admission.get('legacy_embedding') is not False
            or any(admission.get('namespace_'+k) is not True for k in ('network','pid','ipc'))):
        raise ValueError('Missing isolated ownership admission')
    outcomes=value.get('native_only_outcomes',[])
    if len(outcomes)!=1 or outcomes[0].get('state')!='completed' or outcomes[0].get('forwarded')!=0 or value['routes'].get('forwarded')!=0 or value['routes'].get('native',0)<1:
        raise ValueError('Native-only completion or no-forward proof missing')
    retirement=value['graphics_retirement']
    if (retirement.get('process_exit') is not True or retirement.get('resources_released') is not True or retirement.get('ddraw_module_loaded') is not False
            or retirement.get('surviving_owners')!=[] or not retirement.get('counts') or any(retirement['counts'].values())):
        raise ValueError('Graphics owners survived shutdown')
    return dict(verified=True,profile='strict-native-only',fps_policy='diagnostic',worlds=counts[0],phases=phases,
                observed_engine_fps=resident['engine_fps'],original_callbacks_verified=True,normal_quit_verified=True,
                scope='One Apprentice campaign with two active physical camera/HUD stress phases, Mini Cancel, Quit No/resume and Quit Yes/report/Main/exit. No fixed FPS, full pixel equivalence, all14journeys, physical driver or complete original recovery claim.')
