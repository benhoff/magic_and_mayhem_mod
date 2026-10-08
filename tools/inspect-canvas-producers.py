#!/usr/bin/env python3
"""Validate a closed producer stream and separate completion-oracle identities."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct

NAMES = {1:'create',2:'release',3:'bind',4:'clip',5:'fill',6:'copy',7:'jpeg',8:'font',9:'raster',10:'checkpoint',11:'queue_entry',12:'queue_return',13:'failure',14:'rgb_add',15:'bevel',16:'halo',17:'wash',18:'pcx',19:'dib',20:'bmp',21:'fade',22:'minimap',23:'points'}

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def decode(raw, complete=True):
    if len(raw)<64 or raw[:8]!=b'MNMPRO01' or len(raw)>128*1024*1024:
        raise ValueError('Invalid producer stream envelope')
    h=struct.unpack_from('<16I',raw)
    if h[2:5]!=(1,64,0x40209ca7) or h[5] not in (32768,65536) or not 1<=h[6]<=16 or any(h[7:]):
        raise ValueError('Unsupported producer header')
    at=64;records=[];entered=returned=0
    while at<len(raw):
        if len(records)>=h[5] or len(raw)-at<96:raise ValueError('Truncated/over-budget producer record')
        r=struct.unpack_from('<24I',raw,at)
        if r[0]!=96+r[18] or r[0]>len(raw)-at or r[1]!=len(records)+1 or r[2] not in NAMES or r[23]:
            raise ValueError('Invalid producer record bounds/sequence/operation')
        payload=raw[at+96:at+r[0]]
        if r[2] in (8,9):
            if not 40<=r[19]<=1048576 or r[18]!=r[19]+r[20] or struct.unpack_from('<I',payload,0)[0]!=r[19] or any(payload[28:32]):
                raise ValueError('Unclosed producer frame')
            if r[2]==8 and r[20]!=272:raise ValueError('Invalid font coverage input')
            if r[2]==9 and (r[14]>5 or r[15]>11 or r[17]>1 or r[20]!=(64 if r[14]==3 else 512 if r[17] else 0)):raise ValueError('Invalid raster input')
        elif r[2] in (7,20):
            if not 1<=r[18]<=1024 or payload[-1:]!=b'\0' or b'\0' in payload[:-1]:raise ValueError('Invalid JPEG filename')
        elif r[2]==23:
            if r[14]>16384 or r[18]!=r[14]*12:raise ValueError('Invalid point requests')
        elif r[2]==22:
            if not 1<=r[14]<=256 or r[14]%2 or not 1<=r[15]<=256 or r[16] or r[17]>1 or r[18]!=r[14]*r[15]*3 or any(payload[2::3][i]>1 for i in range(r[14]*r[15])):raise ValueError('Invalid minimap terrain input')
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
        records.append((r,payload));at+=r[0]
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
    return {'path':stream.name,'sha256':sha(stream),'records':len(records),'queues':h[6],'operations':dict(Counter(NAMES[r[2]] for r,_ in records)),'failures':failures,'checkpoints':checkpoints,'closed_prefix':True,'all_observed_inputs_admitted':not failures,'original_pixels_used_as_native_inputs':False}

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('directory',type=Path);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    with a.output.open('x') as f:json.dump(analyze(a.directory),f,indent=2);f.write('\n')
if __name__=='__main__':main()
