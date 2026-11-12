#!/usr/bin/env python3
"""Bounded original Realm Options/Quit/Yes input; no engine memory writes."""
import argparse
import ctypes as c
import json
import os
from pathlib import Path
import time
from campaign_route import frame_size
from campaign_realm_exit import read_events,mode4_state,fresh_main,options_point,quit_point


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rect',nargs=4,type=int,required=True)
    parser.add_argument('--frame-size',nargs=2,type=int,required=True)
    parser.add_argument('--experiment',type=Path,required=True)
    parser.add_argument('--thread',type=int,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();width,height=frame_size(args.frame_size)
    x0,y0,w,h=args.rect
    if w<320 or h<240 or args.thread<=0:raise ValueError('Invalid original Realm viewport/owner')
    start=time.monotonic();display=None;held=False
    result=dict(success=False,simulation_writes=False,frame_size=[width,height],actions=[],observations=[],
                scope='Ordinary original Realm Options,mode4 Mini Quit and Yes; typed original mode4/modal/Main observations. No native mode4 dispatch,original save implementation,persistence or lifetime claim.')
    def save():
        temporary=args.output.with_suffix('.tmp')
        temporary.write_text(json.dumps(dict(result,seconds=time.monotonic()-start),indent=2)+'\n');temporary.replace(args.output)
    x=c.CDLL('libX11.so.6');xt=c.CDLL('libXtst.so.6')
    x.XOpenDisplay.argtypes=[c.c_char_p];x.XOpenDisplay.restype=c.c_void_p
    x.XFlush.argtypes=x.XCloseDisplay.argtypes=[c.c_void_p]
    xt.XTestFakeMotionEvent.argtypes=[c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong]
    xt.XTestFakeButtonEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
    trace=args.experiment/'events.bin'
    def rows():return read_events(trace,args.thread)
    def wait_for(label,predicate,seconds):
        deadline=min(start+25,time.monotonic()+seconds)
        while time.monotonic()<deadline:
            observed=predicate(rows())
            if observed:
                result['observations'].append(dict(event=label,seconds=time.monotonic()-start,**observed));save();return observed
            time.sleep(.05)
        raise ValueError('Original Realm exit observation timed out: '+label)
    def click(label,point):
        nonlocal held
        if time.monotonic()-start>=25:raise ValueError('Original Realm exit input exceeded25seconds')
        px,py=point
        if not (0<=px<width and 0<=py<height):raise ValueError('Original Realm exit point is outside viewport')
        if not xt.XTestFakeMotionEvent(display,-1,x0+round(px*w/width),y0+round(py*h/height),0):raise ValueError('Original Realm motion failed')
        x.XFlush(display);time.sleep(.2)
        for down in (1,0):
            if not xt.XTestFakeButtonEvent(display,1,down,0):raise ValueError('Original Realm click failed')
            held=bool(down);x.XFlush(display);time.sleep(.15)
        result['actions'].append(dict(action=label,point=list(point),seconds=time.monotonic()-start));save()
    try:
        display=x.XOpenDisplay(os.environ['DISPLAY'].encode())
        if not display:raise ValueError('Missing original Realm display')
        before=rows();after=before[-1][0] if before else 0
        quit,configuration=quit_point(args.experiment/'game',(width,height));result['mini_configuration']=configuration
        result['after_sequence']=after;save()
        click('original-realm-options',options_point((width,height)))
        mini=wait_for('original-mode4-mini',lambda r:mode4_state(r,after),5)
        click('original-mode4-quit',quit)
        modal=wait_for('original-mode4-confirmation',lambda r:mode4_state(r,mini['sequence'],True),5)
        # Same original4b23f0 dialog constructor as the independently observed
        # campaign Yes button. Final Main observation owns success admission.
        click('original-mode4-yes',(round(265*width/800),round(395*height/600)))
        result['main']=wait_for('original-main-return',lambda r:fresh_main(r,modal['sequence']),10)
        result['success']=True
    except Exception as error:
        result['error']=str(error)
    finally:
        if display:
            if held:xt.XTestFakeButtonEvent(display,1,0,0)
            x.XFlush(display);x.XCloseDisplay(display)
        save()
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
