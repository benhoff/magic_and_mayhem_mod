#!/usr/bin/env python3
"""Exercise native ownership admission and a real isolated X11 output boundary."""
import argparse
from coverage_evidence import write_report
import ctypes
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
from native_output_owner import command, REQUIRED

ROOT=Path(__file__).resolve().parents[1]


def pixel(receipt):
    library=ctypes.CDLL('libX11.so.6')
    library.XOpenDisplay.argtypes=[ctypes.c_char_p];library.XOpenDisplay.restype=ctypes.c_void_p
    library.XDefaultRootWindow.argtypes=[ctypes.c_void_p];library.XDefaultRootWindow.restype=ctypes.c_ulong
    library.XGetImage.argtypes=[ctypes.c_void_p,ctypes.c_ulong,ctypes.c_int,ctypes.c_int,ctypes.c_uint,ctypes.c_uint,ctypes.c_ulong,ctypes.c_int];library.XGetImage.restype=ctypes.c_void_p
    library.XGetPixel.argtypes=[ctypes.c_void_p,ctypes.c_int,ctypes.c_int];library.XGetPixel.restype=ctypes.c_ulong
    library.XDestroyImage.argtypes=[ctypes.c_void_p];library.XCloseDisplay.argtypes=[ctypes.c_void_p]
    previous=os.environ.get('XAUTHORITY');os.environ['XAUTHORITY']=receipt['authority']
    try:
        display=library.XOpenDisplay(receipt['display'].encode())
        if not display:raise RuntimeError('Control display inaccessible')
        image=library.XGetImage(display,library.XDefaultRootWindow(display),0,0,1,1,~0,2)
        if not image:raise RuntimeError('Control pixel unavailable')
        result=library.XGetPixel(image,0,0);library.XDestroyImage(image);library.XCloseDisplay(display)
        return result
    finally:
        if previous is None:os.environ.pop('XAUTHORITY',None)
        else:os.environ['XAUTHORITY']=previous


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True);parser.add_argument('--report',type=Path,required=True)
    args=parser.parse_args()
    if args.report.exists():parser.error('Refusing evidence overwrite')
    declaration=json.loads(args.claims.read_text());sources=declaration['sources']
    def stable():return all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in sources.items())
    if not stable():raise RuntimeError('Declared ownership sources changed before execution')
    parent=ROOT/'working/tests/native-output-owner';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='controls-',dir=parent));(root/'.retain').write_text('Owned native output controls and failures.\n')
    report=dict(schema=1,success=False,claims=declaration['claims'],sources=sources,
        original_game_executed=False,complete_replacement=False,artifacts=str(root.relative_to(ROOT)),
        scope='Native backing admission truth table, real private X11 drawing versus unchanged host pixels, kernel namespace/socket/device exclusion, and actual Qt authenticated private-window discovery/targeted input. No original game or complete reference ownership proof.')
    servers=[]
    env=dict(os.environ,PYTHONDONTWRITEBYTECODE='1',TMPDIR=str(root));env.pop('LD_PRELOAD',None)
    try:
        for path in ('tests/test-native-output-owner.py',):
            result=subprocess.run(['python3','-B',str(ROOT/path)],env=env,capture_output=True,text=True,timeout=60)
            (root/'admission.log').write_text(result.stdout+result.stderr)
            if result.returncode:raise RuntimeError('Ownership admission controls failed')
        binary=root/'backing-admission'
        subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror',str(ROOT/'tests/native-ownership-admission-test.c'),'-o',str(binary)],env=env,check=True)
        report['backing_admission']=json.loads(subprocess.check_output([str(binary)],env=env,text=True))
        spec=importlib.util.spec_from_file_location('owned_run',ROOT/'tools/run-native-owned.py');runner=importlib.util.module_from_spec(spec);spec.loader.exec_module(runner)
        for tag in ('host','legacy'):
            directory=root/tag;directory.mkdir();server,receipt,_=runner.start_display(directory);servers.append((server,receipt))
        host,legacy=[item[1] for item in servers]
        session=root/'session';prefix=root/'prefix';session.mkdir();prefix.mkdir()
        observation=session/'probe.json';observation.write_text(json.dumps(dict(host_display=host['display'],
            namespaces={name:os.stat('/proc/self/ns/'+name).st_ino for name in ('user','pid','net','ipc')})))
        before=pixel(host)
        metadata=dict(origin='menu_action_bridge',**{key:True for key in REQUIRED})
        trial=dict(env,DISPLAY=host['display'],MNM_NATIVE_CPU_GUARD='3',WAYLAND_DISPLAY='host-test',
            XDG_RUNTIME_DIR='/run/user/'+str(os.getuid()),MNM_LEGACY_XCOOKIE='must-not-enter-child')
        argv=command(session,prefix,metadata,trial,legacy)+['python3','-B',str(ROOT/'tests/native-output-owner-child.py'),str(observation)]
        result=subprocess.run(argv,env=trial,capture_output=True,text=True,timeout=30,stdin=subprocess.DEVNULL,close_fds=True)
        (root/'namespace.log').write_text(result.stdout+result.stderr)
        if result.returncode:raise RuntimeError('Kernel output isolation control failed: '+result.stderr.strip())
        report['kernel_boundary']=json.loads(result.stdout)
        report['host_pixel_before']=before;report['host_pixel_after']=pixel(host);report['private_pixel_after']=pixel(legacy)
        if report['host_pixel_after']!=before or report['private_pixel_after']!=0x123456:raise RuntimeError('Private legacy drawing escaped or failed')
        qt=root/'private-input'
        flags=subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Gui','xcb'],env=env,text=True).split()
        subprocess.run(['c++','-std=c++17','-fPIC','-Wall','-Wextra','-Werror',str(ROOT/'tests/native-output-input-test.cpp'),str(ROOT/'apps/qt-shell/window_host.cpp'),*flags,'-o',str(qt)],env=env,check=True)
        runtime=root/'qt-runtime';runtime.mkdir(mode=0o700)
        qt_env=dict(env,QT_QPA_PLATFORM='xcb',DISPLAY=host['display'],XAUTHORITY=host['authority'],
            XDG_RUNTIME_DIR=str(runtime),MNM_LEGACY_DISPLAY=legacy['display'],
            MNM_LEGACY_XCOOKIE=Path(legacy['authority']).read_bytes()[-16:].hex(),MNM_NATIVE_OUTPUT_OWNER=str(root/'legacy/owner.json'))
        report['private_input']=[]
        for negative in (False,True):
            trial=dict(qt_env)
            if negative:trial['MNM_LEGACY_XCOOKIE']='00'*16
            result=subprocess.run([str(qt),*(['--bad-cookie'] if negative else [])],env=trial,capture_output=True,text=True,timeout=10)
            (root/('input-negative.log' if negative else 'input.log')).write_text(result.stdout+result.stderr)
            if result.returncode:raise RuntimeError('Qt private input control failed: '+result.stderr)
            report['private_input'].append(json.loads(result.stdout))
        report['success']=True
    except Exception as error:
        report['error']=str(error)
    finally:
        for server,_ in reversed(servers):
            if server.poll() is None:server.terminate();server.wait(timeout=10)
        report['sources_stable']=stable();report['success']=report['success'] and report['sources_stable']
        write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps({key:report.get(key) for key in ('success','sources_stable','error')}))
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
