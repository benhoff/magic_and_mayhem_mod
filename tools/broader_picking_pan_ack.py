"""Acknowledge source camera motion inside the existing physical pan time budget."""
import time
from rendering_pilot import require
PAN_ACTIONS={phase+'-'+mode+'-pan'for phase in ('high-initial','low','high-returned')for mode in ('sharp','integer')}
POLICY='same-view-source-pan-within-existing-action-budget-v1'

def acknowledged(before,after):
    return bool(before['kind']==1 and after and after['kind']==1 and after['frame']and after['frame']['generation']>before['frame']['generation']
        and after['frame']['nonce']==before['frame']['nonce']and after['thread']==before['thread']
        and after['camera'][0]==before['camera'][0]and after['camera'][1:3]!=before['camera'][1:3])

def perform(journey,action,symbol,hold,settle,advance,*,clock=time):
    require(action in PAN_ACTIONS and symbol==0xff52 and hold==.22 and settle==.8 and advance is True,
            'Unreviewed physical pan action or time budget')
    journey.mark(action,'begin');before=journey.latest();require(before['frame'],'Missing original pan baseline')
    code=journey.input.x.XKeysymToKeycode(journey.input.display,symbol);require(code>0,'Physical camera key absent')
    require(journey.input.xt.XTestFakeKeyEvent(journey.input.display,code,1,0),'Physical camera key refused')
    journey.input.x.XFlush(journey.input.display);pressed=clock.monotonic();deadline=pressed+hold+settle;after=None;observed_at=None
    try:
        while clock.monotonic()<deadline:
            row=journey.latest();now=clock.monotonic()
            if pressed+hold<=now<deadline and acknowledged(before,row):after=row;observed_at=now;break
            clock.sleep(min(.01,max(0,deadline-now)))
    finally:
        release_ok=journey.input.xt.XTestFakeKeyEvent(journey.input.display,code,0,0)
        journey.input.x.XFlush(journey.input.display);released_at=clock.monotonic()
        journey.pan_receipts.append(dict(action=action,before=before,acknowledged_after=after,
            pressed_monotonic=pressed,acknowledged_monotonic=observed_at,released_monotonic=released_at,
            deadline_monotonic=deadline,minimum_hold_seconds=hold,hold_plus_settle_budget_seconds=hold+settle,physical_release_accepted=bool(release_ok)))
        require(release_ok,'Physical camera key release refused')
    released=journey.mark(action,'released')
    journey.actions.append(dict(action=action,symbol=symbol,keycode=code,held_ms=round((released_at-pressed)*1000),
        down_up=True,host_ns=clock.monotonic_ns(),host_focus=journey.input.focus(),source_pan_acknowledged=after is not None,
        original_minimum_hold_ms=220,original_hold_plus_settle_budget_ms=1020))
    require(after is not None and released_at<=deadline,'No acknowledged same-view source camera pan within existing1.02-second action budget')
    clock.sleep(max(0,deadline-clock.monotonic()))
    journey.wait_progress(int(released['world']),minimum=5)
    journey.mark(action,'settled')
