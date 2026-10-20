#!/usr/bin/env python3
"""Inspect bounded read-only World colour checkpoints; no writer attribution."""
import argparse
import hashlib
import json
from pathlib import Path
import struct


def inspect(path, expected_word=None):
    raw=Path(path).read_bytes()
    if len(raw)<64 or raw[:8]!=b'MNMCOL1\0':
        raise ValueError('Invalid colour checkpoint header')
    version,header,size,sequence,width,height,stride,word,count,failure,canvas,word_backend,postlude,reserved=struct.unpack_from('<14I',raw,8)
    if version!=1 or header!=64 or size!=len(raw) or len(raw)!=64+32*count or not 2<=count<=16386 or failure or reserved:
        raise ValueError('Incomplete or unsupported colour checkpoint stream')
    if not 1<=width<=2048 or not 1<=height<=2048 or not width<=stride<=4096 or word>65535 or (expected_word is not None and word!=expected_word):
        raise ValueError('Invalid colour checkpoint geometry/word')
    records=[]
    for i in range(count):
        tag,caller,matches,positions,left,top,right,bottom=struct.unpack_from('<8I',raw,64+i*32)
        if i==0:
            if tag!=0xfffffffe or caller:raise ValueError('Missing begin checkpoint')
        elif i==count-1:
            if tag!=0xffffffff or caller:raise ValueError('Missing completion checkpoint')
        elif tag>13 or not 0x400000<=caller<0x5e0000:
            raise ValueError('Unsupported primitive/caller checkpoint')
        if matches>width*height or (matches and not (left<right<=width and top<bottom<=height)) or (not matches and (left or top or right or bottom or positions!=2166136261)):
            raise ValueError('Invalid colour coverage checkpoint')
        records.append(dict(ordinal=i,tag=tag,caller=caller,matches=matches,positions=positions,bounds=[left,top,right,bottom]))
    changes=[]
    for previous,current in zip(records,records[1:]):
        if (previous['matches'],previous['positions'])!=(current['matches'],current['positions']):
            changes.append(dict(before=previous,after=current))
    return dict(schema=1,path=str(path),sha256=hashlib.sha256(raw).hexdigest(),sequence=sequence,width=width,height=height,stride=stride,word=word,
                numeric_canvas=canvas,numeric_word_backend=word_backend,optional_postlude=postlude,count=count,checkpoints=records,changed_intervals=changes,
                writer_identified=False,live_replacement=False,
                boundary='A change narrows an interval between hooked entries/completion; an unobserved writer may run in that interval. No writer, class, alias or causality attribution.')


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input',type=Path);parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    with args.output.open('x') as output:json.dump(inspect(args.input),output,indent=2);output.write('\n')
