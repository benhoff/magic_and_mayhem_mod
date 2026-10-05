#!/usr/bin/env python3
"""Automate the Qt V8 campaign Enter with a bounded isolated game run."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import sys
ROOT=Path(__file__).resolve().parents[1]
SOURCES=['protocols/include/mnm/menu_v8.h','runtime/menu/campaign_world_observe.h','protocols/include/mnm/menu_v7.h', 'runtime/menu/region_entry.h', 'runtime/menu/channel.h', 'runtime/menu/observer.c', 'apps/qt-shell/menu_bridge.hpp', 'apps/qt-shell/menu_bridge.cpp', 'apps/qt-shell/live_menu_session.hpp', 'apps/qt-shell/live_menu_session.cpp', 'apps/qt-shell/region_entry_widget.hpp', 'apps/qt-shell/region_entry_widget.cpp', 'apps/qt-shell/live_region_entry_controller.hpp', 'apps/qt-shell/live_region_entry_controller.cpp', 'apps/qt-shell/live_region_enter_test.cpp', 'apps/qt-shell/live_menu_test.hpp', 'apps/qt-shell/live_menu_test.cpp', 'tests/menu-region-bridge-test.cpp', 'tests/menu-region-controller-test.cpp', 'tests/region-entry-bridge-reference.c', 'tools/test-region-entry-bridge.py', 'tools/build-menu-observer.py', 'apps/qt-shell/main.cpp', 'apps/qt-shell/CMakeLists.txt', 'tools/run-menu-observer.py']
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
 if actions!=[(3,0),(18,65538),(18,0)]:raise RuntimeError('Unexpected callback trace: '+str(actions))
 ticks=[r for r in rows if r['event']==12]
 if len(ticks)!=3 or any(r['menu_id']!=2 or r['initialized']!=1 or r['object']!=0x6cbb78 or r['active_screen']!=0x6cbb78 or r['thread']!=report['states'][0]['thread'] for r in ticks):raise RuntimeError('Original gameplay did not tick on the menu engine thread')
 raw=(experiment/'channel.bin').read_bytes();words=struct.unpack_from('<48I',raw)
 if len(raw)!=86016 or raw[:8]!=b'MNMMCMD8' or words[5]!=0 or words[36]!=3 or words[39]!=3:raise RuntimeError('V8 Enter acknowledgement/handoff/retirement mismatch')
 for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','dll_sha256')]:
  if hashlib.sha256((experiment/'game'/name).read_bytes()).hexdigest()!=metadata[key]:raise RuntimeError('Staged inputs changed')
 fingerprints={p:hashlib.sha256((args.source_root/p).read_bytes()).hexdigest() for p in SOURCES}
 for p in ('tools/test-live-region-enter.py','tools/test-live-menus.py'):fingerprints[p]=hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
 build=json.loads((experiment/'bridge-build.json').read_text())
 for p,h in fingerprints.items():
  if p.startswith(('runtime/menu/','protocols/')) and build['sources'].get(Path(p).name)!=h:raise RuntimeError('Bridge source changed during build: '+p)
 report.update(source_sha256=metadata['source_sha256'],sources=fingerprints,experiment=str(experiment),original_callback_trace=actions,world_ticks=ticks,bounded_termination=True,channel_retired=True,original_drawing_retained=True,live_validated=True)
 (root/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Live Qt campaign Enter passed',flush=True)
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--shell',type=Path,default=ROOT/'working/build/qt-shell/mnm-qt-shell');parser.add_argument('--source-root',type=Path,default=ROOT,help='Source tree used to compile the shell');parser.add_argument('--validate-run',type=Path);args=parser.parse_args()
 if args.validate_run:validate(args.validate_run,args);return
 harness=load('region_live_harness','tools/test-live-menus.py');harness.validate=lambda root,unused:validate(root,args)
 os.environ['MNM_LIVE_MENU_TEST_REGION_ENTER']='1';sys.argv=[sys.argv[0],'--exit-from','main','--shell',str(args.shell)];harness.main()
if __name__=='__main__':main()
