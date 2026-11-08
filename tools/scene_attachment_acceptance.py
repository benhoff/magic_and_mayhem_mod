"""Admit the ordinary attachment route independently of its pixel proof."""
import struct

from campaign_spell import read_spell_rows
from rendering_pilot import require
from scene_actor_observation import decode
from scene_effects import module


def route(acquired,session,bound):
    driver=acquired['attachment_route']
    require(driver['success'] is True and driver['simulation_writes'] is False
            and driver['policy']=='public-excalibur-self-cast-v1' and driver['cast_requests']==1,
            'Incomplete ordinary single-cast attachment route')
    expected_extent=list(map(int,acquired['configuration']['requested_size'].split('x')))
    require(driver['actual_extent']==expected_extent and
            [r['screen']for r in acquired['states']]==[3,22,14,25,14,7,26,22,3] and
            [r['ack']for r in acquired['states']]==[0,1,2,3,4,5,6,7,8],
            'Original attachment menu sequence/extent differs')
    states=acquired['states']
    require(len({v['thread'] for v in states})==1 and
            all(b['generation']>a['generation'] for a,b in zip(states,states[1:])),
            'Attachment original menu owner/generation differs')
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
            'Actual attachment callbacks/World owner differ')
    inventory=driver['inventory'];recipe=driver['recipe']
    original_inventory=module('scene_attachment_journey').spell_inventory((menus.ROOT/states[5]['wire_path']).read_bytes())
    require(all(inventory[k]==original_inventory[k]for k in
                ('owner','offered','counts','assignments','recipes','ingredients','spells')),
            'Driver inventory differs from saved original Spellbox wire')
    i,a=recipe['ingredient'],recipe['alignment']
    require(0<=i<21 and 0<=a<3 and inventory['offered']&(1<<i)
            and inventory['counts'][a]>0 and inventory['recipes'][i*3+a]==recipe['effect']==83,
            'Selected attachment recipe was not originally offered')
    # Names and input alone cannot prove a spell. Require the original cast
    # observation with the player receiver and mana debit as well as the
    # independently captured active numeric role/queue below.
    spell_path=session/'spell-events.bin';bound(spell_path)
    spells=read_spell_rows(spell_path,complete=True)
    casts=[r for r in spells if r[1]==1 and r[4:8]==(83,0,0,0) and r[15]==1 and r[8]>r[9]]
    require(len(casts)==1 and casts[0][2]==states[0]['thread'], 'No unique original player effect83 mana-debit observation')
    input_path=session/'input-admission.bin';bound(input_path)
    samples=module('campaign_input_observation').decode(input_path.read_bytes())['rows']
    require(driver['spell_bar_before']in samples and driver['spell_bar_selected']in samples and
            driver['spell_bar_selected']['spell_count']==1 and driver['spell_bar_selected']['selected_slot']==0,
            'Sole-spell selection differs from original passive input trace')
    pose_paths=list(session.rglob('scene-actors-0001.bin'))
    require(len(pose_paths)==1, 'Ambiguous attachment actor sidecar');bound(pose_paths[0])
    poses=decode(pose_paths[0].read_bytes());frames={f['generation']:f for f in poses['frames']}
    require(poses['version']==2 and poses['owner']==states[0]['thread'] and
            {r['view']for r in driver['views']}==set(range(4)), 'Missing current attachment owner/views')
    for selected in driver['views']:
        f=frames[selected['generation']]
        hero=next((v for v in f['actors']if v[0]==0),None)
        require(f['view']==selected['view'] and f['extent']==expected_extent and hero and
                hero[1:3]==(0,0) and hero[4]>0 and hero[14]==1 and hero[185] and hero[12]&2
                and any(d[0:2]==(0,1) for d in f['draws']), 'No actual active player child in requested stored view')
    return dict(verified=True,actual_extent=expected_extent,original_callbacks=actions,
                original_cast=list(casts[0]),views=driver['views'],single_cast_request=True,
                original_gameplay_formula_equivalence=False)
