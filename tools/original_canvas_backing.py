"""Correlate real provider calloc/free generations with successful CPU Lock spans."""
import struct


def analyze(rows,raw):
    if len(raw)%24:raise ValueError('Truncated provider allocation event')
    events=list(struct.iter_unpack('<6I',raw))
    footer=[r for r in rows if r[0]==82]
    if len(footer)!=1 or footer[0][4]!=len(events):raise ValueError('Provider journal denominator differs')
    modules={r[2]:(r[3],r[4]) for r in rows if r[0]==81}
    if set(modules)!={0,1}:raise ValueError('Missing pinned provider module')
    generations=[];active={};free_calls={};untracked_frees=0
    for sequence,(op,phase,address,a,b,caller) in enumerate(events):
        module=op%10
        if module not in modules or op not in (100,101,110,111,120,121):raise ValueError('Unknown allocation event')
        base,size=modules[module]
        if not base<=caller<base+size:raise ValueError('Allocation call escaped pinned provider')
        if op<110:
            if not address:continue
            if address in active and active[address].get('free_entry') is None:raise ValueError('Live allocation address reused')
            item=dict(generation=len(generations)+1,module=module,address=address,bytes=a*b,calloc_count=a,calloc_size=b,
                allocation_event=sequence,allocation_phase=phase,caller_rva=caller-base)
            generations.append(item);active[address]=item
        elif op<120:
            item=active.pop(address,None);free_calls[sequence]=item
            if item is not None:item['free_entry']=sequence;item['free_phase']=phase
            else:untracked_frees+=bool(address)
        else:
            if a not in free_calls:raise ValueError('Free completion lacks its exact entry')
            item=free_calls.pop(a)
            if item is not None:
                if item['address']!=address:raise ValueError('Free identity changed')
                item['free_exit']=sequence
    if free_calls:raise ValueError('Incomplete provider free calls')
    locks=[];unmatched=[];matched=set();cohorts={}
    for row in rows:
        if row[0]!=84:continue
        _,phase,version,sequence,pitch,pointer=row
        if sequence>len(events) or not 16<=pitch<=4096:raise ValueError('Lock observation extent')
        span=(6-1)*pitch+16
        candidates=[g for g in generations if g['allocation_event']<sequence and
            ('free_exit' not in g or g['free_exit']>=sequence) and
            g['address']<=pointer and pointer+span<=g['address']+g['bytes']]
        if len(candidates)>1:raise ValueError('Ambiguous backing allocation')
        if not candidates:
            unmatched.append(dict(phase=phase,version=version,pointer=pointer,pitch=pitch,event=sequence));continue
        item=candidates[0];matched.add(item['generation']);cohorts.setdefault(phase,set()).add(item['generation'])
        locks.append(dict(phase=phase,version=version,generation=item['generation'],offset=pointer-item['address'],pitch=pitch,event=sequence))
    if not locks:raise ValueError('No actual provider allocations matched CPU backing')
    backing=[g for g in generations if g['generation'] in matched]
    for item in backing:
        if 'free_exit' not in item:raise ValueError('Observed CPU backing allocation has no completed real free')
    survives=0
    for phase,ids in cohorts.items():
        marks={r[2]:r[3] for r in rows if r[0]==83 and r[1]==phase}
        if set(marks)!=set(range(5)):raise ValueError('Incomplete cohort lifetime boundaries')
        for identity in ids:
            item=generations[identity-1]
            if item['allocation_event']<marks[2] and item['free_entry']>=marks[2]:survives+=1
    return dict(provider_allocation_events=len(events),observed_calloc_generations=len(generations),
        matched_cpu_backing_generations=backing,matched_lock_spans=locks,unmatched_lock_spans=unmatched,
        backing_generations_retained_after_wrapper_retirement=survives,
        all_matched_cpu_backing_real_frees_completed=True,untracked_preexisting_or_noncalloc_frees=untracked_frees,
        physical_gpu_allocation_or_all_writer_closure=False,
        scope_note='Real observed provider calloc blocks and completed free calls only. Generation identities come from allocation event order; numerical address reuse is not identity. Other allocator families, GDI/kernel/GPU allocations and all CPU writer instrumentation remain separate.')
