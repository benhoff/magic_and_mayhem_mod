#!/usr/bin/env python3
"""Read-only scoped interface/anchor export; never a command equivalence test."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from rts_controls_launch import ROOT, HASH, GUARDS, module

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args()
    if a.output.exists():
        raise ValueError('Refusing existing evidence output')
    subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    try:
        executable=ROOT/'original/Arcane_Nocd/Chaos.exe';raw=executable.read_bytes()
        if hashlib.sha256(raw).hexdigest()!=HASH:
            raise ValueError('Unsupported immutable source')
        offsets=module('rts_export_offsets','tools/export-menu-support.py')
        entries=[]
        for address, hexbytes in GUARDS.items():
            expected=bytes.fromhex(hexbytes);at=offsets.image_offset(raw,address,len(expected))
            if raw[at:at+len(expected)]!=expected:
                raise ValueError('Interface anchor mismatch')
            entries.append({'address':address,'bytes':expected.hex()})
        value={'schema':1,'success':True,'source_executable_sha256':HASH,
            'scope':'Static signature/table and documented interface review only. Keyboard/view handlers remain unassigned in the binary inventory. No original command execution or semantic equivalence.',
            'entries':entries,'unassigned_handler_entries':[0x46c070,0x4fdad0],
            'sources':{name:hashlib.sha256((ROOT/name).read_bytes()).hexdigest() for name in
                ['tools/export-rts-controls.py','tools/rts_controls_launch.py','tools/export-menu-support.py','tools/original-manifest.sh']},
            'input_unchanged':executable.read_bytes()==raw}
        if not value['input_unchanged']:
            raise ValueError('Immutable source changed')
    finally:
        subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    value['original_manifest_verified_before_after']=True
    a.output.parent.mkdir(parents=True,exist_ok=True)
    with a.output.open('x') as out:json.dump(value,out,indent=2);out.write('\n')
    print(a.output)

if __name__=='__main__':main()
