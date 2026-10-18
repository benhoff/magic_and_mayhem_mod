"""Real namespace/display probe, launched only by the ownership control runner."""
import ctypes
import json
import os
from pathlib import Path
import socket
import sys


def main():
    parent=json.loads(Path(sys.argv[1]).read_text())
    for name,identity in parent['namespaces'].items():
        if os.stat('/proc/self/ns/'+name).st_ino==identity:
            raise RuntimeError('Host namespace remained reachable: '+name)
    for path in ('/dev/dri','/dev/nvidia0','/dev/kfd','/dev/fb0','/dev/dma_heap'):
        if Path(path).exists():raise RuntimeError('Host graphics device exposed: '+path)
    for key in ('WAYLAND_DISPLAY','XDG_RUNTIME_DIR','DBUS_SESSION_BUS_ADDRESS','LD_PRELOAD','LD_AUDIT','LD_LIBRARY_PATH','MNM_LEGACY_XCOOKIE'):
        if key in os.environ:raise RuntimeError('Host presentation environment exposed: '+key)
    number=parent['host_display'][1:]
    for endpoint in ('/tmp/.X11-unix/X'+number,'\0/tmp/.X11-unix/X'+number):
        with socket.socket(socket.AF_UNIX) as connection:
            try:connection.connect(endpoint)
            except OSError:pass
            else:raise RuntimeError('Host X11 socket reachable')
    library=ctypes.CDLL('libX11.so.6')
    library.XOpenDisplay.argtypes=[ctypes.c_char_p];library.XOpenDisplay.restype=ctypes.c_void_p
    library.XDefaultRootWindow.argtypes=[ctypes.c_void_p];library.XDefaultRootWindow.restype=ctypes.c_ulong
    library.XCreateGC.argtypes=[ctypes.c_void_p,ctypes.c_ulong,ctypes.c_ulong,ctypes.c_void_p];library.XCreateGC.restype=ctypes.c_void_p
    library.XSetForeground.argtypes=[ctypes.c_void_p,ctypes.c_void_p,ctypes.c_ulong]
    library.XFillRectangle.argtypes=[ctypes.c_void_p,ctypes.c_ulong,ctypes.c_void_p,ctypes.c_int,ctypes.c_int,ctypes.c_uint,ctypes.c_uint]
    library.XSync.argtypes=[ctypes.c_void_p,ctypes.c_int]
    library.XFreeGC.argtypes=[ctypes.c_void_p,ctypes.c_void_p]
    library.XCloseDisplay.argtypes=[ctypes.c_void_p]
    display=library.XOpenDisplay(None)
    if not display:raise RuntimeError('Private authenticated display inaccessible')
    gc=library.XCreateGC(display,library.XDefaultRootWindow(display),0,None)
    if not gc:raise RuntimeError('Private graphics allocation failed')
    library.XSetForeground(display,gc,0x123456)
    library.XFillRectangle(display,library.XDefaultRootWindow(display),gc,0,0,2,2)
    library.XSync(display,0);library.XFreeGC(display,gc);library.XCloseDisplay(display)
    print(json.dumps(dict(success=True,namespaces_isolated=True,host_sockets_refused=2,
        graphics_devices_hidden=True,private_x11_written=True)))


if __name__=='__main__':main()
