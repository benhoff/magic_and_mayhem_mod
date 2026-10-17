"""Account exact observed EGL command domains without promoting byte ownership."""
from collections import Counter
import struct


def unique(rows, key):
    result = {}
    for row in rows:
        value = row[key]
        if value in result:
            raise ValueError('Duplicate observed identity: ' + key)
        result[value] = row
    return result


def account(reference, marker):
    if len(marker) != 32 or marker[:8] != b'MNMEGD01':
        raise ValueError('Exact physical interval close marker required')
    h = struct.unpack('<8I', marker)
    physical = unique(reference['egl_physical_observation']['frames'], 'entry')
    if h[2] != 1 or h[6:] != (1, 0) or h[4] != len(physical) or not physical or h[5] != max(physical):
        raise ValueError('Closed physical interval accounting differs')
    if any(f['pid'] != h[3] for f in physical.values()):
        raise ValueError('Physical interval process identity differs')
    calls = unique(reference['observation']['calls'], 'entry')
    submissions = unique(reference['wine_submission_observation']['calls'], 'call')
    packets = unique(reference['wine_blit_packet_observation']['bindings'], 'command')
    leases = unique(reference['api_source_lease_observation']['calls'], 'api_entry')
    refused = unique(reference['cursor_source_gate_binding']['refused'], 'api_entry')
    contexts = unique(reference['egl_worker_context_observation']['calls'], 'call')
    domains, outside = [], []
    for call_id, c in contexts.items():
        if c['pid'] != h[3] or not c['stable_active_context']:
            raise ValueError('Selected process/stable worker context required')
        point = c['points'][0]['context']
        packet, sub = packets.get(point['command']), submissions.get(point['submission'])
        if not packet or not sub or (packet['worker'], packet['submission'], packet['worker_thread']) != (point['worker'], point['submission'], point['win_thread']):
            raise ValueError('Missing or changed exact command/submission/worker join')
        api_entry = sub['api_entry']
        api = calls.get(api_entry)
        if not api:
            raise ValueError('Worker submission lacks original API entry')
        base = dict(egl_call=call_id, api_entry=api_entry, command=point['command'],
                    worker=point['worker'], submission=point['submission'],
                    method=api['method'], raw_flags=api['flags'],
                    source_rectangle=sub['source_rect'], destination_rectangle=sub['destination_rect'])
        if call_id > h[5]:
            if call_id in physical:
                raise ValueError('Physical witness extends beyond declared close')
            outside.append(dict(base, reason='after-explicit-physical-interval-close'))
            continue
        if call_id not in physical:
            raise ValueError('Missing physical witness inside declared interval')
        lease, gate = leases.get(api_entry), refused.get(api_entry)
        if lease:
            if lease['lease']:
                domain = 'accepted-readonly-source-lease'
            elif lease['before'][2] & 4:
                domain = 'refused-after-producer-recording-stopped'
            else:
                domain = 'refused-source-domain'
            base.update(source_metadata=lease['before'], source_lease_result=lease['begin_result'])
        elif gate:
            domain = 'cursor-gate-refused-before-source-lease'
            base['cursor_gate_reason'] = gate['reason']
        else:
            domain = 'API-method-or-flags-outside-source-lease-observer'
        domains.append(dict(base, domain=domain, source_byte_ownership_proved=False))
    if set(physical) != {d['egl_call'] for d in domains}:
        raise ValueError('Observed context and physical interval sets differ')
    counts = Counter(d['domain'] for d in domains)
    signatures = Counter((d['domain'], d['method'], d['raw_flags'],
                          tuple(d.get('source_metadata', [0]*16)[6:10]),
                          d.get('cursor_gate_reason')) for d in domains)
    return dict(closed_physical_calls=len(domains), outside_closed_interval=outside,
                domains=domains, domain_counts=dict(counts),
                signatures=[dict(domain=k[0], method=k[1], raw_flags=k[2],
                                 source_canvas_extent_stride=list(k[3]), cursor_gate_reason=k[4], count=n)
                            for k, n in signatures.items()],
                source_byte_ownership_proved=False, complete_presented_journey=False)
