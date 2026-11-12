"""Exact source identities and conditional obligations for caller-state replay.

This module joins observations. State-model semantics and independent pixel
history require their separate current original comparisons; a join is no oracle.
"""
import hashlib
import struct

FULL_MODELS={10:'glyph',12:'generic',13:'generic',14:'generic',15:'generic',16:'generic',
             22:'word',23:'generic',24:'generic',26:'add',36:'word'}
FAST={17,18,19,20,21,25,57,58,59,60}


def require(value,message):
    if not value:raise ValueError(message)


def obligations(row,requirements,bindings):
    """Use full recovered state where available; never invent volatile masks."""
    tag=row['tag']
    if tag in FULL_MODELS:
        return dict(model=FULL_MODELS[tag],eax=0xffffffff,ecx=0xffffffff,edx=0xffffffff,
                    eflags=0xcc5 if tag in(12,13,14,15,16,22,23,24,36)else 0xcd5,
                    conditional_callees=False)
    require(tag in FAST or tag==52,'Unregistered native state-model family')
    site=next((s for s in requirements['sites']if s['address']==row['caller']),None)
    require(site is not None,'Caller is absent from the pinned continuation inventory')
    if tag==17:
        require(site.get('signature','').startswith('83c414')and len(row['before_arguments'])==5,
                'Fast cdecl scratch arguments lack the exact immediate20-byte cleanup')
    candidates=[(site['analyses'],False)]
    for context in site.get('contextual',[]):
        if context['context_kind']==row['context_kind']and row['enclosing_caller']in context['returns']:
            candidates.append((context['analyses'],False))
    paired=bindings.get(row['id'])
    facts=requirements.get('conditional_callee_requirements',{})
    if paired and paired['word_target']in facts.get('word_targets',[])and paired['generic_target']in facts.get('generic_targets',[]):
        for context in site.get('callee_summary_contextual',[]):
            if context['context_kind']==row['context_kind']and row['enclosing_caller']in context['returns']:
                candidates.append((context['analyses'],True))
    for analyses,conditional in candidates:
        if any(not a['closed']or a['escapes']for a in analyses.values()):continue
        masks={reg:analyses[reg]['required'].get(reg,0)for reg in('eax','ecx','edx','eflags')}
        if masks['eax']&~0xffff or masks['ecx']or masks['edx']or masks['eflags']:continue
        # Arithmetic flags may be dead, while DF remains a preserved ABI input.
        masks['eflags']=0x400
        return dict(model='colour'if tag==52 else'fast',**masks,conditional_callees=conditional)
    raise ValueError('Unresolved consumed volatile state or unmatched conditional caller/selector')


def source_inputs(row):
    args=row['before_arguments'];regs=row['before_registers'];tag=row['tag']
    if tag in(26,52):return dict(x=args[0],y=args[1],frame=None)
    if tag in(10,11,18,19,20,21,25,57,58,59,60):return dict(x=regs[5],y=args[0],frame=regs[6])
    return dict(x=args[1],y=args[2],frame=args[0])


def source_match(row,fields,payload):
    """Compare semantic inputs too; sequence equality alone cannot join a draw."""
    expected=8 if row['tag']==10 else 14 if row['tag']==26 else 26 if row['tag']==52 else 9
    if fields[2]!=expected or fields[22]!=row['caller']:return False
    inputs=source_inputs(row)
    if (fields[8],fields[9])!=(inputs['x'],inputs['y']):return False
    environment=row.get('environment')
    if environment is None or environment['stride']!=fields[7]:return False
    if fields[2]in(8,9):
        if len(payload)!=fields[19]+fields[20]or len(payload)<40 or any(payload[28:32]):return False
        if struct.unpack_from('<I',payload)[0]!=fields[19]:return False
    if fields[2]==8:
        if tuple(fields[15:18])!=tuple(row['before_arguments'][1:4]):return False
        if len(payload)<fields[19]+272 or struct.unpack_from('<I',payload,fields[19])[0]!=environment['format']:return False
    return True


def profile_operations(blocks,decoder):
    """Keep the actual World and source-prefix namespace through sequence resets."""
    result={};last_generation=0;last_chunk=0
    for generation,chunk,stream,request in blocks:
        require(generation==last_generation+1,'Foreign or noncontiguous World identity')
        if chunk:
            require(chunk==last_chunk+1 and not request,'Foreign source-prefix identity')
            source_identity=chunk;last_chunk=chunk
        else:
            source_identity=last_chunk+1;last_generation=generation
        _,operations=decoder(stream,complete=False)
        queue=0
        for fields,payload in operations:
            if fields[2]==11:queue=fields[14]
            if fields[2]not in(8,9,14,26):continue
            key=(generation,source_identity,fields[1])
            require(key not in result,'Duplicate source identity')
            result[key]=dict(generation=generation,chunk=chunk,source=source_identity,queue=queue,
                fields=fields,payload=payload,source_sha256=hashlib.sha256(payload).hexdigest())
    require(last_generation>0,'No complete World in source history')
    return result


def caller_inventory(requirements):
    """Classify every whitelisted site/tag; conditional contexts stay explicit."""
    inventory=[]
    for site in requirements['sites']:
        if site['table']=='trail_raster_sites':tags=[57]
        else:
            mask=int(site['metadata'].strip(',').split(',')[-1].replace('ull',''),0)
            tags=[t for t in range(64)if mask&1<<t]
        for tag in tags:
            if tag in FULL_MODELS:
                inventory.append(dict(caller=site['address'],tag=tag,model=FULL_MODELS[tag],required_full_state=True));continue
            contexts=[dict(context_kind=0,enclosing_caller=0)]
            contexts += [dict(context_kind=c['context_kind'],enclosing_caller=pc)
                for c in site.get('contextual',[])+site.get('callee_summary_contextual',[])for pc in c['returns']]
            proofs=[]
            for context in contexts:
                row=dict(id=1,tag=tag,caller=site['address'],before_arguments=(0,)*5,**context)
                try:proof=obligations(row,requirements,{1:dict(word_target=0x597086,generic_target=0x5947b2)})
                except ValueError:continue
                proofs.append(dict(context=context,obligations=proof))
            require(proofs,'Whitelisted caller has unresolved required output: '+hex(site['address']))
            inventory.append(dict(caller=site['address'],tag=tag,model='colour'if tag==52 else'fast',contexts=proofs,
                scope='Conditional caller/selector contexts are required at actual publication; listed contexts do not assert natural reachability.'))
    require(inventory,'Empty caller inventory')
    return dict(sites=len(requirements['sites']),entries=inventory,verified=True)


def join_native(rows,operations,requirements,bindings,nonce):
    """V5 completion identities bind source publication, including glyph drains."""
    used=set();joined=[];noops=[]
    for row in rows:
        if row['kind']!=1:continue
        epoch=row.get('epoch')
        require(epoch is not None and epoch['nonce']==nonce,'Missing or foreign state session nonce')
        require(row.get('environment')is not None,'Missing state replay inputs')
        key=(epoch['generation_after'],epoch['source_after'],row['sequence_after'])
        operation=operations.get(key)
        if row['tag']==52 and (operation is None or not source_match(row,operation['fields'],operation['payload'])):
            # This is only the source-bound literal no-op branch. The full
            # collector/checker must also verify exact module/source identity,
            # closed complete producer history and required low-AX publication.
            require(row['sequence_before']==row['sequence_after']and epoch['world_before']and epoch['world_after'],
                    'A missing colour source is not a guarded no-op')
            proof=obligations(row,requirements,bindings)
            require(not(row['after_registers'][7]&0xffff),'No-op colour AX differs')
            require(all(row['before_registers'][i]==row['after_registers'][i]for i in(0,1,2,4)),
                    'No-op colour changed a preserved register')
            require(not((row['before_registers'][8]^row['after_registers'][8])&0x400),
                    'No-op colour changed the direction flag')
            require(row['before_fx']==row['after_fx']and row['before_workspace']==row['after_workspace']
                    and row['last_error_before']==row['last_error_after'],'No-op colour changed preserved state')
            noops.append(dict(id=row['id'],obligations=proof));continue
        require(operation is not None and key not in used,'Missing or reused state/source publication')
        require(source_match(row,operation['fields'],operation['payload']),'State/source caller or arguments differ')
        require(operation['queue']==row['queue'],'State/source queue differs')
        require(bool(operation['queue'])==epoch['world_after'],'State/source World-open identity differs')
        proof=obligations(row,requirements,bindings)
        used.add(key);joined.append(dict(row=row,operation=operation,obligations=proof,key=key))
    require(joined,'No native state/source observations')
    return dict(joined=joined,noops=noops,used=used,
                scope='Native publication joins only. Retained/nested observations, original model semantics, provider contracts, independent pixels and retirement need separate checks.')


def retained_census(rows):
    """Account for original calls, including native children, without an ABI shortcut.

    A retained original call is not a new implementation. Its composition proof
    depends on every native child's consumed-state obligation and independent
    pixels; this census alone supplies neither proof.
    """
    positions={r['id']:i for i,r in enumerate(rows)}
    require(len(positions)==len(rows),'Duplicate completion in retained census')
    parents=[];children={r['id']:[]for r in rows};stacks={}
    for row in sorted(rows,key=lambda r:r['id']):
        stack=stacks.setdefault(row['thread'],[])
        while stack and positions[stack[-1]['id']]<positions[row['id']]:stack.pop()
        if stack:
            parent=stack[-1]
            require(row['entry_sp']<parent['entry_sp']and row['return_sp']<=parent['entry_sp'],
                    'Overlapping original/native call intervals without stack nesting')
            children[parent['id']].append(row)
        stack.append(row)
    for parent in rows:
        if parent['kind']!=2:continue
        require(all(parent['before_registers'][i]==parent['after_registers'][i]for i in(0,1,2,4)),
                'Retained original changed a nonvolatile register')
        require(not((parent['before_registers'][8]^parent['after_registers'][8])&0x400),
                'Retained original changed the direction flag')
        require(parent['last_error_before']==parent['last_error_after'],'Retained original changed LastError')
        parents.append(dict(id=parent['id'],tag=parent['tag'],caller=parent['caller'],
            native_children=[r['id']for r in children[parent['id']]if r['kind']==1],
            retained_children=[r['id']for r in children[parent['id']]if r['kind']==2]))
    return dict(retained=len(parents),native=sum(r['kind']==1 for r in rows),
                observations=len(rows),parents=parents,accounted=True,
                scope='Unmodified retained original entries plus explicitly nested children. Semantic equivalence requires current original model proofs and every native consumed-state/source and independent pixel comparison.')


def join_retained_sources(rows,operations,native_used,census,nonce):
    used=set(native_used);roots=[];nested=[]
    parent_of={child:p['id']for p in census['parents']for child in p['retained_children']}
    for row in rows:
        if row['kind']!=2:continue
        epoch=row.get('epoch')
        require(epoch is not None and epoch['nonce']==nonce,'Foreign retained original session')
        key=(epoch['generation_after'],epoch['source_after'],row['sequence_after'])
        op=operations.get(key)
        if op is not None and key not in used and source_match(row,op['fields'],op['payload']):
            require(op['queue']==row['queue']and bool(op['queue'])==epoch['world_after'],
                    'Retained original source queue/World differs')
            used.add(key);roots.append(dict(id=row['id'],tag=row['tag'],key=key));continue
        require(row['id']in parent_of,'Unmatched retained original root source')
        nested.append(dict(id=row['id'],parent=parent_of[row['id']],tag=row['tag']))
    require(used==set(operations),'Unaccounted primitive source without an actual entry/return observation')
    return dict(verified=True,roots=roots,nested=nested,primitive_sources=len(used),
        scope='Every primitive source has one actual root completion. Nested unchanged original calls retain explicit parents; their interior native splices require the separate current original/kernel comparison. This is accounting, not a synthesized state comparison.')


def validate_model_results(raw,joined):
    """Recompute comparisons from expected bytes, independently of service flags."""
    require(len(raw)==16+380*len(joined)and raw[:8]==b'MNMCSO01'
        and struct.unpack_from('<2I',raw,8)==(1,len(joined)),'Model result stream extent/header')
    by_id={j['row']['id']:j for j in joined};seen=set();counts={}
    for at in range(16,len(raw),380):
        header=struct.unpack_from('<8I',raw,at);size,ident,model=header[:3]
        require(size==380 and ident in by_id and ident not in seen and 1<=model<=6,'Foreign/reused model result')
        seen.add(ident);j=by_id[ident];row=j['row'];proof=j['obligations']
        require(model=={'word':1,'generic':2,'add':3,'glyph':4,'fast':5,'colour':6}[proof['model']],'Different state model')
        expected=struct.unpack_from('<9I',raw,at+32)
        masks=(0xffffffff,)*5+(proof['edx'],proof['ecx'],proof['eax'])
        gpr=all(not((a^b)&mask)for a,b,mask in zip(expected,row['after_registers'],masks))
        arguments=model==5 or row['after_arguments']is None or tuple(row['after_arguments'])==struct.unpack_from('<6I',raw,at+68)[:len(row['after_arguments'])]
        workspace=tuple(row['after_workspace'])==struct.unpack_from('<16I',raw,at+92)
        flag_mask=proof['eflags']|(0x10 if model==2 and expected[7]else 0)
        flags=not((expected[8]^row['after_registers'][8])&flag_mask)
        from drawing_caller_state import fx_state
        floating=fx_state(raw[at+156:at+380])==row['after_fx']and row['last_error_before']==row['last_error_after']
        checks=tuple(map(int,(gpr,arguments,workspace,flags,floating)))
        require(checks==header[3:],'Service summary differs from independently compared expected bytes')
        require(all(checks),'Actual publication differs from recovered required caller state: '+str(ident))
        counts[proof['model']]=counts.get(proof['model'],0)+1
    require(seen==set(by_id),'Missing publication result')
    return dict(verified=True,cases=len(joined),models=counts,independent_expected_byte_comparison=True)
