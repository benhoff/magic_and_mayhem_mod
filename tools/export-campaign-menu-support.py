#!/usr/bin/env python3
"""Read-only pinned campaign menu export; definitions in Ghidra are discarded."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
RANGES={'main':(0x4a75c0,0x4a7844),'realm_view':(0x54f0c0,0x552540), 'startup':(0x4bf2b0,0x4bf330),
        'realm_state':(0x54e230,0x54f100),'wizard_start':(0x593ad0,0x593d50)}
def module(name,path):
    s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
def main():
    parent=ROOT/'working/decompiled';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='campaign-menu-support-',dir=parent));print(out,flush=True)
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        export=module('campaign_export','tools/export-menu-support.py');exe=ROOT/'working/game-nocd/Chaos.exe';data=export.pinned_image(exe)
        audit=module('campaign_audit','tools/audit-file-apis.py');rows=audit.instructions(subprocess.check_output(['objdump','-d','-Mintel',str(exe)],text=True))
        for name,(start,end) in RANGES.items():(out/(name+'.asm')).write_text('\n'.join(r[3] for r in rows if start<=r[0]<end)+'\n')
        objects={hex(va):[r[3] for r in rows if hex(va) in r[3]] for va in [0x659408,0x659e51,0x6ddd58,0x6de4e8]}
        (out/'objects.json').write_text(json.dumps(objects,indent=2)+'\n')
        table=list(struct.unpack('<6I',data[export.image_offset(data,0x5c6a60,24):export.image_offset(data,0x5c6a60,24)+24]))
        (out/'tables.json').write_text(json.dumps({'realm_vtable':{'address':'0x5c6a60','scope':'Six reviewed lifecycle/tick/input slots; adjacent data is not claimed','slots':[hex(x) for x in table]}},indent=2)+'\n')
        import configparser
        configs={}
        for realm in ('Celtic','Greek','Medieval'):
            path=exe.parent/'Realms'/realm/'RealmView.cfg';raw=path.read_bytes();cfg=configparser.ConfigParser(interpolation=None);cfg.read_string(raw.decode('latin-1'))
            configs[realm]={'sha256':hashlib.sha256(raw).hexdigest(),'general':dict(cfg['GENERAL']),'region_owners':dict(cfg['REGION_INFO']),'wizard_locations':{name:cfg[name].get('wizardLocation') for name in cfg.sections() if name.startswith('WIZARD_')}}
        (out/'realm-configs.json').write_text(json.dumps(configs,indent=2)+'\n')
        entries=sorted(set([0x4a75c0,0x54fb30,0x54f0c0,0x54fd20,0x54fd70,0x54e690,0x54eec0,0x5517a0,0x552310]+[x for x in table[:6] if 0x400000<=x<0x5a0000]))
        headless=module('campaign_decompiler','tools/decompile-game.py').find_headless(None)
        if not headless:raise ValueError('Ghidra unavailable')
        command=[str(headless),str(ROOT/'working/decompiled/nocd-nn0_667n/project'),'MagicMayhem','-process','Chaos.exe','-readOnly','-noanalysis','-scriptPath',str(ROOT/'tools/ghidra'),'-postScript','ExportPersistenceProgression.java',str(out/'c'),*[hex(x)[2:] for x in entries],'-max-cpu','2']
        env=dict(os.environ,XDG_CONFIG_HOME=str(out/'config'),XDG_CACHE_HOME=str(out/'cache'),GHIDRA_HEADLESS_JAVA_OPTIONS=f'-Dapplication.cachedir={out}/cache -Dapplication.tempdir={out}/tmp')
        with (out/'ghidra.log').open('w') as log:subprocess.run(command,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=600)
        if any(not (out/'c'/f'{x:08x}.c').exists() for x in entries):raise ValueError('Incomplete campaign decompilation')
        metadata={'source_sha256':export.HASH,'scope':'Static only; no campaign launch or native/live integration','ranges':RANGES,'command':command,'sources':{str(Path(__file__).relative_to(ROOT)):hashlib.sha256(Path(__file__).read_bytes()).hexdigest()},'artifacts':{str(p.relative_to(out)):hashlib.sha256(p.read_bytes()).hexdigest() for p in [*out.glob('*.asm'),out/'objects.json',out/'tables.json',out/'realm-configs.json',*list((out/'c').glob('*'))] if p.is_file()}}
        if hashlib.sha256(exe.read_bytes()).hexdigest()!=export.HASH:raise ValueError('Executable changed')
        (out/'manifest.json').write_text(json.dumps(metadata,indent=2)+'\n')
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__':main()
