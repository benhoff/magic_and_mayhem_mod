"""Experimental offline COPY transpositions selected solely by observed metadata."""
import struct


def select(records, observation, bindings):
    rows = {r[1]: r for r, _ in records}
    returns = {r['producer_return_sequence']: r for r in observation['returns']}
    bound = {b['request']: b for b in bindings['bindings']}
    active, world = False, {}
    for r, _ in records:
        if r[2] == 11:
            active = True
        elif r[2] == 12:
            active = False
        world[r[1]] = active
    selected, refused = [], []
    for f, _ in records:
        if f[2] != 6 or f[3:5] != (1, 2) or f[1] < 3:
            continue
        s, restore = rows[f[1] - 1], rows[f[1] - 2]
        if s[2] != 6 or s[3:5] != (5, 1):
            continue
        item = dict(save_sequence=s[1], full_sequence=f[1], restore_sequence=restore[1])
        reason = None
        qs = [returns.get(r[1]) for r in (restore, f, s)]
        bs = [bound.get(q['request']) if q else None for q in qs]
        if any(q is None or b is None for q, b in zip(qs, bs)):
            reason = 'missing-exact-request-binding'
        elif (f[0] != 96 or s[0] != 96 or restore[0] != 96 or
              f[5:7] != (800, 600) or f[8:14] != (0, 0, 0, 0, 800, 600) or
              s[5:7] != (40, 40) or s[8:10] != (0, 0) or
              not 0 <= s[10] < s[12] <= 800 or not 0 <= s[11] < s[13] <= 600 or
              (s[12] - s[10], s[13] - s[11]) != (40, 40) or
              restore[2:5] != (6, 1, 5) or restore[5:7] != (800, 600) or
              restore[10:14] != (0, 0, 40, 40) or
              not 0 <= restore[8] <= 760 or not 0 <= restore[9] <= 560 or
              any(r[14] != 0xffffffff or r[18] for r in (restore, f, s)) or
              f[22] not in (0x46ba01, 0x5597fd) or
              s[22] != 0x4a4c26 or restore[22] != 0x4a4b01):
            reason = 'unsupported-copy-triple'
        else:
            rb, fb, sb = bs
            item.update(restore_request=qs[0]['request'], full_request=qs[1]['request'],
                        save_request=qs[2]['request'], bindings=bs)
            # Binding construction separately checks successful API phases,
            # owners, geometry and exclusive submission/packet/worker identities.
            prefix = fb['producer_entry_prefix']
            if not 0 <= prefix < restore[1] < s[1] < f[1]:
                reason = 'unsupported-full-entry-prefix'
            elif any(world[i] for i in range(prefix + 1, f[1] + 1)):
                reason = 'inside-active-world'
            elif (not all(b['commands'] for b in bs) or
                  max(c['worker_return_record'] for c in rb['commands']) >=
                  min(c['worker_entry_record'] for c in fb['commands']) or
                  max(c['worker_return_record'] for c in fb['commands']) >=
                  min(c['worker_entry_record'] for c in sb['commands'])):
                reason = 'worker-triple-not-completely-ordered'
            elif any(r[1] not in (restore[1], s[1], f[1]) and
                     (r[2] not in (3, 4, 10, 25) or r[2] == 10 and r[3] in (1, 5))
                     for r, _ in records[prefix:f[1]]):
                # Refuse every other draw/lifetime/queue boundary, even on an
                # unrelated canvas. CPU2 checkpoints may precede the pair only.
                reason = 'other-observed-operation-in-full-interval'
        if reason:
            refused.append(dict(item, reason=reason))
        else:
            selected.append(item)
    sequences = [v[k] for v in selected for k in ('save_sequence', 'full_sequence')]
    if len(sequences) != len(set(sequences)):
        raise ValueError('Overlapping COPY transpositions')
    return dict(selected=selected, refused=refused)


def transpose(raw, selected):
    """Swap only admitted adjacent payload-free COPY bodies; preserve wire sequences."""
    offsets, at = {}, 64
    while at < len(raw):
        size, sequence, kind = struct.unpack_from('<3I', raw, at)
        if size < 96 or at + size > len(raw) or sequence in offsets:
            raise ValueError('Invalid native input record bounds')
        offsets[sequence] = (at, size, kind)
        at += size
    result = bytearray(raw)
    occupied = set()
    for item in selected:
        s, f = item['save_sequence'], item['full_sequence']
        if f != s + 1 or s in occupied or f in occupied:
            raise ValueError('Nonadjacent/overlapping COPY transposition')
        sa, fs = offsets[s], offsets[f]
        if sa[1:] != (96, 6) or fs[1:] != (96, 6) or fs[0] != sa[0] + 96:
            raise ValueError('Transposition is not two payload-free COPY records')
        left, right = raw[sa[0]:sa[0] + 96], raw[fs[0]:fs[0] + 96]
        result[sa[0]:sa[0] + 96] = right
        result[fs[0]:fs[0] + 96] = left
        struct.pack_into('<I', result, sa[0] + 4, s)
        struct.pack_into('<I', result, fs[0] + 4, f)
        occupied.update((s, f))
    return bytes(result)
