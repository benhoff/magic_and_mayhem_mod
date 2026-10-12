"""Strict selected Wine submission metadata; no private texture or frame claims."""
import struct
from drawing_api_session import sha


def decode(data, api_calls=None):
    if len(data) < 256 or len(data) > 64+131072*192 or (len(data)-64) % 192:
        raise ValueError('Wine submission envelope')
    h = struct.unpack_from('<16I', data)
    if data[:8] != b'MNMWIN01' or h[2:8] != (1,192,0x40209ca7,131072,32,2048) or any(h[8:]):
        raise ValueError('Wine submission header')
    rows = list(struct.iter_unpack('<48I', data[64:]))
    if rows[-1][1] != 4 or any(r[1] == 4 for r in rows[:-1]):
        raise ValueError('Wine submission terminal')
    pending, completed, identities = {}, [], set()
    entered = 0
    for i,r in enumerate(rows[:-1],1):
        if r[0] != i or r[1] not in (1,2) or r[3] not in (1,2) or not r[4] or r[6] > 32 or bool(r[5]) != bool(r[6]) or any(r[27:]):
            raise ValueError('Wine submission identity/reserve')
        if r[15] & ~0x30003 or r[25] > 1 or r[26] > 1 or not r[10] or max(r[10:12]) > 2048:
            raise ValueError('Wine submission object/rectangle flags')
        if not r[15]&1 and any(r[16:20]) or not r[15]&2 and any(r[20:24]):
            raise ValueError('Wine submission absent rectangle')
        if r[3] == 1 and (not r[11] or r[26]) or r[3] == 2 and any((r[11],r[13],r[24],r[25])):
            raise ValueError('Wine submission method arguments')
        if api_calls is not None and r[5]:
            call = api_calls.get(r[5])
            if call is None or call[1] != 1 or call[6] != r[4]:
                raise ValueError('Wine enclosing API differs')
        identities.update(x for x in r[10:12] if x)
        if r[1] == 1:
            entered += 1
            if r[2] != entered or r[2] in pending or any(r[8:10]):
                raise ValueError('Wine submission entry order')
            pending[r[2]] = r
        else:
            entry = pending.pop(r[2],None)
            if entry is None or r[3:7] != entry[3:7] or r[10:] != entry[10:]:
                raise ValueError('Wine submission return changed/unpaired')
            completed.append(dict(call=r[2],method=r[3],thread=r[4],api_entry=r[5],api_depth=r[6],
                                  first_object=r[10],second_object=r[11],hresult=r[8],last_error=r[9],
                                  source_rect=list(r[16:20]),destination_rect=list(r[20:24]),
                                  argument_flags=r[15],flags=r[14],source_submission_identity_bound=False))
    terminal = rows[-1]
    if pending or terminal[0] != len(rows) or terminal[2] not in (1,2) or terminal[3] != entered or terminal[10] != len(identities) or any(terminal[5:7]) or any(terminal[8:10]) or any(terminal[11:]):
        raise ValueError('Wine submission incomplete terminal/count')
    return dict(schema=1,success=True,boundary='observed_wine_ddraw_submissions',journal_sha256=sha(data),
                calls=completed,unbound_calls=sum(not c['api_entry'] for c in completed),
                source_storage_owned=False,source_submission_identity_bound=False,
                egl_temporal_assignment_performed=False,all_routes_observed=False,
                complete_presented_journey=False)
