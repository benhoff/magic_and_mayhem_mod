#!/usr/bin/env python3
"""Exercise original menu callback bytes and logging hooks in an isolated PE32 fixture."""
import argparse
import importlib.util
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
REPO=Path(__file__).resolve().parents[1]
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,REPO/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
FIELDS='sequence event thread menu_id object vtable initialized next_screen returning argument result active_screen depth last_error fade_active fade_counter'.split()
def decode(data):
    if data[:16]!=b'MNMMENU1'+struct.pack('<II',1,64) or (len(data)-16)%64:raise ValueError('Invalid menu observation log')
    rows=[dict(zip(FIELDS,struct.unpack_from('<16I',data,at))) for at in range(16,len(data),64)]
    if len(rows)>256 or [r['sequence'] for r in rows]!=list(range(1,len(rows)+1)):raise ValueError('Invalid record count/sequence')
    return rows
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--channel',action='store_true',help='Also exercise engine-thread command guards')
    parser.add_argument('--battle',action='store_true',help='Exercise V2 setup/map callbacks, setters and transaction guards')
    args=parser.parse_args()
    if args.battle:args.channel=True
    parent=REPO/'working/tests/menu-observer';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(root,flush=True)
    subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        exporter=module('menu_export','tools/export-menu-support.py');executable=REPO/'working/game-nocd/Chaos.exe'
        data=exporter.pinned_image(executable);header='/* Generated from hash-pinned executable; never edit originals. */\n'
        for name,(start,end) in exporter.CALLBACKS.items():
            at=exporter.image_offset(data,start,end-start);values=data[at:at+end-start]
            header+=f'static const unsigned char {name}_callback[]={{'+','.join(hex(v) for v in values)+'};\n'
        for name,start,end in [('setup_button',0x4ad6a0,0x4ad6f0),('map_button',0x4bbbf0,0x4bbca0),('slider_set',0x4cdf40,0x4cdfe4),('rule_change',0x4ad860,0x4ada70),('list_select',0x4d1060,0x4d113b),('list_get',0x4d1df0,0x4d1e4d),('list_index',0x4d1ed0,0x4d1f35)]:
            at=exporter.image_offset(data,start,end-start)
            header+=f'static const unsigned char {name}[]={{'+','.join(hex(v) for v in data[at:at+end-start])+'};\n'
        (root/'menu_fixture_bytes.h').write_text(header)
        module('menu_build','tools/build-menu-observer.py').build(root,True)
        env=dict(os.environ,WINEPREFIX=str(REPO/'working/tests/menu-observer-wine'),WINEDEBUG='-all',
                 MNM_MENU_OBSERVE='Z:'+str(root/'events.bin').replace('/','\\'))
        if args.channel:
            size=32768 if args.battle else 128
            channel=bytearray(size);channel[:16]=(b'MNMMCMD2' if args.battle else b'MNMMCMD1')+struct.pack('<II',2 if args.battle else 1,size)
            struct.pack_into('<III',channel,16,2,1,1)
            (root/'channel.bin').write_bytes(channel)
            env['MNM_MENU_CHANNEL']='Z:'+str(root/'channel.bin').replace('/','\\')
        else:env.pop('MNM_MENU_CHANNEL',None)
        with (root/'wine.log').open('w') as log:
            subprocess.run(['wine',str(root/'selftest.exe')],env=env,cwd=root,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60)
        rows=decode((root/'events.bin').read_bytes())
        actions=[r for r in rows if r['event']==3][:8]
        expected=[(3,4,0,1),(3,2,0x6e0020,0),(3,0xffffffff,0,0),(22,0,0x6de750,0),(22,1,0x6a5018,0),
                  (22,2,0x658970,0),(22,3,0,1),(22,0xffffffff,0,0)]
        if [(r['menu_id'],r['argument'],r['next_screen'],r['returning']) for r in actions]!=expected:raise ValueError('Unexpected action evidence')
        if any(r['last_error']!=0xabc123 for r in rows) or len({r['thread'] for r in rows})!=1:raise ValueError('Lost error/thread contract')
        if not args.channel and (len(rows)!=17 or len([r for r in rows if r['event']==1])!=1):raise ValueError('Idle ticks not bounded/deduplicated')
        unchanged=hashlib.sha256(executable.read_bytes()).hexdigest()==exporter.HASH
        if not unchanged:raise ValueError('Source executable changed')
        report={'success':True,'source_sha256':exporter.HASH,'input_unchanged':True,'real_game_launched':False,
                'channel_guard_checks':bool(args.channel),'battle_guard_checks':bool(args.battle),'original_callbacks_executed':True,'callback_cases':16,'helper_stubbed':True,'tick_stubbed':True,
                'scope':'Selected bytecode mapping/thiscall ABI and observer installation/logging; no live equivalence',
                'artifacts':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in root.iterdir() if p.is_file()},'records':rows}
        (root/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Menu observer fixture passed',flush=True)
    finally:subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__':main()
