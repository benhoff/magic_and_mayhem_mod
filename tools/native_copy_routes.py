"""Strict metadata-only application copy routing; absence is scoped to a closed run."""
from collections import Counter
import hashlib
from pathlib import Path
import struct

from native_gpu_copy_diagnostics import census

CAPACITY = 65536
ROUTES = ('unprepared', 'cpu_prepared', 'gpu_prepared', 'fill_prepared')


def decode(path):
    raw = Path(path).read_bytes()
    if len(raw)<128:raise ValueError('Copy routes extent')
    h = struct.unpack_from('<32I', raw)
    version=h[2];words=40 if version==1 else 44 if version==2 else 0
    if not words or len(raw)!=128+CAPACITY*words*4:raise ValueError('Copy routes extent/version')
    if raw[:8] != (b'MNMCRT01' if version==1 else b'MNMCRT02') or h[2:6] != (version,128,words*4,CAPACITY) or h[6] != h[8] or not 0 < h[6] <= CAPACITY or h[7] or h[9] or h[14] != 1 or not h[15] or not h[16] or any(h[22:]):
        raise ValueError('Copy routes incomplete, missed, exhausted or unclosed')
    rows = [struct.unpack_from('<'+str(words)+'I', raw, 128 + i * words*4) for i in range(h[6])]
    for i, r in enumerate(rows):
        rva = r[1] - h[15]
        if r[0] != i + 1 or r[2] != (rva if 0 <= rva < h[16] else 0xffffffff) or not r[3] or r[10] & ~65535 or not r[10] & 32768 or r[11] > 1 or r[14] > 1 or r[15] > 3 or r[39]:
            raise ValueError('Copy routes row identity')
        if version==2:
            first,last,source,target=r[40:44]
            if bool(first)!=bool(last) or first>last or (first and (not source or not target)) or (r[15] not in (1,2) and any(r[40:44])) or (r[15]==1 and first!=last):
                raise ValueError('Copy routes emitted command binding')
        state = r[10]
        if not state & 2 or not state & 16 or (r[5] and not state & 1):
            raise ValueError('Copy routes unknown alias or clipper metadata')
        if bool(state & 1) != bool(r[7]) or bool(state & 2) != bool(r[6]) or bool(state & 256) != bool(r[5] and r[4] == r[5]):
            raise ValueError('Copy routes object identity')
        if bool(state & 128) != bool(r[6] and r[6] == r[7]) or bool(state & 32) != bool(r[23]) or bool(r[14]) != bool(r[15]) or bool(r[33]) != (r[15] == 2):
            raise ValueError('Copy routes preparation/alias identity')
        if r[15] == 2 and not state & 4:
            raise ValueError('GPU copy routes require stale CPU source')
    counts = [sum(r[15] == route for r in rows) for route in (2, 1, 0, 3)]
    counts += [sum(bool(r[10] & mask) for r in rows) for mask in (128, 32)]
    counts += [sum(bool(r[12] & (1 if r[11] else 0x8000)) for r in rows), sum(bool(r[10] & 4) for r in rows)]
    if counts != [*h[10:14], *h[17:21]] or any(raw[128 + h[6] * words*4:]):
        raise ValueError('Copy routes counters or unused tail')
    return h, rows


def validate(experiment):
    directory = Path(experiment) / 'lock-capture'
    v2=directory/'copy-routes-v2.bin';v1=directory/'copy-routes-v1.bin'
    if v2.exists() and v1.exists():raise ValueError('Ambiguous copy routes versions')
    path=v2 if v2.exists() else v1
    h, rows = decode(path)
    gh, gpu = census(directory / 'gpu-copy-census-v1.bin')
    if (h[15], h[16]) != (gh[18], gh[19]):
        raise ValueError('Copy routes module mismatch')
    linked = []
    for r in rows:
        if r[15] != 2:
            continue
        if not 1 <= r[33] <= len(gpu):
            raise ValueError('Copy routes missing GPU row')
        g = gpu[r[33] - 1]
        keyed = bool(r[12] & (1 if r[11] else 0x8000))
        if (r[1], r[2], r[5], r[4], r[9], r[8], r[13]) != (g[1], g[3], g[10], g[11], g[12], g[13], g[17]) or (bool(r[10] & 128), bool(r[10] & 32), keyed) != (bool(g[15] & 1), bool(g[15] & 2), bool(g[15] & 4)):
            raise ValueError('Copy routes GPU caller/authority identity')
        if h[2]==2 and tuple(r[40:44])!=(g[7],g[8],g[4],g[5]):raise ValueError('Copy routes GPU command binding')
        linked.append(r[33])
    if sorted(linked) != list(range(1, len(gpu) + 1)):
        raise ValueError('Copy routes GPU rows are not bijective')
    groups = Counter((r[1], r[15], bool(r[12] & (1 if r[11] else 0x8000)), bool(r[10] & 32), bool(r[10] & 128), bool(r[10] & 4), r[32], r[13]) for r in rows)
    branches = {}
    for name, predicate in [('keyed', lambda r: bool(r[12] & (1 if r[11] else 0x8000))), ('attached_clipper', lambda r: bool(r[10] & 32)), ('self_alias', lambda r: bool(r[10] & 128))]:
        selected = [r for r in rows if predicate(r)]
        branches[name] = dict(calls=len(selected), gpu_source_calls=sum(bool(r[10] & 4) for r in selected), prepared_gpu_calls=sum(r[15] == 2 for r in selected), failed_calls=sum(bool(r[13] & 0x80000000) for r in selected), routes=dict(Counter(ROUTES[r[15]] for r in selected)), first_rows=[list(r) for r in selected[:3]])
    return dict(verified=True, census_version=h[2], copies=len(rows), linked_gpu_calls=len(linked), post_retirement_calls_excluded=h[21], module_base=hex(h[15]), module_extent=hex(h[16]), census_sha256=hashlib.sha256(path.read_bytes()).hexdigest(), routes=dict(Counter(ROUTES[r[15]] for r in rows)), failed_calls=sum(bool(r[13] & 0x80000000) for r in rows), branches=branches,
                callers=[dict(return_pc=hex(pc), route=ROUTES[route], keyed=keyed, attached_clipper=clip, self_alias=self_copy, gpu_source=source, preparation_reason_hash=hex(reason), result=hex(result), calls=count) for (pc, route, keyed, clip, self_copy, source, reason, result), count in sorted(groups.items())],
                scope='Closed bounded application Blt/BltFast metadata before terminal RenderStop; later retired callbacks are counted exclusions, not observed branches. Frozen pre-call identity/history/clip/key state and exact bijection to GPU admission census. CPU-prepared is routing, not independent CPU output proof. Zero branch counts prove absence only in this active interval; no all-game reachability or original-driver policy claim.')
