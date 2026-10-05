#!/usr/bin/env python3
"""Export pinned Single Player setup/map/battle dispatch for read-only recovery."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
REPO=Path(__file__).resolve().parents[1]
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,REPO/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def main():
    parent=REPO/'working/decompiled';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='battle-menu-support-',dir=parent));print(root,flush=True)
    subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        exporter=module('menus','tools/export-menu-support.py');exe=REPO/'working/game-nocd/Chaos.exe';data=exporter.pinned_image(exe)
        audit=module('audit','tools/audit-file-apis.py');base,_=audit.pe_imports(data)
        rows=audit.instructions(subprocess.check_output(['objdump','-d','-Mintel',str(exe)],text=True))
        (root/'map.asm').write_text('\n'.join(r[3] for r in rows if 0x4bb2d0<=r[0]<0x4bbef0)+'\n')
        (root/'setup.asm').write_text('\n'.join(r[3] for r in rows if 0x4abf20<=r[0]<0x4af400)+'\n')
        tables={hex(va):[hex(v) for v in struct.unpack_from('<14I',data,exporter.image_offset(data,va,56))] for va in (0x5c6534,0x5c6564,0x5c6940)}
        anchors=[]
        for value in audit.path_strings(data,base):
            if 'SinglePlayerBattle' in value['value'] or 'MapSelectionScreen' in value['value']:
                hits,_=audit.references(rows,int(value['preferred_va'],16));anchors.append({'string':value,'references':hits})
        (root/'anchors.json').write_text(json.dumps({'tables':tables,'strings':anchors},indent=2)+'\n')
        headless=module('decompile','tools/decompile-game.py').find_headless(None)
        command=[str(headless),str(REPO/'working/decompiled/nocd-nn0_667n/project'),'MagicMayhem','-process','Chaos.exe','-readOnly','-noanalysis','-scriptPath',str(REPO/'tools/ghidra'),'-postScript','ExportBattleMenuSupport.java',str(root),'-max-cpu','2']
        env=dict(os.environ,XDG_CONFIG_HOME=str(root/'config'),XDG_CACHE_HOME=str(root/'cache'),GHIDRA_HEADLESS_JAVA_OPTIONS=f'-Dapplication.cachedir={root}/cache -Dapplication.tempdir={root}/tmp')
        with (root/'ghidra.log').open('w') as log:subprocess.run(command,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
        if hashlib.sha256(exe.read_bytes()).hexdigest()!=exporter.HASH:raise ValueError('Source changed')
        files=[root/'setup.asm',root/'map.asm',root/'anchors.json',*list((root/'c').glob('*.c'))]
        (root/'manifest.json').write_text(json.dumps({'source_sha256':exporter.HASH,'command':command,'scope':'Static recovery only','artifacts':{str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest() for p in files}},indent=2)+'\n')
    finally:subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__':main()
