#!/usr/bin/env python3
"""Original campaign defeat OK and fade-start bytecode in private PE32."""
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
 parent=ROOT/'working/tests/campaign-defeat-bridge';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
 subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
 try:
  m=load('region_pe','tools/export-menu-support.py');exe=ROOT/'working/game-nocd/Chaos.exe';data=m.pinned_image(exe);header='/* Hash-pinned original bytes in a private fixture. */\n'
  for name,start,end in [('defeat_ok',0x4747a0,0x4747b5),('fade_start',0x557510,0x55753f)]:
   at=m.image_offset(data,start,end-start);header+='static const unsigned char '+name+'[]={'+','.join(hex(v) for v in data[at:at+end-start])+'};\n'
   print(name+' entry: '+data[at:at+8].hex(),flush=True)
  (out/'campaign_defeat_bytes.h').write_text(header)
  load('region_build','tools/build-menu-observer.py').build(out,selftest=True,fixture=False)
  flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-I',str(out)]
  subprocess.run(flags+['-c',str(ROOT/'tests/campaign-defeat-reference.c'),'-o',str(out/'reference.obj')],check=True)
  subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x400000','/nodefaultlib','/timestamp:0',f'/out:{out/"reference.exe"}',str(out/'reference.obj'),str(out/'kernel32.lib'),str(out/'user32.lib')],check=True)
  size=102400;version=11;channel=bytearray(size);channel[:16]=b'MNMMCM11'+struct.pack('<II',version,size);struct.pack_into('<III',channel,16,2,1,1);(out/'channel.bin').write_bytes(channel)
  env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(out/'wineprefix'),WINEDEBUG='-all',WINEDLLOVERRIDES='winedbg.exe=d',MNM_MENU_CHANNEL='Z:'+str(out/'channel.bin').replace('/','\\'))
  with (out/'wine.log').open('w') as log:subprocess.run(['wine',str(out/'reference.exe')],cwd=out,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60)
  assert hashlib.sha256(exe.read_bytes()).hexdigest()==m.HASH
  sources=['runtime/menu/campaign_defeat.h','protocols/include/mnm/menu_v11.h','tools/test-campaign-defeat-bridge.py','runtime/menu/campaign_quit_observe.h','protocols/include/mnm/menu_v10.h','protocols/include/mnm/menu_v9.h','runtime/menu/campaign_mini_observe.h','runtime/menu/mini.h','protocols/include/mnm/menu_v8.h','runtime/menu/campaign_world_observe.h','protocols/include/mnm/menu_v7.h','runtime/menu/region_entry.h','runtime/menu/channel.h','runtime/menu/observer.c','tests/campaign-defeat-reference.c','tools/test-campaign-defeat-bridge.py','tools/build-menu-observer.py']
  anchors={hex(a):data[m.image_offset(data,a,8):m.image_offset(data,a,8)+8].hex() for a in (0x4747a0,0x557510,0x5595d0)}
  report=dict(success=True,anchors=anchors,source_sha256=m.HASH,real_game_launched=False,campaign_mini_validated=True,scope='Original campaign defeat OK and fade-start bytecode behind V11 original report/display/callback/stack/context guards; stale/modal/retired/one-shot and unsupported requests; text fidelity and bounded strings; audio/focus synthetic, original World/Realm unwind not run in fixture; no scoring equivalence',sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sources})
  (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Campaign defeat V11 fixture passed',flush=True)
 finally:
  if (out/'wineprefix').exists():subprocess.run(['wineserver','-k'],env=dict(os.environ,WINEPREFIX=str(out/'wineprefix')),stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=10,check=False)
  subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__':main()
