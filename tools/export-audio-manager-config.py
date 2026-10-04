#!/usr/bin/env python3
"""Export pinned manager configuration/pool assembly and strings, no game."""
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
REPO=Path(__file__).resolve().parents[1]
def main():
    subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        spec=importlib.util.spec_from_file_location('audio_export',REPO/'tools/export-audio-support.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        module.RANGES.update(manager_constructor=(0x56dd80,0x56de44),source_pool=(0x56e910,0x56ee79),group_parser=(0x56ffc0,0x5700d6),schedule_pool=(0x5719b0,0x571aa9),table_find=(0x59e142,0x59e174))
        exe=REPO/'working/game-nocd/Chaos.exe';root=module.export(exe);data=exe.read_bytes()
        pe=struct.unpack_from('<I',data,0x3c)[0];count=struct.unpack_from('<H',data,pe+6)[0];table=pe+24+struct.unpack_from('<H',data,pe+20)[0]
        strings={}
        for address in (0x5faf28,0x5e4084,0x5d9058,0x5f0448,0x5f043c,0x5f0430,0x5f0408,0x5f0418,0x5da820,0x5f0450,0x5f0460):
            rva=address-0x400000
            for i in range(count):
                size,va,rawsize,offset=struct.unpack_from('<IIII',data,table+i*40+8)
                if va<=rva<va+max(size,rawsize):
                    delta=rva-va
                    if delta>=rawsize:strings[hex(address)]={'value':'','storage':'zero-initialized virtual tail; runtime assignment unverified'}
                    else:
                        raw=offset+delta;end=data.index(b'\0',raw,offset+rawsize)
                        strings[hex(address)]={'value':data[raw:end].decode('ascii'),'storage':'file-backed'}
                    break
            else:raise ValueError('Unmapped loader string')
        path=root/'manager-config-strings.json';path.write_text(json.dumps(strings,indent=2)+'\n')
        if hashlib.sha256(exe.read_bytes()).hexdigest()!=module.HASH:raise ValueError('Input changed during export')
        manifest=json.loads((root/'manifest.json').read_text());manifest['artifacts'][path.name]=hashlib.sha256(path.read_bytes()).hexdigest();(root/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
        print(f'Manager configuration evidence: {root}',flush=True)
    finally:subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__':main()
