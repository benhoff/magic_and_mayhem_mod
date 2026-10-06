#!/usr/bin/env python3
"""Automate V10 campaign Quit, original confirmation No/Yes and normal exit."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import sys
ROOT=Path(__file__).resolve().parents[1]
SOURCES=['protocols/include/mnm/menu_v10.h','runtime/menu/campaign_quit_observe.h','tests/campaign-mini-quit-reference.c','tests/menu-campaign-mini-quit-test.cpp','tools/test-campaign-mini-quit-bridge.py','tools/test-live-campaign-quit.py','apps/qt-shell/live_campaign_quit_test.cpp','protocols/include/mnm/menu_v9.h', 'runtime/menu/mini.h', 'runtime/menu/campaign_mini_observe.h', 'runtime/menu/channel.h', 'runtime/menu/region_entry.h', 'runtime/menu/observer.c', 'tools/build-menu-observer.py', 'tools/run-menu-observer.py', 'tools/test-campaign-mini-bridge.py', 'tools/test-live-campaign-entry.py', 'tools/test-live-qt-campaign-quit.py', 'tests/campaign-mini-bridge-reference.c', 'tests/menu-campaign-mini-bridge-test.cpp', 'apps/qt-shell/menu_bridge.cpp', 'apps/qt-shell/menu_bridge.hpp', 'apps/qt-shell/live_menu_session.cpp', 'apps/qt-shell/live_menu_session.hpp', 'apps/qt-shell/live_menu_test.cpp', 'apps/qt-shell/live_menu_test.hpp', 'apps/qt-shell/live_campaign_mini_test.cpp', 'apps/qt-shell/live_mini_menu_controller.cpp', 'apps/qt-shell/mini_menu_widget.cpp', 'apps/qt-shell/CMakeLists.txt', 'protocols/include/mnm/menu_v8.h', 'runtime/menu/campaign_world_observe.h', 'protocols/include/mnm/menu_v7.h', 'apps/qt-shell/region_entry_widget.hpp', 'apps/qt-shell/region_entry_widget.cpp', 'apps/qt-shell/live_region_entry_controller.hpp', 'apps/qt-shell/live_region_entry_controller.cpp', 'apps/qt-shell/live_region_enter_test.cpp', 'tests/menu-region-bridge-test.cpp', 'tests/menu-region-controller-test.cpp', 'tests/region-entry-bridge-reference.c', 'tools/test-region-entry-bridge.py', 'apps/qt-shell/main.cpp']
def load(name,path):
 s=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
def validate(root,args):
 report=json.loads((root/'report.json').read_text())
 if not report['success'] or not report['campaign_handoff'] or not report['original_viewport_presented'] or [(s['screen'],s['ack']) for s in report['states']]!=[(3,0),(18,1),(18,2)]:raise RuntimeError('Incomplete Qt Enter handoff')
 if len({s['thread'] for s in report['states']})!=1:raise RuntimeError('Engine thread changed')
 lines=(root/'shell.log').read_text().splitlines()
 if not any(l.startswith('Menu launcher exited with status 0.') for l in lines) or any(l.startswith('Smoke test passed:') for l in lines):raise RuntimeError('Missing normal original Quit termination')
 experiment=Path(next(l[20:] for l in lines if l.startswith('Evidence directory: ')));metadata=json.loads((experiment/'manifest.json').read_text())
 rows=load('enter_decode','tools/test-menu-observer.py').decode((experiment/'events.bin').read_bytes());actions=[(r['menu_id'],r['argument']) for r in rows if r['event']==3]
 if actions!=[(3,0),(18,65538),(18,0),(17,3),(17,3),(3,4)]:raise RuntimeError('Unexpected callback trace: '+str(actions))
 ticks=[r for r in rows if r['event']==12]
 if len(ticks)!=3 or any(r['menu_id']!=2 or r['initialized']!=1 or r['object']!=0x6cbb78 or r['active_screen']!=0x6cbb78 or r['thread']!=report['states'][0]['thread'] for r in ticks):raise RuntimeError('Original gameplay did not tick on the menu engine thread')
 raw=(experiment/'channel.bin').read_bytes();words=struct.unpack_from('<48I',raw)
 if len(raw)!=94208 or raw[:8]!=b'MNMMCM10' or words[5]!=0 or words[36]!=6 or words[39]!=0:raise RuntimeError('V8 Enter acknowledgement/handoff/retirement mismatch')
 for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','dll_sha256')]:
  if hashlib.sha256((experiment/'game'/name).read_bytes()).hexdigest()!=metadata[key]:raise RuntimeError('Staged inputs changed')
 fingerprints={p:hashlib.sha256((args.source_root/p).read_bytes()).hexdigest() for p in SOURCES}
 for p in ('tools/test-live-qt-campaign-quit.py','tools/test-live-menus.py'):fingerprints[p]=hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
 build=json.loads((experiment/'bridge-build.json').read_text())
 for p,h in fingerprints.items():
  if p.startswith(('runtime/menu/','protocols/')) and build['sources'].get(Path(p).name)!=h:raise RuntimeError('Bridge source changed during build: '+p)
 mini=report['mini_states']
 if report['viewport_returns']!=2 or [(s['screen'],s['ack']) for s in mini]!=[(17,3),(17,4)] or any(s['thread']!=report['states'][0]['thread'] for s in mini):raise RuntimeError('Incomplete native campaign Mini round trip')
 answers=[r for r in rows if r['event']==22 and r['depth']==5]
 if [r['argument'] for r in answers]!=[1,0] or any(r['next_screen'] or r['returning']!=1 or r['thread']!=report['states'][0]['thread'] for r in answers):raise RuntimeError('Original No/Yes confirmation mismatch')
 flags=[r['result'] for r in rows if r['event']==25 and r['depth']==5]
 if flags!=[0,1] or not report['main_return']:raise RuntimeError('Original campaign exit flag/Main return mismatch')
 exit_evidence=json.loads((root/'original-campaign-exit.json').read_text())
 if not exit_evidence['success'] or exit_evidence['main_return']['sequence']<=answers[-1]['sequence'] or exit_evidence['outcome']['active_screen']!=0x6db967:raise RuntimeError('Original defeat/Realm exit route incomplete')
 report['campaign_exit']=exit_evidence;report['confirmation_answers']=answers;report['exit_flags']=flags
 resumes=[r for r in rows if r['event']==20 and r['active_screen']==0x6cbb78]
 if len(resumes)<1 or any(r['thread']!=report['states'][0]['thread'] for r in resumes):raise RuntimeError('Original World resume not observed')
 report['world_resumes']=resumes
 report.update(source_sha256=metadata['source_sha256'],sources=fingerprints,experiment=str(experiment),original_callback_trace=actions,world_ticks=ticks,bounded_termination=False,normal_exit=True,channel_retired=True,original_drawing_retained=True,live_validated=True)
 (root/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Live Qt campaign Quit No/Yes return passed',flush=True)
def finish_exit(experiment,origin,capture):
 """Validation-only original Battle End OK and fresh original World/Realm return to Main."""
 import ctypes as c,time,configparser
 decode=load('quit_exit_decode','tools/test-menu-observer.py').decode
 x=c.CDLL('libX11.so.6');xt=c.CDLL('libXtst.so.6');x.XOpenDisplay.argtypes=[c.c_char_p];x.XOpenDisplay.restype=c.c_void_p;x.XFlush.argtypes=x.XCloseDisplay.argtypes=[c.c_void_p];x.XKeysymToKeycode.argtypes=[c.c_void_p,c.c_ulong];x.XKeysymToKeycode.restype=c.c_uint;xt.XTestFakeMotionEvent.argtypes=[c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong];xt.XTestFakeButtonEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong];xt.XTestFakeKeyEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
 d=x.XOpenDisplay(os.environ['DISPLAY'].encode())
 if not d:raise RuntimeError('No isolated exit input display')
 def rows():return decode((experiment/'events.bin').read_bytes())
 def wait(predicate,label,seconds=12):
  deadline=time.monotonic()+seconds
  while time.monotonic()<deadline:
   found=predicate(rows())
   if found:return found
   time.sleep(.05)
  raise RuntimeError(label)
 def click(point):
  xt.XTestFakeMotionEvent(d,-1,origin[0]+point[0],origin[1]+point[1],0);x.XFlush(d);time.sleep(.2);xt.XTestFakeButtonEvent(d,1,1,0);x.XFlush(d);time.sleep(.15);xt.XTestFakeButtonEvent(d,1,0,0);x.XFlush(d)
 def control(path,section):
  cfg=configparser.ConfigParser(inline_comment_prefixes=(';',),interpolation=None);cfg.read(experiment/'game/Interface'/path);rect=list(map(int,cfg[section]['Rect2'].split(',')));click(((rect[0]+rect[2])//2,(rect[1]+rect[3])//2))
 def shot(name):
  if capture:
   from PIL import ImageGrab
   ImageGrab.grab(xdisplay=os.environ['DISPLAY']).save(capture.with_name(name+'.png'))
 try:
  outcome=wait(lambda r:[a for a in r if a['event']==28 and a['active_screen']==0x6db967],'Original campaign defeat report not reached')[-1]
  start=outcome['sequence'];time.sleep(.6);shot('original-campaign-defeat');control('BattleEnd/screen (Battle End - Defeat).cfg','TEXTBUTTON_1')
  def main(r):return [a for a in r if a['sequence']>start and a['event'] in (1,4) and a['menu_id']==3 and a['initialized']==1 and not a['next_screen'] and not a['returning'] and not a['fade_active'] and a['active_screen']==0x657ce0]
  returned=wait(main,'Original report/Realm path did not return to fresh Main',20)[-1];shot('original-campaign-main')
  evidence=dict(success=True,outcome=outcome,realm_return=[r for r in rows() if r['event']==28 and r['sequence']>start and r['active_screen']==0x659408],main_return=returned,scope='Original campaign defeat report OK, original World/Realm return to fresh Main; native report/Realm integration not asserted')
  if capture:capture.with_name('original-campaign-exit.json').write_text(json.dumps(evidence,indent=2)+'\n')
  return evidence
 finally:x.XCloseDisplay(d)

def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--finish-exit',action='store_true');parser.add_argument('--answer',choices=['no','yes']);parser.add_argument('--origin-x',type=int);parser.add_argument('--origin-y',type=int);parser.add_argument('--experiment',type=Path);parser.add_argument('--capture',type=Path);parser.add_argument('--shell',type=Path,default=ROOT/'working/build/qt-shell/mnm-qt-shell');parser.add_argument('--source-root',type=Path,default=ROOT,help='Source tree used to compile the shell');parser.add_argument('--validate-run',type=Path);args=parser.parse_args()
 if args.finish_exit:
  finish_exit(args.experiment,(args.origin_x,args.origin_y),args.capture);return
 if args.answer:
  import ctypes as c,time
  decode=load('quit_input_decode','tools/test-menu-observer.py').decode;deadline=time.monotonic()+8
  while time.monotonic()<deadline:
   rows=decode((args.experiment/'events.bin').read_bytes());modal=[r for r in rows if r['event']==18];answers=[r for r in rows if r['event']==22]
   if modal and modal[-1]['argument'] and (not answers or modal[-1]['sequence']>answers[-1]['sequence']):break
   time.sleep(.05)
  else:raise RuntimeError('No fresh original confirmation')
  time.sleep(.7)
  if args.capture:
   from PIL import ImageGrab
   ImageGrab.grab(xdisplay=os.environ['DISPLAY']).save(args.capture)
  x=c.CDLL('libX11.so.6');xt=c.CDLL('libXtst.so.6');x.XOpenDisplay.argtypes=[c.c_char_p];x.XOpenDisplay.restype=c.c_void_p;x.XFlush.argtypes=x.XCloseDisplay.argtypes=[c.c_void_p];xt.XTestFakeMotionEvent.argtypes=[c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong];xt.XTestFakeButtonEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
  d=x.XOpenDisplay(os.environ['DISPLAY'].encode())
  if not d:raise RuntimeError('No isolated input display')
  # Confirmed original 800x600 dialog: No/Yes centers; use only in isolated Xvfb validation.
  if args.origin_x is None or args.origin_y is None:raise RuntimeError('Missing embedded original surface origin')
  point=(args.origin_x+(535 if args.answer=='no' else 265),args.origin_y+395)
  if args.capture:args.capture.with_suffix('.json').write_text(json.dumps(dict(answer=args.answer,origin=[args.origin_x,args.origin_y],point=point))+'\n')
  xt.XTestFakeMotionEvent(d,-1,*point,0);x.XFlush(d);time.sleep(.2);xt.XTestFakeButtonEvent(d,1,1,0);x.XFlush(d);time.sleep(.15);xt.XTestFakeButtonEvent(d,1,0,0);x.XFlush(d);x.XCloseDisplay(d);deadline=time.monotonic()+10
  while time.monotonic()<deadline:
   fresh=decode((args.experiment/'events.bin').read_bytes());answered=[r for r in fresh if r['event']==22 and r['sequence']>modal[-1]['sequence']]
   if answered and answered[-1]['argument']==(1 if args.answer=='no' else 0) and (args.answer=='yes' or any(r['event']==20 and r['sequence']>answered[-1]['sequence'] for r in fresh)):
    if args.answer=='yes':finish_exit(args.experiment,(args.origin_x,args.origin_y),args.capture)
    return
   time.sleep(.05)
  raise RuntimeError('Original confirmation answer/return missing')
 if args.validate_run:validate(args.validate_run,args);return
 harness=load('region_live_harness','tools/test-live-menus.py');harness.validate=lambda root,unused:validate(root,args)
 os.environ['MNM_LIVE_MENU_TEST_CAMPAIGN_QUIT']='1';sys.argv=[sys.argv[0],'--exit-from','main','--shell',str(args.shell)];harness.main()
if __name__=='__main__':main()
