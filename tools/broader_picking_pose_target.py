"""Source-only projected actor base from the exact captured displayed ANI record."""
import hashlib
import struct
from rendering_interaction import signed
from rendering_interaction_live import sprite_target,body_record
from rendering_pilot import require
from scene_actor_observation import crc32c
POLICY='inverse-observed-ANI-body-placement-v2'


def targets(records,history,position,frame):
    body=sprite_target(records,history)
    actors=[a for a in frame['actors']if a[0]==0]
    require(len(actors)==1,'Missing unique source actor slot0')
    actor=actors[0]
    require(actor[1:3]==(0,0)and actor[3]and 0<actor[4]<2**31 and actor[13]==1
            and actor[12]&1 and list(actor[6:9])==position,'Source actor role/footprint/pose differs')
    ani=actor[28:39]
    require(len(history)==3 and ani[0]==0 and ani[1]==actor[172]==history[2],
            'Displayed source ANI/sprite index differs')
    blobs={b[:f[19]]for f,b in records if body_record(f)
           and [signed(f[8]),signed(f[9])]==history[:2]
           and hashlib.sha256(b[:f[19]]).hexdigest()==body['frame_sha256']}
    require(len(blobs)==1,'Missing unique body source bytes')
    blob=blobs.pop();checksum=crc32c(blob)
    draws=[d for d in frame['draws']if d[0:2]==(0,0)and d[7]in(0,31)and d[9]>>16==0x8ad0 and [signed(d[5]),signed(d[6])]==history[:2]]
    require(draws and all(d[11:17]==(len(blob),checksum,body['size'][0],body['size'][1],
                body['origin'][0]%2**32,body['origin'][1]%2**32)and d[17]==history[2]for d in draws),
            'Exact observed body draw differs from captured source sprite')
    x,y=signed(ani[2]),signed(ani[3]);cell={k:v for k,v in body.items()if k!='opaque_pixel'}
    cell.update(point=[signed((history[0]-x)%2**32),signed((history[1]-y)%2**32)],
        expected_cell_xy=[signed(v)for v in position[:2]],animation_body_offset=[x,y],footprint=1,
        displayed_sprite_index=ani[1],animation_record_sha256=hashlib.sha256(struct.pack('<11I',*ani)).hexdigest(),
        actor_source=dict(generation=frame['generation'],nonce=str(frame['nonce']),thread=frame['thread'],
            ordinal=frame['ordinal'],source_entry_sequence=frame['source_entry_sequence'],
            raw_record_sha256=frame['raw_record_sha256']),target_policy=POLICY)
    return body,cell


class PosedRows(list):
    def __init__(self,records,frame):super().__init__(records);self.actor_frame=frame


def bound_targets(records,history,position):
    require(isinstance(records,PosedRows),'Missing same-generation actor source binding')
    return targets(records,history,position,records.actor_frame)
