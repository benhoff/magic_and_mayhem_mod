#!/usr/bin/env python3
"""Read pinned task replacement constants; immutable manifest brackets access."""
import argparse, hashlib, json, struct, subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PIN='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);a=p.parse_args()
 if a.output.exists():p.error('Refusing existing output')
 subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
 try:
  raw=(ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes();assert hashlib.sha256(raw).hexdigest()==PIN
  pe=struct.unpack_from('<I',raw,60)[0];st=pe+24+struct.unpack_from('<H',raw,pe+20)[0]
  def data(va,n):
   for i in range(struct.unpack_from('<H',raw,pe+6)[0]):
    rva,size,off=struct.unpack_from('<3I',raw,st+i*40+12)
    if 0x400000+rva<=va and va+n<=0x400000+rva+size:return raw[off+va-0x400000-rva:off+va-0x400000-rva+n]
   raise ValueError('unbacked address')
  result=dict(success=True,original_sha256=PIN,scope='Static pinned task replacement thresholds and lifetime only; no execution comparison.',constants={hex(va):dict(bytes=data(va,4).hex(),value=struct.unpack('<f',data(va,4))[0])for va in [0x5d8654,0x5d8658]},lifetime_entry=dict(address=hex(0x4328d0),bytes=data(0x4328d0,16).hex()),sources={'tools/inspect-owner-replanning.py':hashlib.sha256(Path(__file__).read_bytes()).hexdigest()})
  a.output.parent.mkdir(parents=True,exist_ok=True)
  with a.output.open('x')as f:json.dump(result,f,indent=2);f.write('\n')
 finally:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
if __name__=='__main__':main()
