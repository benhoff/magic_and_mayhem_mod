#!/usr/bin/env python3
"""Observe original Region/Grimoire/close twice and normal Main Quit in private Wine."""
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

ROOT = Path(__file__).resolve().parents[1]
FIELDS = ('sequence phase thread receiver vtable raw_screen owner depth parent owner_screen initialized '
          'fade_active fade_counter returning catalog display chapter section cursor_aux transition_left '
          'transition_right temporary_count realm_request_alias context callback_vtable callback_method callback_receiver '
          'global_context realm_request realm_kind region_id last_error').split()


def decode(data):
    if len(data) < 16: return []
    if data[:16] != b'MNMBOOK1'+struct.pack('<II',1,128): raise ValueError('Unsupported Grimoire observation header')
    if (len(data)-16)%128: raise ValueError('Incomplete Grimoire record')
    rows = [dict(zip(FIELDS,struct.unpack_from('<32I',data,at))) for at in range(16,len(data),128)]
    if len(rows)>256 or [r['sequence'] for r in rows]!=list(range(1,len(rows)+1)) or any(r['phase'] not in (1,2) for r in rows):
        raise ValueError('Invalid bounded Grimoire sequence')
    return rows


def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path)
    result=importlib.util.module_from_spec(spec);spec.loader.exec_module(result);return result


def main():
    parent=ROOT/'working/tests/live-grimoire-observation';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(root,flush=True)
    paths=['tools/test-live-grimoire-observation.py','tools/test-grimoire-observer.py','tests/grimoire-observer-reference.c',
           'tools/build-menu-observer.py','tools/prepare-menu-observer.py','tools/run-menu-observer.py',
           'tools/menu-game-runner.py','tools/run-game.sh','runtime/shadow/win32_min.h']
    paths += [p.relative_to(ROOT).as_posix() for p in (ROOT/'runtime/menu').iterdir() if p.is_file()]
    paths += [p.relative_to(ROOT).as_posix() for p in (ROOT/'protocols/include/mnm').glob('menu_v*.h')]
    prospective={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(set(paths))}
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior=next(b for b in register['behaviors'] if b['id']=='CP.grimoire-observation')
    scenario=next(s for s in register['scenarios'] if s['id']=='UI.grimoire-original-roundtrip')
    claims=module('book_live_claims','tools/coverage_claims.py')
    claim=[dict(behavior=behavior['id'],contract_sha256=claims.behavior_contract(behavior,{b['id']:b for b in register['builds']}),
                scenarios={scenario['id']:claims.scenario_contract(scenario)})]
    (root/'prospective.json').write_text(json.dumps(dict(sources=prospective,claims=claim),indent=2)+'\n')
    report=dict(success=False,scope=scenario['scope'],sources=prospective,claims=claim)
    server=launcher=display=None;prefix=root/'wineprefix';env=dict(os.environ)
    subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    try:
        experiment=module('book_live_stage','tools/prepare-menu-observer.py').prepare(campaign_observe=True,grimoire_observe=True)
        metadata=json.loads((experiment/'manifest.json').read_text());report.update(experiment=str(experiment),source_sha256=metadata['source_sha256'],dll_sha256=metadata['dll_sha256'])
        with (root/'display').open('w+') as number_file,(root/'xvfb.log').open('w') as log:
            server=subprocess.Popen(['Xvfb','-displayfd',str(number_file.fileno()),'-screen','0','800x600x24','-nolisten','tcp'],pass_fds=(number_file.fileno(),),stdout=log,stderr=subprocess.STDOUT)
            deadline=time.monotonic()+10
            while time.monotonic()<deadline:
                number_file.seek(0);number=number_file.read().strip()
                if number:break
                if server.poll() is not None:raise RuntimeError('Isolated Xvfb exited')
                time.sleep(.1)
            else:raise RuntimeError('No isolated display')
        env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
        env.pop('WAYLAND_DISPLAY',None)
        env.update(DISPLAY=':'+number,WINEDEBUG='-all',LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='4',MNM_MENU_CAMPAIGN_OBSERVE='Z:'+str(experiment/'campaign.bin').replace('/','\\'))
        with (root/'launcher.log').open('w') as log:
            launcher=subprocess.Popen([str(ROOT/'tools/run-menu-observer.py'),str(experiment),'--prefix',str(prefix)],env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
        decoder=module('book_live_menu_decode','tools/test-menu-observer.py')

        def menus():
            p=experiment/'events.bin'
            if not p.exists():return []
            raw=p.read_bytes()
            return decoder.decode(raw) if len(raw)>=16 and (len(raw)-16)%64==0 else []

        def books():
            p=experiment/'grimoire.bin'
            if not p.exists():return []
            raw=p.read_bytes()
            return decode(raw) if len(raw)>=16 and (len(raw)-16)%128==0 else []

        def wait(predicate,why,seconds=15):
            deadline=time.monotonic()+seconds
            while time.monotonic()<deadline:
                value=predicate()
                if value:return value
                if launcher.poll() is not None:raise RuntimeError('Launcher exited during '+why)
                time.sleep(.05)
            raise RuntimeError('Timed out: '+why)

        wait(lambda:any(r['menu_id']==3 and r['initialized']==1 and not r['fade_active'] and not r['returning'] and not r['next_screen'] for r in menus()),'Main readiness',80)
        x=c.CDLL('libX11.so.6');xt=c.CDLL('libXtst.so.6')
        x.XOpenDisplay.argtypes=[c.c_char_p];x.XOpenDisplay.restype=c.c_void_p;x.XFlush.argtypes=x.XCloseDisplay.argtypes=[c.c_void_p]
        x.XKeysymToKeycode.argtypes=[c.c_void_p,c.c_ulong];x.XKeysymToKeycode.restype=c.c_uint
        xt.XTestFakeMotionEvent.argtypes=[c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong]
        xt.XTestFakeButtonEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
        xt.XTestFakeKeyEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
        display=x.XOpenDisplay(env['DISPLAY'].encode())
        if not display:raise RuntimeError('No isolated input display')

        def click(relative,section):
            cfg=configparser.ConfigParser(inline_comment_prefixes=(';',),interpolation=None);cfg.read(experiment/'game'/relative)
            rect=list(map(int,cfg[section]['Rect2'].split(',')));point=((rect[0]+rect[2])//2,(rect[1]+rect[3])//2)
            xt.XTestFakeMotionEvent(display,-1,*point,0);x.XFlush(display);time.sleep(.25)
            for down in (1,0):xt.XTestFakeButtonEvent(display,1,down,0);x.XFlush(display);time.sleep(.15)

        main_cfg='Interface/MainScreen/screen (MainMenu).cfg'
        region_cfg='Interface/RegionEntry/screen (Region Entry).cfg'
        click(main_cfg,'TEXTBUTTON_1')
        ready=lambda r:r['menu_id']==18 and r['event'] in (9,10) and r['initialized']==1 and not r['fade_active'] and not r['returning'] and not r['next_screen']
        wait(lambda:any(ready(r) for r in menus()),'Region Entry readiness')
        journeys=[]
        for iteration in range(2):
            baseline=len(books());menu_baseline=len(menus())
            click(region_cfg,'STANDARDBUTTON_1')
            entered=wait(lambda:next((r for r in reversed(books()[baseline:]) if r['owner']==0x6e0088 and r['catalog'] and r['display'] and not r['fade_active'] and not r['returning'] and not r['transition_left'] and not r['transition_right']),None),'Settled original Grimoire page',30)
            if entered['receiver']!=0x6e0088 or entered['vtable']!=0x5c6b70 or entered['depth']!=5 or entered['parent']!=0x6578c0 or entered['global_context']!=5:
                raise RuntimeError('Unexpected original book identity/stack/context')
            if (entered['callback_vtable'],entered['callback_method'],entered['callback_receiver'])!=(0x5c6b8c,0x4c77c0,0x6e0088):
                raise RuntimeError('Unexpected allocated Grimoire callback identity')
            from PIL import ImageGrab
            ImageGrab.grab(xdisplay=env['DISPLAY']).save(root/f'grimoire-{iteration}.png')
            key=x.XKeysymToKeycode(display,0xff1b)
            for down in (1,0):xt.XTestFakeKeyEvent(display,key,down,0);x.XFlush(display);time.sleep(.15)
            returned=wait(lambda:next((r for r in reversed(books()[baseline:]) if r['phase']==2 and r['owner']==0x6578c0 and r['depth']==4),None),'Original book pop to Region')
            wait(lambda:any(ready(r) for r in menus()[menu_baseline:]),'Fresh Region return')
            if not any(r['returning']==1 for r in books()[baseline:]):raise RuntimeError('No original book return flag')
            journeys.append(dict(entry=entered,returned=returned))
        baseline=len(menus());click(region_cfg,'TEXTBUTTON_2')
        wait(lambda:any(r['menu_id']==3 and r['event'] in (1,4) and r['initialized']==1 and not r['fade_active'] and not r['returning'] and not r['next_screen'] for r in menus()[baseline:]),'Region Cancel/Main return')
        click(main_cfg,'TEXTBUTTON_5')
        if launcher.wait(timeout=30)!=0:raise RuntimeError('Original normal Quit failed')
        actions=[r for r in menus() if r['event']==3]
        if [(r['menu_id'],r['argument']) for r in actions]!=[(3,0),(3,4)]:raise RuntimeError('Unexpected observed Main callbacks')
        if len({r['thread'] for r in books()}|{r['thread'] for r in actions})!=1:raise RuntimeError('Original engine thread changed')
        for name,key in [('Chaos.exe','staged_sha256'),('MnmMenu.dll','dll_sha256')]:
            if hashlib.sha256((experiment/'game'/name).read_bytes()).hexdigest()!=metadata[key]:raise RuntimeError('Staged source changed')
        current={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in prospective}
        if current!=prospective:raise RuntimeError('Source changed during live observation')
        report.update(success=True,normal_quit=True,source_stable=True,original_work_bypassed=False,journeys=journeys,records=books(),main_actions=actions,original_graphics_audio=True,native_book_ownership=False)
        print('Original Region/Grimoire/Escape return twice and normal Quit observed')
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
        (root/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)


if __name__=='__main__':
    main()
