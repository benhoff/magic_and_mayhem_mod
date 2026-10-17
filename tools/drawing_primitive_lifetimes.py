"""Call and snapshot envelopes; temporal observations never prove byte ownership."""
import importlib.util
from pathlib import Path
import struct
from collections import Counter
from drawing_api_session import sha

MAX_ROWS = 2097152


def decode(raw, producer_records=None, callbacks=None, include_call_ids=None):
    if len(raw) < 320 or (len(raw)-64) % 256 or len(raw) > 64+MAX_ROWS*256:
        raise ValueError('Primitive lifetime envelope')
    h = struct.unpack_from('<16I', raw)
    if raw[:8] != b'MNMPLIF1' or h[2:6] != (1,256,0x40209ca7,MAX_ROWS) or not (h[6]|h[7]) or any(h[8:]):
        raise ValueError('Primitive lifetime header')
    frequency = h[6] | h[7] << 32
    active = {}; calls = {}; boundaries = []; pending = None
    previous_time = previous_prefix = previous_queue = 0
    callback_by_record = {p['cadence_record']:p for p in callbacks} if callbacks is not None else None
    if callbacks is not None and len(callback_by_record) != len(callbacks):
        raise ValueError('Duplicate source callback')
    count = (len(raw)-64)//256
    for ordinal, r in enumerate(struct.iter_unpack('<64I', raw[64:]), 1):
        phase = r[1]; tick = r[6] | r[7] << 32
        if r[0] != ordinal or phase not in range(1,8) or not r[3] or any(r[14:16]) or any(r[60:]) or tick < previous_time or r[8] < previous_prefix or r[9] < previous_queue:
            raise ValueError('Primitive lifetime ordinal/clock/prefix/reserved')
        previous_time, previous_prefix, previous_queue = tick, r[8], r[9]
        if producer_records is not None and r[8] > len(producer_records):
            raise ValueError('Primitive lifetime exceeds producer journal')
        if pending is not None and len(pending['calls']) < pending['pending'] and phase != 6:
            raise ValueError('Missing pending callback call identities')
        if phase == 1:
            if r[2] != len(calls)+1 or r[2] in active or r[4] not in (*range(57),100) or r[59] or any(r[24:48]) or r[48] or any(r[49:51]):
                raise ValueError('Primitive entry identity/request/result')
            parent = max((i for i,c in active.items() if c['thread']==r[3]), default=0)
            if r[10] != parent:
                raise ValueError('Primitive parent lifetime')
            c = dict(id=r[2],thread=r[3],tag=r[4],caller=r[5],parent=r[10],ecx=r[16],edx=r[17],arguments=list(r[18:24]),phases=[],boundary_sequences=[])
            active[r[2]] = c; calls[r[2]] = c
        if phase in (1,2,3,4,6):
            c = active.get(r[2])
            if c is None or (r[3],r[4],r[5],r[10],r[16],r[17],list(r[18:24])) != (c['thread'],c['tag'],c['caller'],c['parent'],c['ecx'],c['edx'],c['arguments']):
                raise ValueError('Primitive lifetime identity changed')
            if r[11] > 1 or r[12] > 128 or r[59] not in (0,1) or not r[59] and any(r[24:48]):
                raise ValueError('Primitive request/depth envelope')
            if phase != 6:
                if phase != len(c['phases'])+1 or phase in (1,2) and r[48] or any(r[49:51]):
                    raise ValueError('Primitive phases/result/callback fields')
                c['phases'].append(dict(phase=phase,qpc=tick,producer_prefix=r[8],queue_prefix=r[9],depth=r[12],request=list(r[24:48]),request_present=bool(r[59]),result=r[48],canvas=list(r[51:59])))
                if phase == 3 and any(child['parent']==r[2] for child in active.values()):
                    raise ValueError('Primitive returned with an active nested child')
                if phase == 4:
                    if r[48] != c['phases'][2]['result']:
                        raise ValueError('Primitive original result changed')
                    request=r[24:48]
                    if r[59] and producer_records is not None:
                        emitted=request[1]
                        if not 1 <= emitted <= r[8] or tuple(producer_records[emitted-1][0]) != request:
                            raise ValueError('Primitive completed request differs from producer packet')
                    del active[r[2]]
            else:
                if pending is None or len(pending['calls']) >= pending['pending'] or (r[49],r[50],r[8],r[9]) != (pending['cadence_record'],pending['sequence'],pending['producer_prefix'],pending['queue_prefix']) or r[2] in pending['calls'] or r[48]:
                    raise ValueError('Pending callback identity/prefix')
                pending['calls'].append(r[2]);c['boundary_sequences'].append(r[50])
        elif phase == 5:
            if r[2] or any(r[4:6]) or any(r[10:13]) or any(r[16:49]) or any(r[51:]) or not r[49] or boundaries and (r[49] <= boundaries[-1]['cadence_record'] or (r[50]-boundaries[-1]['sequence'])&0xffffffff != 1):
                raise ValueError('Primitive callback summary')
            pending=dict(cadence_record=r[49],sequence=r[50],qpc=tick,producer_prefix=r[8],queue_prefix=r[9],pending=r[13],calls=[])
            boundaries.append(pending)
            if callback_by_record is not None:
                p=callback_by_record.get(r[49])
                if p is None or (r[50],r[3],r[8],r[9],r[13]) != (p['sequence'],p['thread'],p['producer_prefix'],p['queue_prefix'],p['pending_frames']):
                    raise ValueError('Primitive callback differs from source boundary')
        elif phase == 7:
            if ordinal != count or active or r[13] or r[2] or any(r[4:6]) or any(r[10:13]) or any(r[16:]):
                raise ValueError('Primitive terminal has pending calls or trailing data')
            if producer_records is not None and r[8] != len(producer_records):
                raise ValueError('Primitive terminal producer prefix')
        if r[13] != len(active):
            raise ValueError('Primitive active-call accounting')
        if phase == 6 and len(pending['calls']) == pending['pending'] and set(pending['calls']) != set(active):
            raise ValueError('Pending callback omits an active call')
    if phase != 7 or callback_by_record is not None and {b['cadence_record'] for b in boundaries} != set(callback_by_record):
        raise ValueError('Primitive closure or source callback denominator')
    spanning=[c for c in calls.values() if c['boundary_sequences']]
    for c in spanning:
        c['snapshot_acquisition_envelope_qpc']=[c['phases'][i]['qpc'] for i in (0,1)]
        c['original_execution_envelope_qpc']=[c['phases'][i]['qpc'] for i in (1,2)]
        c['completion_envelope_qpc']=[c['phases'][i]['qpc'] for i in (2,3)]
    result = dict(schema=1,success=True,journal_sha256=sha(raw),frequency=frequency,clock_domain='Windows_QueryPerformanceCounter',
                records=count,calls=len(calls),tag_counts=dict(Counter(str(c['tag']) for c in calls.values())),boundaries=boundaries,
                pending_boundary_calls=spanning,all_pending_calls_identified=True,producer_packets_crosschecked=producer_records is not None,
                source_callbacks_crosschecked=callbacks is not None,source_bytes_immutable=False,complete_presented_interval=False,native_parity_performed=False)
    if include_call_ids is not None:
        if not set(include_call_ids) <= calls.keys():
            raise ValueError('Requested primitive identity absent from lifetime journal')
        result['selected_calls'] = [calls[i] for i in sorted(set(include_call_ids))]
    return result


def analyze(directory):
    from drawing_source_callback_windows import analyze as source_windows
    spec=importlib.util.spec_from_file_location('primitive_producers',Path(__file__).with_name('inspect-canvas-producers.py'))
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    _,records=module.decode((directory/'canvas-producers.bin').read_bytes())
    result=decode((directory/'primitive-lifetimes-0000.bin').read_bytes(),records,source_windows(directory)['points'])
    from engine_cadence import decode as cadence
    frequency,_=cadence(directory/'engine-cadence.bin')
    if result['frequency']!=frequency:raise ValueError('Primitive/cadence QPC frequency differs')
    return result
