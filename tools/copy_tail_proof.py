"""Offline attribution and replay of COPYs omitted by a finite live observer.

Replay fixtures use independent retained history. They cannot recover the
original run's missing GPU outputs or grant whole-operation acceptance.
"""
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct

from rendering_capture import capture
from rendering_pilot import commands, require

MAGIC = b'MNMCPT01'
MAX_CASES = 1024
MAX_BYTES = 2 * 1024**3
WRAPPERS = {
    0x18c05d: (0x18bf40, 'whole opaque'),
    0x18c279: (0x18c140, 'whole keyed'),
    0x18c488: (0x18c360, 'rectangle opaque'),
    0x18c5be: (0x18c4a0, 'rectangle opaque BltFast'),
    0x18c9af: (0x18c8a0, 'rectangle keyed'),
}


def digest(raw):
    return hashlib.sha256(raw).hexdigest()


def command_history(raw, session):
    rows = capture(raw)
    require(rows[0][0]['role'] == 'commands' and rows[0][0]['session'] == session,
            'Wrong command capture role/session')
    require(rows[-2][0]['kind'] == 'ended', 'Command capture lacks END witness')
    require(all(r['kind'] in ('fragment', 'cpu-read', 'ended') for r, _ in rows[1:-1]),
            'Unexpected command capture row')
    stream = b''.join(b for r, b in rows if r['kind'] == 'fragment')
    decoded = list(commands(stream))
    require(decoded[-1][1] == rows[-2][0]['commands'], 'END witness count differs')
    return stream, decoded


def attribute(decoded, routes, live, diagnostic, checkpoints):
    """Join every emitted COPY exactly once, including live prefix and tail."""
    require(diagnostic['selection'] == 'all-copies' and diagnostic['ended'] is True
            and diagnostic['success'] is False and diagnostic['truncated'] is True
            and not diagnostic['error'] and diagnostic['original_pixels_used_as_native_inputs'] is False,
            'Expected retained capped all-COPY negative')
    checked = {r['sequence']: r for r in live}
    require(len(checked) == len(live) == diagnostic['copies_checked'], 'Duplicate live COPY sequence/count')
    owners = {}
    for r in routes:
        for seq in range(r[40], r[41]+1) if r[40] else ():
            require(seq not in owners, 'Overlapping original caller intervals')
            owners[seq] = r
    copies, tail, generation, used = [], [], 0, set()
    for op, seq, payload in decoded:
        if op == 19:
            w = struct.unpack('<5I', payload)
            generation = w[1] | w[2] << 32
        if op != 3:
            require(seq not in checked and seq not in owners, 'COPY binding names another opcode')
            continue
        require(len(payload) == 40 and seq in owners, 'Unowned or malformed COPY command')
        w = list(struct.unpack('<10I', payload)) + [0, 0, 0]
        owner = owners[seq]
        require(owner[15] in (1, 2) and not owner[13] & 0x80000000,
                'Failed/unprepared original COPY owner')
        require(not owner[10] & 128, 'Frozen self-copy needs a separate replay contract')
        keyed = bool(owner[12] & (1 if owner[11] else 0x8000))
        require(w[:2] == list(owner[42:44]) and w[8:10] == [int(keyed), owner[22] if keyed else 0],
                'Caller resource/key disagreement')
        signed = lambda v: v if v < 0x80000000 else v-0x100000000
        sr, dr = list(map(signed, owner[24:28])), list(map(signed, owner[28:32]))
        require(sr[2]-sr[0] == dr[2]-dr[0] and sr[3]-sr[1] == dr[3]-dr[1]
                and sr[0] <= w[2] < w[4] <= sr[2] and sr[1] <= w[3] < w[5] <= sr[3]
                and w[6]-w[2] == dr[0]-sr[0] and w[7]-w[3] == dr[1]-sr[1],
                'Caller/command geometry disagreement')
        used.add(seq)
        if seq in checked:
            r = checked[seq]
            require(r['words'] == w and r['mismatches'] == 0
                    and r['actual_sha256'] == r['expected_sha256'], 'Live COPY prefix disagrees')
        wrapper = WRAPPERS.get(owner[2])
        item = dict(sequence=seq, call=owner[0], caller_rva=hex(owner[2]),
                    wrapper_rva=hex(wrapper[0]) if wrapper else None,
                    wrapper_scope=wrapper[1] if wrapper else 'unknown',
                    route='cpu_prepared' if owner[15] == 1 else 'gpu_prepared',
                    keyed=bool(w[8]), words=w, world_generation=generation,
                    source_extent=list(owner[16:18]), target_extent=list(owner[19:21]),
                    original_live_output_available=seq in checked)
        # Generation checkpoints give exact ordinal brackets, not wall-clock
        # equivalence with the independently clocked action/click journal.
        previous = [c for c in checkpoints if int(c['source_generation']) < generation]
        following = [c for c in checkpoints if generation <= int(c['source_generation'])]
        item['phase_bracket'] = dict(after_checkpoint=previous[-1]['phase'] if previous else None,
                                     through_checkpoint=following[0]['phase'] if following else None,
                                     exact_menu_action_proved=False)
        copies.append(item)
        if seq not in checked:
            tail.append(item)
    require(used == set(owners) and set(checked) <= used, 'Missing census/command bindings')
    require(len(copies) == diagnostic['copies_seen'] and 0 < len(tail) <= MAX_CASES,
            'COPY census/budget mismatch')
    require([x['sequence'] for x in copies[:len(live)]] == list(checked),
            'Observer omission is not a contiguous COPY suffix')
    return tail


def summary(tail):
    return dict(unchecked_commands=len(tail), unique_original_calls=len({x['call'] for x in tail}),
                first_sequence=tail[0]['sequence'], last_sequence=tail[-1]['sequence'],
                world_generation_range=[min(x['world_generation'] for x in tail), max(x['world_generation'] for x in tail)],
                callers=dict(Counter(x['caller_rva'] for x in tail)),
                routes=dict(Counter(x['route'] for x in tail)),
                operations=dict(Counter('keyed' if x['keyed'] else 'opaque' for x in tail)),
                phase_brackets=[dict(after_checkpoint=a, through_checkpoint=b, commands=n)
                    for (a, b), n in Counter((x['phase_bracket']['after_checkpoint'], x['phase_bracket']['through_checkpoint']) for x in tail).items()])


def plane_bytes(plane):
    require(plane['bits'] == 16 and plane['masks'] == (0xf800, 0x7e0, 31),
            'Offline replay requires exact RGB565 inputs')
    return plane['pixels'].astype('<u4').tobytes()


def write_case(stream, item, source, before):
    sw, sh = item['source_extent']; dw, dh = item['target_extent']
    require(len(source) == sw*sh*4 and len(before) == dw*dh*4
            and all(type(v) is int and 1 <= v <= 2048 for v in (sw, sh, dw, dh)), 'Replay plane extent')
    body = struct.pack('<18I', item['sequence'], sw, sh, dw, dh, *item['words']) + source + before
    require(stream.tell()+8+len(body) <= MAX_BYTES, 'Replay fixture byte budget')
    stream.write(struct.pack('<I', len(body))); stream.write(body)


def check_replay(tail, expected, actual):
    require(len(actual) == len(expected) == len(tail), 'Replay output count differs')
    rows = []
    for item, reference, result in zip(tail, expected, actual):
        require(result['sequence'] == reference['sequence'] == item['sequence'], 'Replay sequence differs')
        for role in ('source', 'before'):
            require(result[role+'_sha256'] == reference[role+'_sha256'], 'Replay input differs: '+role)
        require(result['width'] == item['target_extent'][0] and result['height'] == item['target_extent'][1], 'Replay target extent differs')
        require(result['actual_sha256'] == reference['expected_sha256'], 'Offline native COPY destination mismatch')
        rows.append(dict(reference, actual_sha256=result['actual_sha256'],
                         original_live_output_recovered=False, replay_only=True))
    return rows
