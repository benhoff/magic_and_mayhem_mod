"""Match reviewed fixed-wrapper tail entries; retain unknown flag inputs."""
from collections import Counter, defaultdict
import hashlib
import struct
from drawing_api_bindings import decode
from drawing_census import BUILD_SHA256
from drawing_cpu_crosswalk import review as direct_review, _rows

WRAPPERS = {0x4bdd10:(0x6a49e0,0), 0x4bdcb0:(0x6a4fe0,0),
            0x4a2f00:(0x641fc8,1), 0x4a2f70:(0x642000,2)}
PROOF_SHA256 = 'db70f684437cbbf8cfc94a0201cb830e7b88dd1830281e242d1f36e1a0ea5181'


def contracts(proof, staged, image_offset):
    if hashlib.sha256(proof.read_bytes()).hexdigest()!=PROOF_SHA256:
        raise ValueError('Terminal original range proof differs from reviewed execution')
    import json
    record=json.loads(proof.read_text())
    if record['original_sha256']!=BUILD_SHA256 or not record['success'] or not record['original_wrapper_instructions_unchanged']:
        raise ValueError('Terminal proof lacks unchanged original execution')
    for target,(receiver,bit) in WRAPPERS.items():
        raw=bytes.fromhex(record['original_entry_anchors'][hex(target)])
        if not bit:
            expected=b'\xb9'+struct.pack('<I',receiver)+b'\xe9'+struct.pack('<I',(0x58b3e0-target-10)&0xffffffff)
        else:
            expected=(b'\x8a\x0d\x70\x1a\x64\x00\xb0'+bytes([bit])+b'\x84\xc8\x75\x12\x0a\xc8\x88\x0d\x70\x1a\x64\x00\xb9'+
                struct.pack('<I',receiver)+b'\xe9'+struct.pack('<I',(0x58b3e0-target-30)&0xffffffff)+b'\xc3')
        if raw!=expected:raise ValueError('Terminal exact fixed receiver/flag/tail bytes differ')
        offset=image_offset(staged,target,len(raw))
        if staged[offset:offset+len(raw)]!=raw:raise ValueError('Retained staged wrapper bytes differ from original range proof')
    return WRAPPERS


def match(cpu_calls, core_calls, physical_return, wrapper_contracts):
    """Complete per-thread/stack/wrapper groups only, with exact state checks."""
    if wrapper_contracts!=WRAPPERS:raise ValueError('Unreviewed wrapper contract map')
    cpu_groups=defaultdict(list);core_groups=defaultdict(list);gaps=Counter()
    receivers={receiver:target for target,(receiver,_) in wrapper_contracts.items()}
    if len(receivers)!=len(wrapper_contracts):raise ValueError('Wrapper receiver identities are ambiguous')
    for start,end in cpu_calls:
        if start[6]==0x59d98a and start[16] in wrapper_contracts:
            cpu_groups[(start[5],start[7],start[16])].append((start,end))
    for start,end in core_calls:
        if start[6]!=0x58b3e0 or start[7]!=physical_return:continue
        target=receivers.get(start[9])
        if target is None:gaps[('receiver_has_no_reviewed_wrapper',start[7])]+=1;continue
        core_groups[(start[5],start[8],target)].append((start,end))
    results=[];matched=0
    for key,core_group in sorted(core_groups.items()):
        cpu_group=cpu_groups.get(key,[])
        if not cpu_group or len(cpu_group)!=len(core_group):
            gaps[('complete_wrapper_group_count_mismatch',key[2])]+=len(core_group);continue
        cpu_group.sort(key=lambda pair:pair[0][0]);core_group.sort(key=lambda pair:pair[0][0])
        receiver,bit=wrapper_contracts[key[2]];samples=[]
        for (ps,pe),(cs,ce) in zip(cpu_group,core_group):
            if tuple(ps[14:17])!=(1,0,key[2]) or ps[10]!=key[2] or not ps[17] or not ps[34]:
                raise ValueError('Wrapper crosswalk lacks exact readable register CALL target')
            if cs[4]!=ps[4] or pe[7]!=ps[7] or ce[8]!=cs[8] or cs[8]!=ps[7]:
                raise ValueError('Wrapper phase/RET0/stack mismatch')
            expected_eax=ps[10] if not bit else (ps[10]&0xffffff00)|bit
            if cs[9]!=receiver or cs[10]!=ps[9] or cs[11]!=expected_eax or cs[13]!=ps[12] or cs[15:21]!=ps[18:24]:
                raise ValueError('Wrapper exact fixed receiver/register/error/stack effects mismatch')
            if not bit:
                if cs[12]!=ps[11]:raise ValueError('MOV ECX/JMP wrapper changed flags')
            else:
                # TEST/OR replace arithmetic flags. OR with a set bit is nonzero;
                # AF is undefined, and PF/SF depend on the uncaptured flag byte.
                mask=0x8d5
                if cs[12]&0x841 or cs[12]&~mask!=ps[11]&~mask:
                    raise ValueError('Conditional wrapper arithmetic/nonarithmetic flag effects mismatch')
            if ce[9:14]!=pe[8:13]:raise ValueError('Tail callee return differs from CPU callback AFTER')
            if len(samples)<4:samples.append({'thread':cs[5],'cpu_call_id':ps[2],'core_call_id':cs[2],
                'cpu_before_sequence':ps[0],'core_entry_sequence':cs[0],
                'core_return_sequence':ce[0],'cpu_after_sequence':pe[0],
                'original_return_slot':ps[7]-4,'cpu_before_eax':ps[10],
                'core_entry_eax':cs[11],'core_entry_ecx':cs[9]})
            matched+=1
        results.append({'original_cpu_call_site':0x59d98a,'original_call_return':0x59d98c,
            'physical_call_return':physical_return,'observed_wrapper_target':key[2],
            'reviewed_tail_target':0x58b3e0,'fixed_receiver':receiver,'one_shot_bit':bit,
            'count':len(core_group),'proof_samples':samples,'initial_flag_byte_observed':False})
    return {'matched_wrapper_tail_pairs':matched,'counts':results,
        'gaps':[{'reason':reason,'identity':identity,'count':n} for (reason,identity),n in sorted(gaps.items())]}


def review(core_path, core_manifest, core_binding_path, cpu_path, cpu_manifest,
           cpu_binding_path, cohort_manifest, caller_census, inventory, wrapper_contracts):
    before=[p.stat() for p in (core_path,cpu_path,cpu_binding_path,core_binding_path)]
    direct=direct_review(core_path,core_manifest,core_binding_path,cpu_path,cpu_manifest,
        cpu_binding_path,cohort_manifest,caller_census,inventory)
    hooks=[r for r in core_manifest['hooks'] if r['entry']==0x58b3e0]
    if len(hooks)!=1 or hooks[0]['pop']!=0 or hooks[0]['site']:raise ValueError('Wrapper target needs exact core RET0 entry')
    bindings=decode(cpu_binding_path,cpu_manifest)
    exact=[r for r in bindings['hooks'] if r['site']==0x59d98a]
    if len(exact)!=1:raise ValueError('Missing exact CRT CALL return binding')
    def pairs(path,words,events):
        active={};out=[]
        for row in _rows(path,words,events):
            if row[1]==1:active[row[2]]=row
            elif row[1]==2:out.append((active.pop(row[2]),row))
        return out
    # Core SITE records are possible; use the validated terminal's event count.
    with core_path.open('rb') as source:source.seek(-64,2);events=struct.unpack('<16I',source.read(64))[3]
    core=pairs(core_path,32,events)
    cpu=pairs(cpu_path,200,direct['cpu_paired_calls']*2)
    result=match(cpu,core,exact[0]['physical_call_return'],wrapper_contracts)
    paths=(core_path,cpu_path,cpu_binding_path,core_binding_path)
    expected=(direct['core_journal_sha256'],direct['cpu_journal_sha256'],direct['cpu_bindings_sha256'],direct['core_bindings_sha256'])
    for path,initial,digest in zip(paths,before,expected):
        with path.open('rb') as source:actual=hashlib.file_digest(source,'sha256').hexdigest()
        final=path.stat()
        if actual!=digest or (initial.st_ino,initial.st_size,initial.st_mtime_ns)!=(final.st_ino,final.st_size,final.st_mtime_ns):raise ValueError('Terminal journal/bindings changed during review')
    return {'schema':1,'integrity':True,'original_sha256':BUILD_SHA256,**result,
        'direct_cpu_core_pairs':direct['matched_direct_core_pairs'],
        'pending_cpu_pairs':direct['pending_cpu_pairs']-result['matched_wrapper_tail_pairs'],
        'caller_attribution_complete':False,'draw01_complete':False,
        'scope':'Current native replay of independently closed CPU/core journals: exact CRT return/thread/stack '
            'and complete ordered wrapper groups with reviewed unchanged original fixed-receiver/bit/tail bytes, '
            'exact GPR/error/diagnostic effects and identical callee/CPU return state. The initial one-shot byte '
            'is not observed; no shared sequence, atomic read, callback registration, outer cleanup root flow, '
            'physical lifetime/pixels, complete callers/writers or scenario/dispatch closure is asserted.'}
