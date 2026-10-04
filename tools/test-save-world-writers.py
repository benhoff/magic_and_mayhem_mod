#!/usr/bin/env python3
"""Capture hash-pinned original world record writers in an isolated 32-bit harness.
Only a script-generated disposable PE copy receives the fwrite capture shim.
No game process is launched and no immutable source artifact is modified.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile
REPO=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def digest(b): return hashlib.sha256(b).hexdigest()
def run(binary):
    root=Path(tempfile.mkdtemp(prefix='run-',dir=REPO/'working/tests/save-world'))
    pe=bytearray((REPO/'working/game-nocd/Chaos.exe').read_bytes())
    assert digest(pe)==HASH,'unexpected reference executable'
    host=root/'reference'
    subprocess.run(['g++','-m32','-std=c++17','-Wall','-Wextra','-Werror','-fno-pie','-no-pie',str(REPO/'tests/save-world-writer-reference.cpp'),'-o',str(host)],check=True)
    symbols=subprocess.check_output(['nm',str(host)],text=True).splitlines()
    target=int(next(line.split()[0] for line in symbols if line.endswith(' T capture_write')),16)
    base=struct.unpack_from('<I',pe,0x3c)[0];sections=struct.unpack_from('<H',pe,base+6)[0];optional=struct.unpack_from('<H',pe,base+20)[0]
    address=0x59c24f;rva=address-0x400000;offset=None
    for i in range(sections):
        at=base+24+optional+40*i
        va,size,raw=struct.unpack_from('<3I',pe,at+12)
        if va<=rva<va+size: offset=raw+rva-va
    assert offset is not None
    before=bytes(pe[offset:offset+5]);patch=b'\xe9'+struct.pack('<I',(target-address-5)&0xffffffff)
    pe[offset:offset+5]=patch;copy=root/'capture.exe';copy.write_bytes(pe)
    records=[];captures=[]
    for name,address,modes in [('creature',0x524df0,range(3)),('missile',0x49aa90,range(3)),('effect',0x544020,range(4))]:
        for mode in modes:
            output=root/f'{name}-{mode}.bin'
            completed=subprocess.run([str(host),str(copy),f'{address:x}',str(mode),str(output)],check=True,capture_output=True,text=True,timeout=10)
            info=json.loads(completed.stdout);info.update(name=name,case=mode,address=f'0x{address:08x}',sha256=digest(output.read_bytes()))
            records.append(info)
            captures.append(str(output))
    comparison=subprocess.run(['python3',str(REPO/'tests/test-save-world-loader.py'),str(binary),*captures],check=True,capture_output=True,text=True)
    (root/'native-comparison.json').write_text(comparison.stdout)
    print(comparison.stdout,end='')
    assert digest((REPO/'working/game-nocd/Chaos.exe').read_bytes())==HASH,'reference changed during experiment'
    report={'source_sha256':HASH,'patched_copy_sha256':digest(pe),'host_sha256':digest(host.read_bytes()),'runner_sha256':digest(Path(__file__).read_bytes()),'native_inspector_sha256':digest(binary.read_bytes()),'shim':{'address':hex(0x59c24f),'before':before.hex(),'after':patch.hex(),'target':hex(target)},'records':records,'scope':'Selected original writers, synthetic receivers; no original save corpus or simulation restoration'}
    (root/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(root/'report.json')
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('binary',type=Path);args=parser.parse_args()
    (REPO/'working/tests/save-world').mkdir(parents=True,exist_ok=True)
    subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    try: run(args.binary.resolve())
    finally: subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__': main()
