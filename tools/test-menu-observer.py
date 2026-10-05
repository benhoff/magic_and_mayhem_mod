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
    parser.add_argument('--spells',action='store_true',help='Exercise V3 inventory, whole-loadout guards and original shelf/talisman swaps')
    parser.add_argument('--mini',action='store_true',help='Exercise gated V4 battle Mini callbacks and ownership guards')
    parser.add_argument("--mini-disabled",action="store_true",help="Require V4 retirement with the compile gate disabled")
    parser.add_argument("--results",action="store_true",help="Exercise V5 Quick Battle results callbacks and display snapshot guards")
    parser.add_argument("--preferences",action="store_true",help="Exercise V6 Preferences original setters, callbacks and ownership/transaction guards")
    args=parser.parse_args()
    if args.mini_disabled:args.mini=True
    if sum((args.battle,args.spells,args.mini,args.results,args.preferences))>1:parser.error("Choose one extended channel fixture mode")
    if args.battle or args.spells or args.mini or args.results or args.preferences:args.channel=True
    parent=REPO/'working/tests/menu-observer';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(root,flush=True)
    subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        exporter=module('menu_export','tools/export-menu-support.py');executable=REPO/'working/game-nocd/Chaos.exe'
        data=exporter.pinned_image(executable);header='/* Generated from hash-pinned executable; never edit originals. */\n'
        for name,(start,end) in exporter.CALLBACKS.items():
            at=exporter.image_offset(data,start,end-start);values=data[at:at+end-start]
            header+=f'static const unsigned char {name}_callback[]={{'+','.join(hex(v) for v in values)+'};\n'
        for name,start,end in [('setup_button',0x4ad6a0,0x4ad6f0),('map_button',0x4bbbf0,0x4bbca0),('slider_set',0x4cdf40,0x4cdfe4),('rule_change',0x4ad860,0x4ada70),('list_select',0x4d1060,0x4d113b),('list_get',0x4d1df0,0x4d1e4d),('list_index',0x4d1ed0,0x4d1f35),('spell_slot_button',0x576380,0x5765c7),('spell_shelf_button',0x5765d0,0x5767c2),('spell_slot_set',0x579710,0x5797dc),('spell_ok_button',0x576810,0x57682c),('spell_tick_bytes',0x576830,0x576836),('mini_button',0x4b23f0,0x4b24f0),('result_button',0x475860,0x475897),('pref_button_bytes',0x4a9840,0x4a9b00),('pref_slider_bytes',0x4a9700,0x4a97d0),('pref_group_bytes',0x4ce730,0x4ce798)]:
            at=exporter.image_offset(data,start,end-start)
            header+=f'static const unsigned char {name}[]={{'+','.join(hex(v) for v in data[at:at+end-start])+'};\n'
        (root/'menu_fixture_bytes.h').write_text(header)
        module('menu_build','tools/build-menu-observer.py').build(root,True,experimental_mini=args.mini and not args.mini_disabled)
        env=dict(os.environ,WINEPREFIX=str(REPO/'working/tests/menu-observer-wine'),WINEDEBUG='-all',
                 MNM_MENU_OBSERVE='Z:'+str(root/'events.bin').replace('/','\\'))
        if args.channel:
            size=77824 if args.preferences else 73728 if args.results else 69632 if args.mini else 65536 if args.spells else 32768 if args.battle else 128
            channel=bytearray(size);channel[:16]=(b'MNMMCMD6' if args.preferences else b'MNMMCMD5' if args.results else b'MNMMCMD4' if args.mini else b'MNMMCMD3' if args.spells else b'MNMMCMD2' if args.battle else b'MNMMCMD1')+struct.pack('<II',6 if args.preferences else 5 if args.results else 4 if args.mini else 3 if args.spells else 2 if args.battle else 1,size)
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
                'preferences_guard_checks':bool(args.preferences),'result_guard_checks':bool(args.results),'mini_guard_checks':bool(args.mini),'mini_disabled_gate':bool(args.mini_disabled),'spell_guard_checks':bool(args.spells),'channel_guard_checks':bool(args.channel),'battle_guard_checks':bool(args.battle),'original_callbacks_executed':True,'callback_cases':20 if args.preferences else 22 if args.mini else 20 if args.results else 16,'helper_stubbed':True,'tick_stubbed':True,
                'scope':'Selected bytecode mapping/thiscall ABI and observer installation/logging; no live equivalence',
                'artifacts':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in root.iterdir() if p.is_file()},'records':rows}
        (root/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Menu observer fixture passed',flush=True)
    finally:subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__':main()
