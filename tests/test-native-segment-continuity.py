#!/usr/bin/env python3
"""Independent v4 checkpoints and fresh-process segment continuity."""
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('fine',ROOT/'tests/test-native-fine-motion.py')
fine=importlib.util.module_from_spec(spec);spec.loader.exec_module(fine)
old=fine.previous

def wrap(data):return b'MNMNWLD\0'+struct.pack('<IIQ',4,len(data),old.digest(data))+data

def oracle(path,raw,tick,cursor,current=None,history=None,segment_ticks=0,pending=False):
    data=fine.oracle(path,raw,tick,cursor,current[:14] if current else None,pending)[24:]
    tail=7*5+4+(26 if pending else 0)
    extra=struct.pack('<2I2i',*current[14:]) if current else b''
    extension=extra+b'\1'+struct.pack('<I',segment_ticks)+bytes([history is not None])
    if history is not None:extension+=struct.pack('<3i13i3I2i',2,0,0,*history)
    return wrap(data[:-tail]+extension+data[-tail:])

def main():
    binary=str(Path(sys.argv[1]).resolve())
    def run(*args,good=True):
        result=subprocess.run([binary,*map(str,args)],capture_output=True,text=True)
        assert result.returncode==(0 if good else 1),(args,result.stdout,result.stderr)
        assert 'AddressSanitizer' not in result.stderr and 'runtime error:' not in result.stderr,result.stderr
        return result.stdout
    with tempfile.TemporaryDirectory(prefix='mnm-segment-continuity-') as directory:
        root=Path(directory);path=root/'map.bin';raw=old.fixture.fixture();path.write_bytes(raw)
        initial,half,boundary,whole,resumed=[root/name for name in ['initial','half','boundary','whole','resumed']]
        run('move-continuous',path,initial,1,1,1,5,1,1,0)
        assert initial.read_bytes()==oracle(path,raw,0,0,pending=True)
        history=[720,720,16,0,0,240,240,0,72,32,16,-40,0,4,4,0,0,0]
        run('resume',initial,boundary,5);assert boundary.read_bytes()==oracle(path,raw,5,1,history=history)
        current=[720,720,16,0,0,108,108,0,82,32,16,-58,0,5,5,0,0,0]
        run('resume',initial,half,6);assert half.read_bytes()==oracle(path,raw,6,1,current,history,1)
        run('resume',initial,whole,18);run('resume',half,resumed,12);assert whole.read_bytes()==resumed.read_bytes()
        restarted=root/'boundary-resumed';run('resume',boundary,restarted,13);assert restarted.read_bytes()==whole.read_bytes()
        trace=[json.loads(row) for row in run('trace',initial,18).splitlines()]
        assert trace[6:]==[json.loads(row) for row in run('trace',half,12).splitlines()]
        assert trace[14]['entities'][0]['action']==3
        assert [trace[i]['entities'][0]['fine'][0] for i in [5,6,7,8]]==[64,82,92,96]
        python_save=root/'python';python_save.write_bytes(oracle(path,raw,6,1,current,history,1))
        python_output=root/'python-output';run('resume',python_save,python_output,12);assert python_output.read_bytes()==whole.read_bytes()
        # Use a slower acceleration to check scalar adjustment and retained fractional accumulator.
        sizes=struct.unpack_from('<7I',raw,64);scalar=92+sum(sizes[:5])
        slow=bytearray(raw);struct.pack_into('<i',slow,scalar+0x10,64)
        slowmap=root/'slow.bin';slowmap.write_bytes(slow)
        slowhalf,slowwhole,slowresumed=[root/name for name in ['slow-half','slow-whole','slow-resumed']]
        row=json.loads(run('move-continuous',slowmap,slowhalf,1,1,1,5,1,1,10))['entities'][0]
        assert row['rate']==621 and row['previous']==[2,384,192,240,4] and row['accumulator']==93,row
        run('move-continuous',slowmap,slowwhole,1,1,1,5,1,1,22);run('resume',slowhalf,slowresumed,12)
        assert slowwhole.read_bytes()==slowresumed.read_bytes()
        # A zero-scalar routing profile is refused by the recovered cost division;
        # the motion model still supports zero rate without inventing a minimum.
        zero=bytearray(raw);struct.pack_into('<i',zero,scalar+0x10,0)
        zeromap=root/'zero.bin';zeromap.write_bytes(zero);zero_output=root/'zero-output'
        run('move-continuous',zeromap,zero_output,1,1,1,2,1,1,12,good=False);assert not zero_output.exists()
        # Checkpoint before and after prefix exhaustion carries the completed segment.
        longmap=root/'long.bin';longmap.write_bytes(old.fixture.fixture(width=4,height=40))
        longwhole=root/'long-whole';run('move-continuous',longmap,longwhole,1,1,1,1,18,1,70)
        for tick in [40,53,54]:
            a=root/f'long-{tick}';b=root/f'long-resumed-{tick}'
            run('move-continuous',longmap,a,1,1,1,1,18,1,tick);run('resume',a,b,70-tick)
            assert b.read_bytes()==longwhole.read_bytes()
        assert json.loads(run('inspect-json',longwhole))['entities'][0]['position']==[1,18,1]
        # A turning route takes reset setup but preserves accumulated speed; all restarts agree.
        turn=root/'turn';run('move-continuous',slowmap,turn,1,1,1,5,3,1,22)
        turning=[json.loads(row) for row in run('trace',turn,0).splitlines()][0]
        assert len({p[3] for p in turning['entities'][0]['route']})>1
        for tick in [5,8,11]:
            a=root/f'turn-{tick}';b=root/f'turn-resumed-{tick}'
            run('move-continuous',slowmap,a,1,1,1,5,3,1,tick);run('resume',a,b,22-tick);assert b.read_bytes()==turn.read_bytes()
        # Recomputed checksums cannot bypass exact bounded setup/sample replay on restore.
        data=bytearray(half.read_bytes()[24:]);tail=7*5+4;history_at=len(data)-tail-84;extension_at=history_at-6
        current_at=extension_at-72
        bad=root/'bad';refused=root/'refused'
        for at,value,structural in [(extension_at+1,2,True),(current_at,719,True),(current_at+52,6,True),
                                    (history_at,9,False),(history_at+12+20,191,False),
                                    (history_at+12+32,73,True)]:
            changed=bytearray(data);struct.pack_into('<i',changed,at,value);bad.write_bytes(wrap(changed))
            run('inspect-json',bad,good=structural);run('resume',bad,refused,1,good=False);assert not refused.exists()
        # A current-continuity bit without the sample driver is structurally invalid.
        changed=bytearray(data);changed[current_at-2]=0;bad.write_bytes(wrap(changed));run('inspect-json',bad,good=False)
        path.write_bytes(raw[:-1]+bytes([raw[-1]^1]));run('resume',half,refused,1,good=False);assert not refused.exists()
        path.unlink();run('resume',half,refused,1,good=False);assert not refused.exists()
    print('independent v4 bytes, boundary/intra-cell/prefix/turn/speed restart and zero-scalar refusal and malformed/map refusal passed')

if __name__=='__main__':main()
