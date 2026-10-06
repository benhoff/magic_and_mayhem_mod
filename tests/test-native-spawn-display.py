#!/usr/bin/env python3
"""Native static spawn pose; owned fixture and independent initial checkpoint checks."""
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('spawn_ani',ROOT/'tests/test-native-ani-motion.py')
a=importlib.util.module_from_spec(spec);spec.loader.exec_module(a)
def main():
    sandbox,test=map(lambda p:str(Path(p).resolve()),sys.argv[1:])
    def run(binary,*args):
        p=subprocess.run([binary,*map(str,args)],capture_output=True,text=True)
        assert p.returncode==0,(args,p.stdout,p.stderr)
        assert 'AddressSanitizer' not in p.stderr and 'runtime error:' not in p.stderr
        return p.stdout
    with tempfile.TemporaryDirectory(prefix='mnm-spawn-display-') as folder:
        root=Path(folder);frozen=root/'map';frozen.write_bytes(a.segment.old.fixture.fixture())
        ani=root/'movement.ani';ani.write_bytes(a.ani());saved=root/'spawned'
        print(run(test,frozen,ani,saved).strip())
        b=saved.read_bytes();assert b[:8]==b'MNMNWLD\0' and struct.unpack_from('<I',b,8)[0]==8
        assert struct.unpack_from('<I',b,12)[0]==len(b)-24 and struct.unpack_from('<Q',b,16)[0]==a.segment.old.digest(b[24:])
        assert struct.unpack_from('<II',b,24)==(0,0)
        pose_at=len(b)-39-9;assert b[pose_at-1:pose_at+9]==b'\0\1'+struct.pack('<II',8,1)
        state=json.loads(run(sandbox,'inspect-json',saved));e=state['entities'][0]
        assert state['tick']==0 and state['pending']==0 and e['action']==0 and e['route']==[] and 'animation' not in e
        # CLI initializer has terrain policy but also creates an idle v8 state without ticks or queued orders.
        cli=root/'cli';state=json.loads(run(sandbox,'spawn-terrain-ani',frozen,ani,8,cli,1,1,1))
        assert state['tick']==0 and state['pending']==0 and state['entities'][0]['action']==0 and cli.read_bytes()[8]==8
        continued=root/'continued';run(sandbox,'resume',cli,continued,20)
        state=json.loads(run(sandbox,'inspect-json',continued));assert state['tick']==20 and state['entities'][0]['action']==0
        print('Independent v8 initial pose/layout/checksum, zero-tick CLI and fresh-process static idle passed')
if __name__=='__main__':main()
