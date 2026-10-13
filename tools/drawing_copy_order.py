"""Strict bounded copy request entry/return journal, independent of driver order."""
import struct
from drawing_api_session import sha

TAGS = {7, 8, 32, 33, 37, 38, 39, 41, 42, 43}


def decode(data):
    if len(data) < 192 or (len(data) - 64) % 128 or len(data) > 64 + 65536 * 128:
        raise ValueError('Copy order envelope')
    h = struct.unpack_from('<16I', data)
    if data[:8] != b'MNMDCQ01' or h[2:6] != (1, 128, 0x40209ca7, 65536) or any(h[6:]):
        raise ValueError('Copy order header')
    rows = list(struct.iter_unpack('<32I', data[64:]))
    pending, entries, returns, requests, previous = {}, [], [], 0, 0
    for ordinal, r in enumerate(rows, 1):
        if r[0] != ordinal or r[1] not in (1, 2, 3) or r[6] < previous or any(r[22:24]) or any(r[25:]):
            raise ValueError('Copy order identity/reserved/prefix')
        previous = r[6]
        if r[1] == 3:
            if ordinal != len(rows) or r[2] != requests or pending or r[24] or any(r[3:6]) or any(r[8:22]):
                raise ValueError('Copy order incomplete terminal')
            continue
        if not r[2] or not r[3] or r[4] not in TAGS or not all(r[8:12]) or r[10] > 2048 or r[11] > 2048 or r[12] > 4096 or r[12] and r[12] < r[10] or r[20] >= 0x1c0000:
            raise ValueError('Copy request descriptor/owner')
        record = dict(ordinal=ordinal, request=r[2], owner=r[3], tag=r[4], tick=r[5],
                      producer_prefix=r[6], queue=r[7], destination=r[8], source=r[9],
                      destination_extent_stride=list(r[10:13]), destination_position=list(r[13:15]),
                      source_rectangle=list(r[15:19]), raw_key=r[19], caller_rva=r[20],
                      producer_return_sequence=r[21], active_requests=r[24])
        if r[1] == 1:
            requests += 1
            if r[2] != requests or r[2] in pending or r[21]:
                raise ValueError('Copy request duplicate/out-of-order entry')
            pending[r[2]] = r
            entries.append(record)
        else:
            entry = pending.pop(r[2], None)
            if entry is None or r[3:5] != entry[3:5] or r[8:21] != entry[8:21] or not entry[6] < r[21] <= r[6]:
                raise ValueError('Copy request unmatched/changed return')
            returns.append(record)
        if r[24] != len(pending):
            raise ValueError('Copy request active count')
    if rows[-1][1] != 3:
        raise ValueError('Copy order missing terminal')
    return dict(schema=1, success=True, journal_sha256=sha(data), requests=requests,
                entries=entries, returns=returns, terminal_producer_prefix=rows[-1][6],
                terminal_queue=rows[-1][7], driver_order_proved=False,
                api_request_bindings_proved=False, source_or_physical_ownership_proved=False)
