"""Admit the ordinary World palette refresh route independently of its pixel proof."""
import struct

from campaign_spell import read_spell_rows
from rendering_pilot import require
from scene_actor_observation import decode
from scene_effects import module


def route(acquired,session,bound):
    driver=acquired['palette_cycle_route']
    require(driver['success'] is True and driver['simulation_writes'] is False
            and driver['policy']=='public-world-palette-refresh-v1' and driver['cast_requests']==0,
            'Incomplete ordinary no-cast World refresh route')
    expected_extent=list(map(int,acquired['configuration']['requested_size'].split('x')))
    require(driver['actual_extent']==expected_extent and
            [r['screen']for r in acquired['states']]==[3,22,14,25,14,7,26,22,3] and
            [r['ack']for r in acquired['states']]==[0,1,2,3,4,5,6,7,8],
            'Original palette menu sequence/extent differs')
    states=acquired['states']
    require(len({v['thread'] for v in states})==1 and
            all(b['generation']>a['generation'] for a,b in zip(states,states[1:])),
            'Palette original menu owner/generation differs')
    menus=module('run-rendering-menu-acceptance')
    for row in states:
        path=menus.ROOT/row['wire_path'];bound(path)
        observed=menus.state(path)[0]
        require(all(observed[k]==row[k] for k in ('sequence','generation','screen','ready','ack','status','thread','handoff')),
                'Saved original menu wire differs from route')
    event_path=session/'events.bin';bound(event_path)
    rows=module('test-menu-observer').decode(event_path.read_bytes())
    actions=[(r['menu_id'],r['argument'])for r in rows if r['event']==3]
    require(actions[:5]==[(3,2),(22,2),(14,2),(25,0),(14,1)] and
            actions[-4:]==[(7,12),(26,2),(22,3),(3,4)] and
            all(menu==7 for menu,arg in actions[5:-4]) and
            len({r['thread']for r in rows if r['event']in(3,12,20)})==1,
            'Actual palette callbacks/World owner differ')
    inventory=driver['inventory'];recipe=driver['recipe']
    original_inventory=module('scene_attachment_journey').spell_inventory((menus.ROOT/states[5]['wire_path']).read_bytes())
    require(all(inventory[k]==original_inventory[k]for k in
                ('owner','offered','counts','assignments','recipes','ingredients','spells')),
            'Driver inventory differs from saved original Spellbox wire')
    i,a=recipe['ingredient'],recipe['alignment']
    require(0<=i<21 and 0<=a<3 and inventory['offered']&(1<<i)
            and inventory['counts'][a]>0 and inventory['recipes'][i*3+a]==recipe['effect']==74,
            'Selected palette recipe was not originally offered')
    pose_paths=list(session.rglob('scene-actors-0001.bin'))
    require(len(pose_paths)==1, 'Ambiguous palette actor sidecar');bound(pose_paths[0])
    poses=decode(pose_paths[0].read_bytes());frames={f['generation']:f for f in poses['frames']}
    require(poses['version']==2 and poses['owner']==states[0]['thread'], 'Current palette actor owner missing')
    require(driver['first_generation']>=32 and driver['after_generation']>=driver['first_generation']+32 and
            driver['after_generation']in frames,'Missing advancing World after refresh')
    from palette_observation import decode as palette_decode,decode_profiles
    palette_paths=list(session.rglob('palette-observation-0001.bin'))
    profile_paths=list(session.rglob('palette-profiles-0001.bin'))
    require(len(palette_paths)==len(profile_paths)==1,'Missing actual palette/profile transcript')
    for path in (*palette_paths,*profile_paths):bound(path)
    palette=palette_decode(palette_paths[0].read_bytes())
    profile=decode_profiles(profile_paths[0].read_bytes(),palette)
    require(palette['version']==3 and profile['complete'],'Missing actual refresh/profile source')
    actual=[dict(node=n['id'],mode=n['mode'],revision=r['revision'],entry=r['entry_record'],returned=r['return_record'],queue=r['queue'])for n in palette['nodes'].values()for r in n['refreshes']if r['result']and r['before']!=r['after']]
    require(driver['refreshed_nodes']==actual and {v['mode']for v in actual}>={1,9},'Original refresh route differs from actual mutation records')
    return dict(verified=True,actual_extent=expected_extent,original_callbacks=actions,
                actual_refreshes=actual,no_cast_requests=True,observed_profile_queries=len(profile['rows']),
                original_gameplay_formula_equivalence=False)
