#!/usr/bin/env python3
"""Export pinned audio listener projection and camera origin helpers, no game."""
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
        module.RANGES.update(camera_projection=(0x4f7d00,0x4f7e58),camera_origin=(0x4f8230,0x4f8400),
                             coordinate_normalization=(0x48ebf0,0x48ed40))
        exe=REPO/'working/game-nocd/Chaos.exe';root=module.export(exe);data=exe.read_bytes()
        pe=struct.unpack_from('<I',data,0x3c)[0];count=struct.unpack_from('<H',data,pe+6)[0];table=pe+24+struct.unpack_from('<H',data,pe+20)[0]
        rva=0x4f7e58-0x400000
        for i in range(count):
            size,va,rawsize,offset=struct.unpack_from('<IIII',data,table+i*40+8)
            if va<=rva and rva+16<=va+rawsize:
                raw=data[offset+rva-va:offset+rva-va+16];break
        else:raise ValueError('Unmapped projection table')
        path=root/'camera-table.json';path.write_text(json.dumps({'address':'0x004f7e58','hex':raw.hex(),'targets':[hex(x) for x in struct.unpack('<4I',raw)]},indent=2)+'\n')
        if hashlib.sha256(exe.read_bytes()).hexdigest()!=module.HASH:raise ValueError('Input changed during camera export')
        manifest=json.loads((root/'manifest.json').read_text());manifest['artifacts'][path.name]=hashlib.sha256(path.read_bytes()).hexdigest();(root/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
        print(f'Audio camera evidence: {root}',flush=True)
    finally:subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__':main()
