#!/usr/bin/env python3
"""Write explicit hash-pinned native observation bitmap candidates."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess

ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('root',type=Path)
    p.add_argument('output',type=Path)
    args=p.parse_args()
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        root=args.root.resolve();paths=sorted(p for p in root.rglob('*') if p.suffix.lower()=='.spr')
        if not paths or len(paths)>256:raise ValueError('Candidate file count must be 1..256')
        bindings=[]
        for path in paths:
            if not path.resolve().is_relative_to(root) or path.stat().st_size>32*1024*1024:raise ValueError('Candidate path/byte budget exceeded')
            raw=path.read_bytes()
            if len(raw)<24 or struct.unpack_from('<I',raw,8)[0]!=4:
                continue # V1 observation identity is explicitly SPR-v4 only.
            relative=str(path.relative_to(root));suffix=hashlib.sha256(relative.encode()).hexdigest()[:12]
            name=re.sub('[^a-z0-9/_-]','-',str(Path(relative).with_suffix('')).lower())
            name='observed/'+name[:100]+'-'+suffix
            bindings.append({'id':name,'sprite':relative,'sha256':hashlib.sha256(raw).hexdigest()})
        with args.output.open('x') as f:json.dump(bindings,f,indent=2);f.write('\n')
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)


if __name__=='__main__':main()
