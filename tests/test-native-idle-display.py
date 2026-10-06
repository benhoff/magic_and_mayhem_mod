#!/usr/bin/env python3
"""Synthetic bounded ANI Stop pose and independent v8 wire checks; no original input."""
import importlib.util
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('ani_fixture',ROOT/'tests/test-native-ani-motion.py')
a=importlib.util.module_from_spec(spec);spec.loader.exec_module(a)
def main():
    sandbox,test=map(lambda p:str(Path(p).resolve()),sys.argv[1:])
    def run(*args,good=True):
        p=subprocess.run([sandbox,*map(str,args)],capture_output=True,text=True)
        assert p.returncode==(0 if good else 1),(args,p.stdout,p.stderr)
        assert 'AddressSanitizer' not in p.stderr and 'runtime error:' not in p.stderr
    with tempfile.TemporaryDirectory(prefix='mnm-idle-display-') as folder:
        root=Path(folder);frozen=root/'map';frozen.write_bytes(a.segment.old.fixture.fixture())
        ani=root/'movement.ani';ani.write_bytes(a.ani());saved=root/'stopped'
        p=subprocess.run([test,str(frozen),str(ani),str(saved)],capture_output=True,text=True)
        assert p.returncode==0,(p.stdout,p.stderr)
        b=saved.read_bytes();assert b[:8]==b'MNMNWLD\0' and struct.unpack_from('<I',b,8)[0]==8
        assert struct.unpack_from('<I',b,12)[0]==len(b)-24 and struct.unpack_from('<Q',b,16)[0]==a.segment.old.digest(b[24:])
        # Seven empty slots (five bytes each), then pending count. v8 motion tail:
        # terrain=false, pose-present=true, directional sequence10, displayed record2.
        pose_at=len(b)-39-9
        assert b[pose_at-1:pose_at+9]==b'\0\1'+struct.pack('<II',10,2)
        run('inspect-json',saved)
        def write(name,payload,version=8):
            path=root/name;path.write_bytes(b'MNMNWLD\0'+struct.pack('<IIQ',version,len(payload),a.segment.old.digest(payload))+payload);return path
        # Backward-compatible v5 state without a retained display; no timers are smuggled into pose.
        v5=write('v5',b[24:pose_at-1]+b[pose_at+9:],5);run('inspect-json',v5)
        # Same payload masquerading as an earlier format must fail even with its valid checksum.
        run('inspect-json',write('downgrade',b[24:],7),good=False)
        payload=bytearray(b[24:]);payload[pose_at-24]=2
        run('inspect-json',write('bad-flag',payload),good=False)
        run('inspect-json',write('truncated',b[24:pose_at+5]),good=False)
        payload=bytearray(b[24:]);struct.pack_into('<I',payload,pose_at-24+1,4096)
        run('inspect-json',write('bad-sequence',payload),good=False)
        print(p.stdout.strip());print('Independent v8 layout/checksum, v5 read, downgrade, invalid flag, truncation and bounds passed')
if __name__=='__main__':main()
