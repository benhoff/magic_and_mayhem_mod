#!/usr/bin/env python3
"""Pinned read-only campaign battle handoff targets and selected byte anchors."""
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
def main():
 out=Path(tempfile.mkdtemp(prefix='campaign-handoff-',dir=ROOT/'working/decompiled'));print(out,flush=True)
 subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
 try:
  s=importlib.util.spec_from_file_location('handoff_pe',ROOT/'tools/export-menu-support.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m);exe=ROOT/'working/game-nocd/Chaos.exe';data=m.pinned_image(exe)
  def at(va,n):p=m.image_offset(data,va,n);return data[p:p+n]
  table=struct.unpack('<6I',at(0x5c5dd8,24));tick=table[4]
  asm=subprocess.check_output(['objdump','-d','-Mintel',str(exe),'--start-address=0x5510d0','--stop-address=0x5512c0'],text=True);(out/'handoff.asm').write_text(asm)
  report=dict(source_sha256=m.HASH,world_vtable='0x5c5dd8',world_receiver='0x6cbb78',world_tick=hex(tick),world_tick_anchor=at(tick,8).hex(),world_table=list(map(hex,table)),handoff_anchor=at(0x5510d0,8).hex(),scope='Selected static handoff function and world vtable prefix; no execution or complete world lifecycle',sources={'tools/export-campaign-handoff-support.py':hashlib.sha256(Path(__file__).read_bytes()).hexdigest()},artifacts={'handoff.asm':hashlib.sha256(asm.encode()).hexdigest()})
  (out/'manifest.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report),flush=True)
 finally:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
if __name__=='__main__':main()
