#!/usr/bin/env python3
"""Bounded physical Mini ingress or original confirmation click in a supplied viewport."""
import argparse
import ctypes as c
import json
import os
from pathlib import Path
import time
import struct
import subprocess
import hashlib
import sys
sys.path.insert(0,str(Path(__file__).resolve().parent))
from campaign_storage import HERMES_CARD_RECT,hermes_title_line


def tutorial_action(text):
    """Calibrated opening tutorial commands; other text admits no action."""
    words=' '.join(text.lower().split())
    if words=='select zombie spell':return 'select-spell', (523,575), 1
    if words=='summon zombie':return 'summon', (400,280), 3
    if words=='select zombie':return 'select-creature', (400,280), 1
    if words=='move zombie':return 'move-creature', (400,280), 3
    if words=='hermes':return 'dismiss', (400,300), 1
    return None


def recognize_title(crop,mode=7):
    # Arbitrary terrain is not a text line; some Tesseract builds crash on it.
    # Retain the failure as an unrecognized observation, never an input action.
    try:
        result=subprocess.run(['tesseract',str(crop),'stdout','--psm',str(mode)],capture_output=True,text=True,timeout=5)
        return dict(text=result.stdout if result.returncode==0 else '',ocr_returncode=result.returncode,ocr_error=result.stderr if result.returncode else '')
    except subprocess.TimeoutExpired:
        return dict(text='',ocr_returncode=None,ocr_error='Title recognition deadline expired')

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--rect',nargs=4,type=int,required=True);p.add_argument('--mode',choices=['mini','yes','no'],required=True);p.add_argument('--skip-click',action='store_true');p.add_argument('--initiate-tutorial',action='store_true');p.add_argument('--dismiss-dialogue',action='store_true');p.add_argument('--complete-opening',action='store_true');p.add_argument('--input-channel',type=Path);p.add_argument('--output',type=Path,required=True);a=p.parse_args();x0,y0,w,h=a.rect
    if w<320 or h<240:p.error('Viewport too small')
    x=c.CDLL('libX11.so.6');xt=c.CDLL('libXtst.so.6');x.XOpenDisplay.argtypes=[c.c_char_p];x.XOpenDisplay.restype=c.c_void_p;x.XFlush.argtypes=x.XCloseDisplay.argtypes=[c.c_void_p];x.XKeysymToKeycode.argtypes=[c.c_void_p,c.c_ulong];x.XKeysymToKeycode.restype=c.c_uint
    xt.XTestFakeMotionEvent.argtypes=[c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong];xt.XTestFakeButtonEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong];xt.XTestFakeKeyEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong];d=x.XOpenDisplay(os.environ['DISPLAY'].encode())
    if not d:raise RuntimeError('No isolated input display')
    key=0;key_down=False;buttons_down=set();point=(400,300) if a.mode=='mini' else (265,395) if a.mode=='yes' else (535,395)
    observations=[];actions=[];events=[]
    def event(name,**fields):
        row=dict(event=name,time_ms=time.time_ns()//1000000,**fields)
        if a.input_channel:
            data=a.input_channel.read_bytes()
            if len(data)==1088:
                row['input_words']={str(i):struct.unpack_from('<I',data,i*4)[0] for i in (4,5,6,7,16+1,16+27)}
        events.append(row)
    progress_path=a.output.parent/'storage-tutorial-progress.json'
    order=['select-spell','summon','select-creature','move-creature']
    progress=json.loads(progress_path.read_text()) if a.complete_opening and progress_path.exists() else []
    if progress!=order[:len(progress)] or len(progress)>4:raise RuntimeError('Invalid tutorial driver progress')
    def click(point,button=1):
        xt.XTestFakeMotionEvent(d,-1,x0+round(w*point[0]/800),y0+round(h*point[1]/600),0);x.XFlush(d);time.sleep(.1)
        for down in (1,0):
            xt.XTestFakeButtonEvent(d,button,down,0)
            if down:buttons_down.add(button)
            else:buttons_down.discard(button)
            x.XFlush(d);event('button',button=button,down=bool(down),point=point);time.sleep(.1)
    def observe(label):
        from PIL import ImageGrab
        screenshot=a.output.with_name(a.output.stem+'-'+label+'.png')
        image=ImageGrab.grab(xdisplay=os.environ['DISPLAY']).crop((x0,y0,x0+w,y0+h)).resize((800,600));image.save(screenshot)
        for name,rect in [('select-spell',(360,257,590,292)),('summon',(340,82,560,123)),('hermes',(340,220,530,260)),('hermes-card',HERMES_CARD_RECT)]:
            crop=screenshot.with_name(screenshot.stem+'-'+name+'.png');title=image.crop(rect);scale=2 if name=='hermes-card' else 4;title.resize((title.width*scale,title.height*scale)).save(crop)
            mode=6 if name=='hermes-card' else 7
            result=recognize_title(crop,mode)
            observation=dict(image=screenshot.name,image_sha256=hashlib.sha256(screenshot.read_bytes()).hexdigest(),crop=crop.name,crop_sha256=hashlib.sha256(crop.read_bytes()).hexdigest(),rect=rect,**result)
            if name=='hermes-card':observation.update(title_rule='hermes-title-line-v1',ocr_mode=mode)
            observations.append(observation)
            action=None
            if result['ocr_returncode']==0:
                if name=='hermes-card':
                    if hermes_title_line(result['text']):action=('dismiss',(400,300),1)
                else:action=tutorial_action(result['text'])
            if action:return action
        return None
    try:
        if a.input_channel:
            deadline=time.monotonic()+3
            while True:
                data=a.input_channel.read_bytes()
                if len(data)==1088 and data[:8]==b'MNMINK01' and struct.unpack_from('<I',data,20)[0]==1:break
                if time.monotonic()>deadline:raise RuntimeError('Native input channel is inactive before storage input')
                time.sleep(.05)
        action=observe('before') if (a.initiate_tutorial or a.dismiss_dialogue) and a.mode=='mini' else None
        if not a.initiate_tutorial and action and action[0]!='dismiss':action=None
        if not a.initiate_tutorial and not a.skip_click:
            click(point)
        if action:
            name,target,button=action
            click(target,button);actions.append(dict(action=name,logical_point=target,button=button))
            if a.complete_opening and len(progress)<4 and name==order[len(progress)]:
                progress.append(name);progress_path.write_text(json.dumps(progress)+'\n')
        # A dialogue/tutorial click and Escape are separate observed actions.
        # Let a new presented World confirm dismissal before opening Mini.
        send_escape=a.mode=='mini' and not action and (not a.complete_opening or len(progress)==4)
        if send_escape:
            key=x.XKeysymToKeycode(d,0xff1b)
            for down in (1,0):xt.XTestFakeKeyEvent(d,key,down,0);key_down=bool(down);x.XFlush(d);event('escape',down=bool(down));time.sleep(.25)
        event('complete')
        a.output.write_text(json.dumps(dict(success=True,mode=a.mode,skip_click=a.skip_click,input_active_checked=bool(a.input_channel),logical_point=point,tutorial_observations=observations,tutorial_actions=actions,opening_progress=progress,escape_sent=send_escape,events=events,scope='Physical input only; channel words are observations, not engine consumption ACKs; original menu/confirmation state checked separately'),indent=2)+'\n')
    finally:
        if key_down:xt.XTestFakeKeyEvent(d,key,0,0)
        for button in sorted(buttons_down):xt.XTestFakeButtonEvent(d,button,0,0)
        x.XFlush(d);x.XCloseDisplay(d)
if __name__=='__main__':main()
