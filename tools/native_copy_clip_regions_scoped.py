"""Explicit owner-only reading of retained v1 thread refusals; never global credit."""
import struct
import native_copy_clip_regions as wire
from rendering_pilot import require


def decode_owner(raw,caller_rows,owner):
    """Read original bytes without rewriting a failed global census header.

    Recorder error bit4 is set only for a different thread. With every other
    failure bit absent, published owner records are complete. Independently
    balance rejected query/actual callbacks against the closed caller census.
    No output or clip-input credit is assigned to those other-thread calls.
    """
    require(len(raw)==wire.FILE_BYTES and raw[:8]==b'MNMCLP01','Clip census extent/magic')
    h=struct.unpack_from('<32I',raw)
    require(h[2:6]==(1,128,wire.ROW_BYTES,wire.CAPACITY) and 0<h[6]==h[8]<wire.CAPACITY
            and h[10]==1 and h[11]==owner and not any(h[12:]),'Owner clip census incomplete/foreign')
    excluded=[r for r in caller_rows if r[3]!=owner and r[23] and r[15] in (1,2)]
    expected=sum(2 if r[15]==1 else 1 for r in excluded)
    require(h[7]==expected and h[9]==(4 if expected else 0),
            'Clip misses do not exclusively balance other-thread query/actual refusals')
    result=[]
    for i in range(h[6]):
        w=struct.unpack_from('<'+str(wire.ROW_WORDS)+'I',raw,128+i*wire.ROW_BYTES)
        require(w[0]==i+1 and 1<=w[1]<=6 and w[2]==owner and w[3] and w[5]
                and bool(w[4])==bool(w[6]) and w[9] and w[11]
                and w[14]<=256 and w[15]==32+w[14]*16 and not any(w[35:64])
                and w[34] in (0,1,3) and (w[1]==5 or not any(w[33:35]))
                and (w[34] or not w[33]),'Owner clip row identity/reserved fields')
        require(1<=w[24]<=2048 and 1<=w[25]<=2048 and w[26] in (8,16,32)
                and (bool(w[4]) and 1<=w[27]<=2048 and 1<=w[28]<=2048 and w[29] in (8,16,32)
                     or not w[4] and not any(w[27:30]))
                and w[12]&~(0x1000000|0x8000|0x400)==0 and w[30]==bool(w[12]&0x8000)
                and (w[30] or w[31]==0)
                and (w[1]<=3 and w[32]<=256 or w[1]>=4 and w[32]==0xffffffff),
                'Owner clip format/flags/phase')
        region=w[64:72+4*w[14]]
        require(region[:4]==(32,1,w[14],16*w[14]) and not any(w[72+4*w[14]:]),'Owner RGNDATA header/tail')
        rectangles=[list(map(wire.signed,region[8+j*4:12+j*4])) for j in range(w[14])]
        require(all(l<r and t<b for l,t,r,b in rectangles),'Owner RGNDATA empty/inverted rectangle')
        box=[min(v[0] for v in rectangles),min(v[1] for v in rectangles),
             max(v[2] for v in rectangles),max(v[3] for v in rectangles)] if rectangles else [0,0,0,0]
        require(list(map(wire.signed,region[4:8]))==box,'Owner RGNDATA actual bounds')
        result.append(dict(ordinal=w[0],phase=w[1],thread=w[2],target_alias=w[3],source_alias=w[4],
            target_canonical=w[5],source_canonical=w[6],target_incarnation=w[7],source_incarnation=w[8],
            clipper=w[9],window=w[10],clipper_references=w[11],flags=w[12],result=w[13],
            destination=list(map(wire.signed,w[16:20])),source=list(map(wire.signed,w[20:24])),
            target_description=list(w[24:27]),source_description=list(w[27:30]),keyed=bool(w[30]),key=w[31],
            plan_pieces=w[32],fill_value=w[33],fill_observation=w[34],
            region_bounds=box,regions=rectangles,rgndata_words=list(region)))
    require(not any(memoryview(raw)[128+h[6]*wire.ROW_BYTES:]),'Owner clip unused tail')
    return result,dict(raw_header=list(h),owner_thread=owner,other_thread_calls=len(excluded),
                      other_thread_refusals=expected,global_census_accepted=False,
                      owner_records_complete=True,scope='Original World thread only; other threads receive zero clip-input/output credit')
