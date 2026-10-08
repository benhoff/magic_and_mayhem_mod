#!/usr/bin/env python3
"""Decode finite process-local kind8 object diagnostics; never rendering inputs."""
import argparse,hashlib,json,struct
from pathlib import Path

def decode(raw):
    if len(raw)<80 or raw[:8]!=b'MNMK8OB1':raise ValueError('Kind8 diagnostic magic/header')
    h=struct.unpack_from('<18I',raw,8)
    version,header,size,sequence,captured,total,truncated,build=h[:8]
    if version!=1 or header!=80 or size!=len(raw) or not 1<=sequence<=3600 or not 1<=captured<=512 or not captured<=total<=12320 or captured!=min(total,512) or truncated!=int(total>512) or build!=0x40209ca7 or h[-1]!=0 or size!=80+captured*160:raise ValueError('Kind8 diagnostic extent/identity')
    result={'sequence':sequence,'captured':captured,'total':total,'truncated':bool(truncated),'mode':h[8],'auxiliary':h[9],'format':h[10],'canvas':h[11],'width':h[12],'height':h[13],'stride':h[14],'clip_left':h[15],'clip_top':h[16],'rows':[]}
    previous=-1
    for i in range(captured):
        at=80+i*160;ordinal,status,obj,vtable,target,reserved=struct.unpack_from('<6I',raw,at)
        row=struct.unpack_from('<9I',raw,at+24);owned=raw[at+60:at+140];methods=struct.unpack_from('<5I',raw,at+140)
        if ordinal<=previous or ordinal>=12320 or status>2 or reserved or row[6]!=8 or row[1]!=obj:raise ValueError('Kind8 diagnostic row identity')
        if status==1:
            if vtable or target or any(owned) or any(methods):raise ValueError('Unreadable kind8 object retained fields')
        elif struct.unpack_from('<I',owned)[0]!=vtable:raise ValueError('Kind8 vtable identity')
        if status==2 and (target or any(methods)):raise ValueError('Unreadable kind8 vtable retained methods')
        if not status and target!=methods[3]:raise ValueError('Kind8 virtual method identity')
        previous=ordinal
        result['rows'].append({'ordinal':ordinal,'status':status,'object':obj,'vtable':vtable,'method':target,'raw_row':list(row),'object_hex':owned.hex(),'vtable_words':list(methods),'x':struct.unpack('<i',struct.pack('<I',row[2]))[0],'y':struct.unpack('<i',struct.pack('<I',row[3]))[0]})
    return result

def collect(directory):
    paths=sorted(directory.glob('kind8-*.bin'));records=[{'path':str(p),'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),**decode(p.read_bytes())} for p in paths]
    return {'records':records,'classes':sorted({(row['vtable'],row['method']) for record in records for row in record['rows'] if not row['status']})}
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('directory',type=Path);p.add_argument('--output',type=Path);a=p.parse_args();r=collect(a.directory)
    if a.output:
        with a.output.open('x') as f:json.dump(r,f,indent=2);f.write('\n')
    else:print(json.dumps(r,indent=2))
if __name__=='__main__':main()
