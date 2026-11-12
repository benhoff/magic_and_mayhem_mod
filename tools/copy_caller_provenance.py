"""Strict COPY companion/census joins; unresolved callers remain visible."""
from collections import Counter
import hashlib
from pathlib import Path
import struct

from native_copy_routes import decode as routes_decode

STATES = ('resolved', 'unsupported_api', 'code_mismatch', 'unreadable_slot', 'destination_mismatch', 'not_direct_call')
CAPACITY = 65536


def decode(raw, routes_header, routes, layouts):
    if len(raw)<128 or raw[:8] not in (b'MNMCCL01',b'MNMCCL02',b'MNMCCL03'):
        raise ValueError('COPY caller extent/magic')
    h = struct.unpack_from('<32I', raw)
    version=h[2];words=16 if version==1 else 20 if version in (2,3) else 0
    if not words or len(raw)!=128+CAPACITY*words*4 or raw[:8]!=('MNMCCL0'+str(version)).encode():
        raise ValueError('COPY caller version/extent')
    if (h[2:6] != (version, 128, words*4, CAPACITY) or h[6] != h[8] or h[7] or h[9]
            or not 0 < h[6] <= CAPACITY or h[16] != 1 or any(h[20:])
            or h[17:19] != routes_header[15:17] or not h[17] or h[17] > 0xffffffff-h[18]
            or h[19] != len(routes)
            or h[6] != len(routes)):
        raise ValueError('COPY caller incomplete/counter/module identity')
    rows = [struct.unpack_from('<'+str(words)+'I', raw, 128+i*words*4) for i in range(h[6])]
    sites = {a['return_rva']: (v, a) for v in layouts for a in v['apis']}
    for i, (r, c) in enumerate(zip(rows, routes)):
        if (r[0] != i+1 or r[1:3] != c[1:3] or r[8] != c[4] or not 1 <= r[9] <= 6
                or r[10:15] != (c[15], int(bool(c[12] & (1 if c[11] else 0x8000))),
                                int(bool(c[10] & 4)), c[7], c[6]) or r[15]):
            raise ValueError('COPY caller/census row binding')
        if r[9] == 1:
            if (not r[3] or r[3] >= h[18] or r[4] not in (60, 68, 76, 92) or not r[7]
                    or not h[17]+5 <= r[5] < h[17]+h[18] or r[6] != r[5]-h[17]):
                raise ValueError('Resolved caller identity')
        if r[9] != 2:
            selected = sites.get(r[2])
            if selected is None or (r[3], r[4]) != (selected[0]['entry_rva'], selected[0]['return_slot_distance']):
                raise ValueError('Caller frame differs from reviewed layout')
            if r[9] == 1 and bool(c[11]) != selected[1]['fast']:
                raise ValueError('Resolved caller API method differs')
        if version in (2,3):
            mode=r[17]
            if mode>(3 if version==3 else 2) or (r[9]==1 and not mode) or (mode not in (2,3) and (r[16] or r[19])):
                raise ValueError('Caller source-frame mode/identity')
            if mode in (2,3) and (not r[16] or not r[19] or r[2] not in sites or r[18]!=sites[r[2]][0]['hook_length']):
                raise ValueError('Caller lacks verified handoff')
            if mode==3 and r[3] not in (0x18c140,0x18c360,0x18c4a0,0x18c8a0,0x18ca90):
                raise ValueError('Cursor handoff outside its five wrapper profiles')
            if mode==1 and r[18] not in (0,sites[r[2]][0]['hook_length']):
                raise ValueError('Caller detour profile differs')
        if r[9] == 2 and any(r[3:8]):
            raise ValueError('Unsupported API has fabricated frame')
    if ([sum(r[9] == state for r in rows) for state in range(1, 7)] != list(h[10:16])
            or any(raw[128+h[6]*words*4:])):
        raise ValueError('COPY caller counters/unused tail')
    return h, rows


def analyze(experiment, layouts):
    directory = Path(experiment)/'lock-capture'
    rh, routes = routes_decode(directory/'copy-routes-v2.bin')
    paths=[p for p in (directory/'copy-callers-v1.bin',directory/'copy-callers-v2.bin',directory/'copy-callers-v3.bin') if p.exists()]
    if len(paths)!=1:raise ValueError('Unique COPY caller version required')
    raw=paths[0].read_bytes()
    h, rows = decode(raw, rh, routes, layouts)
    grouped = Counter((r[3], r[6], r[10], bool(r[11]), bool(r[12]),r[17] if h[2]>=2 else 1) for r in rows if r[9] == 1)
    return dict(verified=True, version=h[2], scene_handoff_calls=sum(r[9]==1 and h[2]>=2 and r[17]==2 for r in rows), cursor_handoff_calls=sum(r[9]==1 and h[2]==3 and r[17]==3 for r in rows),
                cursor_handoff_frames=sum(h[2]==3 and r[17]==3 for r in rows),
                unresolved_cursor_callers=sum(h[2]==3 and r[17]==3 and r[9]!=1 for r in rows),
                application_calls=len(rows), resolved_calls=sum(r[9] == 1 for r in rows),
                states=dict(Counter(STATES[r[9]-1] for r in rows)),
                sha256=hashlib.sha256(raw).hexdigest(),
                callers=[dict(wrapper_entry_rva=hex(k[0]), caller_return_rva=hex(k[1]),
                              call_instruction_rva=hex(k[1]-5), route=k[2], keyed=k[3], gpu_source=k[4], frame_mode=k[5], calls=count)
                         for k, count in sorted(grouped.items())],
                natural_self_calls=sum(bool(c[10] & 128) for c in routes),
                natural_keyed_gpu_calls=sum(c[15] == 2 and bool(c[12] & (1 if c[11] else 0x8000)) for c in routes),
                boundary='Observed direct callers and frozen source routing only. Unsupported/unreadable/mismatched frames are retained. No global reachability, original-driver pixels, call ABI equivalence or complete renderer acceptance.')
