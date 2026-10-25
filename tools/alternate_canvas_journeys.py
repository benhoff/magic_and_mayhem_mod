"""Original campaign/storage/display journeys under the existing slot watches.

Commands use the admitted V13 menu wire. Escape and confirmation are physical
events on the authenticated private X display. Unknown dialogue is never acted on.
"""
import ctypes as C
import ctypes.util
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import threading
import time

from campaign_storage import HERMES_CARD_RECT, hermes_title_line

HERMES_TITLE_PROBE = (380,70,510,150)
HERMES_INSIDE_CARD = (450,155)


class LeasePulse:
    """Keep the existing host lease alive across synchronous desktop queries."""
    def __init__(self, beat, interval=.05):
        self.beat, self.interval = beat, interval
        self.stop_event = threading.Event()
        self.thread = threading.Thread(target=self.run, name='alternate-host-lease', daemon=True)
        self.error = None

    def run(self):
        try:
            while not self.stop_event.wait(self.interval):self.beat()
        except Exception as error:self.error=error

    def start(self):self.thread.start()

    def close(self):
        self.stop_event.set();self.thread.join()
        if self.error is not None:raise self.error


def keeping_lease(command, env, beat, timeout, text=False):
    child=subprocess.Popen(command,env=env,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=text)
    end=time.monotonic()+timeout
    try:
        while True:
            beat();remaining=end-time.monotonic()
            if remaining<=0:raise subprocess.TimeoutExpired(command,timeout)
            try:
                out,err=child.communicate(timeout=min(.05,remaining))
                result=subprocess.CompletedProcess(command,child.returncode,out,err)
                result.check_returncode();return result
            except subprocess.TimeoutExpired:
                if time.monotonic()>=end:raise
    finally:
        if child.poll() is None:child.kill();child.communicate()


def lane(channel, offset, size):
    before = struct.unpack_from('<I', channel, 128)[0]
    raw = bytes(channel[offset:offset+size])
    after = struct.unpack_from('<I', channel, 128)[0]
    if before & 1 or before != after or len(raw) != size:
        return None
    return raw


def saved_row(payload, name):
    if len(payload) != 12960:
        raise ValueError('Storage lane size')
    count = struct.unpack_from('<I', payload, 24)[0]
    if count > 100:
        raise ValueError('Storage row budget')
    matches = [i+1 for i in range(count) if payload[160+i*128:288+i*128].split(b'\0',1)[0].decode('cp1252') == name]
    if len(matches) != 1:
        raise ValueError('Expected exactly one newly saved row')
    return matches[0]


def mini_ready(channel):
    raw=lane(channel,132,28)
    if raw:
        row=struct.unpack('<7I',raw)
        if row[4]==5:raise ValueError('Menu host lease retired;the original game remains unmodified')
        return row[1]==17 and bool(row[2])
    return False


def enter_mini(channel, player, physical, beat, actions, attempts=6, wait_seconds=4):
    for attempt in range(attempts):
        if mini_ready(channel):return
        actions.append(dict(title_bar_activation=player.activate()))
        physical('escape');end=time.monotonic()+wait_seconds
        while time.monotonic()<end:
            beat()
            if mini_ready(channel):return
            time.sleep(.03)
        observation=player.hermes();actions.append(dict(dialogue=observation))
        # Mini can finish opening while capture/OCR is running. It pauses World;
        # requiring more World calls here would deadlock a successful ingress.
        if mini_ready(channel):return
        end=time.monotonic()+.3
        while time.monotonic()<end:beat();time.sleep(.03)
    raise ValueError('Original campaign Mini did not open')


class Physical:
    def __init__(self, env, case, beat, focus, progress):
        self.env, self.case, self.beat, self.focus = env, case, beat, focus
        self.progress = progress
        self.sequence = 0
        self.x = C.CDLL(ctypes.util.find_library('X11'))
        self.xt = C.CDLL(ctypes.util.find_library('Xtst'))
        self.x.XOpenDisplay.argtypes = [C.c_char_p]
        self.x.XOpenDisplay.restype = C.c_void_p
        self.x.XFlush.argtypes = self.x.XCloseDisplay.argtypes = [C.c_void_p]
        self.xt.XTestFakeMotionEvent.argtypes = [C.c_void_p,C.c_int,C.c_int,C.c_int,C.c_ulong]
        self.xt.XTestFakeButtonEvent.argtypes = [C.c_void_p,C.c_uint,C.c_int,C.c_ulong]

    def window(self):
        self.focus()
        w=self.progress().get('client')
        if (not w or not w['window'] or not w['samples'] or
                (w['width'],w['height']) not in ((640,480),(800,600)) or
                not (0<=w['x']<w['x']+w['width']<=1024 and 0<=w['y']<w['y']+w['height']<=768)):
            raise ValueError('No owned original Win32 client geometry: '+repr(w))
        return w

    def activate(self):
        w=self.window();left,top,right,bottom=w['outer']
        if not (left<=w['x']<right and top<w['y']<bottom):
            raise ValueError('Original title bar geometry unavailable')
        d=self.x.XOpenDisplay(self.env['DISPLAY'].encode())
        try:
            self.xt.XTestFakeMotionEvent(d,-1,(left+right)//2,(top+w['y'])//2,0);self.x.XFlush(d);time.sleep(.1)
            for down in (1,0):
                if not self.xt.XTestFakeButtonEvent(d,1,down,0):raise ValueError('Private title-bar activation failed')
                self.x.XFlush(d);self.beat();time.sleep(.1)
            # Return the pointer to the client without issuing a game command.
            if not self.xt.XTestFakeMotionEvent(d,-1,w['x']+w['width']//2,w['y']+w['height']//2,0):raise ValueError('Private client hover failed')
            self.x.XFlush(d);self.beat();time.sleep(.1)
        finally:
            self.xt.XTestFakeButtonEvent(d,1,0,0);self.x.XFlush(d);self.x.XCloseDisplay(d)
        return w

    def click(self, logical):
        w=self.window();d=self.x.XOpenDisplay(self.env['DISPLAY'].encode())
        try:
            if not self.xt.XTestFakeMotionEvent(d,-1,w['x']+round(w['width']*logical[0]/800),w['y']+round(w['height']*logical[1]/600),0):raise ValueError('Private pointer motion failed')
            self.x.XFlush(d);time.sleep(.15)
            for down in (1,0):
                if not self.xt.XTestFakeButtonEvent(d,1,down,0):raise ValueError('Private click failed')
                self.x.XFlush(d);self.beat();time.sleep(.15)
        finally:
            self.xt.XTestFakeButtonEvent(d,1,0,0);self.x.XFlush(d);self.x.XCloseDisplay(d)
        return w

    def hermes(self):
        from PIL import Image
        w=self.window();path=self.case/('dialogue-%02d.png'%self.sequence);self.sequence+=1
        # ImageMagick's XGetImage path admits the16-bit original desktop.
        keeping_lease(['import','-display',self.env['DISPLAY'],'-window','root',str(path)],self.env,self.beat,10)
        image=Image.open(path)
        if image.size!=(1024,768):raise ValueError('Private capture extent')
        image=image.crop((w['x'],w['y'],w['x']+w['width'],w['y']+w['height']))
        crop=path.with_name(path.stem+'-title.png');title=image.resize((800,600)).crop(HERMES_CARD_RECT);title.resize((title.width*2,title.height*2)).save(crop)
        ocr=keeping_lease(['tesseract',str(crop),'stdout','--psm','6'],self.env,self.beat,5,text=True)
        accepted=ocr.returncode==0 and hermes_title_line(ocr.stdout)
        record=dict(window=w,image=path.name,image_sha256=hashlib.sha256(path.read_bytes()).hexdigest(),crop=crop.name,ocr=ocr.stdout,ocr_exit=ocr.returncode,hermes_title=accepted,lease_pumped=True)
        if not accepted:
            # Short cards expose background beside the title in the broad crop.
            # Preserve that refusal and apply the same exact-line rule to a
            # bounded title-only crop; never normalize punctuation into a match.
            probe=path.with_name(path.stem+'-probe.png')
            title=image.resize((800,600)).crop(HERMES_TITLE_PROBE);title.resize((title.width*4,title.height*4)).save(probe)
            result=keeping_lease(['tesseract',str(probe),'stdout','--psm','6'],self.env,self.beat,5,text=True)
            accepted=hermes_title_line(result.stdout)
            record['title_probe']=dict(image=probe.name,rect=list(HERMES_TITLE_PROBE),ocr=result.stdout,ocr_exit=result.returncode,hermes_title=accepted)
        record['hermes_title']=accepted
        if accepted:record['click_window']=self.click(HERMES_INSIDE_CARD)
        return record

    def diagnostic(self, label):
        path=self.case/(label+'.png')
        keeping_lease(['import','-display',self.env['DISPLAY'],'-window','root',str(path)],self.env,self.beat,10)
        return dict(image=path.name,sha256=hashlib.sha256(path.read_bytes()).hexdigest(),client=self.progress().get('client'))


def campaign_rebuild(*, channel, send, wait, progress, wait_dispatch, physical,
                     beat, focus, env, case, game, video, enabled, frames, record):
    player=Physical(env,case,beat,focus,progress);actions=[];checkpoints=[];rebuilds=[]
    record.update(checkpoints=checkpoints,rebuilds=rebuilds,physical_actions=actions)
    name='owner-watch-'+case.name;save=game/'Save'/(name+'.sav')
    if save.exists():raise ValueError('Owned save name already exists')
    def wait_until(predicate, label, seconds=30):
        end=time.monotonic()+seconds
        while time.monotonic()<end:
            beat();value=predicate()
            if value:return value
            time.sleep(.03)
        raise ValueError(label)
    def world(label, expected):
        actions.append(dict(world_activation=label,window=player.activate()))
        before=progress().get('dispatch_calls',0)
        try:wait_dispatch(before+frames)
        except ValueError:
            record['failed_world']=dict(phase=label,frame_before=before,capture=player.diagnostic('failed-world'))
            raise
        p=progress()
        if bool(p['observed_video'])!=expected:raise ValueError('Original World mode mismatch')
        checkpoints.append(dict(phase=label,dispatch_calls=p['dispatch_calls'],video=p['observed_video'],slot_writes=p['slot_writes'],shutdown_calls=p['shutdown_calls']))
    def mini():
        enter_mini(channel,player,physical,beat,actions)
    def quit_map():
        mini();send(17,15)
        def modal():
            raw=lane(channel,42000,32)
            return raw and struct.unpack('<8I',raw)[1]==1
        wait_until(modal,'Original Quit confirmation absent')
        actions.append(dict(quit_confirmation=True,yes_click_window=player.click((265,395))))
        wait(6);send(6,26);wait(3)
    def load():
        wait(12);payload=wait_until(lambda:lane(channel,110592,12960),'Storage lane unstable')
        if struct.unpack_from('<I',payload,16)[0] or struct.unpack_from('<I',payload,20)[0]:raise ValueError('Original Load is modal/pending')
        send(12,31,saved_row(payload,name))
    send(3,22);wait(18);send(18,23,1);send(18,25)
    world('fresh-campaign',video)
    mini();send(17,29);wait(13);send(13,31,name=name)
    world('after-save',video)
    if not save.is_file() or not save.stat().st_size:raise ValueError('Original Save absent')
    saved_hash=hashlib.sha256(save.read_bytes()).hexdigest()
    mini();send(17,28);load();world('after-mini-load',video)
    quit_map()
    for expected in (not video,video):
        send(3,18);wait(10);payload=wait_until(lambda:lane(channel,44708,48),'Preferences lane unstable')
        values=list(struct.unpack_from('<7I',payload,16));values[2]=int(not expected)
        before=progress()['shutdown_calls'];send(10,19,preferences=values);wait(3)
        wait_until(lambda:progress().get('shutdown_calls',0)>before,'Original display rebuild did not retire graphics')
        send(3,27);load();world('after-main-load-'+str(int(expected)),expected)
        rebuilds.append(dict(video=expected,shutdown_before=before,shutdown_after=progress()['shutdown_calls']))
        quit_map()
    if hashlib.sha256(save.read_bytes()).hexdigest()!=saved_hash:raise ValueError('Load changed the owned saved file')
    record.update(saved_name=name,saved_sha256=saved_hash)
    return record
