#!/usr/bin/env python3
"""Read pinned cast admission text and compressed dispatch data; no binary edits."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    args = p.parse_args()
    if args.output.exists(): p.error('output must be new')
    subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    try:
        executable = ROOT/'original/Arcane_Nocd/Chaos.exe'
        raw = executable.read_bytes()
        if hashlib.sha256(raw).hexdigest() != HASH: raise ValueError('unsupported original')
        pe = struct.unpack_from('<I',raw,60)[0]
        sections = pe+24+struct.unpack_from('<H',raw,pe+20)[0]
        def at(va,size):
            for i in range(struct.unpack_from('<H',raw,pe+6)[0]):
                rva,n,offset=struct.unpack_from('<3I',raw,sections+40*i+12)
                if 0x400000+rva <= va and va+size <= 0x400000+rva+n:
                    return raw[offset+va-0x400000-rva:offset+va-0x400000-rva+size]
            raise ValueError('unbacked range')
        choices=list(at(0x57c678,103));targets=list(struct.unpack('<17I',at(0x57c634,68)))
        if any(i>=len(targets) for i in choices): raise ValueError('invalid compressed table')
        body=at(0x57b710,0xf21)
        report=dict(schema=1,scope='Static pinned57b710 text and compressed dispatch only; no execution comparison.',source_executable_sha256=HASH,entry_anchors={'0x57b710':body[:16].hex()},target_sha256=hashlib.sha256(body).hexdigest(),dispatch={'choice_address':0x57c678,'target_address':0x57c634,'choices':choices,'targets':targets,'resolved':[targets[i] for i in choices]},input_unchanged=executable.read_bytes()==raw,sources={'tools/export-cast-admission.py':hashlib.sha256(Path(__file__).read_bytes()).hexdigest()})
        report['helper_windows'] = {}
        for start, stop in [(0x4114c0,0x411570),(0x4e1360,0x4e1410)]:
            body_window = at(start,stop-start)
            assembly = subprocess.check_output(['objdump','-d','-Mintel',f'--start-address={start}',f'--stop-address={stop}',str(executable)],text=True)
            report['helper_windows'][hex(start)] = dict(start=start,end=stop,sha256=hashlib.sha256(body_window).hexdigest(),assembly=assembly,scope='Bounded helper window; no whole-function or execution claim')
        if not report['input_unchanged']:raise ValueError('input changed')
    finally:
        subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    report['original_manifest_verified_before_after']=True
    args.output.parent.mkdir(parents=True,exist_ok=True)
    with args.output.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
    print(json.dumps({'targets':[hex(v) for v in targets], 'spell_count':len(choices)}))
if __name__ == '__main__':main()
