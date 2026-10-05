#!/usr/bin/env python3
"""Bounded original New Game/Realm tick observation; no native campaign dispatch."""
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
    if data[:16]!=b'MNMCAMP1'+struct.pack('<II',1,128):raise ValueError('Unsupported campaign log')
    rows=[dict(zip(FIELDS,struct.unpack_from('<32I',data,at))) for at in range(16,len(data)-127,128)]
    if len(rows)>256 or [r['sequence'] for r in rows]!=list(range(1,len(rows)+1)):raise ValueError('Invalid campaign log sequence')
    return rows
def main():
    parent=ROOT/'working/tests/live-campaign-entry';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(root,flush=True)
    server=launcher=display=None;prefix=root/'wineprefix';report={'success':False,'scope':'Original Main New Game ingress and forwarded Realm tick only; no Qt campaign integration, region interaction, campaign Mini confirmation or native replacement'}
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
        report['sources']={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in [Path(__file__),ROOT/'runtime/menu/observer.c',ROOT/'runtime/menu/campaign_observe.h',ROOT/'tools/build-menu-observer.py',ROOT/'tools/prepare-menu-observer.py',ROOT/'tools/menu-game-runner.py',ROOT/'tools/run-menu-observer.py']}
        (root/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__':main()
