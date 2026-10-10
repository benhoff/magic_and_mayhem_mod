"""Validate bounded physical ground probes against passive creature snapshots."""
from campaign_gameplay import creatures


def validate_movement_probes(rows, inputs):
    probes = inputs.get('movement_probes', [])
    if not probes:
        return dict(verified=False, reason='Combat required no navigation probes')
    if len(probes) > 8:
        raise ValueError('Exceeded bounded ground probes')
    owner, slot = inputs['player_owner'], inputs['wizard_slot']
    deltas = []
    previous = 0
    for probe in probes:
        begin, end = probe['before_sequence'], probe['after_sequence']
        if not 0 < begin < end <= rows[-1][0] or begin < previous:
            raise ValueError('Invalid ground probe observation interval')
        previous = end
        before = creatures([r for r in rows if r[0] <= begin]).get(slot)
        interval = [r for r in rows if begin < r[0] <= end and r[1] == 1 and r[4] == slot]
        if not before or not interval or interval[-1][0] != end:
            raise ValueError('Missing ground probe snapshots')
        actors = [before] + list(creatures([r]).get(slot) for r in interval)
        if any(a['owner'] != owner or a['type'] != 0 or not a['active'] or a['health'] <= 0 for a in actors):
            raise ValueError('Ground probe wizard identity changed or died')
        after = actors[-1]
        delta = [after['x']-before['x'], after['y']-before['y']]
        screen = probe['screen_delta']
        if len(screen) != 2 or not all(type(v) is int and abs(v) <= 170 for v in screen) or screen == [0, 0]:
            raise ValueError('Invalid bounded ground direction')
        if probe['tile_delta'] != delta:
            raise ValueError('Ground probe delta differs from original snapshots')
        deltas.append(delta)
    if not any(a[0]*b[1]-a[1]*b[0] for a in deltas for b in deltas):
        raise ValueError('No independent movement after bounded ground probes')
    return dict(verified=True, attempts=len(probes), idle_attempts=deltas.count([0, 0]), tile_deltas=deltas,
                scope='Wizard XY changes after physical native ground orders; exact paths, projection and order causality equivalence excluded')
