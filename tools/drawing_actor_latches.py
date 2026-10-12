"""Strict whole-pool raw latch observations; never infer actor generations."""
import struct
from drawing_input_point import decode as decode_point


def slots(words, capacity):
    value = sum(w << (32 * i) for i, w in enumerate(words))
    if value >> capacity:
        raise ValueError('Latch bitmap extends beyond capacity')
    return [i for i in range(capacity) if value & (1 << i)]


def decode(data):
    if len(data) < 64 or (len(data) - 64) % 256 or len(data) > 64 + 2048 * 256:
        raise ValueError('Latch envelope')
    h = struct.unpack_from('<16I', data)
    count = (len(data) - 64) // 256
    if data[:8] != b'MNMDLA01' or h[2:6] != (1, 256, 0x40209ca7, 2048) or h[6] != count or h[7] or h[9] or h[10] != 50 or any(h[11:]) or count and not h[8]:
        raise ValueError('Latch header/loss/owner')
    samples = []
    previous = periodic = None
    pool_id = 0
    for ordinal, r in enumerate(struct.iter_unpack('<64I', data[64:]), 1):
        if r[0] != ordinal or r[2] != h[8] or r[4] > 2 or r[7] & ~15:
            raise ValueError('Latch row identity')
        if bool(r[4]) != bool(r[5]) or bool(r[4]) != bool(r[6]):
            raise ValueError('Latch callback identity')
        if previous is not None and (r[1] - previous) % 2**32 >= 2**31:
            raise ValueError('Latch time order')
        previous = r[1]
        if not r[4]:
            if periodic is not None and (r[1] - periodic) % 2**32 < 50:
                raise ValueError('Latch periodic cadence')
            periodic = r[1]
        active = a = b = []
        first = None
        if bool(r[7] & 1) != bool(r[7] & 8):
            raise ValueError('Latch pool must have two matching collections')
        if r[7] & 1:
            if not 1 <= r[9] <= 256 or not pool_id <= r[8] <= pool_id + 1 or not r[8] or r[56] > 3:
                raise ValueError('Latch pool shape')
            pool_id = r[8]
            active, a, b = (slots(r[start:start + 8], r[9]) for start in (16, 24, 32))
            if [len(active), len(a), len(b)] != list(r[10:13]) or not set(a + b) <= set(active):
                raise ValueError('Latch bitmap counts/active membership')
            selected = sorted(set(a + b))
            if selected:
                if r[57] != selected[0] + 1 or not r[60] or bool(r[62]) != (selected[0] in a) or bool(r[63]) != (selected[0] in b):
                    raise ValueError('Latch first actor disagrees with bitmap')
                first = dict(slot=r[57] - 1, actor_type=r[58], owner=r[59], active=r[60], health=r[61], raw_68=r[62], raw_70=r[63])
            elif any(r[57:]):
                raise ValueError('Unlatched actor has identity data')
            if (r[56] == 1 and not 1 <= r[15] <= r[9]) or (r[56] != 1 and r[15]):
                raise ValueError('Latch hit identity')
        elif any(r[8:13]) or r[15] or any(r[16:40]) or any(r[56:]):
            raise ValueError('Unvalidated pool has actor data')
        if not r[7] & 2 and any(r[13:15]):
            raise ValueError('Unreadable latch scalars have data')
        point = decode_point(struct.pack('<16I', *r[40:56])) if r[7] & 4 else None
        if point and point['owner_thread'] != h[8] or not r[7] & 4 and any(r[40:56]):
            raise ValueError('Latch cursor point identity')
        samples.append(dict(ordinal=ordinal, tick=r[1], thread=r[2], screen=r[3], kind=r[4], call=r[5], phase_ordinal=r[6], flags=r[7], pool_id=r[8], capacity=r[9], active_slots=active, raw_68_slots=a, raw_70_slots=b, raw_count=r[13] if r[7] & 2 else None, raw_mode=r[14] if r[7] & 2 else None, hit_state=r[56], hit_slot=r[15] - 1 if r[56] == 1 else None, first_latched_actor=first, input_point=point))
    return dict(schema=1, success=True, owner=h[8], samples=samples, allocation_ownership_proved=False, actor_generations_proved=False, atomic_pool_snapshot_proved=False, complete_selection_proved=False, source_frame_parity_proved=False)


def bind_callbacks(latches, region):
    if latches['owner'] != region['owner']:
        raise ValueError('Latch/Region owners differ')
    by_phase = {(s['call'], s['kind']): s for s in latches['samples'] if s['kind']}
    if len(by_phase) != sum(bool(s['kind']) for s in latches['samples']) or len(by_phase) != len(region['calls']) * 2:
        raise ValueError('Duplicate/missing/orphan latch callback phase')
    pairs = []
    for call in region['calls']:
        for kind, phase in ((1, 'entry'), (2, 'returned')):
            sample = by_phase.get((call['call'], kind))
            observed = call[phase]
            if sample is None or sample['phase_ordinal'] != observed['phase_ordinal'] or (sample['tick'] - observed['tick']) % 2**32 >= 2**31:
                raise ValueError('Latch phase not bound to exact Region point')
            pairs.append(dict(call=call['call'], kind=kind, region_phase_ordinal=observed['phase_ordinal'], latch_ordinal=sample['ordinal'], mouse_record=call['mouse_record'], mask=call['mask']))
    return dict(success=True, phases=pairs, persistent_selection_proved=False, source_frame_parity_proved=False)
