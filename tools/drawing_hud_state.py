"""Bounded original candidate hit identities and separately identified camera fields."""
import struct
from drawing_api_session import sha


def decode(data):
    if len(data)<64 or (len(data)-64)%128 or len(data)>64+4096*128:
        raise ValueError('HUD state envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMDHI01' or h[2] not in (1,2) or h[3:6]!=(128,0x40209ca7,4096) or h[6]!=(len(data)-64)//128 or h[7] or not h[8] or h[9]!=50 or any(h[10:]):
        raise ValueError('HUD state header/count/loss')
    samples=[];previous=None;pool_id=0
    for ordinal,r in enumerate(struct.iter_unpack('<32I',data[64:]),1):
        if r[0]!=ordinal or r[2]!=h[8] or r[4]&~15 or r[21]>2 or h[2]==1 and any(r[22:]):
            raise ValueError('HUD state identity/flags/reserved')
        if previous is not None and (r[1]-previous)&0xffffffff<50:
            raise ValueError('HUD state cadence/reordering')
        previous=r[1]
        for flag,indices in ((1,(5,6,21)),(2,(7,8)),(4,range(9,14)),(8,range(14,21))):
            if not r[4]&flag and any(r[i] for i in indices):raise ValueError('Invalid HUD fields are populated')
        if r[4]&2:
            if not r[4]&1 or not 1<=r[8]<=256 or not pool_id<=r[7]<=pool_id+1 or not r[7]:
                raise ValueError('HUD pool identity/shape')
            pool_id=r[7]
        if r[4]&4:
            if not r[4]&2 or not 1<=r[9]<=r[8] or r[21]!=1:raise ValueError('HUD candidate actor identity')
        elif r[21]==1:raise ValueError('Unvalidated actor is labeled valid')
        input_point=None
        if h[2]==2:
            if r[22]&~11 or r[31]&~7 or r[22]==8 and any(r[23:]):raise ValueError('HUD input point flags/refusal')
            if r[22]&8 and r[22]!=8:raise ValueError('HUD refused point has active flags')
            if r[22]!=8:
                from drawing_input_point import decode as decode_point
                point=decode_point(struct.pack('<16I',1,64,0x40209ca7,r[22],r[2],r[23],r[24],r[30],bool(r[31]&1),bool(r[31]&2),bool(r[31]&4),r[25],r[26],r[27],r[28],r[29]))
                age=(r[1]-point['cursor_tick'])&0xffffffff
                point['cursor_age_ms']=age if point['cursor_record'] else None
                point['timely_cursor']=point['current_cursor_admissible'] and age<=100
                input_point=point
        samples.append(dict(ordinal=r[0],tick=r[1],thread=r[2],menu_screen=r[3],validity=r[4],hit_class=r[5],draw_index=r[6],observed_pool_id=r[7],pool_capacity=r[8],actor_slot=r[9]-1 if r[4]&4 else None,actor_type=r[10],actor_owner=r[11],actor_active=r[12],actor_health=r[13],camera_orientation=r[14],camera_whole=list(r[15:18]),camera_fraction=list(r[18:21]),candidate_state=r[21],**({'input_point':input_point} if h[2]==2 else {})))
    return dict(schema=1,success=True,**({'wire_version':2} if h[2]==2 else {}),journal_sha256=sha(data),owner_thread=h[8],samples=samples,selection_verified=False,tooltip_verified=False,allocation_ownership_proved=False,actor_generation_proved=False,scope='Read-only candidate hit actor checked against current numeric pool/slot, and readable camera scalars. Current activation, persistent selection, tooltip, generations and allocation ownership require separate evidence.')
