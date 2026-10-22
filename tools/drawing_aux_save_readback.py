"""Decode bounded auxiliary diagnostics; pixels remain external mapped-view oracles."""
import hashlib
from pathlib import Path
import struct
from drawing_observed_inputs import regular_bytes


def analyze(directory):
    directory = Path(directory)
    raw = regular_bytes(directory / 'aux-save-readback.bin',64+257*256)
    if len(raw) < 320 or (len(raw) - 64) % 256 or len(raw) > 64 + 257 * 256:
        raise ValueError('Incomplete auxiliary readback envelope')
    h = struct.unpack_from('<16I', raw)
    if (raw[:8] != b'MNMAUX01' or h[2:13] !=
            (1, 256, 0x40209ca7, 256, 30, 50, 5, 1, 40, 40, 3200) or any(h[13:])):
        raise ValueError('Unsupported auxiliary readback policy/header')
    rows = list(struct.iter_unpack('<64I', raw[64:]))
    if rows[-1][1] != 4 or any(r[1] != 1 for r in rows[:-1]):
        raise ValueError('Missing/early auxiliary readback terminal')
    samples, images, refused = [], [], 0
    entries=set()
    for ordinal, r in enumerate(rows, 1):
        if r[0] != ordinal or not r[3]:
            raise ValueError('Invalid auxiliary row identity')
        if r[1] == 4:
            continue
        if (not r[2] or r[5] & 0x80000000 or r[7] not in (11, 12, 13, 14, 17)
                or r[8] > 9 or any(r[62:]) or r[9:12] not in
                ((1, 0, 0), (1, 1, 0), (1, 1, 1))):
            raise ValueError('Invalid auxiliary diagnostic/call accounting')
        if r[2] in entries:raise ValueError('Duplicate auxiliary API sample')
        entries.add(r[2])
        p = r[24:56]
        if (p[0:2] != (64, 1) or p[16:18] != (64, 1) or
                p[2] & 31 != 19 or p[18] & 31 != 19 or p[6:9] != (5, 40, 40)
                or p[22:25] != (1, 800, 600) or not 30 <= p[4] <= 50
                or p[4] != p[20]):
            raise ValueError('Unadmitted auxiliary source point')
        if r[10] and r[12] & 0x80000000 or r[11] and r[13] & 0x80000000:
            raise ValueError('Extra driver calls after failed predecessor')
        if r[10] and not r[13] & 0x80000000 and not r[11]:
            raise ValueError('Successful extra Lock lacks extra Unlock')
        if r[8]:
            refused += 1
            if r[15] or r[23]:
                raise ValueError('Refused auxiliary sample has oracle identity')
        else:
            if (r[9:12] != (1, 1, 1) or any(x & 0x80000000 for x in r[12:15])
                    or r[15] != len(images) + 1 or r[16:18] != (40, 40)
                    or not 80 <= r[18] <= 32768 or r[19:24] != (16, 0xf800, 0x7e0, 31, 3200)
                    or r[56] & 31 != 19 or r[59] != 5 or r[60] != p[10]):
                raise ValueError('Invalid mapped-view auxiliary oracle')
            name = f'aux-save-oracle-{r[15]:06d}.565'
            data = regular_bytes(directory / name,3200)
            if len(data) != 3200:
                raise ValueError('Auxiliary pixel extent changed')
            images.append(dict(ordinal=r[15], path=name, sha256=hashlib.sha256(data).hexdigest()))
        samples.append(dict(api_entry=r[2], owner=r[3], tick=r[4], interface_kind=r[7],
                            reason=r[8], extra_calls=list(r[9:12]),
                            driver_results=list(r[12:15]), oracle=r[15],
                            point_pair=list(p), after_point=list(r[56:62])))
    end = rows[-1]
    if (end[2] not in (1, 2) or end[5:8] != (len(samples), len(images), refused)
            or end[9] or end[10] or any(end[11:])):
        raise ValueError('Incomplete auxiliary lifecycle/count terminal')
    expected = {i['path'] for i in images}
    actual = {p.name for p in directory.glob('aux-save-oracle-*')}
    if actual != expected:
        raise ValueError('Unexpected/missing auxiliary oracle artifacts')
    return dict(schema=1, success=True, journal_sha256=hashlib.sha256(raw).hexdigest(),
                samples=samples, images=images, refused=refused, budget_skips=end[8],
                diagnostic_driver_calls=sum(sum(r['extra_calls']) for r in samples),
                original_driver_state_restored=all(not s['extra_calls'][2] or
                    not s['driver_results'][2] & 0x80000000 for s in samples),
                original_pixels_used_as_native_inputs=False,
                observer_noninterference_proved=False, original_backing_owned=False,
                scope='Post-save extra read-only nonblocking Lock mapped-view bytes only. Default-off diagnostic may alter cache/timing state. Source points and failures retained; no underlying GPU/CPU alias, baseline noninterference, ordering repair or whole presentation.')
