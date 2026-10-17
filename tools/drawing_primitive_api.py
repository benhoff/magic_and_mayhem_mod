"""Exact primitive/API identities; source memory ownership remains independent."""
import struct
from drawing_api_session import sha

MAX_ROWS = 786433


def decode(raw, api_calls=None, lifetimes=None):
    if len(raw) < 320 or (len(raw)-64) % 256 or len(raw) > 64+MAX_ROWS*256:
        raise ValueError('Primitive API envelope')
    h = struct.unpack_from('<16I', raw)
    if raw[:8] != b'MNMPAPI1' or h[2:7] != (1,256,0x40209ca7,MAX_ROWS,192) or any(h[7:]):
        raise ValueError('Primitive API header')
    selected = {c['id']: c for c in lifetimes['selected_calls']} if lifetimes is not None else None
    pending = {}; complete = []; seen = set(); terminal = None
    for ordinal, r in enumerate(struct.iter_unpack('<64I', raw[64:]), 1):
        if r[0] != ordinal or r[1] not in (1,2,3,4) or any(r[9:16]) or terminal is not None:
            raise ValueError('Primitive API ordinal/phase/reserved')
        if r[1] == 4:
            if pending or r[2] not in (1,2) or not r[3] or r[4] != len(complete) or any(r[5:]):
                raise ValueError('Primitive API terminal accounting')
            terminal = r
            continue
        p = r[16:]; tick = p[14] | p[15] << 32
        if not r[2] or not r[3] or r[4] > 35 or not r[5] or p[0] != 1 or p[1] != r[3] or p[2] > 3 or p[7] > 1 or p[8] > 128 or p[11] > 128 or p[12] > 1 or p[13] or p[46] > 1 or p[47] or not tick:
            raise ValueError('Primitive API point envelope')
        if p[2] == 1:
            if not p[3] or not p[11] or p[5] not in (*range(57),100) or not p[12] and any(p[16:40]):
                raise ValueError('Primitive API active identity/request')
        elif any(p[3:9]) or any(p[11:13]) or any(p[16:48]):
            raise ValueError('Primitive API inactive fields')
        if r[1] == 1:
            if r[2] in seen or r[7] or r[8]:
                raise ValueError('Primitive API duplicate entry/result')
            seen.add(r[2]); pending[r[2]] = dict(entry=r[2],thread=r[3],method=r[4],destination=r[5],source=r[6],primitive=p[3],points=[])
        c = pending.get(r[2])
        if c is None or len(c['points'])+1 != r[1] or (r[3],r[4],r[5],r[6],p[3]) != (c['thread'],c['method'],c['destination'],c['source'],c['primitive']):
            raise ValueError('Primitive API changed identity or unpaired phase')
        if c['points'] and tick < c['points'][-1]['qpc'] or r[1] != 3 and r[8] or r[1] == 3 and not r[8]:
            raise ValueError('Primitive API clock/completion identity')
        if selected is not None and p[3]:
            f = selected.get(p[3])
            if f is None or (p[1],p[4],p[5],p[6],p[12],list(p[16:40])) != (f['thread'],f['parent'],f['tag'],f['caller'],int(f['phases'][1]['request_present']),f['phases'][1]['request']):
                raise ValueError('Primitive API differs from exact lifetime/request')
            if not f['phases'][1]['qpc'] <= tick <= f['phases'][2]['qpc']:
                raise ValueError('Primitive API lies outside exact active original body')
        c['points'].append(dict(phase=r[1],qpc=tick,state=p[2],producer_prefix=p[9],queue_prefix=p[10],point=list(p)))
        if r[1] == 2:
            c['result'] = r[7]
        if r[1] == 3:
            if r[7] != c['result']:
                raise ValueError('Primitive API result changed')
            c['record'] = r[8]
            if api_calls is not None:
                a = api_calls.get(r[2])
                if a is None or (a[0],a[1],a[2],a[3],a[6],a[8]) != (r[8],1,r[4],r[5],r[3],r[7]):
                    raise ValueError('Primitive API differs from completed surface call')
            complete.append(c);del pending[r[2]]
    if terminal is None:
        raise ValueError('Primitive API missing terminal')
    if api_calls is not None and seen != {a[29] for a in api_calls.values() if a[1] == 1}:
        raise ValueError('Primitive API surface-call denominator differs')
    return dict(schema=1,success=True,journal_sha256=sha(raw),calls=complete,
                exact_lifetimes_crosschecked=lifetimes is not None,api_calls_crosschecked=api_calls is not None,
                active_primitive_calls=sum(bool(c['primitive']) for c in complete),
                source_bytes_immutable=False,all_backing_aliases_observed=False,complete_presented_interval=False)


def analyze(directory):
    from drawing_api_session import decode as api_decode
    from drawing_primitive_lifetimes import decode as lifetime_decode
    from drawing_source_callback_windows import analyze as callback_decode
    from drawing_wine_submission import decode as submission_decode
    from drawing_wine_blit_packet import decode as packet_decode
    import importlib.util
    from pathlib import Path
    raw = (directory/'api-primitive-identities.bin').read_bytes()
    ids = {r[19] for r in struct.iter_unpack('<64I',raw[64:]) if r[1] != 4 and r[19]}
    spec = importlib.util.spec_from_file_location('primitive_packets',Path(__file__).with_name('inspect-canvas-producers.py'))
    module = importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    _, packets = module.decode((directory/'canvas-producers.bin').read_bytes())
    lifetime = lifetime_decode((directory/'primitive-lifetimes-0000.bin').read_bytes(),packets,callback_decode(directory)['points'],ids)
    api_raw = (directory/'session.bin').read_bytes()
    api_decode(api_raw)
    calls = {r[29]:r for r in struct.iter_unpack('<32I',api_raw[64:]) if r[1] != 4}
    result = decode(raw,calls,lifetime)
    submissions = submission_decode((directory/'wine-submission.bin').read_bytes(),calls,(directory/'wine-lifetime.bin').read_bytes())
    submission_by_id = {s['call']:s for s in submissions['calls']}
    commands = packet_decode((directory/'wine-blit-packet.bin').read_bytes(),submission_by_id)
    api_by_id = {c['entry']:c for c in result['calls']}
    chains = []
    for command in commands['bindings']:
        submission = submission_by_id.get(command['submission'])
        call = api_by_id.get(submission['api_entry']) if submission else None
        chains.append(dict(**command,api_entry=submission['api_entry'] if submission else 0,
                           primitive=call['primitive'] if call else 0,
                           thread=call['thread'] if call else 0))
    result.update(consumer_chains=chains,unbound_consumer_chains=sum(not c['primitive'] for c in chains),
                  pending_commands=commands['pending_commands'],unknown_worker_events=commands['unknown_worker_events'],
                  scope='Exact same-thread engine wrapper identity through surface API, observed Wine submission and emitted worker command; no timing-based parent assignment or immutable backing ownership.')
    return result
