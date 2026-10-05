#!/usr/bin/env python3
"""Pinned read-only Region Entry contract export; no installed binary writes."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
def load(name,path):
    s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
def main():
    out=Path(tempfile.mkdtemp(prefix='region-entry-support-',dir=ROOT/'working/decompiled'));print(out,flush=True)
    subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    try:
        export=load('region_pe','tools/export-menu-support.py');exe=ROOT/'working/game-nocd/Chaos.exe';data=export.pinned_image(exe)
        rows=load('region_disasm','tools/audit-file-apis.py').instructions(subprocess.check_output(['objdump','-d','-Mintel',str(exe)],text=True))
        (out/'region-entry.asm').write_text('\n'.join(r[3] for r in rows if 0x4b2730<=r[0]<0x4b3c00)+'\n')
        (out/'references.asm').write_text('\n'.join(r[3] for r in rows if '6578c0' in r[3] or '65793c' in r[3])+'\n')
        at=export.image_offset(data,0x5c667c,40);table=struct.unpack('<10I',data[at:at+40]);(out/'table.json').write_text(json.dumps({'address':hex(0x5c667c),'scope':'Ten exported prefix slots; selected lifecycle/build slots reviewed; table incomplete','slots':list(map(hex,table))},indent=2)+'\n')
        entries=sorted(set([0x4b2730,0x4b27e0,0x4b2960,0x4b2990,0x4b29e0,0x4b2ae0,0x4b3390,0x4b3420,0x4b3600,0x4b3880,0x4b38c0,0x4b3900,*table]))
        ghidra=load('region_ghidra','tools/decompile-game.py').find_headless(None)
        if not ghidra:raise ValueError('Ghidra unavailable')
        command=[str(ghidra),str(ROOT/'working/decompiled/nocd-nn0_667n/project'),'MagicMayhem','-process','Chaos.exe','-readOnly','-noanalysis','-scriptPath',str(ROOT/'tools/ghidra'),'-postScript','ExportPersistenceProgression.java',str(out/'c'),*[hex(x)[2:] for x in entries],'-max-cpu','2']
        env=dict(os.environ,XDG_CONFIG_HOME=str(out/'config'),XDG_CACHE_HOME=str(out/'cache'),GHIDRA_HEADLESS_JAVA_OPTIONS=f'-Dapplication.cachedir={out}/cache -Dapplication.tempdir={out}/tmp')
        with (out/'ghidra.log').open('w') as log:subprocess.run(command,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=600)
        if any(not (out/'c'/f'{x:08x}.c').exists() for x in entries):raise ValueError('Incomplete Region Entry decompilation')
        metadata={'source_sha256':export.HASH,'scope':'Static Region Entry contract only; no native/live integration','entries':list(map(hex,entries)),'command':command,'sources':{str(Path(__file__).relative_to(ROOT)):hashlib.sha256(Path(__file__).read_bytes()).hexdigest()},'artifacts':{str(p.relative_to(out)):hashlib.sha256(p.read_bytes()).hexdigest() for p in [*out.glob('*.asm'),out/'table.json',*(out/'c').glob('*')] if p.is_file()}}
        if hashlib.sha256(exe.read_bytes()).hexdigest()!=export.HASH:raise ValueError('Source PE changed')
        (out/'manifest.json').write_text(json.dumps(metadata,indent=2)+'\n')
    finally:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
if __name__=='__main__':main()
