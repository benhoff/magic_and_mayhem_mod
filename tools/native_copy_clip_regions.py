"""Decode stopped, bounded PE32 provider clip-region observations."""
import struct
from pathlib import Path
from rendering_pilot import require

CAPACITY=8192
ROW_WORDS=1096
ROW_BYTES=ROW_WORDS*4
FILE_BYTES=128+CAPACITY*ROW_BYTES


def signed(value):return value if value<2**31 else value-2**32


def decode(raw):
    require(len(raw)==FILE_BYTES and raw[:8]==b'MNMCLP01','Clip census extent/magic')
    h=struct.unpack_from('<32I',raw)
    require(h[2:6]==(1,128,ROW_BYTES,CAPACITY) and h[6]==h[8]<=CAPACITY
            and h[7]==h[9]==0 and h[10]==1 and bool(h[11])==bool(h[6])
            and not any(h[12:]),'Clip census incomplete, exhausted, missed or unclosed')
    result=[]
    for i in range(h[6]):
        w=struct.unpack_from('<'+str(ROW_WORDS)+'I',raw,128+i*ROW_BYTES)
        require(w[0]==i+1 and 1<=w[1]<=6 and w[2]==h[11] and w[3] and w[5]
                and bool(w[4])==bool(w[6]) and w[9] and w[11]
                and w[14]<=256 and w[15]==32+w[14]*16 and not any(w[35:64])
                and w[34] in (0,1,3) and (w[1]==5 or not any(w[33:35]))
                and (w[34] or not w[33]),
                'Clip observation ordinal, identities, region count or reserved fields')
        require(1<=w[24]<=2048 and 1<=w[25]<=2048 and w[26] in (8,16,32)
                and (bool(w[4]) and 1<=w[27]<=2048 and 1<=w[28]<=2048 and w[29] in (8,16,32)
                     or not w[4] and not any(w[27:30]))
                and w[12]&~(0x1000000|0x8000|0x400)==0 and w[30]==bool(w[12]&0x8000)
                and (w[30] or w[31]==0)
                and (w[1]<=3 and w[32]<=256 or w[1]>=4 and w[32]==0xffffffff),
                'Clip observation format, flags or query/actual phase')
        region=w[64:72+4*w[14]]
        require(region[:4]==(32,1,w[14],16*w[14]) and not any(w[72+4*w[14]:]),
                'RGNDATA header or nonzero clip tail')
        rectangles=[list(map(signed,region[8+j*4:12+j*4])) for j in range(w[14])]
        require(all(l<r and t<b for l,t,r,b in rectangles),'Empty/inverted RGNDATA rectangle')
        box=[min(v[0] for v in rectangles),min(v[1] for v in rectangles),
             max(v[2] for v in rectangles),max(v[3] for v in rectangles)] if rectangles else [0,0,0,0]
        require(list(map(signed,region[4:8]))==box,'RGNDATA bounds differ from actual list')
        result.append(dict(ordinal=w[0],phase=w[1],thread=w[2],target_alias=w[3],source_alias=w[4],
            target_canonical=w[5],source_canonical=w[6],target_incarnation=w[7],source_incarnation=w[8],
            clipper=w[9],window=w[10],clipper_references=w[11],flags=w[12],result=w[13],
            destination=list(map(signed,w[16:20])),source=list(map(signed,w[20:24])),
            target_description=list(w[24:27]),source_description=list(w[27:30]),keyed=bool(w[30]),key=w[31],
            plan_pieces=w[32],fill_value=w[33],fill_observation=w[34],
            region_bounds=box,regions=rectangles,rgndata_words=list(region)))
    require(not any(memoryview(raw)[128+h[6]*ROW_BYTES:]),'Clip census unused tail is not zero')
    return h,result


def read(path):return decode(Path(path).read_bytes())


def plan_regions(record):
    """Independent scalar clipping order from the recorded input region list."""
    dl,dt,dr,db=record['destination'];sl,st,sr,sb=record['source']
    dw,dh,_=record['target_description'];sw,sh,_=record['source_description']
    require(dr-dl==sr-sl and db-dt==sb-st and dr>dl and db>dt,'Copy rectangle extents differ')
    pieces=[];result=0
    for l,t,r,b in record['regions']:
        l,t,r,b=max(dl,l,0),max(dt,t,0),min(dr,r,dw),min(db,b,dh)
        if l>=r or t>=b:continue
        x,y=sl+l-dl,st+t-dt
        if x<0 or y<0 or x+r-l>sw or y+b-t>sh:
            result=0x88760096;break
        pieces.append([x,y,x+r-l,y+b-t,l,t])
    return pieces,result
