#!/usr/bin/env python3
"""Automate the Qt V9 campaign Mini Cancel return with a bounded isolated game run."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import sys
ROOT=Path(__file__).resolve().parents[1]
SOURCES=['protocols/include/mnm/menu_v9.h', 'runtime/menu/mini.h', 'runtime/menu/campaign_mini_observe.h', 'runtime/menu/channel.h', 'runtime/menu/region_entry.h', 'runtime/menu/observer.c', 'tools/build-menu-observer.py', 'tools/run-menu-observer.py', 'tools/test-campaign-mini-bridge.py', 'tools/test-live-campaign-entry.py', 'tools/test-live-campaign-mini.py', 'tests/campaign-mini-bridge-reference.c', 'tests/menu-campaign-mini-bridge-test.cpp', 'apps/qt-shell/menu_bridge.cpp', 'apps/qt-shell/menu_bridge.hpp', 'apps/qt-shell/live_menu_session.cpp', 'apps/qt-shell/live_menu_session.hpp', 'apps/qt-shell/live_menu_test.cpp', 'apps/qt-shell/live_menu_test.hpp', 'apps/qt-shell/live_campaign_mini_test.cpp', 'apps/qt-shell/live_mini_menu_controller.cpp', 'apps/qt-shell/mini_menu_widget.cpp', 'apps/qt-shell/CMakeLists.txt', 'protocols/include/mnm/menu_v8.h', 'runtime/menu/campaign_world_observe.h', 'protocols/include/mnm/menu_v7.h', 'apps/qt-shell/region_entry_widget.hpp', 'apps/qt-shell/region_entry_widget.cpp', 'apps/qt-shell/live_region_entry_controller.hpp', 'apps/qt-shell/live_region_entry_controller.cpp', 'apps/qt-shell/live_region_enter_test.cpp', 'tests/menu-region-bridge-test.cpp', 'tests/menu-region-controller-test.cpp', 'tests/region-entry-bridge-reference.c', 'tools/test-region-entry-bridge.py', 'apps/qt-shell/main.cpp']
def load(name,path):
 s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
def validate(root,args):
 report=json.loads((root/'report.json').read_text())
 if not report['success'] or not report['campaign_handoff'] or not report['original_viewport_presented'] or [(s['screen'],s['ack']) for s in report['states']]!=[(3,0),(18,1),(18,2)]:raise RuntimeError('Incomplete Qt Enter handoff')
 if len({s['thread'] for s in report['states']})!=1:raise RuntimeError('Engine thread changed')
 lines=(root/'shell.log').read_text().splitlines()
 if not any(l.startswith('Smoke test passed:') for l in lines):raise RuntimeError('Missing bounded original termination')
 experiment=Path(next(l[20:] for l in lines if l.startswith('Evidence directory: ')));metadata=json.loads((experiment/'manifest.json').read_text())
 rows=load('enter_decode','tools/test-menu-observer.py').decode((experiment/'events.bin').read_bytes());actions=[(r['menu_id'],r['argument']) for r in rows if r['event']==3]
 if actions!=[(3,0),(18,65538),(18,0),(17,4),(17,4)]:raise RuntimeError('Unexpected callback trace: '+str(actions))
 ticks=[r for r in rows if r['event']==12]
 if len(ticks)!=3 or any(r['menu_id']!=2 or r['initialized']!=1 or r['object']!=0x6cbb78 or r['active_screen']!=0x6cbb78 or r['thread']!=report['states'][0]['thread'] for r in ticks):raise RuntimeError('Original gameplay did not tick on the menu engine thread')
 raw=(experiment/'channel.bin').read_bytes();words=struct.unpack_from('<48I',raw)
 if len(raw)!=90112 or raw[:8]!=b'MNMMCMD9' or words[5]!=0 or words[36]!=5 or words[39]!=3:raise RuntimeError('V8 Enter acknowledgement/handoff/retirement mismatch')
 for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','dll_sha256')]:
  if hashlib.sha256((experiment/'game'/name).read_bytes()).hexdigest()!=metadata[key]:raise RuntimeError('Staged inputs changed')
 fingerprints={p:hashlib.sha256((args.source_root/p).read_bytes()).hexdigest() for p in SOURCES}
 for p in ('tools/test-live-campaign-mini.py','tools/test-live-menus.py'):fingerprints[p]=hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
 build=json.loads((experiment/'bridge-build.json').read_text())
 for p,h in fingerprints.items():
  if p.startswith(('runtime/menu/','protocols/')) and build['sources'].get(Path(p).name)!=h:raise RuntimeError('Bridge source changed during build: '+p)
 mini=report['mini_states']
 if report['viewport_returns']!=2 or [(s['screen'],s['ack']) for s in mini]!=[(17,3),(17,4)] or any(s['thread']!=report['states'][0]['thread'] for s in mini):raise RuntimeError('Incomplete native campaign Mini round trip')
 resumes=[r for r in rows if r['event']==20 and r['active_screen']==0x6cbb78]
 if len(resumes)<2 or any(r['thread']!=report['states'][0]['thread'] for r in resumes):raise RuntimeError('Original World resume not observed twice')
 report['world_resumes']=resumes
 report.update(source_sha256=metadata['source_sha256'],sources=fingerprints,experiment=str(experiment),original_callback_trace=actions,world_ticks=ticks,bounded_termination=True,channel_retired=True,original_drawing_retained=True,live_validated=True)
 (root/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Live Qt campaign Mini Cancel return passed',flush=True)
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--send-escape',action='store_true');parser.add_argument('--shell',type=Path,default=ROOT/'working/build/qt-shell/mnm-qt-shell');parser.add_argument('--source-root',type=Path,default=ROOT,help='Source tree used to compile the shell');parser.add_argument('--validate-run',type=Path);args=parser.parse_args()
 if args.send_escape:
  import ctypes as c,time
  x=c.CDLL('libX11.so.6');xt=c.CDLL('libXtst.so.6');x.XOpenDisplay.argtypes=[c.c_char_p];x.XOpenDisplay.restype=c.c_void_p;x.XFlush.argtypes=x.XCloseDisplay.argtypes=[c.c_void_p];x.XKeysymToKeycode.argtypes=[c.c_void_p,c.c_ulong];x.XKeysymToKeycode.restype=c.c_uint;xt.XTestFakeKeyEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
  d=x.XOpenDisplay(os.environ['DISPLAY'].encode())
  if not d:raise RuntimeError('No isolated input display')
  key=x.XKeysymToKeycode(d,0xff1b);xt.XTestFakeKeyEvent(d,key,1,0);x.XFlush(d);time.sleep(.08);xt.XTestFakeKeyEvent(d,key,0,0);x.XFlush(d);x.XCloseDisplay(d);return
 if args.validate_run:validate(args.validate_run,args);return
 harness=load('region_live_harness','tools/test-live-menus.py');harness.validate=lambda root,unused:validate(root,args)
 os.environ['MNM_LIVE_MENU_TEST_CAMPAIGN_MINI']='1';sys.argv=[sys.argv[0],'--exit-from','main','--shell',str(args.shell)];harness.main()
if __name__=='__main__':main()
