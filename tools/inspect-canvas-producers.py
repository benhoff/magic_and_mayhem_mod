#!/usr/bin/env python3
"""Validate a closed producer stream and separate completion-oracle identities."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct

NAMES = {1:'create',2:'release',3:'bind',4:'clip',5:'fill',6:'copy',7:'jpeg',8:'font',9:'raster',10:'checkpoint',11:'queue_entry',12:'queue_return',13:'failure',14:'rgb_add',15:'bevel',16:'halo',17:'wash',18:'pcx',19:'dib',20:'bmp',21:'fade',22:'minimap',23:'points',24:'minimap_owned_overlay',25:'owned_payload_source'}

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def decode(raw, complete=True):
    if len(raw)<64 or raw[:8] not in (b'MNMPRO01',b'MNMPRO02',b'MNMPRO03') or len(raw)>(512 if raw[:8]==b'MNMPRO02' else 128)*1024*1024:
        raise ValueError('Invalid producer stream envelope')
    h=struct.unpack_from('<16I',raw)
    owned=raw[:8]==b'MNMPRO02'
    extended=raw[:8]==b'MNMPRO03'
    if h[2:5]!=((3 if extended else 2 if owned else 1),64,0x40209ca7) or h[5] not in ((131072,) if extended else (32768,65536,131072,262144) if owned else (32768,65536)) or (h[6]!=32 if extended else not 1<=h[6]<=16) or any(h[7:]):
        raise ValueError('Unsupported producer header')
    at=64;records=[];entered=returned=0;owned_sources={};owned_bytes=0
    while at<len(raw):
        if len(records)>=h[5] or len(raw)-at<96:raise ValueError('Truncated/over-budget producer record')
        r=struct.unpack_from('<24I',raw,at)
        if r[0]!=96+r[18] or r[0]>len(raw)-at or r[1]!=len(records)+1 or r[2] not in NAMES or (r[2]==24 and not owned) or (r[2]==25 and not extended) or r[23]:
            raise ValueError('Invalid producer record bounds/sequence/operation')
        payload=raw[at+96:at+r[0]]
        wire_size=r[0]
        if extended and r[2] in (8,9) and r[4]:
            source=owned_sources.get(r[4])
            if r[18] or source is None or source[0][14]!=r[2] or source[0][17]!=r[17] or source[0][19:21]!=r[19:21]:raise ValueError('Invalid owned-source reference')
            payload=source[1];r=list(r);r[4]=0;r[18]=len(payload);r[0]=96+len(payload);r=tuple(r)
        if r[2]==25:
            if r[14] not in (8,9) or not 40<=r[19]<=1048576 or r[18]!=r[19]+r[20] or struct.unpack_from('<I',payload,0)[0]!=r[19] or any(payload[28:32]) or (r[20]!=272 if r[14]==8 else r[17]>1 or r[20] not in (0,64,512)) or len(owned_sources)>=2048 or len(payload)>16*1024*1024-owned_bytes:raise ValueError('Invalid owned-source definition')
            owned_sources[r[1]]=r,payload;owned_bytes+=len(payload)
        elif r[2] in (8,9):
            if not 40<=r[19]<=1048576 or r[18]!=r[19]+r[20] or struct.unpack_from('<I',payload,0)[0]!=r[19] or any(payload[28:32]):
                raise ValueError('Unclosed producer frame')
            if r[2]==8 and r[20]!=272:raise ValueError('Invalid font coverage input')
            if r[2]==9 and (r[14]>5 or r[15]>11 or r[17]>1 or r[20]!=(64 if r[14]==3 else 512 if r[17] else 0)):raise ValueError('Invalid raster input')
        elif r[2] in (7,20):
            if not 1<=r[18]<=1024 or payload[-1:]!=b'\0' or b'\0' in payload[:-1]:raise ValueError('Invalid JPEG filename')
        elif r[2]==24:
            if len(payload)<148:raise ValueError('Truncated owned minimap metadata')
            m=struct.unpack_from('<37I',payload)
            if m[0]>2 or m[9]>3 or max(m[10:13])>1 or m[17]>1024 or len(payload)!=148+m[17]*12 or (m[0]!=1 and m[17]) or (r[17]>3 if m[0]==2 else r[17]!=0):raise ValueError('Invalid owned minimap metadata')
        elif r[2]==23:
            if r[14]>16384 or r[18]!=r[14]*12:raise ValueError('Invalid point requests')
        elif r[2]==22:
            if not 1<=r[14]<=256 or r[14]%2 or not 1<=r[15]<=256 or r[16]>(3 if owned else 0) or (owned and r[19]!=0xfffffffe) or r[17]>1 or r[18]!=r[14]*r[15]*3 or any(payload[2::3][i]>1 for i in range(r[14]*r[15])):raise ValueError('Invalid minimap terrain input')
        elif r[2]==19:
            if not 40<=r[19]<=1064 or r[20]>2097152 or r[18]!=r[19]+r[20]:raise ValueError('Invalid closed DIB input')
        elif r[2]==18:
            if not 128<=r[19]<=1048576 or r[20]!=768 or r[18]!=r[19]+768 or r[17]:raise ValueError('Invalid PCX producer input')
        elif r[2]==14:
            if r[18]!=20 or r[14]:raise ValueError('Invalid RGB addition input')
        elif payload:raise ValueError('Unexpected producer payload')
        if r[2]==11:
            if entered!=returned or r[14]!=entered+1:raise ValueError('Noncontiguous producer queue entry')
            entered+=1
        if r[2]==12:
            if entered!=returned+1 or r[14]!=entered:raise ValueError('Noncontiguous producer queue return')
            returned+=1
        records.append((r,payload));at+=wire_size
    if complete and (entered!=h[6] or returned!=h[6] or not records or records[-1][0][2]!=12):
        raise ValueError('Incomplete producer/World prefix')
    return h,records

def analyze(directory):
    stream=directory/'canvas-producers.bin';raw=stream.read_bytes();h,records=decode(raw)
    done=directory/'canvas-producers.done';mark=done.read_bytes()
    if len(mark)!=32 or mark[:8]!=b'MNMPDONE':raise ValueError('Missing final producer marker')
    m=struct.unpack_from('<8I',mark)
    failures=[{'sequence':r[1],'code':r[14],'detail':hex(r[15])} for r,_ in records if r[2]==13]
    checkpoints=[]
    for r,_ in records:
        if r[2]!=10:continue
        p=directory/f'producer-oracle-{r[14]:04}.565'
        if not r[3] or not 1<=r[5]<=2048 or not 1<=r[6]<=2048 or p.stat().st_size!=r[5]*r[6]*2:raise ValueError('Unavailable completion oracle')
        checkpoints.append({'sequence':r[1],'oracle':r[14],'canvas':r[3],'width':r[5],'height':r[6],'reason':r[15],'queue':r[16],'path':p.name,'sha256':sha(p)})
    if m[2:]!=(1,h[6],len(records),len(failures),len(checkpoints),len(raw)):
        raise ValueError('Producer completion counts changed')
    result={'path':stream.name,'sha256':sha(stream),'records':len(records),'queues':h[6],'operations':dict(Counter(NAMES[r[2]] for r,_ in records)),'failures':failures,'checkpoints':checkpoints,'closed_prefix':True,'all_observed_inputs_admitted':not failures, 'original_pixels_used_as_native_inputs':False}
    if raw[:8]==b'MNMPRO02':
        overlays=[struct.unpack_from('<37I',b) for r,b in records if r[2]==24]
        terrain=[(r,b) for r,b in records if r[2]==22]
        timeline=[];queue=0
        for r,b in records:
            if r[2]==11:queue=r[14]
            if r[2]==24:
                meta=struct.unpack_from('<37I',b)
                if meta[0]==2:timeline.append({'queue':queue,'sequence':r[1],'view':meta[9],'outline_entry':r[17],'center':[meta[3],meta[4]]})
        result['minimap_owned']={'timeline':timeline,'terrain_calls':len(terrain),'overlay_calls':len(overlays),
            'kinds':dict(Counter(str(m[0]) for m in overlays)),
            'orientations':sorted({m[9] for m in overlays}|{r[16] for r,b in terrain}),
            'centers':[list(p) for p in sorted({(m[3],m[4]) for m in overlays})],
            'fog':sorted({m[11] for m in overlays}), 'flash':sorted({m[12] for m in overlays}),
            'rgb555':sorted({m[10] for m in overlays}),
            'hidden_terrain_cells':sum(sum(b[2::3]) for r,b in terrain),
            'creature_entries':sum(m[17] for m in overlays),
            'source_only_inputs':True,'original_work_bypassed':False}
    return result

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('directory',type=Path);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    with a.output.open('x') as f:json.dump(analyze(a.directory),f,indent=2);f.write('\n')
if __name__=='__main__':main()
