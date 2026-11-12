"""Decode bounded original World resource boundaries, with retained-provider scope."""
import struct
from rendering_pilot import require

GLOBALS=(0x5d9e38,0x6de6d9,0x689498,0x68949c,0x6def58,0x6def60,
         0x6def64,0x6b1248,0x6b124c,0x6e1f94,0x6e1f98,0x6e266c,
         *range(0x6de558,0x6de57c,4),*range(0x6a28f8,0x6a291c,4),
         0x6903e8,0x6e1f9c,0x656624)
OFFSETS=(0xfc2a,0xfc2e,0xfc32,0xfc36,0xfc3a,0xfc3e,0xfc42,0xfc46,0xfc4a,0xfc4e,0xfc52,0x100df)


def decode(raw):
    require(raw[:8]==b'MNMMRES1' and 64+320<=len(raw)<=64+48*320
            and (len(raw)-64)%320==0, 'World resource observation extent/header')
    h=struct.unpack_from('<16I',raw)
    require(h[2:10]==(1,64,320,48,0x40209ca7,33,12,4) and not any(h[10:]),
            'World resource observation schema/build/budget')
    rows=list(struct.iter_unpack('<80I',raw[64:]))
    require([r[0] for r in rows]==list(range(1,len(rows)+1))
            and rows[-1][1]==9 and all(r[1] in (1,2,3) for r in rows[:-1]),
            'Incomplete or disordered resource observation')
    require(all(r[11]==0 and r[12:14]==(55,0x6cbb78) and not any(r[15:16])
                and not any(r[71:]) for r in rows), 'Refused resource snapshot or reserved fields')
    require(len({r[3] for r in rows[:-1]})==1 and rows[0][3]>0,
            'Resource snapshot original owner thread changed')
    return [dict(ordinal=r[0],phase=r[1],epoch=r[2],thread=r[3],tick=r[4],menu_event=r[5],
                 vtable=r[6],active=r[7],stack_owner=r[8],depth=r[9],menu_generation=r[10],
                 result=r[14],globals=dict(zip(GLOBALS,r[16:49])),
                 owner=dict(zip(OFFSETS,r[49:61])),shutdown=r[61],return_flag=r[62],
                 canvas_interfaces=list(r[63:67]),canvas_tables=list(r[67:71])) for r in rows]


def assess(raw,flow):
    rows=decode(raw);body=rows[:-1];epochs=[r for r in body if r['phase']==1]
    require([r['epoch'] for r in epochs]==[1,2] and len(flow['worlds'])==2,
            'Missing actual initial/later resource epochs')
    require(all(r['vtable']==0x5c5dd8 and r['active']==1 and r['stack_owner']==0x6cbb78
                and r['depth']==4 for r in epochs), 'Unowned or inactive World resource acquisition')
    require(all(r['thread']==flow['states'][0]['thread'] for r in body), 'Resource snapshots belong to another original thread')
    require(all(r['canvas_interfaces']==epochs[0]['canvas_interfaces']
                and r['canvas_tables']==epochs[0]['canvas_tables'] for r in body),
            'Persistent provider boundary identities changed within or between maps')
    bitmap_fields=tuple(range(0x6de558,0x6de57c,4))+tuple(range(0x6a28f8,0x6a291c,4))
    require(all(any(r['globals'][p] for p in bitmap_fields) for r in epochs),
            'Empty fixture cannot supply populated original map resources')
    pairs=[]
    for epoch in (1,2):
        before=[r for r in body if r['phase']==2 and r['epoch']==epoch]
        after=[r for r in body if r['phase']==3 and r['epoch']==epoch and r['active']==0]
        require(before and after, 'No inactive original World resource retirement boundary')
        post=after[0];pre=next((r for r in reversed(before) if r['ordinal']+1==post['ordinal']),None)
        require(pre is not None and pre['active']==1 and pre['vtable']==post['vtable']==0x5c5dd8,
                'Resource retirement lacks the paired populated owner')
        require(post['shutdown']==1 and post['globals'][0x5d9e38]==0xffffffff
                and post['globals'][0x6e1f94]==post['globals'][0x6e1f98]==0
                and post['owner'][0xfc52]==0 and post['globals'][0x6903e8]==0
                and post['globals'][0x6e266c]==0
                and all(post['globals'][p]==0 for p in bitmap_fields),
                'Original World bitmap/queue/selection storage survives exit')
        if pre['owner'][0xfc52]:
            # 46ab30 clears two sixteen-byte iterator fields (fc2a..fc46).
            # It frees the map through fc4a without clearing that address;
            # a stale map value is not a surviving owned allocation proof.
            require(all(post['owner'][p]==0 for p in OFFSETS[:8]), 'Exhausted segmented queue retains iterator fields')
        require(all(pre['canvas_interfaces']) and pre['canvas_interfaces']==post['canvas_interfaces']
                and pre['canvas_tables']==post['canvas_tables'],
                'Persistent display-provider identities changed without accounting')
        pairs.append(dict(epoch=epoch,before=pre,after=post))
    require(pairs[0]['after']['ordinal']<epochs[1]['ordinal']
            and rows[-1]['epoch']==2, 'Later resources acquired before prior World retirement')
    return dict(verified=True,world_epochs=2,retirements=pairs,
                initial_resources=epochs[0],later_resources=epochs[1],
                scope='Live original World-owned bitmap/pointer-queue retirement and fresh populated '
                      'map acquisition, with persistent display-provider identities retained explicitly. '
                      'Existing complete local World-exit model binds these fields; no claim that '
                      'all simulation entity destructors or external allocator internals are recovered.')
