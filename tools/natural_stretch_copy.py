"""Bind natural stretched calls to an independent checkpoint and GPU readback."""
import hashlib,json,struct
from collections import defaultdict
from pathlib import Path
import numpy as np
from rendering_pilot import History,commands,require
from rendering_capture import capture
from rendering_menu_acceptance import artifact
from rendering_copy_readiness import signed
from native_copy_routes import decode,validate


def digest(plane):
    return hashlib.sha256(plane.astype('<u4').tobytes()).hexdigest()


def expected_stretch(source,before,sr,dr,key):
    l,t,r,b=sr;x,y,right,bottom=dr
    require(0<=l<r<=source.shape[1] and 0<=t<b<=source.shape[0]
            and 0<=x<right<=before.shape[1] and 0<=y<bottom<=before.shape[0], 'Stretch extent')
    xx=l+np.arange(right-x,dtype='int64')*(r-l)//(right-x)
    yy=t+np.arange(bottom-y,dtype='int64')*(b-t)//(bottom-y)
    sampled=source[np.ix_(yy,xx)].copy()
    result=before.copy();target=result[y:bottom,x:right]
    mask=np.ones(sampled.shape,dtype=bool) if key is None else sampled!=key
    target[mask]=sampled[mask]
    return result,dict(changed_pixels=int(np.count_nonzero(result!=before)),
        preserved_key_pixels=int(np.count_nonzero(~mask)),
        exterior_pixels=int(before.size-target.size))


def verify_gpu_checkpoint(proof,diagnostic,pixels):
    role=proof['gpu_plane']
    require(role in ('source','before'),'Unknown GPU checkpoint plane')
    prefix='source' if role=='source' else 'target'
    require(diagnostic['words'][0 if role=='source' else 1]==proof['target']
            and [diagnostic[prefix+'_width'],diagnostic[prefix+'_height']]==proof['target_size']
            and diagnostic[role+'_sha256']==proof['expected_sha256']==digest(pixels),
            'Actual GPU checkpoint differs from independent stretch result')


def analyze(reader,acquired,scratch):
    require(acquired['success'] and acquired['sources_stable'] and acquired['original_game_runs']==1
            and acquired['private_wine_stopped'] and acquired['prefix_pool']['returned']
            and acquired['original_manifest_verified_before_after'], 'Complete normally retired acquisition required')
    scratch=Path(scratch);scratch.mkdir(parents=True,exist_ok=True)
    lock=scratch/'lock-capture';lock.mkdir(exist_ok=True)
    path=lock/'copy-routes-v2.bin';path.write_bytes(artifact(reader,acquired,'copy-routes-v2.bin'))
    (lock/'gpu-copy-census-v1.bin').write_bytes(artifact(reader,acquired,'gpu-copy-census-v1.bin'))
    routes=validate(scratch)
    h,rows=decode(path)
    require(h[2]==2,'V2 call binding required')
    selected=[(i,r) for i,r in enumerate(rows) if r[15]==0 and r[13]==0 and r[32]==0x4cd0c222]
    require(selected,'No successful natural stretch')
    mapping=defaultdict(set)
    for r in rows:
        if r[40]:
            mapping[r[6]].add(r[43]);mapping[r[7]].add(r[42])
    streams=[a for a in acquired['artifacts'] if '/capture/commands-' in a['path'] and a['path'].endswith('.bin')]
    require(len(streams)==1,'One command lifetime required')
    records=capture(reader.read_bytes(streams[0]['record']))
    require(records[-1][0]['complete'] and records[-2][0]['kind']=='ended','Complete command close/END required')
    command_bytes=b''.join(body for row,body in records if row['kind']=='fragment')
    command_list=list(commands(command_bytes));by_seq={s:(o,p) for o,s,p in command_list}
    owners=set()
    for r in rows:
        if not r[40]:continue
        for seq in range(r[40],r[41]+1):
            require(seq not in owners and seq in by_seq and by_seq[seq][0]==3
                    and struct.unpack_from('<2I',by_seq[seq][1])==tuple(r[42:44]),
                    'Canonical resource mapping lacks unique emitted COPY identity')
            owners.add(seq)
    diagnostic=json.loads(artifact(reader,acquired,'copy-diagnostics/report.json'))
    require(diagnostic['selection']=='all-copies' and diagnostic['ended'] and not diagnostic['error']
            and diagnostic['original_pixels_used_as_native_inputs'] is False,'Complete all-copy readbacks required')
    checks={}
    for line in artifact(reader,acquired,'copies.jsonl').splitlines():
        d=json.loads(line);s=d['sequence'];w=d['words']
        require(type(s)is int and s not in checks and len(w)==13 and not any(w[10:])
                and not d['mismatches'] and d['actual_sha256']==d['expected_sha256']
                and d['pixels']==d['target_width']*d['target_height'],'Invalid GPU comparison')
        require(s in by_seq and by_seq[s][0]==3 and by_seq[s][1]==struct.pack('<10I',*w[:10]), 'GPU diagnostic command identity')
        for key in ('source_sha256','before_sha256','actual_sha256','expected_sha256'):
            require(isinstance(d[key],str) and len(d[key])==64 and all(c in '0123456789abcdef' for c in d[key]),'Malformed digest')
        checks[s]=d
    cost=sum((d['source_width']*d['source_height']+2*d['pixels'])*4 for d in checks.values())
    require(diagnostic['copies_checked']==diagnostic['copies_enqueued']==len(checks)
            and diagnostic['copies_seen']==sum(op==3 for op,_,_ in command_list)>=len(checks)
            and diagnostic['diagnostic_native_readbacks']==3*len(checks)
            and diagnostic['pixels_compared']==sum(d['pixels'] for d in checks.values())
            and diagnostic['diagnostic_readback_bytes']==cost<=32*1024**3
            and len(checks)<=8192,'Incomplete diagnostic denominator')
    checkpoints={};readbacks={};proofs=[]
    for i,r in selected:
        sr=list(map(signed,r[24:28]));dr=list(map(signed,r[28:32]))
        require(r[1]==0x58cc63 and r[10]&64 and not r[10]&128 and r[11]==0
                and r[12]==0x1008000 and r[18]==r[21]==16 and not r[23]
                and (sr[2]-sr[0],sr[3]-sr[1])!=(dr[2]-dr[0],dr[3]-dr[1]),'Unexpected natural stretch contract')
        require(len(mapping[r[6]])==len(mapping[r[7]])==1,'Ambiguous resource incarnation')
        target=next(iter(mapping[r[6]]));source=next(iter(mapping[r[7]]))
        prev=next((x for x in reversed(rows[:i]) if x[40]),None)
        nxt=next((x for x in rows[i+1:] if x[40]),None)
        require(prev and nxt and prev[0]==r[0]-1 and nxt[0]==r[0]+1
                and nxt[40]==prev[41]+2 and nxt[40]==nxt[41]
                and target in nxt[42:44], 'Stretch lacks unique adjacent checkpoint/readback')
        seq=prev[41]+1
        require(seq not in checkpoints and by_seq[seq][0]==2
                and struct.unpack_from('<5I',by_seq[seq][1])==(target,0,0,r[19],r[20]),'Unique full destination UPDATE required')
        require(nxt[40] in checks,'Missing actual GPU source readback')
        proof=dict(call=r[0],caller=hex(r[1]),source=source,target=target,
            source_rect=sr,target_rect=dr,source_key=r[22],checkpoint=seq,gpu_copy=nxt[40],hresult=0,
            source_size=[r[16],r[17]],target_size=[r[19],r[20]],
            gpu_plane='source' if nxt[42]==target else 'before')
        checkpoints[seq]=(r,proof);readbacks[nxt[40]]=proof;proofs.append(proof)
    history=History({});births={}
    for op,seq,payload in command_list:
        if op==1:births[struct.unpack_from('<I',payload)[0]]=seq
        if seq in checkpoints:
            r,proof=checkpoints[seq];source=history.handles[proof['source']];target=history.handles[proof['target']]
            require(source['bits']==target['bits']==16 and source['masks']==target['masks']==(0xf800,0x7e0,31)
                    and source['pixels'].shape==(r[17],r[16]) and target['pixels'].shape==(r[20],r[19]),'Independent source format/extent')
            expected,counts=expected_stretch(source['pixels'],target['pixels'],proof['source_rect'],proof['target_rect'],proof['source_key'])
            actual=history.image(payload[20:],r[19],r[20],16)
            require(np.array_equal(actual,expected),'Natural stretch checkpoint pixel mismatch: '+str(seq))
            proof.update(counts,source_create=births[proof['source']],target_create=births[proof['target']],
                input_source_sha256=digest(source['pixels']),before_sha256=digest(target['pixels']),
                expected_sha256=digest(expected),actual_checkpoint_sha256=digest(actual),pixels=int(expected.size))
            # The admitted result never seeds its own expected history.
            history.last_sequence=seq;history.counts[op]+=1;target['pixels']=expected
            if target['tags'] is not None:target['tags'][:]=-1
            continue
        if seq in readbacks:
            proof=readbacks[seq];d=checks[seq]
            verify_gpu_checkpoint(proof,d,history.handles[proof['target']]['pixels'])
            proof['actual_gpu_checkpoint_sha256']=d[proof['gpu_plane']+'_sha256'];proof['gpu_checkpoint_verified']=True
        history.apply(op,seq,payload)
        if op==7:births.pop(struct.unpack_from('<I',payload)[0])
    require(not history.handles and not history.palette and all(p.get('gpu_checkpoint_verified') for p in proofs),'Incomplete resource/readback retirement')
    require(any(p['changed_pixels']>0 for p in proofs),'No meaningful stretched output')
    return dict(success=True,natural_stretch_calls=len(proofs),proofs=proofs,
        compared_pixels=sum(p['pixels'] for p in proofs),mismatching_pixels=0,
        diagnostic_copies=len(checks),diagnostic_copies_seen=diagnostic['copies_seen'],
        unrelated_copies_not_checked=diagnostic['copies_seen']-len(checks),diagnostic_truncated=diagnostic['truncated'],
        copy_calls=len(rows),tracked_alias_calls=sum(bool(r[10]&128) for r in rows),
        command_operations=dict(history.counts),remaining_surfaces=0,closed_call_census=True,route_validation=routes,
        scope='Bounded native RGB565 keyed floor-sampled CPU stretch, complete independent destination checkpoint and actual full GPU source or destination-before hash immediately after upload. Ordinary physical Grimoire actions and normally retired original session; no original driver parity, GPU-only stretch or alias reachability claim.')
