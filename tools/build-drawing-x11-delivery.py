#!/usr/bin/env python3
"""Build the opt-in native Unix X11 delivery metadata observer."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
ROOT=Path(__file__).resolve().parents[1]
def build(glx=False,egl=False):
 out=ROOT/('working/build/drawing-egl-delivery' if egl else 'working/build/drawing-glx-delivery' if glx else 'working/build/drawing-x11-delivery');out.mkdir(parents=True,exist_ok=True);binary=out/('libmnm-drawing-egl-delivery.so' if egl else 'libmnm-drawing-glx-delivery.so' if glx else 'libmnm-drawing-x11-delivery.so')
 subprocess.run(['gcc','-std=c11','-O2','-fPIC','-shared','-Wall','-Wextra','-Werror','-pthread',str(ROOT/('runtime/display/egl_delivery.c' if egl else 'runtime/display/glx_delivery.c' if glx else 'runtime/display/x11_delivery.c')),*([str(ROOT/'runtime/display/glx_symbol.S')] if glx or egl else []),'-ldl','-lX11',*(['-lxcb','-lxcb-shm'] if egl else []),'-o',str(binary)],check=True)
 sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in (('runtime/display/x11_delivery.c','runtime/display/glx_delivery.c','runtime/display/glx_symbol.S','tools/build-drawing-x11-delivery.py') if glx or egl else ('runtime/display/x11_delivery.c','tools/build-drawing-x11-delivery.py'))}
 if egl:
  for name in ('runtime/display/egl_delivery.c','runtime/display/egl_xcb_image.h','runtime/display/egl_xcb_resize_crop.h','runtime/display/egl_destination_trace.h','runtime/display/egl_drawable_rebind.h','runtime/display/egl_swap_completion.h','runtime/display/egl_blit_trace.h','runtime/display/egl_window_trace.h','runtime/display/egl_viewport_policy.h','runtime/display/egl_physical.h','runtime/display/egl_worker_context.h','protocols/include/mnm/drawing_worker_context_v1.h'):sources[name]=hashlib.sha256((ROOT/name).read_bytes()).hexdigest()
 (out/'manifest.json').write_text(json.dumps(dict(schema=1,binary_sha256=hashlib.sha256(binary.read_bytes()).hexdigest(),sources=sources,scope='Selected Unix X11 calls only; no source bytes, all-route, frame or driver-equivalence claim.'),indent=2)+'\n');return binary
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--glx',action='store_true');p.add_argument('--egl',action='store_true');a=p.parse_args();print(build(a.glx,a.egl))
