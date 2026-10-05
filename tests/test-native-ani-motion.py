#!/usr/bin/env python3
"""Independent v5 ANI checkpoints and fresh-process controller continuation."""
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('segment',ROOT/'tests/test-native-segment-continuity.py')
segment=importlib.util.module_from_spec(spec);spec.loader.exec_module(segment)

def ani(delay=2,event=2,repeat=False):
    records=[];starts=[0]
    for sequence in range(16):
        program=[(1,delay),(0,100+sequence),(0,101+sequence),(5,event),(6,-1)]
        if repeat:program=[(2,3),(1,delay),(0,100+sequence),(0,101+sequence),(3,-1),(5,event),(6,-1)]
        records.extend([(6,-1)] if sequence<8 else program)
        starts.append(len(records))
    size=44+4*len(starts)+44*len(records)
    return b'ANI\0'+struct.pack('<5I',size,len(records),5,0,len(starts))+b'fixture.spr\0'.ljust(20,b'\0')+struct.pack('<17I',*starts)+b''.join(struct.pack('<Ii9I',op,arg,*([0]*9)) for op,arg in records)

def cursor(values):
    sequence,pc,displayed,active,delay,elapsed,repeats,breaking=values
    return b'\1'+struct.pack('<II',sequence,pc)+b'\1'+struct.pack('<I',displayed)+bytes([active])+struct.pack('<4I',delay,elapsed,repeats,breaking)

def oracle(path,raw,animation,tick,index,current=None,history=None,current_animation=None,previous_animation=None,segment_ticks=0,pending=False):
    data=segment.oracle(path,raw,tick,index,current,history,segment_ticks,pending)[24:]
    tail=7*5+4+(26 if pending else 0)
    extension_at=len(data)-tail-6-(84 if history else 0)
    if history:data=data[:-tail]+cursor(previous_animation)+data[-tail:]
    if current:data=data[:extension_at]+cursor(current_animation)+data[extension_at:]
    binding_at=53+len(str(path.resolve()).encode())
    binding=b'\1'+struct.pack('<I',len(animation))+animation+struct.pack('<I',8)
    data=data[:binding_at]+binding+data[binding_at:]
    return b'MNMNWLD\0'+struct.pack('<IIQ',5,len(data),segment.old.digest(data))+data

def main():
    binary=str(Path(sys.argv[1]).resolve())
    def run(*args,good=True):
        p=subprocess.run([binary,*map(str,args)],capture_output=True,text=True)
        assert p.returncode==(0 if good else 1),(args,p.stdout,p.stderr)
        assert 'AddressSanitizer' not in p.stderr and 'runtime error:' not in p.stderr,p.stderr
        return p.stdout
    with tempfile.TemporaryDirectory(prefix='mnm-ani-motion-') as folder:
        root=Path(folder);path=root/'map';raw=segment.old.fixture.fixture();path.write_bytes(raw)
        asset=root/'movement.ani';animation=ani();asset.write_bytes(animation)
        if len(sys.argv)>2:
            restored=subprocess.run([sys.argv[2],str(path),str(asset)],capture_output=True,text=True)
            assert restored.returncode==0,(restored.stdout,restored.stderr)
        initial,boundary,half,event,whole,resumed=[root/name for name in ['initial','boundary','half','event','whole','resumed']]
        def move(output,tick,map_path=path,target=(5,1,1)):
            return run('move-ani',map_path,asset,8,output,1,1,1,*target,tick)
        move(initial,0);assert initial.read_bytes()==oracle(path,raw,animation,0,0,pending=True)
        history=[720,720,16,0,0,240,240,0,72,32,16,-40,0,4,4,0,0,0]
        previous=[10,3,2,1,2,1,0,0]
        move(boundary,5);assert boundary.read_bytes()==oracle(path,raw,animation,5,1,history=history,previous_animation=previous)
        current=[720,720,16,0,0,108,108,0,82,32,16,-58,0,5,5,0,0,0]
        now=[10,3,2,1,2,2,0,0]
        move(half,6);assert half.read_bytes()==oracle(path,raw,animation,6,1,current,history,now,previous,1)
        after=[720,720,16,0,0,168,168,0,92,32,16,0,0,0,0,0,0,0]
        reset=[10,2,1,1,2,0,0,0]
        move(event,7);assert event.read_bytes()==oracle(path,raw,animation,7,1,after,history,reset,previous,2)
        move(whole,20);run('resume',half,resumed,14);assert whole.read_bytes()==resumed.read_bytes()
        trace=[json.loads(row) for row in run('trace',initial,20).splitlines()]
        assert trace[6:]==[json.loads(row) for row in run('trace',half,14).splitlines()]
        python_save=root/'python';python_save.write_bytes(oracle(path,raw,animation,6,1,current,history,now,previous,1))
        python_output=root/'python-output';run('resume',python_save,python_output,14);assert python_output.read_bytes()==whole.read_bytes()
        for tick in [5,7,8,11]:
            a=root/f'split-{tick}';b=root/f'resumed-{tick}';move(a,tick);run('resume',a,b,20-tick);assert b.read_bytes()==whole.read_bytes()
        # Delays/repeats persist and a turn restarts the selected directional sequence.
        turn=root/'turn';move(turn,30,target=(5,3,1))
        for tick in [6,8,11]:
            a=root/f'turn-{tick}';b=root/f'turn-restored-{tick}';move(a,tick,target=(5,3,1));run('resume',a,b,30-tick);assert b.read_bytes()==turn.read_bytes()
        longmap=root/'long-map';longmap.write_bytes(segment.old.fixture.fixture(width=4,height=40))
        long=root/'long';move(long,80,longmap,(1,18,1))
        for tick in [40,53,54]:
            a=root/f'long-{tick}';b=root/f'long-restored-{tick}';move(a,tick,longmap,(1,18,1));run('resume',a,b,80-tick);assert b.read_bytes()==long.read_bytes()
        asset.write_bytes(ani(delay=1,repeat=True))
        repeat_half,repeat_whole,repeat_restored=[root/name for name in ['repeat-half','repeat-whole','repeat-restored']]
        row=json.loads(move(repeat_half,6))['entities'][0]
        assert row['animation'][6]==2,row
        move(repeat_whole,24);run('resume',repeat_half,repeat_restored,18);assert repeat_restored.read_bytes()==repeat_whole.read_bytes()
        sizes=struct.unpack_from('<7I',raw,64);scalar=92+sum(sizes[:5])
        slow=bytearray(raw);struct.pack_into('<i',slow,scalar+0x10,64)
        slowmap=root/'slow-map';slowmap.write_bytes(slow)
        slow_half,slow_whole,slow_restored=[root/name for name in ['slow-half','slow-whole','slow-restored']]
        move(slow_half,10,slowmap);move(slow_whole,28,slowmap);run('resume',slow_half,slow_restored,18)
        assert slow_restored.read_bytes()==slow_whole.read_bytes()
        asset.write_bytes(ani(delay=0,event=0));stopped=root/'stopped'
        run('move-ani',path,asset,8,stopped,1,1,1,5,1,1,8,good=False);assert not stopped.exists()
        # ANI bytes are owned: later source deletion or edits cannot alter restoration.
        asset.unlink();owned=root/'owned';run('resume',half,owned,14);assert owned.read_bytes()==whole.read_bytes()
        asset.write_bytes(ani(event=7));refused=root/'refused'
        run('move-ani',path,asset,8,refused,1,1,1,5,1,1,3,good=False);assert not refused.exists()
        data=bytearray(half.read_bytes()[24:]);tail=7*5+4
        history_at=len(data)-tail-115;extension_at=history_at-6;cursor_at=extension_at-31
        for at,value in [(cursor_at+1,11),(cursor_at+5,65535),(cursor_at+18,1),(history_at+84+5,65535)]:
            changed=bytearray(data);struct.pack_into('<I',changed,at,value)
            bad=root/'bad';bad.write_bytes(b'MNMNWLD\0'+struct.pack('<IIQ',5,len(changed),segment.old.digest(changed))+changed)
            run('inspect-json',bad);run('resume',bad,refused,1,good=False);assert not refused.exists()
        # Structural parsing preserves opaque owned bytes; resource resolution rejects bad ANI.
        changed=bytearray(data);changed[58+len(str(path.resolve()).encode())]=0
        bad=root/'bad-ani';bad.write_bytes(b'MNMNWLD\0'+struct.pack('<IIQ',5,len(changed),segment.old.digest(changed))+changed)
        run('inspect-json',bad);run('resume',bad,refused,1,good=False);assert not refused.exists()
        huge=bytearray(initial.read_bytes()[24:]);struct.pack_into('<I',huge,54+len(str(path.resolve()).encode()),8*1024*1024+1)
        bad=root/'huge-ani';bad.write_bytes(b'MNMNWLD\0'+struct.pack('<IIQ',5,len(huge),segment.old.digest(huge))+huge)
        run('inspect-json',bad,good=False)
        changed=bytearray(data);changed[cursor_at]=0
        bad=root/'missing-cursor';bad.write_bytes(b'MNMNWLD\0'+struct.pack('<IIQ',5,len(changed),segment.old.digest(changed))+changed)
        run('inspect-json',bad,good=False)
        path.write_bytes(raw[:-1]+bytes([raw[-1]^1]));run('resume',half,refused,1,good=False);assert not refused.exists()
    print('Independent v5 bytes, ANI event reset, owned assets, boundary/turn/prefix restarts and malformed refusal passed')
if __name__=='__main__':main()
