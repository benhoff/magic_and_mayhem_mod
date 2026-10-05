#!/usr/bin/env python3
"""Automate the Qt V7 Region Entry round trip with a bounded isolated game run."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import sys
ROOT=Path(__file__).resolve().parents[1]
SOURCES=['protocols/include/mnm/menu_v7.h', 'runtime/menu/region_entry.h', 'runtime/menu/channel.h', 'runtime/menu/observer.c', 'apps/qt-shell/menu_bridge.hpp', 'apps/qt-shell/menu_bridge.cpp', 'apps/qt-shell/live_menu_session.hpp', 'apps/qt-shell/live_menu_session.cpp', 'apps/qt-shell/region_entry_widget.hpp', 'apps/qt-shell/region_entry_widget.cpp', 'apps/qt-shell/live_region_entry_controller.hpp', 'apps/qt-shell/live_region_entry_controller.cpp', 'apps/qt-shell/live_region_entry_test.cpp', 'apps/qt-shell/live_menu_test.hpp', 'apps/qt-shell/live_menu_test.cpp', 'tests/menu-region-bridge-test.cpp', 'tests/menu-region-controller-test.cpp', 'tests/region-entry-bridge-reference.c', 'tools/test-region-entry-bridge.py', 'tools/build-menu-observer.py', 'apps/qt-shell/main.cpp', 'apps/qt-shell/CMakeLists.txt', 'tools/run-menu-observer.py']
def load(name,path):
 s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
def validate(root,args):
 report=json.loads((root/'report.json').read_text());screens=[3,18,18,18,18,18,3,18]
 if not report['success'] or [(s['screen'],s['ack']) for s in report['states']]!=list(zip(screens,range(8))):raise RuntimeError('Incomplete Qt Region Entry round trip')
 if len({s['thread'] for s in report['states']})!=1:raise RuntimeError('Engine thread changed')
 if [s['difficulty'] for s in report['states'] if s['screen']==18]!=[0,1,2,3,0,0]:raise RuntimeError('Original difficulties differ from Qt controls')
 lines=(root/'shell.log').read_text().splitlines()
 if not any(l.startswith('Menu launcher exited with status 0.') for l in lines):raise RuntimeError('Original Quit did not finish normally')
 experiments=[l[20:] for l in lines if l.startswith('Evidence directory: ')]
 if len(experiments)!=1:raise RuntimeError('Missing provenance')
 experiment=Path(experiments[0]);metadata=json.loads((experiment/'manifest.json').read_text())
 rows=load('region_decode','tools/test-menu-observer.py').decode((experiment/'events.bin').read_bytes());actions=[(r['menu_id'],r['argument']) for r in rows if r['event']==3]
 expected=[(3,0),(18,65537),(18,65538),(18,65539),(18,65536),(18,1),(3,0),(18,1),(3,4)]
 if actions!=expected:raise RuntimeError('Unexpected original callback sequence: '+str(actions))
 raw=(experiment/'channel.bin').read_bytes();words=struct.unpack_from('<48I',raw)
 if len(raw)!=81920 or raw[:8]!=b'MNMMCMD7' or words[5]!=0 or words[36]!=9:raise RuntimeError('V7 channel did not retire after Cancel/Quit acknowledgement')
 for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','dll_sha256')]:
  if hashlib.sha256((experiment/'game'/name).read_bytes()).hexdigest()!=metadata[key]:raise RuntimeError('Staged inputs changed')
 fingerprints={p:hashlib.sha256((args.source_root/p).read_bytes()).hexdigest() for p in SOURCES}
 for p in ('tools/test-live-region-entry.py','tools/test-live-menus.py'):
  fingerprints[p]=hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
 report.update(source_sha256=metadata['source_sha256'],sources=fingerprints,experiment=str(experiment),original_callback_trace=actions,normal_exit=True,channel_retired=True,original_drawing_retained=True,live_validated=True)
 (root/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Live Qt Region Entry round trip passed',flush=True)
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--shell',type=Path,default=ROOT/'working/build/qt-shell/mnm-qt-shell');parser.add_argument('--source-root',type=Path,default=ROOT,help='Source tree used to compile the shell');parser.add_argument('--validate-run',type=Path);args=parser.parse_args()
 if args.validate_run:validate(args.validate_run,args);return
 harness=load('region_live_harness','tools/test-live-menus.py');harness.validate=lambda root,unused:validate(root,args)
 os.environ['MNM_LIVE_MENU_TEST_REGION']='1';sys.argv=[sys.argv[0],'--exit-from','main','--shell',str(args.shell)];harness.main()
if __name__=='__main__':main()
