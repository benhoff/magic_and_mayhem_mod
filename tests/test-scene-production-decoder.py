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

def fixture(version=4):
    case_count=32 if version==6 else 8
    # Authored contract values, independent of the decoder's constants.
    fixed=[(1,0x5d8000,0x123b39),(2,0x689930,0x240),(10,0x6c4830,4),
      (11,0x6db8d2,12),(12,0x6dbe95,8),(13,0x6e8100,4),(14,0x6deffc,384),
      (15,0x6df188,60),(16,0x6e1ac8,1008),(17,0x6cbb78,0x1031b),(18,0x6c5490,0x66d2)]
    if version>=3:fixed.append((19,0x689b59,16))
    descriptors=[];n=0
    for kind,base,size in fixed:descriptors.append((kind,base,size,n,0,0,0));n+=size
    for kind,manager in [(20,0x6def58),(21,0x689498),(22,0x6b1248)]:descriptors.append((kind,0,0,n,0,0,manager))
    rows=[]
    for i in range(case_count):
        r=[0]*40;r[0]=i+1;r[1]=5 if version==6 else 4 if version>=4 else 3 if version>=2 else 2;r[5]=512+i*128;r[8]=r[9]=n;r[16]=1;r[17]=2;r[30]=r[31]=1
        if version>=2:r[34]=r[35]=1
        if version==6:
            r[39]=1
            if i>=16:r[22]=30;r[23]=10;r[24]=7
        rows.extend(r)
    h=[0]*16;h[2]=version;h[3]=160;h[4]=case_count;h[5]=rows[1];h[9]=7;h[10]=10000000;h[13]=16;h[14]=1536;h[15]=2
    data=bytearray(struct.pack('<16I',*h));data[:8]=b'MNMSPR01'
    if version==6:
        stats=[0]*152;stats[:4]=[1,608,60,50];stats[4:12]=[100000,160000,512,1712,100,1300,100,1300]
        stats[13]=16;stats[14]=16;stats[18:24]=[1200,60000,1200,50,0,128];stats[24+50]=1200
        data.extend(struct.pack('<152I',*stats))
    data.extend(struct.pack('<'+str(case_count*40)+'I',*rows))
    for tag in (1,2):
        data.extend(struct.pack('<4I',tag,len(descriptors),n,0));data.extend(b''.join(struct.pack('<7I',*r) for r in descriptors));data.extend(bytes(n))
    return data,n

def run(output=None):
    data,n=fixture();state=64+8*160;payload=state+16+15*28;second=payload+n
    def word(source,at,value):
        out=bytearray(source);struct.pack_into('<I',out,at,value);return out
    controls={'magic':b'BADMAGIC'+data[8:],'version':word(data,8,7),'version3-mode4':word(data,8,3),
      'version1-mode3':word(data,8,1),'row-size':word(data,12,164),'case-limit':word(data,16,17),
      'critical-total':word(data,24,1),'zero-owner':word(data,36,0),'zero-clock':word(data,40,0),
      'truncated':data[:-1],'trailing':data+b'x','row-reserved':word(data,64+37*4,1),
      'fixed-region-size':word(data,state+16+8,1),'duplicate-region':word(data,state+16+28,1),
      'missing-protected-kind':word(data,state+16+14*28,23),
      'pool-shape':word(data,state+16+12*28+16,1),'state-reserved':word(data,state+12,1)}
    # Contradict the runtime's zero-change assertion with a real retained RNG byte.
    altered=bytearray(data);rng=sum(r[2] for r in struct.iter_unpack('<7I',data[state+16:state+16+8*28]))
    altered[second+16+15*28+rng]=1;controls['independent-byte-contradiction']=altered
    with tempfile.TemporaryDirectory() as directory:
        p=Path(directory)/'trace.bin';p.write_bytes(data);assert summarize(p)['protected_state_invariance'];assert summarize(p)['scene_record_equality']
        old,_=fixture(1);p.write_bytes(old);assert summarize(p)['protected_state_invariance']
        old,_=fixture(2);p.write_bytes(old);assert summarize(p)['protected_state_invariance']
        old,_=fixture(3);p.write_bytes(old);assert summarize(p)['protected_state_invariance']
        modern,_=fixture(5);modern=word(modern,64+14*4,1000);modern=word(modern,64+37*4,750);modern=word(modern,64+39*4,1)
        p.write_bytes(modern);assert summarize(p)['cases'][0]['extra_build_and_surround_ms']==0.025
        controls['consumer-over-total']=word(modern,64+37*4,1001)
        controls['consumer-call-count']=word(modern,64+39*4,2)
        budget,_=fixture(6);p.write_bytes(budget);result=summarize(p)
        assert result['protected_state_invariance'] and result['schedule']['advancing_simulation_fps']==20 and not result['schedule']['higher_render_cadence']
        controls['budget-size']=word(budget,64+4,604)
        controls['budget-target']=word(budget,64+12,49)
        controls['budget-warm-limit']=word(budget,64+13*4,17)
        controls['budget-attempt-count']=word(budget,64+14*4,15)
        controls['budget-histogram-count']=word(budget,64+(24+50)*4,1199)
        controls['budget-interval-sum']=word(budget,64+19*4,0)
        controls['budget-before-warm']=word(budget,64+13*4,15)
        controls['budget-deadline-admission']=word(budget,64+608+16*160+22*4,5)
        for name,bad in controls.items():
            p.write_bytes(bad)
            try:decode(p)
            except ValueError:pass
            else:raise AssertionError('Malformed diagnostic accepted: '+name)
        for field in (35,36):
            p.write_bytes(word(data,64+field*4,0 if field==35 else 1));assert not summarize(p)['protected_state_invariance']
        p.write_bytes(word(data,64+32*4,1));assert not summarize(p)['scene_record_equality']
    result=dict(success=True,decoder_refusals=len(controls),controls=list(controls),negative_history_counts=2,negative_scene_equality=1,version1_compatibility=True,version2_compatibility=True,version3_compatibility=True,version4_compatibility=True,version5_compatibility=True,budget_reference=True)
    if output:Path(output).write_text(json.dumps(result,indent=2)+'\n')
    return result

if __name__=='__main__':print(json.dumps(run(sys.argv[1] if len(sys.argv)>1 else None),indent=2))
