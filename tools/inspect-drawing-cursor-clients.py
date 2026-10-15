#!/usr/bin/env python3
"""Pinned original cursor painting clients and exact retained foreign-copy caller."""
import argparse
import importlib.util
import json
from pathlib import Path
import re
import subprocess
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--executable',type=Path,required=True);p.add_argument('--pending-reference',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        spec=importlib.util.spec_from_file_location('canvas',ROOT/'tools/inspect-drawing-canvas-storage.py');pe=importlib.util.module_from_spec(spec);spec.loader.exec_module(pe)
        data=a.executable.read_bytes()
        if pe.sha(data)!=pe.BUILD:raise ValueError('Unsupported executable')
        inventory_path=ROOT/'research/runtime/coverage/binary-nocd.json';inventory=json.loads(inventory_path.read_text());entries={f['entry']:f for f in inventory['functions']};windows={}
        for entry in (0x4a3720,0x4a45b0,0x4a4910,0x4eaaf0):
            f=entries.get(entry)
            if f is None:
                if entry!=0x4eaaf0:raise ValueError('Unreviewed missing function')
                start,end=entry,entry+8
                at=pe.offset(data,entry);stub=data[at:at+8]
                if stub[0]!=0xe8 or stub[5:]!=bytes.fromhex('c21400'):raise ValueError('Timer callback anchor differs')
                target=entry+5+int.from_bytes(stub[1:5],'little',signed=True)
                if target!=0x4a4910:raise ValueError('Timer callback target differs')
            else:
                if len(f['ranges'])!=1:raise ValueError('Unreviewed fragmented function')
                start,end=f['ranges'][0]
            if start!=entry or end-start>8192:raise ValueError('Function boundary differs')
            out=subprocess.run(['objdump','-d','-Mintel','--wide',f'--start-address={start}',f'--stop-address={end}',str(a.executable)],capture_output=True,text=True,check=True).stdout
            lines=[x for x in out.splitlines() if re.match(r'^\s*[0-9a-f]+:',x)]
            if not lines or int(lines[0].split(':')[0],16)!=entry:raise ValueError('Entry alignment differs')
            at=pe.offset(data,start);windows[hex(entry)]=dict(start=start,end=end,whole_inventory_function=f is not None,bytes_sha256=pe.sha(data[at:at+end-start]),instructions=lines)
        pending_raw=a.pending_reference.read_bytes();pending=json.loads(pending_raw)
        frames=[f for f in pending['pending']['frames'] if f['tag']==32]
        if len(frames)!=1:raise ValueError('Retained cursor caller not unique')
        frame=frames[0];caller=frame['original_caller_rva']+0x400000
        if caller!=0x4a4b01 or data[pe.offset(data,caller-5):pe.offset(data,caller)]!=bytes.fromhex('e85f780e00'):
            raise ValueError('Exact original copy caller bytes differ')
        wanted={0x5c513c,0x5c5140,0x5c51fc,0x5c5210,0x5c5294,0x5c5298}
        imports=[x for x in inventory['imports'] if x['iat_va'] in wanted]
        if {x['iat_va'] for x in imports}!=wanted:raise ValueError('Import identity missing')
        names=['tools/inspect-drawing-cursor-clients.py','tools/inspect-drawing-canvas-storage.py','research/runtime/coverage/binary-nocd.json']
        if pe.sha(a.executable.read_bytes())!=pe.BUILD:raise ValueError('Executable changed')
        a.report.write_text(json.dumps(dict(schema=1,success=True,sources={n:pe.sha((ROOT/n).read_bytes()) for n in names},source_executable_sha256=pe.BUILD,windows=windows,imports=imports,retained_pending=dict(path=str(a.pending_reference),sha256=pe.sha(pending_raw),frame=frame),exact_copy_link=dict(caller=caller,call=caller-5,target=0x58c360,within_function=0x4a4910,source_wrapper=0x641fc8,destination_wrapper=0x6a4fe0),source_overlap_proved=False,source_ownership_proved=False,gameplay_picking_recovered=False,original_game_executed=False,scope='Exact static cursor clients/timer wrapper/imports and retained foreign tag32direct-call identity. Source/destination wrappers come from instruction arguments only; numerical canvas IDs do not prove allocation/aliasing or complete cursor/input lifecycle.'),indent=2)+'\n')
        print(json.dumps(dict(success=True,foreign_call=hex(caller),function='0x4a4910')))
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)


if __name__=='__main__':main()
