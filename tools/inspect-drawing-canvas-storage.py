#!/usr/bin/env python3
"""Entry-aligned pinned canvas lifecycle instructions; no allocation ownership."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
ENTRIES={'create':(0x58ad90,0x58b263,bytes.fromhex('a154816f00')),
         'release':(0x58b3e0,0x58b4b8,bytes.fromhex('53568bf157')),
         'lock':(0x58b660,0x58b815,bytes.fromhex('83ec0833c0'))}


def sha(data):return hashlib.sha256(data).hexdigest()


def offset(data,va):
    pe=struct.unpack_from('<I',data,60)[0];base=struct.unpack_from('<I',data,pe+52)[0]
    if base!=0x400000:raise ValueError('Pinned image base changed')
    at=pe+24+struct.unpack_from('<H',data,pe+20)[0];rva=va-base
    for i in range(struct.unpack_from('<H',data,pe+6)[0]):
        virtual,start,size,file=struct.unpack_from('<4I',data,at+40*i+8)
        if start<=rva<start+size:return file+rva-start
    raise ValueError('Entry not file backed')


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--executable',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        data=a.executable.read_bytes()
        if sha(data)!=BUILD:raise ValueError('Unsupported executable')
        windows={}
        for name,(start,end,prefix) in ENTRIES.items():
            at=offset(data,start)
            if data[at:at+len(prefix)]!=prefix:raise ValueError('Pinned entry prologue changed')
            out=subprocess.run(['objdump','-d','-Mintel',f'--start-address={start}',f'--stop-address={end}',str(a.executable)],capture_output=True,text=True,check=True).stdout
            rows=[line for line in out.splitlines() if re.match(r'^\s*[0-9a-f]+:',line)]
            if not rows or int(rows[0].split(':')[0],16)!=start:raise ValueError('Disassembly did not start at entry')
            windows[name]=dict(start=start,end=end,prefix_hex=prefix.hex(),bytes_sha256=sha(data[at:at+end-start]),instructions=rows)
        if sha(a.executable.read_bytes())!=BUILD:raise ValueError('Executable changed')
        a.report.write_text(json.dumps(dict(schema=1,success=True,source_executable_sha256=BUILD,sources={'tools/inspect-drawing-canvas-storage.py':sha(Path(__file__).read_bytes())},windows=windows,entry_alignment_verified=True,original_game_executed=False,scope='Pinned original creation/release/lock instructions decoded from exact checked entry bytes. Static control flow only; no driver/COM/allocation lifetime or original execution proof.'),indent=2)+'\n')
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)


if __name__=='__main__':main()
