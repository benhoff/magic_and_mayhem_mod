"""Exact retained CPU_READ identity joins, separate from original caller proof."""
from collections import Counter
import hashlib
import re
import struct

from rendering_capture import capture
from rendering_pilot import commands, require

KINDS = ('dc_or_panel_materialization', 'explicit_owned_lock_reader',
         'implicit_owned_lock_reader', 'explicit_reader_without_lock')


def sha(data):return hashlib.sha256(data).hexdigest()


def counters(raw):
    require(len(raw) == 48, 'CPU-read counter extent')
    w = struct.unpack('<12I', raw)
    require(raw[:8] == b'MNMRDC01' and w[2:4] == (1, 48) and not any(w[9:]),
            'CPU-read counter version/reserved fields')
    require(0 < w[4] <= 4096 and sum(w[5:9]) == w[4], 'CPU-read counter completion sum')
    return dict(last_completed_ticket=w[4], completed=w[4],
                kinds={name:w[5+i] for i,name in enumerate(KINDS)},
                session_id_in_counter_file=False)


def reply_header(w):
    require(len(w) == 10 and all(type(v) is int and 0 <= v <= 0xffffffff for v in w),
            'CPU-read request words')
    require(w[0] and w[1] and not w[2] and w[3] and 1 <= w[4] <= 2048
            and 1 <= w[5] <= 2048 and (w[6] or w[7]) and (w[8] or w[9]),
            'CPU-read request identity/extent')
    return b'MNMCREAD'+struct.pack('<14I', 1, 64, w[3], w[1], w[2], w[0], w[4], w[5],
                                  w[4]*w[5]*2, *w[6:10], 1)


def lifecycle(raw, requests):
    require(len(raw) <= 128*256, 'Lifecycle byte budget')
    lines = raw.decode('ascii').splitlines()
    require(len(lines) <= 128, 'Lifecycle record budget')
    by_ticket = {}; reasons = Counter()
    for ordinal, line in enumerate(lines, 1):
        parts = line.split()
        require(len(parts) == 20 and re.fullmatch('[a-z0-9_]{1,40}', parts[0])
                and all(re.fullmatch('[0-9a-f]{8}', x) for x in parts[1:]),
                'Lifecycle record encoding')
        reason = parts[0]; values = [int(x, 16) for x in parts[1:]]; reasons[reason] += 1
        require(reasons[reason] <= 4, 'Lifecycle per-reason bound')
        if reason not in ('resident_cpu_materialized', 'resident_lock_cpu_materialized'):continue
        if reason == 'resident_cpu_materialized':
            # values[2] is the logger's generic kind argument, hardcoded zero;
            # it is NOT the completion-counter reader-kind enum.
            ticket, surface = values[3:5]
            detail = dict(record=ordinal, reason=reason, observed_object_address=hex(values[0]),
                          observed_thread=values[1], original_caller_address=None)
            require(values[0] and values[1] and not values[2] and not values[5]
                    and not any(values[7:]), 'Materialization lifecycle fields')
        else:
            ticket, surface, family, route, caller = values[:5]
            detail = dict(record=ordinal, reason=reason, family=family, route=route,
                          observed_caller_address=hex(caller), caller_module_unassigned=True,
                          original_caller_address=None)
            require(not any(values[6:]), 'Owned-lock lifecycle reserved fields')
        require(ticket in requests and ticket not in by_ticket
                and requests[ticket]['surface'] == surface, 'Lifecycle ticket/surface ambiguity')
        if reason == 'resident_cpu_materialized':
            require(values[6] == requests[ticket]['payload_bytes'], 'Lifecycle materialization extent')
        by_ticket[ticket] = detail
    return by_ticket, dict(records=len(lines), reasons=dict(reasons), capacity=128,
        per_reason_capacity=4, duplicate_rows_suppressed=True, contended_rows_may_drop=True,
        complete_original_callback_history=False)


def join_reads(raw, session, count_bytes, comparison_reads, lifecycle_bytes, terminal_reply):
    """Reconcile existing complete reply comparisons; do not re-run their oracle."""
    rows = capture(raw)
    require(rows[0][0]['role'] == 'commands' and rows[0][0]['session'] == session,
            'Wrong CPU-read capture role/session')
    require(rows[-2][0]['kind'] == 'ended'
            and all(row['kind'] in ('fragment', 'cpu-read', 'ended') for row,_ in rows[1:-1]),
            'CPU-read command capture closure')
    observed = {}
    for row, body in rows:
        if row['kind'] != 'cpu-read':continue
        seq = row['sequence']
        require(type(seq) is int and 0 < seq <= 0xffffffff and seq not in observed,
                'Duplicate/invalid captured CPU reply sequence')
        observed[seq] = (row, body)
    compared = {x['sequence']:x for x in comparison_reads}
    require(len(compared) == len(comparison_reads), 'Duplicate independent CPU-read comparison')
    stream = b''.join(body for row,body in rows if row['kind'] == 'fragment')
    counts = counters(count_bytes)
    live, last_created, current_generation, nonce, command_session = {}, 0, 0, None, None
    result, requests = [], {}
    for op, seq, payload in commands(stream):
        if op == 1:
            require(len(payload) >= 28, 'Short CREATE')
            w = struct.unpack_from('<7I', payload)
            require(w[0] > last_created and len(live) < 64 and 1 <= w[1] <= 2048 and 1 <= w[2] <= 2048
                    and w[3] in (8,16,24,32) and len(payload) == 28+w[1]*w[2]*(w[3]//8), 'CREATE identity/extent')
            live[w[0]] = dict(width=w[1], height=w[2], bits=w[3], masks=w[4:7], created_sequence=seq)
            last_created = w[0]
        elif op == 7:
            require(len(payload) == 4 and struct.unpack('<I',payload)[0] in live, 'Unknown DELETE')
            del live[struct.unpack('<I',payload)[0]]
        elif op == 19:
            require(len(payload) == 20, 'WORLD extent')
            w = struct.unpack('<5I',payload); generation=w[1]|w[2]<<32; ticket_nonce=w[3]|w[4]<<32
            require(w[0] in live and generation == current_generation+1 and ticket_nonce
                    and (nonce is None or nonce == ticket_nonce), 'World generation/nonce identity')
            current_generation, nonce = generation, ticket_nonce
        elif op == 21:
            require(len(payload) == 40, 'CPU-read request extent')
            w = struct.unpack('<10I',payload); header = reply_header(w)
            require(w[1] == len(result)+1 and w[0] in live and seq in observed and seq in compared,
                    'CPU-read ticket/surface/reply gap')
            surface = live[w[0]]; source_generation=w[6]|w[7]<<32
            require(surface['bits'] == 16 and surface['masks'] == (0xf800,0x7e0,31)
                    and (surface['width'],surface['height']) == w[4:6]
                    and 0 < source_generation <= current_generation and (w[8]|w[9]<<32) == nonce,
                    'CPU-read live format/history identity')
            require(command_session is None or w[3] == command_session, 'Mixed CPU command sessions')
            command_session = w[3]
            row, body = observed[seq]; expected_bytes = 64+w[4]*w[5]*2
            require(len(body) == expected_bytes and body[:64] == header, 'CPU-read reply header/extent mismatch')
            check = compared[seq]
            require(check['bytes'] == len(body) and check['sha256'] == sha(body),
                    'CPU-read independent comparison hash mismatch')
            item = dict(sequence=seq, ticket=w[1], surface=w[0], command_session=w[3],
                width=w[4], height=w[5], payload_bytes=expected_bytes-64,
                source_world_generation=source_generation, world_generation_at_request=current_generation,
                history_nonce=str(nonce), created_sequence=surface['created_sequence'],
                reply_sha256=sha(body), payload_sha256=sha(body[64:]),
                captured_reply_row=row['index'], captured_reply_host_us=row['host_us'],
                independent_pixel_comparison_reused=True, original_caller_address=None)
            result.append(item); requests[w[1]] = item
        elif op == 8:
            require(not live, 'CPU-read history surface retirement incomplete')
            require(seq == rows[-2][0]['commands'], 'CPU-read END count disagreement')
    require(result and len(result) == len(observed) == len(compared) == counts['completed'],
            'CPU-read request/reply/completion denominator differs')
    require(sha(terminal_reply) == result[-1]['reply_sha256'], 'Terminal CPU reply differs')
    detail, logging = lifecycle(lifecycle_bytes, requests)
    active = [i for i,name in enumerate(KINDS) if counts['kinds'][name]]
    for item in result:
        item['completion_kind_from_uniform_aggregate'] = active[0] if len(active) == 1 else None
        item['lifecycle_detail'] = detail.get(item['ticket'])
        item['per_request_original_completion_recorded'] = item['ticket'] in detail
    return dict(requests=result, request_count=len(result), complete_reply_matches=len(result),
        captured_payload_bytes=sum(x['payload_bytes'] for x in result),
        counters=counts, lifecycle=logging, lifecycle_detail_count=len(detail),
        requests_without_lifecycle_detail=len(result)-len(detail),
        requests_without_original_caller=len(result), terminal_reply_matches=True,
        aggregate_completion_reconciles=True, exact_original_callers_recovered=0,
        command_stream_sha256=sha(stream),
        boundary='Counters are co-retained per acquisition and carry no session ID. Uniform completion class is aggregate evidence; generic lifecycle kind=0 is not the reader enum. Sparse logs do not identify every original caller or distinguish GetDC from panel preflight. Existing independent reply comparisons are reused, not rerun.')
