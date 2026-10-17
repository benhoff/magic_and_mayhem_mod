#!/usr/bin/env python3
"""Open original Mini through physical Escape, handling only witnessed Hermes."""
import argparse
import ctypes as c
import json
import os
from pathlib import Path
import struct
import time

from campaign_camera_dialogue import CameraDialogueGuard


def rows(path):
    data=path.read_bytes()
    if data[:16]!=b'MNMMENU1'+struct.pack('<II',1,64):raise ValueError('Invalid original menu journal')
    result=list(struct.iter_unpack('<16I',data[16:16+(len(data)-16)//64*64]))
    if [r[0] for r in result]!=list(range(1,len(result)+1)):raise ValueError('Original menu journal sequence')
    return result


def new_mini(path,after):
    return next((list(r) for r in rows(path) if r[0]>after and r[1]==3 and r[3]==17),None)


def open_mini(ready,escape,observed,record,sleep=time.sleep,clock=time.monotonic):
    ready()
    for attempt in range(4):
        escape();record('camera-mini-escape',attempt=attempt)
        deadline=clock()+3
        while clock()<deadline:
            witness=observed()
            if witness:return witness
            sleep(.05)
        # Never send a second Escape without a witnessed blocking modal.
        witness=observed()
        if witness:return witness
        if not ready():raise ValueError('No original Mini callback and no recognized blocking Hermes')
    raise ValueError('Bounded camera Mini transition exhausted')


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rect',nargs=4,type=int,required=True)
    parser.add_argument('--experiment',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();x0,y0,width,height=args.rect
    if width<320 or height<240:parser.error('Native viewport too small')
    x=c.CDLL('libX11.so.6');xt=c.CDLL('libXtst.so.6')
    x.XOpenDisplay.argtypes=[c.c_char_p];x.XOpenDisplay.restype=c.c_void_p
    x.XFlush.argtypes=x.XCloseDisplay.argtypes=[c.c_void_p]
    x.XKeysymToKeycode.argtypes=[c.c_void_p,c.c_ulong];x.XKeysymToKeycode.restype=c.c_uint
    xt.XTestFakeKeyEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
    xt.XTestFakeButtonEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
    xt.XTestFakeMotionEvent.argtypes=[c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong]
    display=x.XOpenDisplay(os.environ['DISPLAY'].encode())
    if not display:raise ValueError('No private input display')
    start=time.monotonic();actions=[];held_key=0;held_button=False
    result=dict(success=False)
    def record(action,**fields):actions.append(dict(action=action,seconds=time.monotonic()-start,**fields))
    def click(px,py):
        nonlocal held_button
        if not xt.XTestFakeMotionEvent(display,-1,x0+round(width*px/800),y0+round(height*py/600),0):raise ValueError('Hermes motion failed')
        x.XFlush(display)
        for down in (1,0):
            if not xt.XTestFakeButtonEvent(display,1,down,0):raise ValueError('Hermes click failed')
            held_button=bool(down);x.XFlush(display);time.sleep(.08)
    def escape():
        nonlocal held_key
        key=x.XKeysymToKeycode(display,0xff1b)
        if not key or not xt.XTestFakeKeyEvent(display,key,1,0):raise ValueError('Escape press failed')
        held_key=key;x.XFlush(display);time.sleep(.25)
        if not xt.XTestFakeKeyEvent(display,key,0,0):raise ValueError('Escape release failed')
        held_key=0;x.XFlush(display)
    guard=CameraDialogueGuard(args.output,click,record,os.environ.get('MNM_PUBLIC_RESIDENT_WORLD')=='1')
    try:
        events=args.experiment/'events.bin';before=len(rows(events));result['before_sequence']=before
        witness=open_mini(guard.ensure_ready,escape,lambda:new_mini(events,before),record)
        result.update(success=True,mini_callback=witness)
    except Exception as error:
        result['error']=str(error);raise
    finally:
        if held_key:xt.XTestFakeKeyEvent(display,held_key,0,0)
        if held_button:xt.XTestFakeButtonEvent(display,1,0,0)
        x.XFlush(display);x.XCloseDisplay(display)
        result.update(seconds=time.monotonic()-start,actions=actions,camera_dialogue=guard.report())
        args.output.write_text(json.dumps(result,indent=2)+'\n')


if __name__=='__main__':main()
