#!/usr/bin/env python3
"""Export pinned attenuation-map global references and immutable assembly evidence."""
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile
REPO=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
GLOBALS={0x6c5c5c:'attenuation byte storage',0x6cb8c2:'layer offsets',0x6cb942:'row offsets',0x5e1404:'attenuation enable flag'}
def main():
    subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        executable=REPO/'working/game-nocd/Chaos.exe'
        if hashlib.sha256(executable.read_bytes()).hexdigest()!=HASH:raise ValueError('Unsupported executable')
        assembly=subprocess.run(['objdump','-d','-Mintel',str(executable)],text=True,capture_output=True,check=True).stdout
        parent=REPO/'working/decompiled';parent.mkdir(parents=True,exist_ok=True);root=Path(tempfile.mkdtemp(prefix='audio-map-',dir=parent))
        (root/'program.asm').write_text(assembly);lines=assembly.splitlines();references=[]
        for i,line in enumerate(lines):
            match=re.match(r'\s*([0-9a-f]+):\s',line)
            if not match:continue
            for address,name in GLOBALS.items():
                if re.search(r'\b0x'+format(address,'x')+r'\b',line):
                    references.append({'address':hex(int(match[1],16)),'global':hex(address),'meaning':name,'line':i+1,
                        'context':lines[max(0,i-16):i+20]})
        (root/'references.json').write_text(json.dumps(references,indent=2)+'\n')
        if hashlib.sha256(executable.read_bytes()).hexdigest()!=HASH:raise ValueError('Input changed during export')
        (root/'manifest.json').write_text(json.dumps({'source_sha256':HASH,'input_unchanged':True,
            'scope':'static references; pointer ownership and semantics require manual review',
            'artifacts':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in root.iterdir()}},indent=2)+'\n')
        print(f'Audio map evidence: {root}; {len(references)} references',flush=True)
    finally:subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__':main()
