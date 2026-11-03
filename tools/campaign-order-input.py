#!/usr/bin/env python3
"""Bounded ordinary-input checkpoint selection/move probe; no simulation writes."""
import argparse
import ctypes as c
import json
import os
from pathlib import Path
import time
from campaign_gameplay import read_rows,creatures
from campaign_order_observation import decode
from campaign_input_observation import decode as inputs
from campaign_input_admission import fresh_hover


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--experiment',type=Path,required=True)
    p.add_argument('--rect',type=int,nargs=4,required=True)
    p.add_argument('--frame-size',type=int,nargs=2,required=True)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();result=dict(success=False,completed=False,simulation_writes=False,actions=[],scope=__doc__)
    x=c.CDLL('libX11.so.6');xt=c.CDLL('libXtst.so.6')
    x.XOpenDisplay.argtypes=[c.c_char_p];x.XOpenDisplay.restype=c.c_void_p
    x.XFlush.argtypes=x.XCloseDisplay.argtypes=[c.c_void_p]
    x.XKeysymToKeycode.argtypes=[c.c_void_p,c.c_ulong];x.XKeysymToKeycode.restype=c.c_uint
    xt.XTestFakeKeyEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
    xt.XTestFakeMotionEvent.argtypes=[c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong]
    xt.XTestFakeButtonEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
    display=None;held=[]
    def orders():return decode((a.experiment/'order-admission.bin').read_bytes())['rows']
    def actors():return creatures(read_rows(a.experiment/'gameplay-events.bin'))
    def wait(predicate,seconds=5):
        end=time.monotonic()+seconds
        while time.monotonic()<end:
            value=predicate()
            if value:return value
            time.sleep(.05)
        return None
    def require(value,message):
        if not value:raise ValueError(message)
        return value
    def key(sym):
        code=require(x.XKeysymToKeycode(display,sym),'Unmapped key')
        for down in (1,0):
            require(xt.XTestFakeKeyEvent(display,code,down,0),'Key failed')
            if down:held.append(('key',code))
            else:held.remove(('key',code))
            x.XFlush(display);time.sleep(.15)
        result['actions'].append(dict(keysym=sym,last_order_sequence=orders()[-1]['sequence']))
    def motion(point):
        x0,y0,w,h=a.rect;width,height=a.frame_size
        require(xt.XTestFakeMotionEvent(display,-1,x0+round(point[0]*w/width),y0+round(point[1]*h/height),0),'Motion failed')
        x.XFlush(display);time.sleep(.3)
        observation=inputs((a.experiment/'input-admission.bin').read_bytes())['rows']
        original=[r for r in observation if r['kind']==3]
        result['actions'].append(dict(motion=list(point),latest_original_callback=original[-1] if original else None,
                                      latest_world=observation[-1] if observation else None))
    try:
        display=require(x.XOpenDisplay(os.environ['DISPLAY'].encode()),'Display unavailable')
        require(wait(lambda:orders() and actors()),'Original observers unavailable')
        initial=actors();support=[v for v in initial.values() if v['active'] and v['health']>0 and v['owner']==0 and v['type']==14]
        require(len(support)==1,'Probe requires sole living owned Zombie')
        support=support[0];result['support_before']=support
        key(ord('w'));key(ord('w'));time.sleep(.4)
        selection_after=orders()[-1]['sequence']
        key(ord('x'))
        selected=wait(lambda:next((r for r in reversed(orders()) if r['sequence']>selection_after and r['kind']==1 and r['flags']&1 and r['primary']==[support['slot']]),None))
        result['selection']=selected
        require(selected,'X did not produce sole-Zombie primary selection')
        width,height=a.frame_size
        # Compare small and large real motions without synthesising original
        # callback records. A final hover must still satisfy the existing gate.
        points=[(width//2,height//2+70),(width//2+1,height//2+70),
                (width//2+5,height//2+70),(width//2+35,height//2+70),(width//2,height//2+70)]
        before=inputs((a.experiment/'input-admission.bin').read_bytes())['rows'][-1]['sequence']
        for point in points:motion(point)
        admitted=wait(lambda:fresh_hover(inputs((a.experiment/'input-admission.bin').read_bytes())['rows'],points[-1],before,False,False))
        result['hover']=admitted
        require(admitted and admitted['mode'] in (0,1),'No original callback/later World hover for move')
        move_before=actors()[support['slot']];result['move_before']=move_before
        last=orders()[-1]['sequence']
        for down in (1,0):
            require(xt.XTestFakeButtonEvent(display,3,down,0),'Button failed')
            if down:held.append(('button',3))
            else:held.remove(('button',3))
            x.XFlush(display);time.sleep(.15)
        submitted=wait(lambda:next((dict(row=r,command=cmd) for r in orders() if r['sequence']>last for cmd in r['records']
                                   if cmd['opcode']==5 and cmd.get('actor_slot')==support['slot']),None))
        result['submission']=submitted
        require(submitted,'No original opcode5 submission for selected Zombie')
        moved=wait(lambda:(v if (v:=actors().get(support['slot'])) and v['active'] and v['health']>0 and v['type']==14 and v['owner']==0
                           and v['sequence']>move_before['sequence'] and any(v[k]!=move_before[k] for k in ('x','y','z')) else None),8)
        result['support_after']=moved
        require(moved,'Submitted move lacks independent original movement observation')
        result['success']=True
    except Exception as error:result['error']=str(error)
    finally:
        if display:
            for kind,code in reversed(held):
                if kind=='key':xt.XTestFakeKeyEvent(display,code,0,0)
                else:xt.XTestFakeButtonEvent(display,code,0,0)
            x.XFlush(display);x.XCloseDisplay(display)
        result['completed']=True
        a.output.write_text(json.dumps(result,indent=2)+'\n')
    # Preserve negative probes while the Qt owner completes normal cleanup.
    return 0


if __name__=='__main__':raise SystemExit(main())
