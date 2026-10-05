#!/usr/bin/env python3
"""Synthetic PE32 forwarding probe checks; no original artifact consumption."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
def load(name,path):
    s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
def main():
    parent=ROOT/'working/tests/campaign-observer';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(root,flush=True)
    load('campaign_build','tools/build-menu-observer.py').build(root,selftest=True,fixture=False)
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
    subprocess.run(flags+['-c',str(ROOT/'tests/campaign-observer-test.c'),'-o',str(root/'fixture.obj')],check=True)
    subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x400000','/nodefaultlib','/timestamp:0',f'/out:{root/"fixture.exe"}',str(root/'fixture.obj'),str(root/'kernel32.lib'),str(root/'menu.lib')],check=True)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(ROOT/'working/tests/menu-observer-wine'),WINEDEBUG='-all',MNM_MENU_CAMPAIGN_OBSERVE='Z:'+str(root/'campaign.bin').replace('/','\\'))
    with (root/'wine.log').open('w') as log:subprocess.run(['wine',str(root/'fixture.exe')],cwd=root,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60)
    rows=load('campaign_decoder','tools/test-live-campaign-entry.py').decode((root/'campaign.bin').read_bytes())
    if len(rows)!=4 or [r['phase'] for r in rows]!=[1,2,1,2] or [r['visible_regions'] for r in rows]!=[0,0,8,8]:raise ValueError('Unbounded/lost state observation')
    if any(r['last_error']!=0xabc123 for r in rows) or len({r['thread'] for r in rows})!=1:raise ValueError('Thread/error mismatch')
    report={'success':True,'real_game_launched':False,'scope':'Synthetic forwarding tick, original-slot/byte guards, repeat rejection, receiver guard, LastError/return preservation, 300 idle ticks deduplicated and two-phase state change logging; not original-bytecode or live equivalence','records':rows,'sources':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in ['runtime/menu/observer.c','runtime/menu/campaign_observe.h','tools/build-menu-observer.py','tests/campaign-observer-test.c',str(Path(__file__).relative_to(ROOT))]}}
    (root/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Campaign observer fixture passed',flush=True)
if __name__=='__main__':main()
