#!/usr/bin/env python3
"""Bounded physical Mini ingress or original confirmation click in a supplied viewport."""
import argparse
import ctypes as c
import json
import os
from pathlib import Path
import time
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--rect',nargs=4,type=int,required=True);p.add_argument('--mode',choices=['mini','yes'],required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();x0,y0,w,h=a.rect
    if w<320 or h<240:p.error('Viewport too small')
    x=c.CDLL('libX11.so.6');xt=c.CDLL('libXtst.so.6');x.XOpenDisplay.argtypes=[c.c_char_p];x.XOpenDisplay.restype=c.c_void_p;x.XFlush.argtypes=x.XCloseDisplay.argtypes=[c.c_void_p];x.XKeysymToKeycode.argtypes=[c.c_void_p,c.c_ulong];x.XKeysymToKeycode.restype=c.c_uint
    xt.XTestFakeMotionEvent.argtypes=[c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong];xt.XTestFakeButtonEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong];xt.XTestFakeKeyEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong];d=x.XOpenDisplay(os.environ['DISPLAY'].encode())
    if not d:raise RuntimeError('No isolated input display')
    key=0;point=(400,160) if a.mode=='mini' else (265,395)
    try:
        xt.XTestFakeMotionEvent(d,-1,x0+round(w*point[0]/800),y0+round(h*point[1]/600),0);x.XFlush(d);time.sleep(.1)
        for down in (1,0):xt.XTestFakeButtonEvent(d,1,down,0);x.XFlush(d);time.sleep(.1)
        if a.mode=='mini':
            key=x.XKeysymToKeycode(d,0xff1b)
            for down in (1,0):xt.XTestFakeKeyEvent(d,key,down,0);x.XFlush(d);time.sleep(.15)
        a.output.write_text(json.dumps(dict(success=True,mode=a.mode,logical_point=point,scope='Physical input only; original menu/confirmation state checked separately'),indent=2)+'\n')
    finally:
        if key:xt.XTestFakeKeyEvent(d,key,0,0)
        xt.XTestFakeButtonEvent(d,1,0,0);x.XFlush(d);x.XCloseDisplay(d)
if __name__=='__main__':main()
