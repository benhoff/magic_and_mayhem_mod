"""Decode one bounded negative source-snapshot diagnostic; never admit a frame."""
import struct
from drawing_api_session import sha

REASONS = {1: 'source getter refused', 2: 'source metadata invalid or changed',
           3: 'call geometry outside full-frame contract', 4: 'canvas identity changed',
           5: 'protected source bytes changed', 6: 'worker completion incomplete',
           7: 'snapshot acquisition already active', 8: 'snapshot owner mismatch',
           9: 'snapshot allocation failed', 10: 'snapshot budget exceeded', 11: 'pending-copy admission point missing'}


def decode(data):
    if len(data) != 256 or data[:8] != b'MNMSHR01':
        raise ValueError('Held rejection envelope')
    h = struct.unpack_from('<16I', data)
    r = struct.unpack_from('<48I', data, 64)
    if h[2:6] != (1, 192, 0x40209ca7, 1) or any(h[11:]) or h[7] > h[6]:
        raise ValueError('Held rejection header')
    if h[8] > 2048 or h[9] > 1024 or h[10] > 1024**3:
        raise ValueError('Held rejection budget')
    # Owner-mismatch is also the diagnostic for an invalid phase. Preserve the
    # rejected scalar instead of rejecting the very failure the emitter records.
    if r[0] != 1 or r[1] not in REASONS or (r[2] not in (1, 2) and r[1] != 8) or not all(r[3:6]) or r[14] > 1:
        raise ValueError('Held rejection identity')
    if not r[14] and any(r[8:12]):
        raise ValueError('Unreadable rectangle has owned values')
    return dict(schema=1, success=True, admitted=False, journal_sha256=sha(data),
                reason=REASONS[r[1]], reason_code=r[1], phase=r[2], api_entry=r[3],
                lease=r[4], thread=r[5], tick=r[6], source_status=r[7],
                rectangle=list(r[8:12]) if r[14] else None,
                destination=list(r[12:14]), flags=r[15], before=list(r[16:32]),
                after=list(r[32:48]), commands=h[6], completed=h[7],
                prior_records=h[8], prior_planes=h[9], prior_bytes=h[10],
                source_ownership_proved=False)
