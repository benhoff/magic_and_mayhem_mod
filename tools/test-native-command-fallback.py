#!/usr/bin/env python3
"""Exhaust native recovery and verify direct X11 input in the real shell fallback.

Uses a synthetic command producer and an independent X11 window in a fake repo.
No Wine, original game, global desktop input outside the isolated Xvfb or GPU
driver equivalence is exercised.
"""
import argparse
import ctypes as C
import hashlib
import json
import os
from pathlib import Path
import signal
import subprocess
import tempfile
import time

import importlib.util

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("launch_fixture", ROOT/"tools/test-native-command-launch.py")
launch = importlib.util.module_from_spec(spec)
spec.loader.exec_module(launch)
SOURCES = sorted(set(launch.SOURCES + ["tools/test-native-command-fallback.py"]))

PRODUCER = r'''#!/usr/bin/env python3
import argparse,ctypes as C,json,mmap,struct,time
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--stream');p.add_argument('--input');p.add_argument('--command-channel');p.add_argument('--render-control');p.add_argument('--capture-locks',action='store_true');a=p.parse_args()
x=C.CDLL('libX11.so.6')
def api(name,result,args):
 f=getattr(x,name);f.restype=result;f.argtypes=args;return f
ptr=C.c_void_p;win=C.c_ulong
d=api('XOpenDisplay',ptr,[C.c_char_p])(None);assert d
root=api('XDefaultRootWindow',win,[ptr])(d)
w=api('XCreateSimpleWindow',win,[ptr,win,C.c_int,C.c_int,C.c_uint,C.c_uint,C.c_uint,win,win])(d,root,1100,300,800,600,0,0,0x226644)
api('XStoreName',C.c_int,[ptr,win,C.c_char_p])(d,w,b'MagicMayhem')
api('XSelectInput',C.c_int,[ptr,win,C.c_long])(d,w,1|4)
api('XMapWindow',C.c_int,[ptr,win])(d,w)
flush=api('XFlush',C.c_int,[ptr]);flush(d)
pending=api('XPending',C.c_int,[ptr]);next_event=api('XNextEvent',C.c_int,[ptr,ptr])
class InputEvent(C.Structure):
 _fields_=[('type',C.c_int),('serial',win),('synthetic',C.c_int),('display',ptr),('window',win)]
event=C.create_string_buffer(192);inputs=[]
pack=lambda *v:struct.pack('<'+'I'*len(v),*v)
control_file=open(a.render_control,'r+b');control=mmap.mmap(control_file.fileno(),0)
def word(m,n):return struct.unpack_from('<I',m,n)[0]
def store(m,n,v):struct.pack_into('<I',m,n,v)
channel=Path(a.command_channel);sequence=0;streams=[];current=None;failed=False
def start(path):
 global current,failed
 f=path.open('r+b');m=mmap.mmap(f.fileno(),0);streams.append((f,m));current=m;failed=False
 assert word(m,8)==2
 data=b'MNMCMD01'+pack(1,16)+pack(1,1,64)+pack(1,4,3,24,0xff0000,0xff00,0xff)+bytes([127,67,31])*12+pack(6,2,4)+pack(1)
 m[64:64+len(data)]=data;store(m,24,1);store(m,20,len(data))
start(channel)
Path('original.json').write_text(json.dumps({'window':w}))
deadline=time.monotonic()+30
while time.monotonic()<deadline:
 while pending(d):
  next_event(d,event);e=InputEvent.from_buffer(event)
  if e.type in (2,4):
   inputs.append({'type':e.type,'synthetic':bool(e.synthetic),'window':e.window,'time':time.monotonic()})
   Path('input-events.json').write_text(json.dumps(inputs))
 if not failed and word(current,36)==word(current,20):
  store(current,28,2);store(current,24,3);failed=True
  Path('failed.json').write_text(json.dumps({'sessions':len(streams),'reason':2}))
 request=word(control,20)
 if request!=sequence and not word(control,40):
  sequence=request;length=word(control,44);name=control[64:64+length].decode()
  assert name.startswith('Z:');start(Path(name[2:].replace('\\','/')))
  store(control,48,1);store(control,36,1);store(control,32,sequence)
 time.sleep(.002)
'''


def wait_for(predicate, child, message, seconds=15):
    deadline = time.monotonic()+seconds
    while time.monotonic()<deadline:
        assert child.poll() is None, 'Shell exited: '+message
        value = predicate()
        if value:
            return value
        time.sleep(.01)
    raise AssertionError(message)


def check_case(binary, directory, fullscreen):
    (directory/'tools').mkdir(parents=True)
    producer = directory/'tools/run-opengl-game.py'
    producer.write_text(PRODUCER)
    producer.chmod(0o755)
    env = dict(os.environ, QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1',
               QT_SCALE_FACTOR='1', MNM_RENDER_CONTINUOUS='1')
    x, xt = C.CDLL('libX11.so.6'), C.CDLL('libXtst.so.6')
    api, ptr, win = launch.api, C.c_void_p, C.c_ulong
    display = api(x, 'XOpenDisplay', ptr, [C.c_char_p])(None)
    assert display, 'Run under an isolated Xvfb'
    root = api(x, 'XDefaultRootWindow', win, [ptr])(display)
    query = api(x, 'XQueryTree', C.c_int, [ptr,win,C.POINTER(win),C.POINTER(win),C.POINTER(C.POINTER(win)),C.POINTER(C.c_uint)])
    free = api(x, 'XFree', C.c_int, [ptr])
    fetch = api(x, 'XFetchName', C.c_int, [ptr,win,C.POINTER(ptr)])
    translate = api(x, 'XTranslateCoordinates', C.c_int, [ptr,win,win,C.c_int,C.c_int,C.POINTER(C.c_int),C.POINTER(C.c_int),C.POINTER(win)])
    motion = api(xt, 'XTestFakeMotionEvent', C.c_int, [ptr,C.c_int,C.c_int,C.c_int,C.c_ulong])
    button = api(xt, 'XTestFakeButtonEvent', C.c_int, [ptr,C.c_uint,C.c_int,C.c_ulong])
    key = api(xt, 'XTestFakeKeyEvent', C.c_int, [ptr,C.c_uint,C.c_int,C.c_ulong])
    flush = api(x, 'XFlush', C.c_int, [ptr])

    def tree(window):
        rr, pp, children, count = win(),win(),C.POINTER(win)(),C.c_uint()
        assert query(display,window,C.byref(rr),C.byref(pp),C.byref(children),C.byref(count))
        result = pp.value,[children[i] for i in range(count.value)]
        if children:
            free(children)
        return result

    def title(window):
        name = ptr()
        fetch(display,window,C.byref(name))
        value = C.string_at(name).decode(errors='replace') if name else ''
        if name:
            free(name)
        return value

    def click(window, px, py):
        xx, yy, child = C.c_int(),C.c_int(),win()
        assert translate(display,window,root,px,py,C.byref(xx),C.byref(yy),C.byref(child))
        motion(display,-1,xx.value,yy.value,0)
        button(display,1,1,0);button(display,1,0,0);flush(display)

    command = [str(binary),'--repo',str(directory),'--native-commands','--scaling','smooth']
    if fullscreen:
        command.append('--fullscreen')
    with (directory/'shell.log').open('w') as log:
        child = subprocess.Popen(command,env=env,stdout=log,stderr=log,start_new_session=True)
        try:
            shell = wait_for(lambda:next((w for w in tree(root)[1] if title(w)=='Magic & Mayhem Workshop'),0),child,'Shell missing')
            time.sleep(.25)
            click(shell,65,20)
            original_path = directory/'original.json'
            wait_for(original_path.exists,child,'Launch did not create original window')
            original = json.loads(original_path.read_text())['window']

            def attached():
                current = original
                for _ in range(16):
                    current = tree(current)[0]
                    if current==shell:
                        return True
                    if current==root or not current:
                        return False
                return False

            wait_for(attached,child,'Original window was not attached after native refusal')
            failed = json.loads((directory/'failed.json').read_text())
            assert failed=={'sessions':4,'reason':2}, failed
            input_path = next((directory/'working/runtime/render').glob('*.input'))
            import struct
            data = input_path.read_bytes()
            assert struct.unpack_from('<I',data,20)[0]==0, 'Forwarded input lease remained active'
            assert all(not (v&0x80000000) for v in struct.unpack_from('<256I',data,64)), 'Forwarded held keys survived fallback'
            sent = time.monotonic()
            click(original,300,200)
            key(display,38,1,0);key(display,38,0,0);flush(display)
            events_path = directory/'input-events.json'

            def received():
                if not events_path.exists():
                    return None
                try:
                    events = json.loads(events_path.read_text())
                except json.JSONDecodeError:
                    return None
                return events if {e['type'] for e in events}=={2,4} else None

            events = wait_for(received,child,'Direct mouse/keyboard input did not reach original window',2)
            assert all(e['window']==original and not e['synthetic'] for e in events), events
            assert child.poll() is None
            return dict(fullscreen=fullscreen,recoveries=3,producer_reason='GAP',original_attached=True,
                        forwarded_input_inactive=True,direct_input=True,
                        observed_input_seconds=max(e['time'] for e in events)-sent)
        finally:
            os.killpg(child.pid,signal.SIGTERM)
            child.wait(timeout=5)
            api(x,'XCloseDisplay',C.c_int,[ptr])(display)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build',type=Path)
    args = parser.parse_args()
    parent = ROOT/'working/tests/native-command-fallback'
    parent.mkdir(parents=True,exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-',dir=parent))
    fingerprints = {p:launch.sha(ROOT/p) for p in SOURCES}
    binary = args.build.resolve()/'mnm-qt-shell'
    report = dict(schema=1,success=False,source_sha256=fingerprints,binary_sha256=launch.sha(binary),
                  scope='Synthetic actual-shell GAP/retry exhaustion, foreign-window attachment, inactive forwarded lease and direct X11 mouse/key delivery; normal/fullscreen. No original execution or user-hardware latency claim.',runs=[])
    try:
        for fullscreen in (False,True):
            directory = run/('fullscreen' if fullscreen else 'windowed')
            report['runs'].append(check_case(binary,directory,fullscreen))
        assert fingerprints=={p:launch.sha(ROOT/p) for p in SOURCES}
        assert report['binary_sha256']==launch.sha(binary)
        report['success']=True
    except Exception as error:
        report['error']=str(error)
    (run/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({'success':report['success'],'report':str(run/'report.json')}),flush=True)
    if not report['success']:
        raise SystemExit(1)


if __name__=='__main__':
    main()
