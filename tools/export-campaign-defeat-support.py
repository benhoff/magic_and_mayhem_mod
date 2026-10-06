#!/usr/bin/env python3
"""Export pinned campaign defeat report display and original OK lifecycle for read-only recovery."""
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
    root=Path(tempfile.mkdtemp(prefix='campaign-defeat-support-',dir=parent));print(root,flush=True)
    subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        exporter=module('menus','tools/export-menu-support.py');exe=REPO/'working/game-nocd/Chaos.exe';data=exporter.pinned_image(exe)
        audit=module('audit','tools/audit-file-apis.py');base,_=audit.pe_imports(data)
        rows=audit.instructions(subprocess.check_output(['objdump','-d','-Mintel',str(exe)],text=True))
        (root/'result.asm').write_text('\n'.join(r[3] for r in rows if 0x473930<=r[0]<0x474c70)+'\n')
        (root/'setup.asm').write_text('\n'.join(r[3] for r in rows if 0x557000<=r[0]<0x559850)+'\n')
        ranges={'display_string_setter':(0x4d28c0,0x4d2922),
                'display_number_setter':(0x4d29a0,0x4d2a20),
                'escape_ingress':(0x46c1af,0x46c228),
                'result_producer':(0x46bc00,0x46bc22)}
        for name,(start,end) in ranges.items():
            (root/(name+'.asm')).write_text('\n'.join(r[3] for r in rows if start<=r[0]<end)+'\n')
        tables={hex(va):[hex(v) for v in struct.unpack_from('<'+str(count)+'I',data,exporter.image_offset(data,va,count*4))] for va,count in ((0x5c5ebc,12),)}
        anchors=[]
        for value in audit.path_strings(data,base):
            if 'BattleEnd' in value['value']:
                hits,_=audit.references(rows,int(value['preferred_va'],16));anchors.append({'string':value,'references':hits})
        (root/'anchors.json').write_text(json.dumps({'tables':tables,'strings':anchors,'result_object_references':[r[3] for r in rows if '0x6db967' in r[3]]},indent=2)+'\n')
        headless=module('decompile','tools/decompile-game.py').find_headless(None)
        command=[str(headless),str(REPO/'working/decompiled/nocd-nn0_667n/project'),'MagicMayhem','-process','Chaos.exe','-readOnly','-noanalysis','-scriptPath',str(REPO/'tools/ghidra'),'-postScript','ExportPersistenceProgression.java',str(root/'c'),'473930','473d70','4747a0','474b50','473c10','557510','474a50','-max-cpu','2']
        env=dict(os.environ,XDG_CONFIG_HOME=str(root/'config'),XDG_CACHE_HOME=str(root/'cache'),GHIDRA_HEADLESS_JAVA_OPTIONS=f'-Dapplication.cachedir={root}/cache -Dapplication.tempdir={root}/tmp')
        with (root/'ghidra.log').open('w') as log:subprocess.run(command,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
        if hashlib.sha256(exe.read_bytes()).hexdigest()!=exporter.HASH:raise ValueError('Source changed')
        files=[root/'setup.asm',root/'result.asm',root/'anchors.json',*[root/(name+'.asm') for name in ranges],*list((root/'c').glob('*.c'))]
        (root/'manifest.json').write_text(json.dumps({'source_sha256':exporter.HASH,'command':command,'scope':'Static recovery only','artifacts':{str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest() for p in files}},indent=2)+'\n')
    finally:subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__':main()
