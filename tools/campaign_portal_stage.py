"""Observed physical portal approach; no simulation writes or outcome inference."""
import ctypes as c
import json
import math
import os
from pathlib import Path
import struct
import time
from campaign_gameplay import creatures,read_rows
from campaign_input_observation import decode as input_rows
from campaign_input_admission import fresh_hover
from campaign_order_observation import decode as order_rows
from campaign_target_admission import portal_candidate_after_hover,portal_destination,portal_move_submission,primary_selection_after,portal_staging_ground
from campaign_route import portal_points,approach_point
from campaign_report import report_callback


def probe_points(hero,goal,size,view):
    """Authored four-view probes; only actual original ground admits a move."""
    if view not in (0,1,2,3):raise ValueError('Invalid camera probe view')
    dx,dy=goal['x']-hero['x'],goal['y']-hero['y']
    dx,dy=((dx,dy),(dy,-dx),(-dx,-dy),(-dy,dx))[view]
    return list(dict.fromkeys(portal_points(hero,dict(x=hero['x']+dx,y=hero['y']+dy,z=goal['z']),size)))


def is_portal_choice(choice,case,hero,target):
    return (choice=='portal' and case in ('victory','progression') and hero.get('type')==0 and hero.get('owner')==0
        and hero.get('active') and hero.get('health',0)>0 and target.get('type')==24
        and target.get('owner')==1 and target.get('health',1)<=0
        and all(type(v.get('slot')) is int and 0<=v['slot']<256 for v in (hero,target))
        and hero['slot']!=target['slot'])


def known_follower(actors,hero,legacy):
    follow=legacy.get('support_follow',{});source=follow.get('source',{});ref=follow.get('reference_observed',{})
    if (type(source.get('slot')) is not int or type(ref.get('source')) is not int
            or type(ref.get('reference')) is not int or ref['source']!=source['slot']
            or ref['reference']!=hero['slot'] or ref.get('source_health',0)<=0
            or ref.get('target_health',0)<=0):return None
    actor=actors.get(source['slot'])
    if (actor is None or actor.get('type')!=14 or actor.get('owner')!=0
            or not actor.get('active') or actor.get('health',0)<=0):return None
    return actor


class PortalStage:
    def __init__(self,experiment,rect,size,output,legacy_output=None):
        self.experiment=Path(experiment);self.rect=tuple(rect);self.size=tuple(size);self.output=Path(output);self.legacy_output=Path(legacy_output) if legacy_output is not None else None
        if self.size not in ((640,480),(800,600)) or len(self.rect)!=4 or min(self.rect[2:])<320:
            raise ValueError('Invalid owned portal viewport')
        self.display=None;self.paused=False;self.held=set();self.view=0;self.started=None
        self.result=dict(schema=1,kind='observed_portal_input_stage',completed=False,simulation_writes=False,
            semantic_outcome_accepted=False,attempts=[],actions=[],moves=[],scope='Original ground/candidate and command observations only. Camera probes and ordinary pause are authored input policy; damage, delivery and natural outcome remain independent.')

    def save(self):
        self.result['seconds']=0 if self.started is None else time.monotonic()-self.started
        temporary=self.output.with_suffix('.tmp');temporary.write_text(json.dumps(self.result)+'\n');temporary.replace(self.output)

    def orders(self):return order_rows((self.experiment/'order-admission.bin').read_bytes())['rows']
    def actors(self):return creatures(read_rows(self.experiment/'gameplay-events.bin'))
    def inputs(self):return input_rows((self.experiment/'input-admission.bin').read_bytes())['rows']
    def report(self):
        raw=(self.experiment/'events.bin').read_bytes();end=16+(len(raw)-16)//64*64
        events=list(struct.iter_unpack('<16I',raw[16:end]))
        return report_callback(events,events[0][2] if events else None)

    def open(self):
        if self.display:return
        self.x=c.CDLL('libX11.so.6');self.xt=c.CDLL('libXtst.so.6')
        self.x.XOpenDisplay.argtypes=[c.c_char_p];self.x.XOpenDisplay.restype=c.c_void_p
        self.x.XFlush.argtypes=self.x.XCloseDisplay.argtypes=[c.c_void_p]
        self.x.XKeysymToKeycode.argtypes=[c.c_void_p,c.c_ulong];self.x.XKeysymToKeycode.restype=c.c_uint
        self.xt.XTestFakeKeyEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
        self.xt.XTestFakeMotionEvent.argtypes=[c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong]
        self.xt.XTestFakeButtonEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
        self.display=self.x.XOpenDisplay(os.environ['DISPLAY'].encode())
        if not self.display:raise RuntimeError('Owned portal display unavailable')

    def key(self,symbol):
        code=self.x.XKeysymToKeycode(self.display,symbol)
        if not code:raise RuntimeError('Unmapped portal key')
        for down in (1,0):
            if not self.xt.XTestFakeKeyEvent(self.display,code,down,0):raise RuntimeError('Portal key failed')
            if down:self.held.add(('key',code))
            else:self.held.discard(('key',code))
            self.x.XFlush(self.display);time.sleep(.12)
        self.result['actions'].append(dict(keysym=symbol));self.save()

    def right(self):
        for down in (1,0):
            if not self.xt.XTestFakeButtonEvent(self.display,3,down,0):raise RuntimeError('Portal right button failed')
            if down:self.held.add(('button',3))
            else:self.held.discard(('button',3))
            self.x.XFlush(self.display);time.sleep(.08)

    def hover(self,point,hero,goal,follower,deadline,staging=False):
        rows=self.inputs();after=rows[-1]['sequence'] if rows else 0
        already=bool(rows and rows[-1]['input_active']==1 and abs(rows[-1]['x']-point[0])<=1 and abs(rows[-1]['y']-point[1])<=1)
        x0,y0,w,h=self.rect;width,height=self.size
        if not self.xt.XTestFakeMotionEvent(self.display,-1,x0+round(point[0]*w/width),y0+round(point[1]*h/height),0):raise RuntimeError('Portal motion failed')
        self.x.XFlush(self.display);until=min(time.monotonic()+.35,deadline);ready=None
        while time.monotonic()<until:
            ready=fresh_hover(self.inputs(),point,after,already,False)
            if ready is not None:break
            time.sleep(.025)
        current=self.orders();candidate=portal_candidate_after_hover(current,ready,hero,goal,0,follower) if ready else None
        staging_goal=portal_staging_ground(current,ready,hero,goal,0) if staging and ready else None
        admitted=staging_goal is not None if staging else ready is not None and candidate is not None and portal_destination(hero,goal,ready['ground'])
        sample=dict(point=list(point),probe_view=self.view,after=after,ready=ready,candidate=candidate,
            admitted=admitted,button_sent=False,purpose='staging' if staging else 'portal',staging_goal=staging_goal)
        self.result['attempts'].append(sample);self.save()
        return sample if admitted else None

    def approach(self,hero,goal):
        if self.started is None:self.started=time.monotonic();self.result.update(before=dict(hero),goal=dict(goal))
        deadline=self.started+30
        if time.monotonic()>=deadline or len(self.result['attempts'])>=96:raise RuntimeError('Observed portal approach budget exhausted')
        self.open();current=self.actors();hero=current.get(hero['slot'])
        if hero is None or not hero['active'] or hero['health']<=0:return
        if self.report() is not None:return
        if 'dialogue_hint' not in self.result:
            self.key(0xff0d);self.result['dialogue_hint']=True
        selected=None;selection_until=min(time.monotonic()+.6,deadline)
        while time.monotonic()<selection_until:
            selected=primary_selection_after(self.orders(),hero['slot'],0)
            if selected is not None:break
            time.sleep(.025)
        if selected is None:raise RuntimeError('Portal stage lacks sole-Wizard selection')
        legacy=json.loads(self.legacy_output.read_text()) if self.legacy_output is not None and self.legacy_output.is_file() else {}
        follower=known_follower(current,hero,legacy)
        pause_began=time.monotonic();self.key(0x20);self.paused=True;pause_deadline=min(deadline,pause_began+8)
        sample=None
        try:
            # One unrotated probe, then the opposite view avoids the retained
            # wrong high-ground ray. Every result still needs original picking.
            probes=probe_points(hero,goal,self.size,self.view)
            if probes:sample=self.hover(probes[0],hero,goal,follower,pause_deadline)
            if sample is None:
                for _ in range(2):self.key(ord('.'));self.view=(self.view-1)%4
                best=None
                for point in probe_points(hero,goal,self.size,self.view):
                    if time.monotonic()>=pause_deadline-1.5 or len(self.result['attempts'])>=96:break
                    candidate=self.hover(point,hero,goal,follower,pause_deadline-1.5)
                    if candidate is None:continue
                    if candidate['ready']['ground']==[goal[k] for k in ('x','y','z')]:sample=candidate;break
                    if best is None:best=point
                if sample is None and best is not None and time.monotonic()<pause_deadline-1.5:
                    sample=self.hover(best,hero,goal,follower,pause_deadline-1.5)
                if sample is None and not probe_points(hero,goal,self.size,self.view) and time.monotonic()<pause_deadline-1.5:
                    dx,dy=goal['x']-hero['x'],goal['y']-hero['y']
                    dx,dy=((dx,dy),(dy,-dx),(-dx,-dy),(-dy,dx))[self.view]
                    physical=approach_point(hero,dict(x=hero['x']+dx,y=hero['y']+dy,z=goal['z']),self.size)
                    sample=self.hover(physical,hero,goal,follower,pause_deadline-1.5,staging=True)

            if sample is not None:
                after=self.orders()[-1]['sequence'];sample['button_requested']=True;sample['order_after']=after;self.save();self.right();sample['button_sent']=True
                until=min(time.monotonic()+.8,pause_deadline)
                proof=None
                while time.monotonic()<until:
                    proof=portal_move_submission(self.orders(),hero,sample['staging_goal'] if sample['purpose']=='staging' else goal,after)
                    if proof:break
                    time.sleep(.025)
                sample['move_submission']=proof
                if proof:self.result['moves'].append(proof)
                else:sample['unmatched_returns']=[r for r in self.orders() if r['kind']==3 and r['sequence']>after][-2:]
                self.save()
        finally:
            if self.paused:self.key(0x20);self.paused=False
            self.result.setdefault('pauses',[]).append(dict(seconds=time.monotonic()-pause_began,budget=8));self.save()
        # Let the one original command run. Do not restart its path each frame.
        until=min(time.monotonic()+5,deadline)
        while time.monotonic()<until:
            report=self.report()
            if report is not None:self.result['original_report_observed']=report;self.save();break
            current=self.actors().get(hero['slot'])
            if current is None or not current['active'] or current['health']<=0:break
            self.result['latest_wizard']=current;time.sleep(.15)
        self.save()

    def close(self):
        try:
            if self.display and self.paused:self.key(0x20);self.paused=False
        finally:
            if self.display:
                for kind,code in self.held:
                    if kind=='key':self.xt.XTestFakeKeyEvent(self.display,code,0,0)
                    else:self.xt.XTestFakeButtonEvent(self.display,code,0,0)
                self.x.XFlush(self.display);self.x.XCloseDisplay(self.display);self.display=None
            self.result['completed']=True;self.save()
