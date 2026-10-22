"""Strict scalar source-prefix bounds around actual driver worker callbacks."""
import struct
from drawing_api_session import sha


def decode(data, legacy):
    if len(data) < 192 or (len(data) - 64) % 128 or len(data) > 64 + 131072 * 128:
        raise ValueError('Worker producer point envelope')
    header = struct.unpack_from('<16I', data)
    if data[:8] != b'MNMWPP01' or header[2:7] != (1, 128, 0x40209ca7, 131072, 64) or any(header[7:]):
        raise ValueError('Worker producer point header')
    if len(legacy) < 256 or len(legacy)>64+131072*192 or (len(legacy) - 64) % 192 or legacy[:8] != b'MNMWBK01':
        raise ValueError('Legacy worker envelope')
    legacy_header = struct.unpack_from('<16I', legacy)
    if legacy_header[2:7] != (1, 192, 0x40209ca7, 131072, 42) or any(legacy_header[7:]):
        raise ValueError('Legacy worker header')
    original, pending_legacy = {}, {}
    legacy_rows=list(struct.iter_unpack('<48I',legacy[64:]))
    for ordinal,row in enumerate(legacy_rows[:-1],1):
        if row[0]!=ordinal or row[1] not in (1,2) or row[3]!=42 or not row[4] or any(row[34:]):
            raise ValueError('Legacy worker identity/reserved')
        if row[1] == 1:
            if row[2]!=len(original)+1 or row[2] in original or row[6]:
                raise ValueError('Duplicate legacy worker')
            original[row[2]] = pending_legacy[row[2]] = row
        else:
            entry=pending_legacy.pop(row[2],None)
            if entry is None or entry[3:5]!=row[3:5] or entry[7:]!=row[7:] or (row[5]-entry[5])%2**32>=2**31:
                raise ValueError('Legacy worker unmatched/changed return')
    terminal=legacy_rows[-1]
    if pending_legacy or terminal[0]!=len(legacy_rows) or terminal[1]!=4 or terminal[2] not in (1,2) or terminal[3]!=len(original) or any(terminal[6:]):
        raise ValueError('Legacy worker incomplete terminal/count')
    rows = list(struct.iter_unpack('<32I', data[64:]))
    pending, complete, entered, seen = {}, [], 0, set()
    for ordinal, row in enumerate(rows[:-1], 1):
        if row[0] != ordinal or row[1] not in (1, 2) or not row[2] or not row[3] or row[5] > 2 or row[7] or any(row[24:]):
            raise ValueError('Worker producer point identity/reserved')
        worker = original.get(row[2])
        if worker is None or (row[3], row[6]) != (worker[4], worker[0]):
            raise ValueError('Worker producer point legacy identity changed')
        point = row[8:24]
        if row[5]:
            if any(point):
                raise ValueError('Unavailable getter contains a source point')
        elif point[:2] != (64, 1) or point[2] not in (1, 3, 7) or point[3] > 1048576 or any(point[6:12]) or point[12] > 128 or point[13] > 128 or point[14] != row[3] or point[15]:
            raise ValueError('Invalid pointer-free recorder point')
        record = dict(ordinal=ordinal, phase=row[1], worker=row[2], thread=row[3],
                      tick=row[4], getter_status=row[5], legacy_entry_record=row[6], point=list(point))
        if row[1] == 1:
            entered += 1
            if row[2] in seen:
                raise ValueError('Duplicate worker point entry')
            seen.add(row[2])
            pending[row[2]] = record
        else:
            before = pending.pop(row[2], None)
            if before is None or before['thread'] != row[3] or before['legacy_entry_record'] != row[6] or (row[4]-before['tick'])%2**32>=2**31:
                raise ValueError('Unpaired worker producer return')
            if not row[5] and not before['getter_status'] and point[3] < before['point'][3]:
                raise ValueError('Worker producer prefix regressed')
            complete.append(dict(worker=row[2], thread=row[3], before=before, after=record,
                                 both_points_available=not row[5] and not before['getter_status'],
                                 source_storage_owned=False))
    terminal = rows[-1]
    if terminal[0] != len(rows) or terminal[1] != 3 or terminal[2] != entered or not terminal[3] or terminal[5] or terminal[6] != len(complete) or terminal[7] != len(pending) or any(terminal[8:]) or pending or set(original) != {c['worker'] for c in complete}:
        raise ValueError('Worker producer point incomplete terminal/coverage')
    return dict(schema=1, success=True, journal_sha256=sha(data), calls=complete,
                unavailable_pairs=sum(not c['both_points_available'] for c in complete),
                producer_changes_during_worker=sum(c['both_points_available'] and c['before']['point'][3] != c['after']['point'][3] for c in complete),
                source_storage_owned=False, general_replay_order_proved=False,
                complete_presented_journey=False)
