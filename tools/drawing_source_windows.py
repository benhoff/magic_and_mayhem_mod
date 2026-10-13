"""Temporal containment of diagnostic source reads in accepted update intervals."""
from bisect import bisect_right


def bind(samples,windows,final_qpc,owner):
    starts=[w['qpc'] for w in windows]
    if not starts or starts!=sorted(set(starts)) or final_qpc<=starts[-1]:
        raise ValueError('Source/update interval boundaries')
    ids=[s['submission'] for s in samples]
    if len(ids)!=len(set(ids)):raise ValueError('Duplicate temporal source identity')
    ends=starts[1:]+[final_qpc];assignments=[];crossings=[];outside=[];counts={w['sequence']:0 for w in windows}
    for s in samples:
        begin,end=s.get('qpc_before'),s.get('qpc_after')
        if type(begin) is not int or type(end) is not int or not 0<begin<=end<2**64:
            raise ValueError('Missing/invalid source performance-counter interval')
        index=bisect_right(starts,begin)-1
        identity=dict(submission=s['submission'],status=s['status'],source_thread=s['thread'],qpc_before=begin,qpc_after=end)
        if index<0 or end>=final_qpc:
            outside.append(dict(identity,reason='before_first_interval' if index<0 else 'at_or_after_final_boundary'));continue
        if end>=ends[index]:
            crossings.append(dict(identity,sequence_at_start=windows[index]['sequence'],boundary=ends[index]));continue
        w=windows[index];counts[w['sequence']]+=1
        assignments.append(dict(identity,sequence=w['sequence'],counter_before=w['counter_before'],begin_record=w['begin_record'],interval_start=starts[index],interval_end=ends[index],captured_on_counter_owner=s['thread']==owner,observed_queue_records=[q['record'] for q in w['queues']]))
    return dict(schema=1,success=True,source_samples=len(samples),assigned_samples=len(assignments),crossing_samples=len(crossings),outside_samples=len(outside),assignments=assignments,crossings=crossings,outside=outside,intervals=[dict(sequence=w['sequence'],begin=w['qpc'],end=end,source_samples=counts[w['sequence']],observed_queues=len(w['queues']),advancing=w['advancing']) for w,end in zip(windows,ends)],all_samples_classified=True,all_samples_contained=bool(samples) and not crossings and not outside,producer_identity_bound=False,worker_identity_bound=False,all_presentations_in_update_interval_retained=False,complete_presented_journey=False)
