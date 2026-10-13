#!/usr/bin/env python3
"""Read bounded opt-in drawing contention/recovery diagnostics; never modify them."""
import argparse
import json
from pathlib import Path
import struct

EVENTS = {1: 'wait_timeout', 2: 'wait_clock', 3: 'owner_sample',
          4: 'surface_drop', 5: 'checkpoint_refusal', 6: 'recovery_reset'}


def decode(data):
    if len(data) < 32 or data[:8] != b'MNMCOPY1':
        raise ValueError('Copy timing header')
    version, size, frequency, limit, reserved = struct.unpack_from('<IIQII', data, 8)
    if (version, size, limit, reserved) != (1, 100, 512, 0):
        raise ValueError('Copy timing version/bounds')
    if (len(data)-32) % size or (len(data)-32)//size > limit:
        raise ValueError('Copy timing truncated/oversized records')
    records, waits = [], {}
    for at in range(32, len(data), size):
        words = struct.unpack_from('<25I', data, at)
        event, session, thread, tick, low, high = words[:6]
        if event not in EVENTS:
            raise ValueError('Unknown copy timing event')
        values = words[6:]
        record = dict(event=EVENTS[event], session=session, thread=thread,
                      tick_ms=tick, clock=low | high << 32, values=list(values))
        records.append(record)
        if event in (1, 2, 3):
            key = (session, thread, values[18])
            if not values[18] or event in waits.setdefault(key, {}):
                raise ValueError('Invalid/duplicate wait correlation')
            waits[key][event] = record
    summaries = []
    for (session, thread, identity), group in waits.items():
        item = dict(session=session, thread=thread, identity=identity,
                    complete=len(group) == 3)
        if 1 in group:
            v = group[1]['values']
            item.update(site=v[0], target=v[1], elapsed_tick_ms=v[2], attempts=v[3],
                        wait_calls=v[4], successful_wait_returns=v[5],
                        intervening_published_acquisitions=(v[7]-v[6]) & 0xffffffff,
                        sampled_thread_changes=v[8], coherent_samples=v[9],
                        unavailable_samples=v[10], maximum_wait_span_tick_ms=v[11])
        if 2 in group:
            v = group[2]['values']; start = v[2] | v[3] << 32
            item['elapsed_qpc_ms'] = (group[2]['clock']-start)*1000/frequency if frequency and start else None
        if 3 in group:
            v = group[3]['values']
            if v[2] not in (0, 1):
                raise ValueError('Owner coherence flag')
            item['owner_coherent'] = bool(v[2])
            if v[2]:
                start = v[9] | v[10] << 32
                item.update(owner_thread=v[3], owner_acquisition=v[4],
                            owner_site=v[6], owner_target=v[7], owner_phase=v[8],
                            owner_publication_age_qpc_ms=(group[3]['clock']-start)*1000/frequency if frequency and start else None)
        summaries.append(item)
    return dict(version=version, frequency=frequency, records=records, waits=summaries,
                at_capacity=len(records) == limit,
                scope='Bounded observation; missing records and scheduling causes cannot be inferred.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input', type=Path)
    args = parser.parse_args()
    print(json.dumps(decode(args.input.read_bytes()), indent=2))


if __name__ == '__main__':
    main()
