#!/usr/bin/env python3
"""Bounded original New Game/Realm tick observation; no native campaign dispatch."""
import argparse
import configparser
import ctypes as c
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import signal
import struct
import subprocess
import tempfile
import time
ROOT=Path(__file__).resolve().parents[1]
FIELDS='sequence phase thread receiver vtable screen owner depth tick_state returning visible_regions realm_name hit_map auxiliary_callback wizard_count player last_region initialized fade_active tutorial world_return selected_region outcome parent owner_screen context result last_error spell_flag grimoire_flag character_flag mini_flag'.split()
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def decode(data):
    if len(data)<16:return []
    if data[:16] not in (b'MNMCAMP1'+struct.pack('<II',1,128),b'MNMCAMP2'+struct.pack('<II',2,128)):raise ValueError('Unsupported campaign log')
    rows=[dict(zip(FIELDS,struct.unpack_from('<32I',data,at))) for at in range(16,len(data)-127,128)]
    if len(rows)>256 or [r['sequence'] for r in rows]!=list(range(1,len(rows)+1)):raise ValueError('Invalid campaign log sequence')
    return rows
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    choice=parser.add_mutually_exclusive_group()
    choice.add_argument('--mini-cancel',action='store_true',help='Observe campaign gameplay Escape/Mini/Cancel and Escape return twice')
    choice.add_argument('--region-enter',action='store_true',help='Observe original Enter through the first three original gameplay ticks')
    choice.add_argument('--region-return',action='store_true',help='Observe four original difficulty choices and fresh campaign Cancel through Realm resume to Main')
    args=parser.parse_args()
    if args.mini_cancel:args.region_enter=True
    parent=ROOT/'working/tests/live-campaign-entry';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(root,flush=True)
    scope = ('Original fresh New Game ingress, four Region Entry difficulty choices and Cancel/Realm resume to Main; no live Enter, loaded Realm, Qt campaign integration or replacement'
             if args.region_return else 'Original Main New Game ingress and forwarded Realm tick only; no Qt campaign integration, region interaction, campaign Mini confirmation or native replacement')
    if args.region_enter:scope='Original New Game, difficulty choices and Enter through three original gameplay ticks; no Qt Enter integration, loaded Realm or native replacement'
    server=launcher=display=None;prefix=root/'wineprefix';report={'success':False,'scope':scope}
    env=dict(os.environ)
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        experiment=module('campaign_stage','tools/prepare-menu-observer.py').prepare(campaign_observe=True)
        metadata=json.loads((experiment/'manifest.json').read_text());report.update(experiment=str(experiment),source_sha256=metadata['source_sha256'],dll_sha256=metadata['dll_sha256'])
        cfg=configparser.ConfigParser(inline_comment_prefixes=(';',),interpolation=None);cfg.read(experiment/'game/Interface/MainScreen/screen (MainMenu).cfg')
        rect=list(map(int,cfg['TEXTBUTTON_1']['Rect2'].split(',')));point=[(rect[0]+rect[2])//2,(rect[1]+rect[3])//2];report['click_point']=point
        with (root/'display').open('w+') as number_file,(root/'xvfb.log').open('w') as log:
            server=subprocess.Popen(['Xvfb','-displayfd',str(number_file.fileno()),'-screen','0','800x600x24','-nolisten','tcp'],pass_fds=(number_file.fileno(),),stdout=log,stderr=subprocess.STDOUT)
            deadline=time.monotonic()+10
            while time.monotonic()<deadline:
                number_file.seek(0);number=number_file.read().strip()
                if number:break
                if server.poll() is not None:raise RuntimeError('Xvfb exited')
                time.sleep(.1)
            else:raise RuntimeError('No isolated display')
        env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
        env.pop('WAYLAND_DISPLAY',None);env.update(DISPLAY=':'+number,WINEDEBUG='-all',LIBGL_ALWAYS_SOFTWARE='1',MNM_MENU_CAMPAIGN_OBSERVE='Z:'+str(experiment/'campaign.bin').replace('/','\\'))
        with (root/'launcher.log').open('w') as log:
            launcher=subprocess.Popen([str(ROOT/'tools/run-menu-observer.py'),str(experiment),'--seconds','100','--prefix',str(prefix)],env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
        observer=module('campaign_menu_decode','tools/test-menu-observer.py');events=experiment/'events.bin'
        deadline=time.monotonic()+80
        while time.monotonic()<deadline:
            rows=observer.decode(events.read_bytes()) if events.exists() and (events.stat().st_size-16)%64==0 else []
            if any(r['event'] in (1,4) and r['menu_id']==3 and r['initialized']==1 and not r['next_screen'] and not r['returning'] and not r['fade_active'] for r in rows):break
            if launcher.poll() is not None:raise RuntimeError('Launcher exited before Main readiness')
            time.sleep(.1)
        else:raise RuntimeError('Main did not become ready within 80 seconds')
        x=c.CDLL('libX11.so.6');xt=c.CDLL('libXtst.so.6');x.XOpenDisplay.argtypes=[c.c_char_p];x.XOpenDisplay.restype=c.c_void_p;x.XFlush.argtypes=x.XCloseDisplay.argtypes=[c.c_void_p]
        xt.XTestFakeMotionEvent.argtypes=[c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong];xt.XTestFakeButtonEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
        display=x.XOpenDisplay(env['DISPLAY'].encode())
        if not display:raise RuntimeError('No input display')
        xt.XTestFakeMotionEvent(display,-1,*point,0);xt.XTestFakeButtonEvent(display,1,1,0);x.XFlush(display);time.sleep(.1);xt.XTestFakeButtonEvent(display,1,0,0);x.XFlush(display)
        deadline=time.monotonic()+15;campaign=experiment/'campaign.bin'
        while time.monotonic()<deadline:
            rows=decode(campaign.read_bytes()) if campaign.exists() else []
            if rows and time.monotonic()>deadline-5:break
            if launcher.poll() is not None:raise RuntimeError('Launcher exited during campaign ingress')
            time.sleep(.1)
        actions=observer.decode(events.read_bytes());rows=decode(campaign.read_bytes()) if campaign.exists() else []
        new_game=[r for r in actions if r['event']==3 and r['menu_id']==3 and r['argument']==0]
        if len(new_game)!=1 or new_game[0]['next_screen']!=0x659408:raise RuntimeError('Original New Game callback/request not observed')
        if not rows or not any(r['phase']==2 for r in rows):raise RuntimeError('No forwarded Realm tick evidence')
        if any(r['receiver']!=0x659408 or r['vtable']!=0x5c6a60 or r['screen']!=4 or r['context']!=5 or r['wizard_count']!=9 for r in rows):raise RuntimeError('Campaign build/state mismatch')
        if len({r['thread'] for r in rows}|{new_game[0]['thread']})!=1:raise RuntimeError('Campaign observation changed thread')
        entry_trace=[]
        if args.region_return or args.region_enter:
            cfg_entry=configparser.ConfigParser(inline_comment_prefixes=(';',),interpolation=None);cfg_entry.read(experiment/'game/Interface/RegionEntry/screen (Region Entry).cfg')
            def click_control(section):
                r=list(map(int,cfg_entry[section]['Rect2'].split(',')));px=(r[0]+r[2])//2;py=(r[1]+r[3])//2
                xt.XTestFakeMotionEvent(display,-1,px,py,0);x.XFlush(display);time.sleep(.25);xt.XTestFakeButtonEvent(display,1,1,0);x.XFlush(display);time.sleep(.15);xt.XTestFakeButtonEvent(display,1,0,0);x.XFlush(display);time.sleep(.15)
            def entry_rows():return [r for r in observer.decode(events.read_bytes()) if r['event'] in (9,10) and r['menu_id']==18]
            deadline=time.monotonic()+5
            while time.monotonic()<deadline:
                entry_trace=entry_rows()
                if any(r['initialized']==1 and not r['fade_active'] and not r['next_screen'] and not r['returning'] and r['result']==4 for r in entry_trace):break
                time.sleep(.05)
            else:raise RuntimeError('Campaign Region Entry did not become ready')
            choices=[]
            for choice in [1,2,3,0]:
                before=len(entry_rows());click_control('RADIOBUTTON_'+str(choice+1));deadline=time.monotonic()+3
                while time.monotonic()<deadline:
                    fresh=entry_rows()[before:]
                    if any(r['argument']==choice for r in fresh):break
                    time.sleep(.05)
                else:raise RuntimeError('Original difficulty choice not observed: '+str(choice))
                choices.append(choice)
            if args.region_enter:
                click_control('TEXTBUTTON_1');deadline=time.monotonic()+40
                while time.monotonic()<deadline:
                    trace=observer.decode(events.read_bytes())
                    world=[r for r in trace if r['event']==12 and r['menu_id']==2 and r['initialized']==1 and r['active_screen']==0x6cbb78]
                    if len(world)==3:break
                    if launcher.poll() is not None:raise RuntimeError('Launcher exited during campaign battle loading')
                    time.sleep(.05)
                else:raise RuntimeError('Original Enter did not reach three gameplay ticks')
                rows=decode(campaign.read_bytes());entry_trace=entry_rows()
                if len({r['thread'] for r in world}|{new_game[0]['thread']})!=1:raise RuntimeError('Campaign world tick changed thread')
                report.update(region_enter=True,difficulty_choices=choices,entry_records=entry_trace,world_ticks=world,world_owner_screen=2)
                if args.mini_cancel:
                    xt.XTestFakeKeyEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong];x.XKeysymToKeycode.argtypes=[c.c_void_p,c.c_ulong];x.XKeysymToKeycode.restype=c.c_uint
                    def key(sym):
                        code=x.XKeysymToKeycode(display,sym);xt.XTestFakeKeyEvent(display,code,1,0);x.XFlush(display);time.sleep(.1);xt.XTestFakeKeyEvent(display,code,0,0);x.XFlush(display);time.sleep(.2)
                    def trace():return observer.decode(events.read_bytes())
                    def mini_ready(after):return [r for r in trace()[after:] if r['event']==16 and r['menu_id']==17 and r['initialized']==1 and not r['next_screen'] and not r['returning'] and not r['fade_active'] and r['active_screen']==0x6a5088]
                    mini=[];resumes=[]
                    for iteration in range(2):
                        before=len(trace());resume_before=len([r for r in trace() if r['event']==20]);ready=[]
                        for attempt in range(8):
                            key(0xff1b);deadline=time.monotonic()+1
                            while time.monotonic()<deadline:
                                ready=mini_ready(before)
                                if ready:break
                                time.sleep(.05)
                            if ready:break
                        else:raise RuntimeError('Original campaign Escape did not open Mini')
                        if ready[-1]['argument']!=2 or ready[-1]['result']!=5:raise RuntimeError('Unexpected campaign Mini mode/buttons')
                        if not any(r['event']==17 and r['argument']==0 and r['result']==5 for r in trace()[before:]):raise RuntimeError('Not the campaign five-button context')
                        from PIL import ImageGrab
                        ImageGrab.grab(xdisplay=env['DISPLAY']).save(root/('mini-'+str(iteration)+'.png'));mini.append(ready[-1])
                        if iteration==0:
                            cfg_entry.read(experiment/'game/Interface/MiniMenu/screen (Mini Menu).cfg');click_control('TEXTBUTTON_5')
                        else:key(0xff1b)
                        deadline=time.monotonic()+5
                        while time.monotonic()<deadline:
                            returned=[r for r in trace() if r['event']==20]
                            if len(returned)>resume_before and returned[-1]['active_screen']==0x6cbb78:break
                            time.sleep(.05)
                        else:raise RuntimeError('Original Mini did not resume World')
                        resumes.append(returned[-1]);time.sleep(.2)
                    report.update(campaign_mini=True,mini_entries=mini,world_resumes=resumes,scope='Original fresh campaign Escape -> mode 2 five-button Mini -> Cancel and Escape -> original World resume twice; no native Mini dispatch, quit/save/load or timer/mana pause equivalence')

            else:
                click_control('TEXTBUTTON_2');deadline=time.monotonic()+10
                while time.monotonic()<deadline:
                    rows=decode(campaign.read_bytes())
                    if any(r['phase']==4 and r['owner_screen']==3 and not r['returning'] for r in rows):break
                    time.sleep(.05)
                else:raise RuntimeError('Fresh campaign Cancel did not return to Main')
                entry_trace=entry_rows()
                if not any(r['phase']==3 and r['returning']==1 for r in rows):raise RuntimeError('Pending Realm exit was not observed before resume')
                report.update(region_return=True,difficulty_choices=choices,entry_records=entry_trace,loaded_realm_return=False)
        from PIL import ImageGrab
        ImageGrab.grab(xdisplay=env['DISPLAY']).save(root/'campaign.png')
        for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','dll_sha256')]:
            if hashlib.sha256((experiment/'game'/name).read_bytes()).hexdigest()!=metadata[key]:raise RuntimeError('Staged input changed')
        report.update(success=True,new_game_action=new_game[0],records=rows,live_validated=True,original_tick_forwarded=True,original_work_bypassed=False,
            realm_resources_observed=any(r['phase']==2 and r['realm_name'] and r['hit_map'] and r['auxiliary_callback'] and r['visible_regions'] for r in rows))
        print('Original campaign ingress observed; resources initialized: '+str(report['realm_resources_observed']),flush=True)
    finally:
        if display:x.XCloseDisplay(display)
        if launcher is not None and launcher.poll() is None:
            os.killpg(launcher.pid,signal.SIGTERM)
            try:launcher.wait(timeout=5)
            except subprocess.TimeoutExpired:os.killpg(launcher.pid,signal.SIGKILL);launcher.wait()
        if prefix.exists():subprocess.run(['wineserver','-k'],env=dict(env,WINEPREFIX=str(prefix)),stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=10,check=False)
        if server is not None:
            server.terminate()
            try:server.wait(timeout=5)
            except subprocess.TimeoutExpired:server.kill();server.wait()
        report['sources']={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in [Path(__file__),ROOT/'runtime/menu/observer.c',ROOT/'runtime/menu/campaign_observe.h',ROOT/'runtime/menu/region_entry_observe.h',ROOT/'runtime/menu/campaign_world_observe.h',ROOT/'runtime/menu/campaign_mini_observe.h',ROOT/'tools/build-menu-observer.py',ROOT/'tools/prepare-menu-observer.py',ROOT/'tools/menu-game-runner.py',ROOT/'tools/run-menu-observer.py']}
        (root/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__':main()
