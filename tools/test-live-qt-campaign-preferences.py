#!/usr/bin/env python3
"""Bounded automatic V12 campaign Preferences Cancel/OK/reopen/gameplay test."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import sys
ROOT = Path(__file__).resolve().parents[1]
SOURCES = ['protocols/include/mnm/menu_v12.h','runtime/menu/preferences.h',
 'runtime/menu/mini.h','runtime/menu/channel.h','runtime/menu/observer.c',
 'runtime/menu/campaign_mini_observe.h','runtime/menu/campaign_world_observe.h',
 'runtime/menu/region_entry.h','runtime/menu/campaign_defeat.h','runtime/menu/campaign_quit_observe.h',
 'apps/qt-shell/live_campaign_preferences_test.cpp','apps/qt-shell/live_region_enter_test.cpp',
 'apps/qt-shell/live_menu_session.cpp','apps/qt-shell/live_menu_session.hpp',
 'apps/qt-shell/menu_bridge.cpp','apps/qt-shell/menu_bridge.hpp',
 'apps/qt-shell/live_menu_test.cpp','apps/qt-shell/live_menu_test.hpp','apps/qt-shell/CMakeLists.txt',
 'apps/qt-shell/live_preferences_menu_controller.cpp','apps/qt-shell/live_mini_menu_controller.cpp',
 'apps/qt-shell/preferences_widget.cpp','apps/qt-shell/engine_preferences_store.cpp',
 'apps/qt-shell/main.cpp','tools/build-menu-observer.py','tools/run-menu-observer.py',
 'tools/test-live-qt-campaign-preferences.py','tools/test-live-qt-campaign-defeat.py',
 'tools/test-live-campaign-mini.py','tools/test-live-menus.py']
def load(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path)
    result=importlib.util.module_from_spec(spec);spec.loader.exec_module(result);return result

def validate(root,args):
    report=json.loads((root/'report.json').read_text())
    if not report['success'] or not report['campaign_handoff'] or not report['original_viewport_presented']:
        raise RuntimeError('Campaign Preferences harness failed: '+str(report))
    lines=(root/'shell.log').read_text().splitlines()
    if not any(l.startswith('Menu launcher exited with status 0.') for l in lines) or any(l.startswith('Smoke test passed:') for l in lines):
        raise RuntimeError('Missing normal original Quit termination')
    experiment=Path(next(l[20:] for l in lines if l.startswith('Evidence directory: ')))
    metadata=json.loads((experiment/'manifest.json').read_text())
    rows=load('preferences_decode','tools/test-menu-observer.py').decode((experiment/'events.bin').read_bytes())
    actions=[(r['menu_id'],r['argument']) for r in rows if r['event']==3]
    expected=[(3,0),(18,65538),(18,0),(17,2),(10,65537),(10,1),(17,2),(10,65537),(10,0),(17,2),(10,1),(17,3),(6,21),(3,4)]
    if actions!=expected:raise RuntimeError('Unexpected callback trace: '+str(actions))
    thread=report['states'][0]['thread']
    observed=report['campaign_preferences_states']
    if [(s['screen'],s['ack']) for s in observed]!=[(17,3),(10,4),(10,5),(17,6),(10,7),(10,8),(17,9),(10,10),(17,11),(6,12)] or any(s['thread']!=thread for s in observed):
        raise RuntimeError('Incomplete campaign Mini/Preferences round trip')
    initial=report['initial'];committed=report['committed'];preferences=[s for s in observed if s['screen']==10]
    preview=list(initial);preview[1]=committed[1]
    if [s['values'] for s in preferences]!=[initial,preview,initial,preview,committed] or any(s['parent']!=17 or s['depth']!=6 for s in preferences):
        raise RuntimeError('Original preview/rollback/OK/reopened values mismatch')
    if any(not report[k] for k in ('cancel_file_unchanged','store_verified','final_cancel_store_unchanged','main_return')) or report['viewport_returns']!=8:
        raise RuntimeError('Preference file/store/return ownership mismatch')
    raw=(experiment/'channel.bin').read_bytes();words=struct.unpack_from('<48I',raw)
    if len(raw)!=106496 or raw[:8]!=b'MNMMCM12' or words[5]!=0 or words[36]!=14 or words[39]!=0:
        raise RuntimeError('Final V12 acknowledgement/retirement mismatch')
    import configparser
    cfg=configparser.ConfigParser();cfg.read(experiment/'game/CFG/prefs.cfg')
    if cfg.getint('SOUND','SFXVolume')!=committed[1] or cfg.getint('VIDEO','DialogSpeed')!=committed[4]:raise RuntimeError('Original writer mismatch')
    store=root/'config/mnm-qt-shell/engine-preferences.json'
    if json.loads(store.read_text())['values']!=committed:raise RuntimeError('Persisted store mismatch')
    ticks=[r for r in rows if r['event']==12]
    resumes=[r for r in rows if r['event']==20 and r['active_screen']==0x6cbb78]
    if len(ticks)!=3 or len(resumes)!=5 or any(r['thread']!=thread for r in ticks+resumes):raise RuntimeError('Original World initialization/resume mismatch')
    closes=[r for r in rows if r['event']==3 and r['menu_id']==10 and r['argument'] in (0,1)]
    # Each observed original Preferences close must be followed by its own World
    # resume before the next original Mini action; Quit/Yes resumes World before the report; report OK resumes it again.
    for close,resume in zip(closes,resumes):
        following=[r for r in rows if r['event']==3 and r['menu_id']==17 and r['sequence']>close['sequence']]
        if not close['sequence']<resume['sequence']<following[0]['sequence']:raise RuntimeError('Preferences did not resume gameplay before next Escape/Mini')
    answers=[r for r in rows if r['event']==22 and r['depth']==5]
    if len(answers)!=1 or answers[0]['argument']!=0:raise RuntimeError('Original Yes confirmation missing')
    accepted=[r for r in rows if r['event']==3 and r['menu_id']==6 and r['argument']==21]
    if not answers[0]['sequence']<resumes[3]['sequence']<accepted[0]['sequence']<resumes[4]['sequence']:
        raise RuntimeError('Original Quit/report World-resume order mismatch')
    for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','dll_sha256')]:
        if hashlib.sha256((experiment/'game'/name).read_bytes()).hexdigest()!=metadata[key]:raise RuntimeError('Staged inputs changed')
    fingerprints={p:hashlib.sha256((args.source_root/p).read_bytes()).hexdigest() for p in SOURCES}
    build=json.loads((experiment/'bridge-build.json').read_text())
    for p,h in fingerprints.items():
        if p.startswith(('runtime/menu/','protocols/')) and build['sources'].get(Path(p).name)!=h:raise RuntimeError('Bridge fingerprint mismatch: '+p)
    report.update(source_sha256=metadata['source_sha256'],sources=fingerprints,experiment=str(experiment),
                  original_callback_trace=actions,world_ticks=ticks,world_resumes=resumes,
                  confirmation_answers=answers,normal_exit=True,channel_retired=True,live_validated=True,
                  lifecycle_observed=True,pause_equivalence=False)
    (root/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Live campaign Preferences Cancel/OK/reopen/gameplay return passed',flush=True)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--shell',type=Path,default=ROOT/'working/build/qt-campaign-preferences-live/mnm-qt-shell')
    parser.add_argument('--source-root',type=Path,default=ROOT,help='Source tree used to compile the shell')
    parser.add_argument('--validate-run',type=Path)
    args=parser.parse_args()
    if args.validate_run:validate(args.validate_run,args);return
    harness=load('preferences_live_harness','tools/test-live-menus.py')
    harness.validate=lambda root,unused:validate(root,args)
    for key in list(os.environ):
        if key.startswith('MNM_LIVE_MENU_TEST_'):os.environ.pop(key)
    os.environ['MNM_LIVE_MENU_TEST_CAMPAIGN_PREFERENCES']='1'
    sys.argv=[sys.argv[0],'--exit-from','main','--shell',str(args.shell)]
    harness.main()
if __name__=='__main__':main()
