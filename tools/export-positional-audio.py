#!/usr/bin/env python3
"""Export hash-guarded positional audio geometry, tables and conversion helpers."""
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
        module.RANGES.update(distance_helper=(0x4eac10,0x4ead40),x_difference=(0x40e4e0,0x40e550),
            y_difference=(0x40eb70,0x40ebe0),camera_coordinates=(0x4f7d00,0x4f7e00),
            float_to_integer=(0x59bee0,0x59bf07))
        exe=REPO/'working/game-nocd/Chaos.exe';root=module.export(exe);data=exe.read_bytes()
        pe=struct.unpack_from('<I',data,0x3c)[0];count=struct.unpack_from('<H',data,pe+6)[0]
        table=pe+24+struct.unpack_from('<H',data,pe+20)[0]
        def read(address,length):
            rva=address-0x400000
            for i in range(count):
                size,va,rawsize,offset=struct.unpack_from('<IIII',data,table+i*40+8)
                if va<=rva and rva+length<=va+rawsize:return data[offset+rva-va:offset+rva-va+length]
            raise ValueError('Unmapped constant')
        constants={}
        for address,length in [(0x571920,16),(0x571930,16),(0x5c5388,4),(0x5c7708,4)]:
            raw=read(address,length);constants[hex(address)]={'hex':raw.hex(),'values':list(struct.unpack('<4I',raw)) if length==16 else struct.unpack('<f',raw)[0]}
        (root/'positional-constants.json').write_text(json.dumps(constants,indent=2)+'\n')
        if hashlib.sha256(exe.read_bytes()).hexdigest()!=module.HASH:raise ValueError('Input changed during constants export')
        manifest=json.loads((root/'manifest.json').read_text())
        manifest['artifacts']['positional-constants.json']=hashlib.sha256((root/'positional-constants.json').read_bytes()).hexdigest()
        (root/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
        print(f'Positional audio evidence: {root}',flush=True)
    finally:subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__':main()
