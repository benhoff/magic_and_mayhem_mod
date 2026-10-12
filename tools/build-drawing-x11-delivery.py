#!/usr/bin/env python3
"""Build the opt-in native Unix X11 delivery metadata observer."""
import hashlib
import json
from pathlib import Path
import subprocess
ROOT=Path(__file__).resolve().parents[1]
def build():
 out=ROOT/'working/build/drawing-x11-delivery';out.mkdir(parents=True,exist_ok=True);binary=out/'libmnm-drawing-x11-delivery.so'
 subprocess.run(['gcc','-std=c11','-O2','-fPIC','-shared','-Wall','-Wextra','-Werror','-pthread',str(ROOT/'runtime/display/x11_delivery.c'),'-ldl','-o',str(binary)],check=True)
 sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in ('runtime/display/x11_delivery.c','tools/build-drawing-x11-delivery.py')}
 (out/'manifest.json').write_text(json.dumps(dict(schema=1,binary_sha256=hashlib.sha256(binary.read_bytes()).hexdigest(),sources=sources,scope='Selected Unix X11 calls only; no source bytes, all-route, frame or driver-equivalence claim.'),indent=2)+'\n');return binary
if __name__=='__main__':print(build())
