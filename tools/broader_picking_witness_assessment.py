"""Keep all exact-ANI assessment and explicitly enforce displayed event deadlines."""
from broader_picking_pose_assessment import with_actor_frames as pose_assessment
from rendering_pilot import require

def event_deadlines(acquired,observed):
    paints={int(row['paint']):row for row,_ in observed['paints'].values()}
    for window in acquired['proof']['picking_windows']:
        start=int(window['released']['host_us']);deadline=start+8_000_000
        require(all(start<int(paints[fact['paint']]['host_us'])<=deadline for fact in window['witnesses']),
                'Displayed exact picking witness outside original eight-second event deadline')
    return True

def with_actor_frames(frames):
    original=pose_assessment(frames)
    def assess(reader,acquired,compared,observed,**kwargs):
        proof=original(reader,acquired,compared,observed,**kwargs)
        proof['all_witness_paints_within_eight_seconds']=event_deadlines(acquired,observed)
        return proof
    return assess
