"""Actual physical scene-route admission; pixel/attachment checks stay separate."""
import json
from pathlib import Path

from campaign_gameplay import read_rows
from rendering_pilot import require


def route(acquired, session, output, bound, campaign):
    actions=campaign.validate_events(session,cycles=1,difficulty=1,normal_quit=True)
    path=output/'input-0.json';bound(path)
    driver=json.loads(path.read_text())
    require(driver.get('success') is True and driver.get('simulation_writes') is False
            and driver.get('policy')=='physical-wizard-first-contact-v1'
            and driver.get('actual_extent')==list(map(int,acquired['configuration']['requested_size'].split('x'))),
            'Incomplete actual scene physical-input route')
    trace=session/'gameplay-events.bin';bound(trace);rows=read_rows(trace)
    require(rows and [r[0] for r in rows]==list(range(1,len(rows)+1)), 'Gameplay event order differs')
    event=tuple(driver['melee']);wizard=driver['wizard']
    require(len(event)==16 and event in rows and event[1]==2 and event[12]<event[11]
            and driver['before_sequence']<event[0]
            and wizard['slot']==0 and wizard['type']==0 and wizard['owner']==0
            and (event[4:7]==(0,0,0) or event[8:11]==(0,0,0)),
            'Physical route lacks the named original wizard melee damage')
    rotations=[v['ordinal'] for v in driver['actions'] if v['action']=='scene-view-key']
    require(rotations==[1,2,3,4], 'Physical route omitted bounded view requests')
    return dict(verified=True,original_menu_actions=actions,melee=list(event),
                input_policy=driver['policy'],actual_extent=driver['actual_extent'],
                spell_request_is_not_spell_proof=True,party_lethal_proved=False)
