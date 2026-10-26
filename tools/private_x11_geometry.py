"""Bounded read-only private X11 geometry and input-selection facts."""
import ctypes as C
import ctypes.util
from private_x11_windows import snapshot


class Attributes(C.Structure):
    _fields_=[('x',C.c_int),('y',C.c_int),('width',C.c_int),('height',C.c_int),
              ('border_width',C.c_int),('depth',C.c_int),('visual',C.c_void_p),
              ('root',C.c_ulong),('window_class',C.c_int),('bit_gravity',C.c_int),
              ('win_gravity',C.c_int),('backing_store',C.c_int),('backing_planes',C.c_ulong),
              ('backing_pixel',C.c_ulong),('save_under',C.c_int),('colormap',C.c_ulong),
              ('map_installed',C.c_int),('map_state',C.c_int),('all_event_masks',C.c_long),
              ('your_event_mask',C.c_long),('do_not_propagate_mask',C.c_long),
              ('override_redirect',C.c_int),('screen',C.c_void_p)]


def inspect(display):
    result=snapshot(display)
    x=C.CDLL(ctypes.util.find_library('X11'))
    x.XOpenDisplay.argtypes=[C.c_char_p];x.XOpenDisplay.restype=C.c_void_p
    x.XCloseDisplay.argtypes=[C.c_void_p]
    x.XGetWindowAttributes.argtypes=[C.c_void_p,C.c_ulong,C.POINTER(Attributes)]
    handler=C.CFUNCTYPE(C.c_int,C.c_void_p,C.c_void_p);errors=[]
    @handler
    def ignore_destroyed(connection,error):errors.append(1);return 0
    x.XSetErrorHandler.argtypes=[C.c_void_p];x.XSetErrorHandler.restype=C.c_void_p
    previous=x.XSetErrorHandler(C.cast(ignore_destroyed,C.c_void_p));d=x.XOpenDisplay(display.encode())
    if not d:
        x.XSetErrorHandler(previous);raise ValueError('Private display unavailable')
    try:
        for row in result['windows']:
            a=Attributes()
            if x.XGetWindowAttributes(d,row['window'],C.byref(a)):
                row['attributes']={k:getattr(a,k) for k in ['x','y','width','height','border_width','depth','window_class','map_state','all_event_masks','your_event_mask','override_redirect']}
        result['attribute_races']=len(errors)
        return result
    finally:x.XCloseDisplay(d);x.XSetErrorHandler(previous)


def move_game_left(display,desktop,window):
    """One declared desktop-exposure stimulus on the private game's outer window.

    Window movement leaves game dispatch, clip lists and native drawing inputs
    untouched; Wine/GDI supplies the resulting ordinary visible region.
    """
    x=C.CDLL(ctypes.util.find_library('X11'))
    x.XOpenDisplay.argtypes=[C.c_char_p];x.XOpenDisplay.restype=C.c_void_p
    x.XCloseDisplay.argtypes=[C.c_void_p]
    x.XGetWindowAttributes.argtypes=[C.c_void_p,C.c_ulong,C.POINTER(Attributes)]
    x.XQueryTree.argtypes=[C.c_void_p,C.c_ulong,C.POINTER(C.c_ulong),C.POINTER(C.c_ulong),C.POINTER(C.POINTER(C.c_ulong)),C.POINTER(C.c_uint)]
    x.XMoveWindow.argtypes=[C.c_void_p,C.c_ulong,C.c_int,C.c_int];x.XSync.argtypes=[C.c_void_p,C.c_int];x.XFree.argtypes=[C.c_void_p]
    d=x.XOpenDisplay(display.encode())
    if not d:raise ValueError('Private exposure display unavailable')
    try:
        root,parent=C.c_ulong(),C.c_ulong();children=C.POINTER(C.c_ulong)();count=C.c_uint()
        ok=x.XQueryTree(d,window,C.byref(root),C.byref(parent),C.byref(children),C.byref(count))
        if children:x.XFree(children)
        if not ok or parent.value!=desktop:raise ValueError('Exposure requires direct private Wine Desktop child')
        a=Attributes()
        if not x.XGetWindowAttributes(d,window,C.byref(a)) or a.map_state!=2 or a.window_class!=1 or not 1<=a.width<=2048 or not 1<=a.height<=2048:raise ValueError('Exposure requires mapped ordinary game window')
        before=[a.x,a.y,a.width,a.height];left=-(a.width//2)
        x.XMoveWindow(d,window,left,a.y);x.XSync(d,0)
        if not x.XGetWindowAttributes(d,window,C.byref(a)) or a.x!=left:raise ValueError('Private desktop movement did not apply')
        return dict(window=window,desktop=desktop,before=before,after=[a.x,a.y,a.width,a.height],scope='One ordinary half-off-desktop outer-window movement; no game state,clip-list API,draw commands or pixel input edits. This is declared desktop stress,not an untouched journey.')
    finally:x.XCloseDisplay(d)
