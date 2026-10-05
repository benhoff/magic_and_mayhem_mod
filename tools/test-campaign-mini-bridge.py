#!/usr/bin/env python3
"""Original campaign Mini Cancel and guarded lifecycle forwarding in private PE32."""
import argparse
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
 parser=argparse.ArgumentParser(description=__doc__);args=parser.parse_args()
 parent=ROOT/'working/tests/campaign-mini-bridge';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
 subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
 try:
  m=load('region_pe','tools/export-menu-support.py');exe=ROOT/'working/game-nocd/Chaos.exe';data=m.pinned_image(exe);header='/* Hash-pinned original bytes in a private fixture. */\n'
  for name,start,end in [('mini_button',0x4b23f0,0x4b24f0)]:
   at=m.image_offset(data,start,end-start);header+='static const unsigned char '+name+'[]={'+','.join(hex(v) for v in data[at:at+end-start])+'};\n'
   print(name+' entry: '+data[at:at+8].hex(),flush=True)
  (out/'campaign_mini_bytes.h').write_text(header)
  load('region_build','tools/build-menu-observer.py').build(out,selftest=True,fixture=False)
  flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-I',str(out)]
  subprocess.run(flags+['-c',str(ROOT/'tests/campaign-mini-bridge-reference.c'),'-o',str(out/'reference.obj')],check=True)
  subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x400000','/nodefaultlib','/timestamp:0',f'/out:{out/"reference.exe"}',str(out/'reference.obj'),str(out/'kernel32.lib'),str(out/'user32.lib')],check=True)
  size=90112;version=9;channel=bytearray(size);channel[:16]=b'MNMMCMD9'+struct.pack('<II',version,size);struct.pack_into('<III',channel,16,2,1,1);(out/'channel.bin').write_bytes(channel)
  env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(ROOT/'working/tests/menu-observer-wine'),WINEDEBUG='-all',MNM_MENU_CHANNEL='Z:'+str(out/'channel.bin').replace('/','\\'))
  with (out/'wine.log').open('w') as log:subprocess.run(['wine',str(out/'reference.exe')],cwd=out,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60)
  assert hashlib.sha256(exe.read_bytes()).hexdigest()==m.HASH
  sources=['protocols/include/mnm/menu_v9.h','runtime/menu/campaign_mini_observe.h','runtime/menu/mini.h','protocols/include/mnm/menu_v8.h','runtime/menu/campaign_world_observe.h','protocols/include/mnm/menu_v7.h','runtime/menu/region_entry.h','runtime/menu/channel.h','runtime/menu/observer.c','tests/campaign-mini-bridge-reference.c','tools/test-campaign-mini-bridge.py','tools/build-menu-observer.py']
  anchors={hex(a):data[m.image_offset(data,a,8):m.image_offset(data,a,8)+8].hex() for a in (0x4b23f0,0x5595d0)}
  report=dict(success=True,anchors=anchors,source_sha256=m.HASH,real_game_launched=False,campaign_mini_validated=True,scope='Original campaign Mini Cancel callback behind V9 mode/caller/stack/callback guards, stale/modal/retired/one-shot rejection, unsupported Preferences/Quit; synthetic guarded Mini tick/World resume forwards return and LastError; no full pause or original parent execution',sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sources})
  (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Campaign Mini V9 fixture passed',flush=True)
 finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__':main()
