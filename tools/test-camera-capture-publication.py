#!/usr/bin/env python3
"""Actual camera helper deadline under injected durable writeback delay.

Run the unmodified Python CameraDialogueGuard against a Qt producer using either
the former durable publisher or the current atomic capture publisher. The first
durable sync is delayed eleven seconds. This is a host capture handshake test,
not original rendering equivalence or a hardware/performance acceptance claim.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
import threading
import time
from PIL import Image
from campaign_camera_dialogue import CameraDialogueGuard

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();out=args.output.resolve();out.mkdir(parents=True,exist_ok=False)
    (out/'.retain').write_text('Camera capture publication regression; preserve negative control.\n')
    declaration=json.loads(args.claims.read_text());sources=dict(declaration['sources'])
    if any(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()!=h for p,h in sources.items()):raise RuntimeError('Prospective camera sources changed')
    for name in ('apps/qt-shell/native_capture_request.hpp','tests/native-camera-capture-probe.cpp',
                 'tools/test-camera-capture-publication.py','tools/campaign_camera_dialogue.py'):
        sources[name]=hashlib.sha256((ROOT/name).read_bytes()).hexdigest()
    report=dict(schema=1,success=False,sources=sources,claims=declaration['claims'],scope=__doc__,cases=[])
    child=None
    try:
        flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Gui'],text=True))
        with (out/'build.log').open('x') as log:
            subprocess.run(['c++','-std=c++17','-fPIC','-rdynamic','-O2','-Wall','-Wextra','-Werror','-pedantic',
                '-I'+str(ROOT/'apps/qt-shell'),str(ROOT/'tests/native-camera-capture-probe.cpp'),'-o',str(out/'producer'),*flags],stdout=log,stderr=subprocess.STDOUT,check=True)
        expected={route:bytes(value for y in range(600) for x in range(800)
            for value in ((x+index*17)&255,(y*3)&255,(x+y+index*29)&255))
            for index,route in enumerate(('native','original'))}
        name='camera-mini-input-0-00'
        for mode in ('durable','current'):
            directory=out/mode;directory.mkdir();observed=[];errors=[];ack_times=[]
            env=dict(os.environ,MNM_TEST_CAPTURE_DIRECTORY=str(directory))
            started=time.monotonic()
            child=subprocess.Popen([str(out/'producer'),mode,str(directory)],env=env)
            def observe():
                try:
                    while time.monotonic()-started<20:
                        for route in ('native','original'):
                            path=directory/(name+'-'+route+'.png')
                            if route not in observed and path.exists():
                                with Image.open(path) as image:
                                    assert image.size==(800,600) and image.convert('RGB').tobytes()==expected[route]
                                observed.append(route)
                        path=directory/(name+'.json')
                        if path.exists():
                            meta=json.loads(path.read_text())
                            assert meta==dict(native_saved=True,original_saved=True,fallback=False)
                            assert set(observed)=={'native','original'},'ACK before complete images'
                            ack_times.append(time.monotonic()-started);return
                        time.sleep(.005)
                except Exception as error:errors.append(str(error))
            observer=threading.Thread(target=observe);observer.start()
            guard=CameraDialogueGuard(directory/'mini-input-0.json',lambda *_:None,lambda *_args,**_kwargs:None,True)
            error=None;capture=None
            try:capture=guard.capture()
            except ValueError as exc:error=str(exc)
            code=child.wait(timeout=5);observer.join(timeout=5);child=None
            assert not observer.is_alive() and not errors and code==0 and ack_times,(errors,code,ack_times)
            sync=(directory/'sync-entered').exists()
            if mode=='durable':assert error=='Camera dialogue capture deadline' and sync and ack_times[0]>=10,(error,sync,ack_times)
            else:assert error is None and capture is not None and not sync and ack_times[0]<3,(error,sync,ack_times)
            report['cases'].append(dict(mode=mode,success=True,helper_error=error,
                acknowledgement_seconds=ack_times[0],durable_sync_entered=sync,
                exact_lossless_images=2,exact_pixels=960000,ack_after_complete_images=True,
                actual_helper_accepted=capture is not None,request_removed=not (directory/'capture-request.json').exists()))
        report['success']=True
    except Exception as error:report['error']=str(error)
    finally:
        if child is not None and child.poll() is None:child.kill();child.wait()
        report['sources_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in sources.items())
        report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({k:report.get(k) for k in ('success','cases','error','sources_stable')}))
    return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
