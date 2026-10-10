#!/usr/bin/env python3
"""Answer an observed campaign Quit confirmation through the native viewport."""
import argparse
import ctypes as c
import hashlib
import json
import os
from pathlib import Path
import struct
import time

HEADER=b'MNMMENU1'+struct.pack('<II',1,64)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rect',nargs=4,type=int,required=True)
    p.add_argument('--answer',choices=['no','yes'],required=True)
    p.add_argument('--experiment',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();x0,y0,w,h=a.rect
    if w<320 or h<240:p.error('Native viewport too small')
    start=time.monotonic();result=dict(success=False,answer=a.answer)
    def rows():
        data=(a.experiment/'events.bin').read_bytes()
        if data[:16]!=HEADER:raise RuntimeError('Invalid original menu trace')
        return list(struct.iter_unpack('<16I',data[16:16+(len(data)-16)//64*64]))
    def wait(predicate,label):
        deadline=time.monotonic()+12
        while time.monotonic()<deadline:
            found=predicate(rows())
            if found:return found[-1]
            time.sleep(.05)
        raise RuntimeError(label)
    display=None;x=None
    try:
        quit_row=wait(lambda rs:[r for r in rs if r[1]==3 and r[3]==17 and r[9]==3],'No original campaign Quit callback')
        modal=wait(lambda rs:[r for r in rs if r[1]==18 and r[0]>quit_row[0] and r[9]],'No fresh original Quit confirmation')
        # Capture the actual native framebuffer plus separate original-owned
        # publication; a desktop screenshot cannot establish native rendering.
        name='quit-'+('00-no' if a.answer=='no' else '01-yes')+'-confirmation'
        request=a.output.parent/'capture-request.json';temp=request.with_suffix('.tmp')
        temp.write_text(json.dumps(dict(name=name)));temp.replace(request)
        deadline=time.monotonic()+10;metadata=a.output.parent/(name+'.json')
        while not metadata.exists():
            if time.monotonic()>deadline:raise RuntimeError('Native Quit capture deadline exceeded')
            time.sleep(.1)
        meta=json.loads(metadata.read_text())
        if not meta['native_saved'] or not meta['original_saved'] or meta['fallback']:raise RuntimeError('Independent native Quit capture failed')
        result['capture']=dict(name=name,metadata=meta,image_sha256={r:hashlib.sha256((a.output.parent/(name+'-'+r+'.png')).read_bytes()).hexdigest() for r in ('native','original')})
        x=c.CDLL('libX11.so.6');xt=c.CDLL('libXtst.so.6')
        x.XOpenDisplay.argtypes=[c.c_char_p];x.XOpenDisplay.restype=c.c_void_p
        x.XFlush.argtypes=x.XCloseDisplay.argtypes=[c.c_void_p]
        xt.XTestFakeMotionEvent.argtypes=[c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong]
        xt.XTestFakeButtonEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
        display=x.XOpenDisplay(os.environ['DISPLAY'].encode())
        if not display:raise RuntimeError('No native Quit input display')
        point=[535 if a.answer=='no' else 265,395]
        if not xt.XTestFakeMotionEvent(display,-1,x0+round(w*point[0]/800),y0+round(h*point[1]/600),0):raise RuntimeError('Native Quit motion failed')
        x.XFlush(display);time.sleep(.2)
        for down in (1,0):
            if not xt.XTestFakeButtonEvent(display,1,down,0):raise RuntimeError('Native Quit click failed')
            x.XFlush(display);time.sleep(.15)
        answer=wait(lambda rs:[r for r in rs if r[1]==22 and r[0]>modal[0] and r[9]==(1 if a.answer=='no' else 0)],'Original Quit answer was not observed')
        if answer[2]!=quit_row[2] or answer[4]!=0x6a5088:raise RuntimeError('Original Quit receiver/thread changed')
        result.update(quit_callback=list(quit_row),modal=list(modal),original_answer=list(answer),logical_point=point)
        if a.answer=='no':
            resume=wait(lambda rs:[r for r in rs if r[1]==20 and r[0]>answer[0] and r[11]==0x6cbb78],'Original No did not resume World')
            if resume[2]!=answer[2]:raise RuntimeError('Original World resume thread changed')
            result['world_resume']=list(resume)
        result['success']=True
    except Exception as error:
        result['error']=str(error);raise
    finally:
        if display:
            # Release only this helper's button; it owns no other input state.
            xt.XTestFakeButtonEvent(display,1,0,0);x.XFlush(display);x.XCloseDisplay(display)
        result['seconds']=time.monotonic()-start
        a.output.write_text(json.dumps(result,indent=2)+'\n')

if __name__=='__main__':main()
