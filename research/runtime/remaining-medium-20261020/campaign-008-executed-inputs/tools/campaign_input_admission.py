"""Bounded physical-input test policy; no game-state mutation or admission proof."""
import math


def logical_input_point(x, y):
    if not math.isfinite(x) or not math.isfinite(y) or not 0 <= x < 800 or not 0 <= y < 600:
        raise ValueError('Physical input is outside the logical viewport')
    return int(x), int(y)




def living_engaged_targets(actors, hits):
    """Keep current living type/owner matches for proven melee recipients.

    Slots are not lifetime identities; this rejects changed roles without
    claiming to detect same-type, same-owner recycling.
    """
    selected = {}
    for hit in hits:
        slot = hit[8]
        actor = actors.get(slot)
        if (actor and actor['active'] and actor['health'] > 0
                and actor['type'] == hit[9] and actor['owner'] == hit[10]):
            selected.pop(slot, None)
            selected[slot] = actor
    return list(selected.values())


def ground_waypoint(wizard, target):
    """Bounded physical isometric projection; walking progress is not a camera basis."""
    dx, dy, dz = (target[k] - wizard[k] for k in ('x', 'y', 'z'))
    if not all(math.isfinite(v) for v in (dx, dy, dz)):
        raise ValueError('Invalid observed ground position')
    sx, sy = 32 * (dx - dy), 16 * (dx + dy - dz)
    scale = max(1, abs(sx) / 170, abs(sy) / 110)
    return logical_input_point(400 + sx / scale, 300 + sy / scale)


def pursuit_waypoint(wizard, enemy):
    """Physical-driver heuristic toward a living engaged target; no simulation model."""
    if wizard['health'] <= 0 or enemy['health'] <= 0 or not enemy['active']:
        return None
    dx, dy = (enemy[k] - wizard[k] for k in ('x', 'y'))
    if math.hypot(dx, dy) <= 3:
        return None
    return ground_waypoint(wizard, enemy)


def spell_selection_points(spell):
    """Switch through a different known starting spell before selecting a toggle."""
    points = {'zombie': (523, 575), 'fireball': (477, 575)}
    if spell not in points:
        raise ValueError('Unsupported starting spell')
    other = 'fireball' if spell == 'zombie' else 'zombie'
    return (points[other], points[spell])


def fresh_hover(rows, point, after_sequence, already_there=False, require_cast=False):
    """Require original motion dispatch followed by a later World hover update.

    Qt publication alone is insufficient: original cursor/ground picking can
    still describe the previous HUD location when the right press arrives.
    """
    def matches(position):
        return len(position) == 2 and all(abs(a-b) <= 1 for a, b in zip(position, point))
    before = [r for r in rows if r['sequence'] <= after_sequence]
    if not before:
        return None
    if already_there and (before[-1]['input_active'] != 1
                          or not matches([before[-1]['x'], before[-1]['y']])):
        return None
    motion_counter = before[-1]['world_counter'] if already_there else None
    motion_sequence = after_sequence
    for row in rows:
        if row['sequence'] <= after_sequence:
            continue
        if (row.get('kind') == 3 and row['flags'] & 64 and matches(row['mouse'])
                and row.get('callback_mask',0) & 96 and not row['mouse_flags']):
            motion_counter = row['world_counter']
            motion_sequence = row['sequence']
        if (motion_counter is None or row.get('kind') != 1 or row['sequence'] <= motion_sequence
                or not 0 < (row['world_counter']-motion_counter) % 2**32 < 2**31
                or row['flags'] & 40 != 40 or row['input_active'] != 1
                or not matches([row['x'], row['y']])):
            continue
        if require_cast and (row['mode'] != 2 or row['cursor_type'] != 5
                or not 0 <= row['selected_slot'] < row['spell_count'] or not row['receiver_slot_plus_one']
                or any(not 0 <= value < bound for value, bound in zip(row['ground'], row['ground_bounds']))):
            continue
        return row
    return None


class DialogueReadiness:
    def __init__(self, resident, quiet_seconds=12, observations=12, dismissals=12, seconds=90):
        if (not isinstance(resident, bool) or not math.isfinite(quiet_seconds)
                or quiet_seconds < 0 or not 2 <= observations <= 48
                or not 1 <= dismissals <= 12 or not 0 < seconds <= 90):
            raise ValueError('Invalid bounded dialogue policy')
        self.routes = ('native',) if resident else ('native', 'original')
        self.quiet_seconds = quiet_seconds
        self.max_observations, self.max_dismissals, self.seconds = observations, dismissals, seconds
        self.observations = self.dismissals = self.quiet = 0
        self.started = self.previous = self.quiet_since = None

    def observe(self, visible, now):
        if (set(visible) != {'native', 'original'} or any(type(v) is not bool for v in visible.values())
                or not math.isfinite(now) or self.previous is not None and now < self.previous):
            raise ValueError('Invalid dialogue observation')
        if self.started is None:
            self.started = now
        self.previous = now
        self.observations += 1
        if self.observations > self.max_observations or now - self.started >= self.seconds:
            raise RuntimeError('Dialogue readiness observation bound exhausted')
        if any(visible[r] for r in self.routes):
            self.quiet = 0
            self.quiet_since = None
            if not all(visible[r] for r in self.routes):
                return 'wait'
            if self.dismissals >= self.max_dismissals:
                raise RuntimeError('Dialogue dismissal bound exhausted')
            self.dismissals += 1
            return 'dismiss'
        if self.quiet_since is None:
            self.quiet_since = now
        self.quiet += 1
        return 'ready' if self.quiet >= 2 and now - self.quiet_since >= self.quiet_seconds else 'wait'


def validate_readiness_trace(inputs, resident):
    if inputs.get('input_readiness_policy') not in {'continuous-quiet-distinct-spell-v1','continuous-quiet-distinct-spell-v2'}:
        raise ValueError('Unsupported input readiness trace')
    captures = {c['name'] for c in inputs['captures']}
    stages = []
    for rows, initial in ((inputs['dialogue_checks'], True),
                          (inputs.get('additional_dialogue_checks', []), False)):
        gate = None
        previous_stage = None
        final = None
        for row in rows:
            stage = row['stage']
            if stage != previous_stage:
                if gate is not None and final != 'ready':
                    raise ValueError('Unfinished dialogue readiness stage')
                if stage in stages or initial and stage != 'dialogue-check':
                    raise ValueError('Duplicate or invalid dialogue stage')
                minimum = 12 if initial or stage.startswith('after-') else 2
                if row['quiet_seconds'] != minimum:
                    raise ValueError('Changed dialogue quiet requirement')
                gate = DialogueReadiness(resident, minimum, 12 if initial and inputs['input_readiness_policy'].endswith('-v1') else 48)
                previous_stage = stage
                stages.append(stage)
            if row['capture'] not in captures or row['visible'] != {
                    r: 'hermes' in row['texts'][r].lower() for r in ('native', 'original')}:
                raise ValueError('Missing or inconsistent dialogue capture')
            final = gate.observe(row['visible'], row['observed_monotonic'])
            if final != row['decision']:
                raise ValueError('Dialogue decision differs from retained observations')
        if gate is not None and final != 'ready':
            raise ValueError('Unfinished dialogue readiness stage')
    if not stages or stages[0] != 'dialogue-check':
        raise ValueError('Missing introductory dialogue stage')
    return {'stages': stages, 'continuous_quiet_verified': True,
            'scope': 'Physical-input test readiness only; original cast/combat outcomes remain independent.'}
