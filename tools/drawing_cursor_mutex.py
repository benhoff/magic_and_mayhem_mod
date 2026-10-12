"""Strict original mutex-call observations; raw results do not prove ownership."""
import struct
from drawing_input_point import decode as decode_point


def decode(data):
    if len(data) < 64 or (len(data) - 64) % 128 or len(data) > 64 + 65536 * 128:
        raise ValueError('Incomplete cursor mutex envelope')
    h = struct.unpack_from('<14I', data, 8)
    if data[:8] != b'MNMDMX01' or h[:4] != (1, 128, 0x40209ca7, 65536) or any(h[10:]):
        raise ValueError('Unsupported cursor mutex header')
    count, dropped, pending, calls, overflow, identities = h[4:10]
    if count != (len(data) - 64) // 128 or count != calls * 2 or dropped or pending or overflow or identities > calls:
        raise ValueError('Cursor mutex loss/pending/identity gap')
    rows = list(struct.iter_unpack('<32I', data[64:]))
    entries = {}
    completed = []
    live = set()
    next_id = 0
    for i, row in enumerate(rows, 1):
        if row[0] != i or not 1 <= row[1] <= calls or row[2] not in (1, 2, 3, 4) or row[3] not in (1, 2) or not row[4] or row[6] >= 0x1c0000 or any(row[13:15]) or row[15] not in (1, 2):
            raise ValueError('Invalid cursor mutex row identity')
        if row[2] != 1 and any(row[11:13]) or row[2] != 2 and row[2] != 1 and row[8]:
            raise ValueError('Unexpected cursor mutex arguments')
        if row[15] == 1:
            point = decode_point(struct.pack('<16I', *row[16:]))
            if point['owner_thread'] != row[4]:
                raise ValueError('Cursor mutex point owner differs')
        elif any(row[16:]):
            raise ValueError('Refused cursor mutex point has data')
        else:
            point = None
        sample = dict(ordinal=i, call=row[1], operation=row[2], phase=row[3], thread=row[4], tick=row[5], caller_rva=row[6], mutex_id=row[7], parameter=row[8], raw_result=row[9], last_error=row[10], security_nonnull=row[11], name_nonnull=row[12], point=point)
        if row[3] == 1:
            if row[1] in entries or row[9]:
                raise ValueError('Duplicate cursor mutex entry/result')
            if row[7] and row[7] not in live:
                raise ValueError('Cursor mutex entry references retired identity')
            entries[row[1]] = sample
        else:
            begin = entries.pop(row[1], None)
            if begin is None or any(begin[k] != sample[k] for k in ('operation', 'thread', 'caller_rva', 'parameter', 'security_nonnull', 'name_nonnull')):
                raise ValueError('Cursor mutex return arguments/identity differ')
            if (row[5] - begin['tick']) % 2**32 >= 2**31:
                raise ValueError('Cursor mutex return time differs')
            if row[2] == 1:
                if begin['mutex_id'] or bool(row[7]) != bool(row[9]):
                    raise ValueError('Cursor mutex creation identity differs')
                if row[7] not in live and row[7]:
                    next_id += 1
                    if row[7] != next_id:
                        raise ValueError('Cursor mutex creation generation gap')
                    live.add(row[7])
            elif begin['mutex_id'] != row[7]:
                raise ValueError('Cursor mutex handle identity changed')
            if row[2] == 4 and row[9] and row[7]:
                if row[7] not in live:
                    raise ValueError('Cursor mutex duplicate close')
                live.remove(row[7])
            completed.append(dict(call=row[1], entry=begin, returned=sample))
    if entries or len(completed) != calls or next_id != identities:
        raise ValueError('Cursor mutex unfinished calls/creation count')
    return dict(schema=1, calls=completed, created_identities=identities, remaining_live_ids=sorted(live), real_mutex_ownership_proved=False, safe_source_synchronization_proved=False, scope='Exact paired original import calls, numeric CreateMutex/CloseHandle identities and sampled API/cursor prefixes. Raw Wait/Release results remain uninterpreted; no ownership, complete aliases, source protection, native equivalence or original suppression claim.')
