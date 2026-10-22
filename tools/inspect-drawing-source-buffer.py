#!/usr/bin/env python3
"""Exact inventory-bound NoCD source-buffer setup and referenced client calls."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import subprocess
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--executable',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        spec=importlib.util.spec_from_file_location('canvas',ROOT/'tools/inspect-drawing-canvas-storage.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        data=evidence_bytes(a.executable)
        if module.sha(data)!=module.BUILD:raise ValueError('Wrong executable')
        inventory=json.loads(evidence_text(ROOT/'research/runtime/coverage/binary-nocd.json'));function=next(f for f in inventory['functions'] if f['entry']==0x58d540)
        if function['ranges']!=[[0x58d540,0x58d5a9]]:raise ValueError('Reviewed function boundary differs')
        start,end=function['ranges'][0];at=module.offset(data,start);code=data[at:at+end-start]
        out=subprocess.run(['objdump','-d','-Mintel','--wide',f'--start-address={start}',f'--stop-address={end}',str(a.executable)],capture_output=True,text=True,check=True).stdout
        rows=[x for x in out.splitlines() if re.match(r'^\s*[0-9a-f]+:',x)]
        if not rows or int(rows[0].split(':')[0],16)!=start:raise ValueError('Wrong disassembly entry')
        if module.sha(evidence_bytes(a.executable))!=module.BUILD:raise ValueError('Input changed')
        copies={}
        for entry in (0x58bf40,0x58d320):
            fn=next(f for f in inventory['functions'] if f['entry']==entry);begin,finish=fn['ranges'][0]
            text=subprocess.run(['objdump','-d','-Mintel','--wide',f'--start-address={begin}',f'--stop-address={finish}',str(a.executable)],capture_output=True,text=True,check=True).stdout
            lines=[x for x in text.splitlines() if re.match(r'^\s*[0-9a-f]+:',x)]
            if not lines or int(lines[0].split(':')[0],16)!=begin:raise ValueError('Copy entry alignment differs')
            off=module.offset(data,begin);copies[hex(entry)]=dict(start=begin,end=finish,bytes_sha256=module.sha(data[off:off+finish-begin]),instructions=lines)
        names=['tools/inspect-drawing-source-buffer.py','tools/inspect-drawing-canvas-storage.py','research/runtime/coverage/binary-nocd.json']
        a.report.write_text(json.dumps(dict(schema=1,success=True,sources={n:module.sha(evidence_bytes(ROOT/n)) for n in names},source_executable_sha256=module.BUILD,original_game_executed=False,entry=start,end=end,bytes_sha256=module.sha(code),entry16_hex=code[:16].hex(),instructions=rows,copies=copies,scope='Exact reviewed original function boundary and entry-aligned instructions; semantics/external allocator/driver/ownership execution remain separate.'),indent=2)+'\n')
        print('\n'.join(rows))
        for window in copies.values():print('\n'.join(window['instructions']))
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)


if __name__=='__main__':main()
