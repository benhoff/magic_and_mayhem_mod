#!/usr/bin/env python3
"""Independent adversarial diagnostic inputs; no original game or GPU needed."""
import json
from pathlib import Path
import struct
import sys
import tempfile

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from scene_production import decode,summarize

def fixture(version=2):
    # Authored contract values, independent of the decoder's constants.
    fixed=[(1,0x5d8000,0x123b39),(2,0x689930,0x240),(10,0x6c4830,4),
      (11,0x6db8d2,12),(12,0x6dbe95,8),(13,0x6e8100,4),(14,0x6deffc,384),
      (15,0x6df188,60),(16,0x6e1ac8,1008),(17,0x6cbb78,0x1031b),(18,0x6c5490,0x66d2)]
    descriptors=[];n=0
    for kind,base,size in fixed:descriptors.append((kind,base,size,n,0,0,0));n+=size
    for kind,manager in [(20,0x6def58),(21,0x689498),(22,0x6b1248)]:descriptors.append((kind,0,0,n,0,0,manager))
    rows=[]
    for i in range(8):
        r=[0]*40;r[0]=i+1;r[1]=3 if version==2 else 2;r[5]=512+i*128;r[8]=r[9]=n;r[16]=1;r[17]=2;r[30]=r[31]=1
        if version==2:r[34]=r[35]=1
        rows.extend(r)
    h=[0]*16;h[2]=version;h[3]=160;h[4]=8;h[5]=rows[1];h[9]=7;h[10]=10000000;h[13]=16;h[14]=1536;h[15]=2
    data=bytearray(struct.pack('<16I',*h));data[:8]=b'MNMSPR01';data.extend(struct.pack('<320I',*rows))
    for tag in (1,2):
        data.extend(struct.pack('<4I',tag,14,n,0));data.extend(b''.join(struct.pack('<7I',*r) for r in descriptors));data.extend(bytes(n))
    return data,n

def run(output=None):
    data,n=fixture();state=64+8*160;payload=state+16+14*28;second=payload+n
    def word(source,at,value):
        out=bytearray(source);struct.pack_into('<I',out,at,value);return out
    controls={'magic':b'BADMAGIC'+data[8:],'version':word(data,8,3),
      'version1-mode3':word(data,8,1),'row-size':word(data,12,164),'case-limit':word(data,16,17),
      'critical-total':word(data,24,1),'zero-owner':word(data,36,0),'zero-clock':word(data,40,0),
      'truncated':data[:-1],'trailing':data+b'x','row-reserved':word(data,64+37*4,1),
      'fixed-region-size':word(data,state+16+8,1),'duplicate-region':word(data,state+16+28,1),
      'missing-protected-kind':word(data,state+16+13*28,19),
      'pool-shape':word(data,state+16+11*28+16,1),'state-reserved':word(data,state+12,1)}
    # Contradict the runtime's zero-change assertion with a real retained RNG byte.
    altered=bytearray(data);rng=sum(r[2] for r in struct.iter_unpack('<7I',data[state+16:state+16+8*28]))
    altered[second+16+14*28+rng]=1;controls['independent-byte-contradiction']=altered
    with tempfile.TemporaryDirectory() as directory:
        p=Path(directory)/'trace.bin';p.write_bytes(data);assert summarize(p)['protected_state_invariance'];assert summarize(p)['scene_record_equality']
        old,_=fixture(1);p.write_bytes(old);assert summarize(p)['protected_state_invariance']
        for name,bad in controls.items():
            p.write_bytes(bad)
            try:decode(p)
            except ValueError:pass
            else:raise AssertionError('Malformed diagnostic accepted: '+name)
        for field in (35,36):
            p.write_bytes(word(data,64+field*4,0 if field==35 else 1));assert not summarize(p)['protected_state_invariance']
        p.write_bytes(word(data,64+32*4,1));assert not summarize(p)['scene_record_equality']
    result=dict(success=True,decoder_refusals=len(controls),controls=list(controls),negative_history_counts=2,negative_scene_equality=1,version1_compatibility=True)
    if output:Path(output).write_text(json.dumps(result,indent=2)+'\n')
    return result

if __name__=='__main__':print(json.dumps(run(sys.argv[1] if len(sys.argv)>1 else None),indent=2))
