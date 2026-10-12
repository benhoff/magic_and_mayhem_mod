"""Strict bounded Region callback observation, separate from semantic selection."""
import struct


def decode(data):
    if len(data) < 64 or (len(data) - 64) % 256 or len(data) > 64 + 8192 * 256:
        raise ValueError('Incomplete Region callback envelope')
    h = struct.unpack_from('<14I', data, 8)
    if data[:8] != b'MNMDRE01' or h[0] not in (1, 2) or h[1:4] != (256, 0x40209ca7, 8192) or any(h[11:]):
        raise ValueError('Unsupported Region callback header')
    count, dropped, owner, foreign, pending, calls, phases = h[4:11]
    if count % 2 or count != (len(data) - 64) // 256 or calls * 2 != count or phases != count or (count and not owner):
        raise ValueError('Incomplete Region callback identities')
    if dropped or foreign or pending:
        raise ValueError('Region callback loss/foreign-thread/pending gap')
    rows = list(struct.iter_unpack('<64I', data[64:]))
    result = []
    for at in range(0, count, 2):
        begin, end = rows[at:at + 2]
        if begin[:3] != (at + 1, at // 2 + 1, 1) or end[:3] != (at + 2, at // 2 + 1, 2):
            raise ValueError('Broken Region callback pair')
        if begin[5] != owner or end[5] != owner or begin[6] != end[6] or begin[8:18] != end[8:18] or begin[18]:
            raise ValueError('Region callback argument/owner identity differs')
        if begin[3] >= end[3] or not begin[3] or end[3] > count or (end[4] - begin[4]) % 2**32 >= 2**31:
            raise ValueError('Region callback phase/time order differs')
        samples = []
        for row in (begin, end):
            flags = row[7]
            if flags & ~(127 if h[0] == 1 else 255) or any(row[42:] if h[0] == 1 else row[45:]) or row[24] not in (0, 1, 2):
                raise ValueError('Region callback flags/reserved/state differ')
            if not flags & 2 and any(row[20:22]):
                raise ValueError('Invalid control observation has data')
            if not flags & 4 and any(row[22:32]):
                raise ValueError('Invalid hit observation has data')
            if row[24] == 1:
                if not row[25] or not 1 <= row[27] <= row[26] <= 256:
                    raise ValueError('Invalid observed pool/slot identity')
            elif any(row[25:32]):
                raise ValueError('Unvalidated actor has identity data')
            if h[0] == 2 and row[24] != 1 and any(row[42:44]):
                raise ValueError('Unvalidated actor has latch data')
            if h[0] == 2 and not flags & 128 and row[44]:
                raise ValueError('Invalid latch counter has data')
            if not flags & 8 and any(row[32:39]):
                raise ValueError('Invalid camera observation has data')
            for bit, index in ((16, 39), (32, 40), (64, 41)):
                if not flags & bit and row[index]:
                    raise ValueError('Invalid scalar observation has data')
            # V1 recorded +4/+a8/+174 under incorrect type/owner/activity labels.
            # Keep its raw wire intact; normalize only current decoded names.
            actor_type, actor_owner, actor_active = (row[29], row[30], row[28]) if h[0] == 1 else row[28:31]
            samples.append(dict(phase=row[2], phase_ordinal=row[3], tick=row[4], thread=row[5], flags=flags, result=row[18], last_error=row[19], control_id=row[20], active=row[21], hit_class=row[22], draw_index=row[23], actor_state=row[24], pool_id=row[25], capacity=row[26], slot_plus_one=row[27], actor_type=actor_type, actor_owner=actor_owner, actor_active=actor_active, actor_health=row[31], camera=list(row[32:39]), mode=row[39], cursor_type=row[40], selected_ring_index=row[41], actor_flag_68=row[42] if h[0] == 2 else None, actor_flag_70=row[43] if h[0] == 2 else None, raw_latch_count=row[44] if h[0] == 2 and flags & 128 else None))
        result.append(dict(call=at // 2 + 1, mask=begin[6], mouse_record=list(begin[8:18]), entry=samples[0], returned=samples[1]))
    timeline = sorted((r[3], r[1], r[2]) for r in rows)
    if [x[0] for x in timeline] != list(range(1, count + 1)):
        raise ValueError('Duplicate/missing Region phase ordinal')
    stack = []
    for _, call, phase in timeline:
        if phase == 1:
            stack.append(call)
        elif not stack or stack.pop() != call:
            raise ValueError('Broken Region callback nesting')
    if stack:
        raise ValueError('Unreturned Region callback')
    return dict(schema=1, wire_version=h[0], legacy_actor_labels_normalized=h[0] == 1, calls=result, owner=owner, dropped=dropped, foreign_thread_calls=foreign, pending=pending, phase_count=phases, persistent_selection_proved=False, tooltip_proved=False, scope='Bounded exact callback records and read-only candidate/camera scalars; numeric pool IDs do not prove allocations/generations. Callback delivery and scalar changes alone do not establish persistent selection or tooltip semantics.')
