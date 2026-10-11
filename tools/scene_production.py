"""Strict bounded state-audit decoding. No game pointers are dereferenced here."""
from collections import Counter
import hashlib
import struct

REGIONS={1:'module-data-unclassified',2:'renderer-unclassified',10:'world-counter',11:'mode-sequence',
         12:'pacing-target',13:'pacing-baseline',14:'global-ANI',15:'scheduler',16:'RNG',
         17:'gameplay-object',18:'map-context',20:'creature-pool',21:'effect-pool',22:'third-pool'}
STATE_MAX=8*1024*1024

def decode(path):
    if not 64<=path.stat().st_size<=64*1024*1024:raise ValueError('Scene production size limit')
    data=path.read_bytes();h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMSPR01' or h[2:4]!=(1,160) or not 1<=h[4]<=16 or h[5]>2 or h[12]>4096 or h[15] not in (0,2,4):raise ValueError('Scene production header')
    frequency=h[10]|h[11]<<32
    if not frequency or not h[9]:raise ValueError('Scene production clock/owner')
    at=64;end=at+h[4]*160+h[12]*32
    if end>len(data):raise ValueError('Scene production records truncated')
    rows=list(struct.iter_unpack('<40I',data[at:at+h[4]*160]));at+=h[4]*160
    diffs=list(struct.iter_unpack('<8I',data[at:end]));at=end
    for i,r in enumerate(rows):
        if r[0]!=i+1 or r[1]!=h[5] or r[7]>2 or r[8]>STATE_MAX or r[9]>STATE_MAX or any(r[34:]):raise ValueError('Scene production row')
    for d in diffs:
        if not 1<=d[0]<=h[4] or d[1] not in REGIONS or d[3]>=d[7] or max(d[4:6])>255 or d[6]!=(d[1]>=10):raise ValueError('Scene production difference')
    states=[]
    for tag in range(1,h[15]+1):
        if at+16>len(data):raise ValueError('Scene state header truncated')
        t,count,n,reserved=struct.unpack_from('<4I',data,at);at+=16
        if t!=tag or not 1<=count<=16 or n>STATE_MAX or reserved or at+count*28+n>len(data):raise ValueError('Scene state extent')
        regions=list(struct.iter_unpack('<7I',data[at:at+count*28]));at+=count*28
        payload=data[at:at+n];at+=n;offset=0;seen=set()
        for kind,base,size,position,capacity,bound,manager in regions:
            if kind not in REGIONS or kind in seen or position!=offset or size>n-offset or bound>capacity:raise ValueError('Scene state region')
            seen.add(kind);offset+=size
        if offset!=n:raise ValueError('Scene state payload')
        states.append(dict(tag=tag,regions=regions,payload=payload,sha256=hashlib.sha256(payload).hexdigest()))
    if at!=len(data):raise ValueError('Scene production trailing bytes')
    if any(r[7]<2 for r in rows) and not states:raise ValueError('Scene production retained state missing')
    # Independent byte comparison of retained pairs; runtime hashes are not equality proof.
    retained=[]
    for a,b in zip(states[::2],states[1::2]):
        if a['regions']!=b['regions']:raise ValueError('Retained state shape mismatch')
        counts={}
        for kind,base,n,offset,*_ in a['regions']:
            counts[REGIONS[kind]]=sum(x!=y for x,y in zip(a['payload'][offset:offset+n],b['payload'][offset:offset+n]))
        retained.append(counts)
    return h,rows,diffs,retained

def summarize(path):
    h,rows,diffs,retained=decode(path);frequency=h[10]|h[11]<<32
    cases=[]
    for r in rows:
        cases.append(dict(case=r[0],mode=r[1],world_counter=r[2],update_sequence=r[3],primary_calls=r[5],
          status=('equal','critical-difference','refused')[r[7]],critical_changed_regions=r[10],critical_changed_bytes=r[11],
          module_changed_bytes=r[12],renderer_changed_bytes=r[13],extra_worlds=r[16],total_window_worlds=r[17],
          counter_unchanged=r[18]==r[19],sequence_unchanged=r[20]==r[21],
          extra_production_ms=(r[14]|r[15]<<32)*1000/frequency,
          primary_scene_draws=r[30],extra_scene_draws=r[31],raw_scene_changed_bytes=r[32],scene_refusals=r[33],
          first_critical_difference=None if not r[25] else dict(region=REGIONS[r[25]],offset=r[26],before=r[27],after=r[28])))
    good=not h[6] and not h[7] and not h[8] and len(cases)>=8 and all(r['status']=='equal' and r['counter_unchanged'] and r['sequence_unchanged'] and r['extra_worlds']==1 and r['primary_scene_draws'] and r['extra_scene_draws'] and not r['scene_refusals'] for r in cases)
    return dict(mode=('observe','full','slice')[h[5]],cases=cases,critical_failures=h[6],refusals=h[7],foreign=h[8],
        original_scene_calls=h[14],actual_queue_calls=h[13],extra_worlds=sum(r['extra_worlds'] for r in cases),
        protected_state_invariance=good,independently_compared_retained_pairs=retained,
        scene_record_equality=bool(cases) and all(c['primary_scene_draws']==c['extra_scene_draws'] and c['raw_scene_changed_bytes']==0 and not c['scene_refusals'] for c in cases),
        differences=[dict(case=d[0],region=REGIONS[d[1]],base=hex(d[2]),offset=d[3],before=d[4],after=d[5],critical=bool(d[6])) for d in diffs],
        scope='Exact bounded protected records/counters/ANI/RNG/scheduler. Module/renderer writes are unclassified; nested pointers,map/timer state and driver/pixel equivalence excluded. No60FPS/default activation claim.')
