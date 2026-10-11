"""Review exact original CALLs and inherited return slots in closed core journals."""
from collections import Counter, defaultdict
import hashlib
import struct
from drawing_census import trace, validate_callers, owners, BUILD_SHA256
from drawing_census_bindings import decode, physical


def review(path, manifest, binding_path, caller_census, inventory):
    """Never rewrite raw callers or infer a CALL from an inherited continuation."""
    before=path.stat();binding_before=binding_path.stat()
    decoded=trace(path,manifest,caller_census,inventory)
    bindings=decode(binding_path,manifest)
    hooks={r['tag']:r for r in manifest['hooks']}
    callers=validate_callers(caller_census,BUILD_SHA256,{r['entry'] for r in hooks.values()})
    active={};slots=defaultdict(list);returns=defaultdict(list)
    counts=Counter();gaps=Counter();samples=defaultdict(list)
    with path.open('rb') as source:
        source.seek(64)
        for _ in range(decoded['events']):
            row=struct.unpack('<32I',source.read(128))
            sequence,event,cid,tag,phase,thread,entry,caller,sp=row[:9]
            hook=hooks[tag];pop=hook['pop']
            if type(pop) is not int or not 0<=pop<=0xffff or sp<=4+pop:raise ValueError('Invalid original return-slot extent')
            slot=sp-4-pop;slot_key=(thread,slot);return_key=(thread,sp)
            if event==1:
                if row[28]!=len(active)+1:raise ValueError('Core entry active-frame count mismatch')
                address=physical(caller,bindings)
                original=callsite=target=parent_entry=None;root=cid;depth=0
                kind='observed_callee_pending_call_flow_review'
                if address['kind']=='exact_core_trampoline_call_return':
                    original=address['original_call_return'];callsite=address['original_call_site'];target=address['target']
                    exact=callers.get(original)
                    if target==entry and exact and exact['entry']==entry and exact['call_va']==callsite:kind='exact_relocated_original_call'
                    else:
                        kind='relocated_call_pending_target_or_static_census_review';gaps[(kind,entry,caller)]+=1
                elif address['kind']=='exact_core_leave':
                    parent=active[slots[slot_key][-1]] if slots[slot_key] else None
                    if parent and parent['sp']==sp:
                        original=parent['continuation'];root=parent['root'];depth=parent['depth']+1;parent_entry=parent['entry']
                        kind='same_return_slot_inherited_continuation';gaps[('inherited_original_transfer_edge_unreviewed',entry,caller)]+=1
                        if original is None:gaps[('inherited_continuation_root_unresolved',entry,caller)]+=1
                    else:
                        kind='unmatched_core_leave';gaps[(kind,entry,caller)]+=1
                else:
                    exact=callers.get(caller)
                    if exact and exact['entry']==entry:
                        original=caller;callsite=exact['call_va'];target=entry;kind='exact_original_call'
                    else:gaps[(kind,entry,caller)]+=1
                key=(phase,entry,caller,kind,original,callsite,target,parent_entry,depth)
                active[cid]={'sp':sp,'slot_key':slot_key,'return_key':return_key,'continuation':original,'root':root,'depth':depth,'entry':entry}
                slots[slot_key].append(cid);returns[return_key].append(cid);counts[key]+=1
                if len(samples[key])<4:samples[key].append({'sequence':sequence,'call_id':cid,'thread':thread,'entry_return_slot':slot,'normalized_return_sp':sp,'continuation_root_call_id':root,'inherited_parent_call_id':slots[slot_key][-2] if parent_entry is not None else None})
            elif event==2:
                frame=active.get(cid)
                if not frame or not returns[return_key] or returns[return_key][-1]!=cid or slots[frame['slot_key']][-1]!=cid:raise ValueError('Core return is not newest matching thread/stack frame')
                returns[return_key].pop();slots[frame['slot_key']].pop();del active[cid]
                if row[28]!=len(active):raise ValueError('Core return active-frame count mismatch')
            elif row[28]!=len(active):raise ValueError('Core SITE active-frame count mismatch')
    if active or sum(counts.values())!=decoded['paired_calls']:raise ValueError('Core continuation review has open frames or wrong pair count')
    with path.open('rb') as source:digest=hashlib.file_digest(source,'sha256').hexdigest()
    with binding_path.open('rb') as source:binding_digest=hashlib.file_digest(source,'sha256').hexdigest()
    after=path.stat();binding_after=binding_path.stat()
    def stable(a,b):return (a.st_ino,a.st_size,a.st_mtime_ns)==(b.st_ino,b.st_size,b.st_mtime_ns)
    if not stable(before,after) or not stable(binding_before,binding_after) or digest!=decoded['sha256'] or binding_digest!=bindings['sha256']:raise ValueError('Core journal/bindings changed during caller review')
    rows=[]
    for key,hits in sorted(counts.items(),key=lambda pair:repr(pair[0])):
        phase,entry,caller,kind,original,callsite,target,parent_entry,depth=key
        rows.append({'phase':phase,'entry':entry,'physical_caller':caller,'classification':kind,'original_return_continuation':original,'original_call_site':callsite,'original_call_target':target,'inherited_parent_entry':parent_entry,'inherited_depth':depth,'count':hits,'continuation_discovered_owners':owners(original-1,inventory) if original is not None else [],'proof_samples':samples[key]})
    return {'schema':1,'integrity':True,'original_sha256':BUILD_SHA256,'journal_sha256':digest,'bindings_sha256':binding_digest,'paired_calls':decoded['paired_calls'],'site_events':decoded['site_events'],'counts':rows,'gaps':[{'reason':reason,'entry':entry,'physical_caller':pc,'count':n} for (reason,entry,pc),n in sorted(gaps.items())],'exact_relocated_pairs':sum(n for k,n in counts.items() if k[3]=='exact_relocated_original_call'),'exact_original_pairs':sum(n for k,n in counts.items() if k[3]=='exact_original_call'),'inherited_continuation_pairs':sum(n for k,n in counts.items() if k[3]=='same_return_slot_inherited_continuation'),'caller_attribution_complete':False,'draw01_complete':False,'scope':'Independent closed-journal pairing, active-count/newest-return and exact per-run core binding review. Physical callers remain intact. A same-thread/same-original-return-slot/same-cleanup inherited continuation identifies the observed parent/root, not an original CALL edge to the child or semantic owner. Discovered continuation ranges do not fill unknown functions/flows; unresolved indirect/tail callers, writers and scenario/dispatch closure remain pending.'}
