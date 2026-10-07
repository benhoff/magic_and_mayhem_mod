#!/usr/bin/env python3
"""Exercise the real Qt Launch button with a synthetic mapped-command producer."""
import argparse
import ctypes as C
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import signal
import struct
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('presentation_sources', ROOT/'tools/test-viewport-presentation.py')
presentation = importlib.util.module_from_spec(spec)
spec.loader.exec_module(presentation)
SOURCES = sorted(set(presentation.SOURCES + [
    'tools/test-native-command-launch.py',
    'apps/qt-shell/live_command_session.cpp', 'apps/qt-shell/live_command_session.hpp',
    'apps/qt-shell/live_command_renderer.cpp', 'apps/qt-shell/live_command_renderer.hpp',
    'apps/qt-shell/command_channel.cpp', 'apps/qt-shell/command_channel.hpp',
    'apps/qt-shell/render_control.cpp', 'apps/qt-shell/render_control.hpp',
    'renderer/command_consumer.cpp', 'renderer/command_state.hpp',
    'protocols/include/mnm/render_command_ring.h',
    'protocols/include/mnm/render_commands_v1.h', 'protocols/include/mnm/render_commands_v2.h',
    'protocols/include/mnm/render_control_v1.h',
]))

# Runs only in a temporary fake repository. No Wine, game data or original hooks.
PRODUCER = '''#!/usr/bin/env python3
import argparse,json,mmap,os,struct,time
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--stream');p.add_argument('--input');p.add_argument('--command-channel');p.add_argument('--render-control');p.add_argument('--capture-locks',action='store_true');a=p.parse_args()
pack=lambda *v:struct.pack('<'+'I'*len(v),*v)
channel=Path(a.command_channel)
with channel.open('r+b') as f:
 m=mmap.mmap(f.fileno(),0);version=struct.unpack_from('<I',m,8)[0]
 info={'continuous_environment':os.environ.get('MNM_RENDER_CONTINUOUS'),'version':version,'control':bool(a.render_control),'session':struct.unpack_from('<I',m,16)[0]}
 if a.render_control:
  d=Path(a.render_control).read_bytes();assert len(d)==576 and d[:8]==b'MNMRCV01';assert struct.unpack_from('<I',d,16)[0]==info['session']
 records=[(1,pack(1,4,3,24,0xff0000,0xff00,0xff)+bytes(36)),(6,pack(1))]
 if version==2:
  # Exceeds bounded stream's 4096-command cap. The first PRESENT is black.
  for i in range(2100):records.extend([(2,pack(1,0,0,4,3)+bytes([127,67,31])*12),(6,pack(1))])
 else:records.extend([(7,pack(1)),(8,b'')])
 data=b'MNMCMD01'+pack(1,16)+b''.join(pack(op,i+1,len(d))+d for i,(op,d) in enumerate(records))
 assert len(data)<1048576
 m[64:64+len(data)]=data;struct.pack_into('<I',m,24,1);struct.pack_into('<I',m,20,len(data))
 if version==1:struct.pack_into('<I',m,24,2)
 info['commands']=len(records);info['published']=len(data);Path('launched.json').write_text(json.dumps(info))
 deadline=time.monotonic()+45
 while time.monotonic()<deadline and not struct.unpack_from('<I',m,32)[0]:time.sleep(.02)
'''


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def api(lib, name, result, arguments):
    fn = getattr(lib, name)
    fn.restype, fn.argtypes = result, arguments
    return fn


def check_case(binary, directory, mode, scale, fullscreen):
    tools = directory/'tools'
    tools.mkdir(parents=True)
    launcher = tools/'run-opengl-game.py'
    launcher.write_text(PRODUCER)
    launcher.chmod(0o755)
    env = dict(os.environ, QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', QT_SCALE_FACTOR=scale)
    env.pop('MNM_RENDER_CONTINUOUS', None)
    if mode is not None:
        env['MNM_RENDER_CONTINUOUS'] = mode
    x = C.CDLL('libX11.so.6')
    xt = C.CDLL('libXtst.so.6')
    display = api(x, 'XOpenDisplay', C.c_void_p, [C.c_char_p])(None)
    assert display, 'Run under Xvfb'
    window_t = C.c_ulong
    root = api(x, 'XDefaultRootWindow', window_t, [C.c_void_p])(display)
    query = api(x, 'XQueryTree', C.c_int, [C.c_void_p, window_t, C.POINTER(window_t), C.POINTER(window_t), C.POINTER(C.POINTER(window_t)), C.POINTER(C.c_uint)])
    fetch = api(x, 'XFetchName', C.c_int, [C.c_void_p, window_t, C.POINTER(C.c_void_p)])
    free = api(x, 'XFree', C.c_int, [C.c_void_p])
    geom = api(x, 'XGetGeometry', C.c_int, [C.c_void_p, window_t, C.POINTER(window_t), C.POINTER(C.c_int), C.POINTER(C.c_int), C.POINTER(C.c_uint), C.POINTER(C.c_uint), C.POINTER(C.c_uint), C.POINTER(C.c_uint)])
    image = api(x, 'XGetImage', C.c_void_p, [C.c_void_p, window_t, C.c_int, C.c_int, C.c_uint, C.c_uint, C.c_ulong, C.c_int])
    pixel = api(x, 'XGetPixel', C.c_ulong, [C.c_void_p, C.c_int, C.c_int])
    destroy = api(x, 'XDestroyImage', C.c_int, [C.c_void_p])
    flush = api(x, 'XFlush', C.c_int, [C.c_void_p])
    motion = api(xt, 'XTestFakeMotionEvent', C.c_int, [C.c_void_p, C.c_int, C.c_int, C.c_int, C.c_ulong])
    button = api(xt, 'XTestFakeButtonEvent', C.c_int, [C.c_void_p, C.c_uint, C.c_int, C.c_ulong])

    def windows(parent):
        rr, pp, children, count = window_t(), window_t(), C.POINTER(window_t)(), C.c_uint()
        assert query(display, parent, C.byref(rr), C.byref(pp), C.byref(children), C.byref(count))
        found = [children[i] for i in range(count.value)]
        if children:
            free(children)
        return found

    def title(w):
        name = C.c_void_p()
        fetch(display, w, C.byref(name))
        value = C.string_at(name).decode(errors='replace') if name else ''
        if name:
            free(name)
        return value

    def geometry(w):
        rr, xx, yy = window_t(), C.c_int(), C.c_int()
        ww, hh, border, depth = C.c_uint(), C.c_uint(), C.c_uint(), C.c_uint()
        assert geom(display, w, C.byref(rr), C.byref(xx), C.byref(yy), C.byref(ww), C.byref(hh), C.byref(border), C.byref(depth))
        return xx.value, yy.value, ww.value, hh.value

    command = [str(binary), '--repo', str(directory), '--native-commands', '--scaling', 'smooth']
    if fullscreen:
        command.append('--fullscreen')
    with (directory/'shell.log').open('w') as log:
        child = subprocess.Popen(command, cwd=ROOT, env=env, stdout=log, stderr=log, start_new_session=True)
        try:
            deadline = time.monotonic()+20
            window = 0
            while time.monotonic()<deadline:
                assert child.poll() is None, 'Shell exited before launch'
                window = next((w for w in windows(root) if title(w)=='Magic & Mayhem Workshop'), 0)
                if window:
                    break
                time.sleep(.02)
            assert window, 'Shell window missing'
            time.sleep(.25)
            xx, yy, _, _ = geometry(window)
            factor = float(scale)
            motion(display, -1, xx+round(65*factor), yy+round(20*factor), 0)
            button(display, 1, 1, 0)
            button(display, 1, 0, 0)
            flush(display)
            info = directory/'launched.json'
            while not info.exists() and time.monotonic()<deadline:
                assert child.poll() is None, 'Shell exited while launching'
                time.sleep(.02)
            assert info.exists(), 'Launch button did not start the synthetic producer'
            result = json.loads(info.read_text())
            continuous = mode != '0'
            assert result['continuous_environment']==('1' if continuous else '0'), result
            assert result['version']==(2 if continuous else 1) and result['control']==continuous, result
            expected = 0x1f437f if continuous else 0
            samples = []
            while time.monotonic()<deadline:
                _, _, width, height = geometry(window)
                samples = []
                for fraction in (.4, .5, .6):
                    snapshot = image(display, window, round(width*fraction), round(height*.3), 1, 1, C.c_ulong(-1).value, 2)
                    assert snapshot, 'Cannot sample visible window'
                    samples.append(pixel(snapshot, 0, 0)&0xffffff)
                    destroy(snapshot)
                if samples==[expected]*3:
                    # Allow all >4096 records to drain; ensure the frame persists.
                    time.sleep(3)
                    snapshot = image(display, window, width//2, round(height*.3), 1, 1, C.c_ulong(-1).value, 2)
                    retained = pixel(snapshot, 0, 0)&0xffffff
                    destroy(snapshot)
                    assert retained==expected, ('Retained native image disappeared', hex(retained))
                    channel = next((directory/'working/runtime/render').glob('*.commands'))
                    with channel.open('rb') as source:
                        header = source.read(64)
                    if continuous:
                        assert struct.unpack_from('<I', header, 36)[0]==result['published'], 'Commands did not drain beyond the bounded limit'
                    result['drained_beyond_bounded_limit'] = continuous
                    result.update(onscreen_rgb=hex(expected), scale=scale, fullscreen=fullscreen, mode=mode or 'default')
                    return result
                time.sleep(.05)
            raise AssertionError(('Native command view did not become visible', samples, result))
        finally:
            os.killpg(child.pid, signal.SIGTERM)
            child.wait(timeout=5)
            api(x, 'XCloseDisplay', C.c_int, [C.c_void_p])(display)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path)
    parser.add_argument('--captured-run', action='append', type=Path, default=[], help='Inspect a previously completed experiment under working/')
    args = parser.parse_args()
    binary = args.build.resolve()/'mnm-qt-shell'
    parent = ROOT/'working/tests/native-command-launch'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    fingerprints = {p: sha(ROOT/p) for p in SOURCES}
    report = dict(schema=1, scope='Real Qt shell Launch button with synthetic producer, default/explicit continuous and explicit bounded selection, composed pixels in normal/fullscreen and high-DPI windows. Optional prior captures are read-only startup observations. No new original game or user-driver validation.', source_sha256=fingerprints, binary_sha256=sha(binary), runs=[], captured_startups=[])
    try:
        for captured in args.captured_run:
            report['captured_startups'].append(inspect_capture(captured))
        for name, mode, scale, fullscreen in [('default', None, '1', False), ('explicit', '1', '1', True), ('bounded', '0', '1', False), ('default-hidpi', None, '1.5', True)]:
            result = check_case(binary, run/name, mode, scale, fullscreen)
            report['runs'].append(result)
        assert fingerprints=={p: sha(ROOT/p) for p in SOURCES}, 'Sources changed during validation'
        assert report['binary_sha256']==sha(binary), 'Binary changed during validation'
        report.update(success=True, native_integration={'all_match': True, 'default_continuous': True, 'explicit_bounded': True, 'matching_child_environment_and_channels': True, 'beyond_bounded_command_limit': True, 'windowed_fullscreen_hidpi': True})
    except Exception as error:
        report.update(success=False, error=str(error))
    (run/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps({'success': report['success'], 'report': str(run/'report.json')}))
    if not report['success']:
        raise SystemExit(1)


def inspect_capture(directory):
    directory = directory.resolve()
    assert directory.is_relative_to(ROOT/'working'), 'Captured experiments must be under working/'
    manifest_path = directory/'manifest.json'
    manifest = json.loads(manifest_path.read_text())
    archive = directory/'lock-capture/session-00000001.bin'
    data = archive.read_bytes()
    assert data[:16]==b'MNMCMD01'+struct.pack('<II', 1, 16)
    at, surfaces, first = 16, {}, None
    counts = {}
    while at<len(data):
        operation, sequence, size = struct.unpack_from('<III', data, at)
        assert at+12+size<=len(data)
        payload = data[at+12:at+12+size]
        counts[operation] = counts.get(operation, 0)+1
        # Only reconstruct the prefix before the first PRESENT, not later copies.
        if first is None:
            if operation==1:
                identity, width, height, bits, *masks = struct.unpack_from('<7I', payload)
                assert len(payload[28:])==width*height*(bits//8)
                surfaces[identity] = (width, height, bits, masks, bytearray(payload[28:]))
            elif operation==2:
                identity, x, y, width, height = struct.unpack_from('<5I', payload)
                sw, sh, bits, masks, pixels = surfaces[identity]
                stride = bits//8
                assert x+width<=sw and y+height<=sh and len(payload[20:])==width*height*stride
                for row in range(height):
                    pixels[((y+row)*sw+x)*stride:((y+row)*sw+x+width)*stride] = payload[20+row*width*stride:20+(row+1)*width*stride]
            elif operation==6:
                identity = struct.unpack('<I', payload)[0]
                width, height, bits, masks, pixels = surfaces[identity]
                first = dict(sequence=sequence, width=width, height=height, bits=bits, masks=masks, native_nonzero_bytes=sum(bool(v) for v in pixels), native_sha256=hashlib.sha256(pixels).hexdigest())
            elif operation not in (5, 10):
                raise ValueError('Capture prefix needs additional operations before its first PRESENT')
        at += 12+size
    assert first is not None
    return dict(directory=str(directory.relative_to(ROOT)), manifest_sha256=sha(manifest_path), archive_sha256=sha(archive), continuous=manifest['continuous_commands'], first_present=first, present_records=counts.get(6, 0), end_records=counts.get(8, 0))


if __name__=='__main__':
    main()
