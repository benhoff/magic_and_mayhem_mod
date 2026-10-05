#!/usr/bin/env python3
"""Independent v3 wire oracle and fresh-process intra-cell continuation."""
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('previous', ROOT/'tests/test-native-movement-session.py')
previous = importlib.util.module_from_spec(spec); spec.loader.exec_module(previous)

def wrap(data):
    return b'MNMNWLD\0'+struct.pack('<IIQ', 3, len(data), previous.digest(data))+data

def oracle(path, raw, tick, cursor, fine=None, pending=False):
    data = previous.oracle(path, raw, tick, cursor, pending)[24:]
    tail = 7*5+4+(26 if pending else 0)
    extension = b'\1'+bytes([fine is not None])
    if fine is not None:
        extension += struct.pack('<13iI', *fine)
    return wrap(data[:-tail]+extension+data[-tail:])

def main():
    binary = str(Path(sys.argv[1]).resolve())
    def run(*args, good=True):
        r = subprocess.run([binary, *map(str,args)], capture_output=True, text=True)
        assert r.returncode == (0 if good else 1), (args,r.stdout,r.stderr)
        assert 'AddressSanitizer' not in r.stderr and 'runtime error:' not in r.stderr, r.stderr
        return r.stdout
    with tempfile.TemporaryDirectory(prefix='mnm-fine-motion-') as directory:
        root = Path(directory); path = root/'map.bin'; raw=previous.fixture.fixture();path.write_bytes(raw)
        initial,half,whole,resumed=[root/name for name in ['initial','half','whole','resumed']]
        run('move-fine',path,initial,1,1,1,5,1,1,0)
        assert initial.read_bytes()==oracle(path,raw,0,0,pending=True)
        run('resume',initial,half,2)
        fields=[720,720,16,0,0,60,60,0,42,32,16,-10,0,1]
        expected=oracle(path,raw,2,0,fields)
        assert half.read_bytes()==expected
        run('resume',initial,whole,17);run('resume',half,resumed,15)
        assert whole.read_bytes()==resumed.read_bytes()==oracle(path,raw,17,4)
        trace=[json.loads(line) for line in run('trace',initial,17).splitlines()]
        restarted=[json.loads(line) for line in run('trace',half,15).splitlines()]
        assert trace[2:]==restarted
        assert [trace[i]['entities'][0]['fine'][0] for i in range(6)]==[32,32,42,52,62,64]
        assert trace[-1]['entities'][0]['position']==[5,1,1]
        python_save=root/'python';python_save.write_bytes(expected)
        output=root/'python-resumed';run('resume',python_save,output,15)
        assert output.read_bytes()==whole.read_bytes()
        # Slower samples exercise a complete animation cycle, its residual reset,
        # and a save with a nonzero fractional accumulator from a slowed profile.
        sizes=struct.unpack_from('<7I',raw,64)
        scalar_start=92+sum(sizes[:5]);object_start=92+sum(sizes[:4])
        slow=bytearray(raw)
        for i in range(48):struct.pack_into('<i',slow,scalar_start+0xd8+i*4,3)
        struct.pack_into('<i',slow,object_start+0x77c,1)
        slowmap=root/'slow.bin';slowmap.write_bytes(slow)
        slowhalf,slowwhole,slowresumed=[root/name for name in ['slow-half','slow-whole','slow-resumed']]
        run('move-fine',slowmap,slowhalf,1,1,1,2,1,1,26)
        row=json.loads(run('inspect-json',slowhalf))['entities'][0]
        assert row['frame']==0 and row['progress']==36 and row['accumulator']==18, row
        run('move-fine',slowmap,slowwhole,1,1,1,2,1,1,140)
        run('resume',slowhalf,slowresumed,114)
        assert slowwhole.read_bytes()==slowresumed.read_bytes()
        assert json.loads(run('inspect-json',slowwhole))['entities'][0]['action']==3
        # More than 16 route points still replans deterministically in sample mode.
        longmap=root/'long.bin';longmap.write_bytes(previous.fixture.fixture(width=40,height=4))
        longhalf,longwhole,longresumed=[root/name for name in ['long-half','long-whole','long-resumed']]
        run('move-fine',longmap,longhalf,1,1,1,18,1,1,40)
        run('move-fine',longmap,longwhole,1,1,1,18,1,1,72)
        run('resume',longhalf,longresumed,32)
        assert longwhole.read_bytes()==longresumed.read_bytes()
        assert json.loads(run('inspect-json',longwhole))['entities'][0]['position']==[18,1,1]
        # Diagonal and toroidal fine coordinates match signed /6 and snap at completion.
        for name,start,target,direction in [('seam',[0,1,1],[11,1,1],6),('diagonal',[1,1,1],[2,2,1],3)]:
            save=root/name
            row=json.loads(run('move-fine',path,save,*start,*target,2))['entities'][0]
            assert row['route'][0][3]==direction
            assert row['fine']==([-10,32,16] if name=='seam' else [38,38,16])
            done=root/(name+'-done');row=json.loads(run('resume',save,done,8))['entities'][0]
            assert row['position']==target and row['fine']==[target[0]*32,target[1]*32,target[2]*16]
        # Recomputed checksums: profile mismatch, forbidden flags, progress, frame,
        # accumulator and spatial inconsistencies must fail without publishing.
        data=bytearray(expected[24:]);at=len(data)-(7*5+4)-56
        bad=root/'bad';refused=root/'refused'
        for offset,value,structurally_valid in [(at,721,True),(at+4,0,False),(at+16,-1,False),
                     (at+20,192,False),(at+24,999,False),(at+32,99,False),(at+52,12,False),(at+52,2,True)]:
            changed=bytearray(data);struct.pack_into('<i',changed,offset,value);bad.write_bytes(wrap(changed))
            run('inspect-json',bad,good=structurally_valid)
            run('resume',bad,refused,1,good=False);assert not refused.exists()
        for offset,value in [(at-2,2),(at-2,0),(at-1,2)]:
            changed=bytearray(data);changed[offset]=value;bad.write_bytes(wrap(changed));run('inspect-json',bad,good=False)
        for name,offset,value in [('frozen-animation',object_start+0x108,1),
                                   ('reverse',object_start+0x722,1),
                                   ('special-type',object_start+0xa8,12),
                                   ('invalid-sample',scalar_start+0xd8,193)]:
            unsupported=bytearray(raw)
            if name=='reverse':unsupported[offset]=value
            else:struct.pack_into('<i',unsupported,offset,value)
            resource=root/(name+'.bin');resource.write_bytes(unsupported)
            run('move-fine',resource,refused,1,1,1,2,1,1,1,good=False);assert not refused.exists()
        path.write_bytes(raw[:-1]+bytes([raw[-1]^1]));run('resume',half,refused,1,good=False);assert not refused.exists()
        path.unlink();run('resume',half,refused,1,good=False);assert not refused.exists()
    print('independent v3 bytes, intra-cell/cycle/fractional/prefix restart, seam/diagonal and malformed/map refusal passed')

if __name__=='__main__':main()
