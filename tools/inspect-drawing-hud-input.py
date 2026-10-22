#!/usr/bin/env python3
"""Pinned bounded HUD owners and gameplay virtual clients; static only."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse
import importlib.util
import json
from pathlib import Path
import re
import struct
import subprocess
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--executable',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        spec=importlib.util.spec_from_file_location('canvas',ROOT/'tools/inspect-drawing-canvas-storage.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        data=evidence_bytes(a.executable)
        if module.sha(data)!=module.BUILD:raise ValueError('Unsupported executable')
        inventory=json.loads(evidence_text(ROOT/'research/runtime/coverage/binary-nocd.json'));by_entry={f['entry']:f for f in inventory['functions']}
        table=struct.unpack_from('<6I',data,module.offset(data,0x5c5dd8));entries=[0x4a3c70,0x4f8960,0x4fcf50,0x4fd280,0x4fff60,0x5015f0,*table[:4]];windows={}
        for entry in entries:
            fn=by_entry.get(entry)
            if fn is None:
                if entry not in (0x46a460,0x46ab30):raise ValueError('Unreviewed function boundary')
                start,end=entry,entry+1024
            else:
                if len(fn['ranges'])!=1:raise ValueError('Fragmented function boundary')
                start,end=fn['ranges'][0]
            if start!=entry or end-start>32768:raise ValueError('Function bound differs')
            out=subprocess.run(['objdump','-d','-Mintel','--wide',f'--start-address={start}',f'--stop-address={end}',str(a.executable)],capture_output=True,text=True,check=True).stdout
            lines=[x for x in out.splitlines() if re.match(r'^\s*[0-9a-f]+:',x)]
            if not lines or int(lines[0].split(':')[0],16)!=entry:raise ValueError('Wrong entry alignment')
            at=module.offset(data,start);windows[hex(entry)]=dict(start=start,end=end,whole_inventory_function=fn is not None,bytes_sha256=module.sha(data[at:at+end-start]),instructions=lines)
        disassembly=subprocess.run(['objdump','-d','-Mintel','--wide',str(a.executable)],capture_output=True,text=True,check=True).stdout
        direct_global_references=[x for x in disassembly.splitlines() if re.match(r'^\s*[0-9a-f]+:',x) and ('0x656600' in x or '0x657d40' in x)]
        names=['tools/inspect-drawing-hud-input.py','tools/inspect-drawing-canvas-storage.py','research/runtime/coverage/binary-nocd.json']
        if module.sha(evidence_bytes(a.executable))!=module.BUILD:raise ValueError('Input changed')
        a.report.write_text(json.dumps(dict(schema=1,success=True,sources={n:module.sha(evidence_bytes(ROOT/n)) for n in names},source_executable_sha256=module.BUILD,gameplay_table=list(table),windows=windows,direct_global_references=direct_global_references,original_game_executed=False,scope='Pinned HUD owner/callees and four gameplay virtual-client instruction windows. Static only; virtual role names/selection/tooltip state and indirect/external bodies need separate review.'),indent=2)+'\n');print(json.dumps(dict(success=True,entries=[hex(x) for x in entries])))
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)


if __name__=='__main__':main()
