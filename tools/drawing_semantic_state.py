"""Original scalar state samples; menu appearance alone is not pause equivalence."""
import struct
from drawing_api_session import sha


def decode(data):
    if len(data)<64 or (len(data)-64)%192 or len(data)>64+4096*192:
        raise ValueError('Semantic state envelope')
    h=struct.unpack_from('<16I',data)
    if data[:8]!=b'MNMDSS01' or h[2:6]!=(1,192,0x40209ca7,4096) or h[6]!=(len(data)-64)//192 or h[7] or not h[8] or h[9]!=50 or any(h[10:]):
        raise ValueError('Semantic state header/count/loss')
    samples=[];previous=None
    for ordinal,r in enumerate(struct.iter_unpack('<48I',data[64:]),1):
        if r[0]!=ordinal or r[2]!=h[8] or r[7]>1 or r[8]&~63 or any(r[42:]):
            raise ValueError('Semantic state identity/reserved')
        if previous is not None and (r[1]-previous)&0xffffffff<50:
            raise ValueError('Semantic state cadence/reordering')
        previous=r[1]
        for flag,start,end in ((1,9,10),(2,10,12),(4,12,15),(8,15,17),(16,17,18),(32,18,42)):
            if not r[8]&flag and any(r[start:end]):raise ValueError('Invalid semantic fields are populated')
        if r[8]&2 and not r[8]&1 or r[8]&4 and r[14]>255:raise ValueError('Semantic validity/mode')
        samples.append(dict(ordinal=r[0],tick=r[1],thread=r[2],menu_screen=r[3],generation=r[4],request=r[5],status=r[6],ready=bool(r[7]),validity=r[8],depth=r[9],top_screen=r[10],top_initialized=r[11],world_counter=r[12],accepted_sequence=r[13],mode=r[14],camera=list(r[15:17]),effect_counter=r[17],ani_fields=list(r[18:42])))
    return dict(schema=1,success=True,journal_sha256=sha(data),samples=samples,owner_thread=h[8],pause_equivalence=False,selection_verified=False,tooltip_verified=False,complete_presented_journey=False)


def menu_interval(observation,screen,min_ms=500):
    groups=[];current=[]
    for s in observation['samples']:
        if s['top_screen']==screen and s['validity']==63:
            current.append(s)
        elif current:groups.append(current);current=[]
    if current:groups.append(current)
    admitted=[]
    for rows in groups:
        duration=(rows[-1]['tick']-rows[0]['tick'])&0xffffffff
        if duration<min_ms:continue
        keys=('world_counter','accepted_sequence','mode','camera','effect_counter','ani_fields')
        admitted.append(dict(screen=screen,samples=len(rows),duration_ms=duration,first_ordinal=rows[0]['ordinal'],last_ordinal=rows[-1]['ordinal'],first_tick=rows[0]['tick'],last_tick=rows[-1]['tick'],stable={k:all(s[k]==rows[0][k] for s in rows) for k in keys},first={k:rows[0][k] for k in keys},last={k:rows[-1][k] for k in keys},allanimation_pause_proved=False))
    return admitted
