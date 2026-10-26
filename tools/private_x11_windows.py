"""Read a bounded window-title tree on an explicitly supplied private display."""
import ctypes as C
import ctypes.util


def snapshot(display):
    x = C.CDLL(ctypes.util.find_library('X11'))
    x.XOpenDisplay.argtypes = [C.c_char_p]; x.XOpenDisplay.restype = C.c_void_p
    x.XDefaultRootWindow.argtypes = [C.c_void_p]; x.XDefaultRootWindow.restype = C.c_ulong
    x.XQueryTree.argtypes = [C.c_void_p, C.c_ulong, C.POINTER(C.c_ulong), C.POINTER(C.c_ulong), C.POINTER(C.POINTER(C.c_ulong)), C.POINTER(C.c_uint)]
    x.XFetchName.argtypes = [C.c_void_p, C.c_ulong, C.POINTER(C.c_void_p)]
    x.XFree.argtypes = [C.c_void_p]; x.XCloseDisplay.argtypes = [C.c_void_p]
    handler_type = C.CFUNCTYPE(C.c_int, C.c_void_p, C.c_void_p)
    errors = []
    @handler_type
    def ignore_destroyed_window(connection, error):
        errors.append(1)
        return 0
    x.XSetErrorHandler.argtypes = [C.c_void_p]; x.XSetErrorHandler.restype = C.c_void_p
    previous = x.XSetErrorHandler(C.cast(ignore_destroyed_window, C.c_void_p))
    d = x.XOpenDisplay(display.encode())
    if not d:
        x.XSetErrorHandler(previous)
        raise ValueError('Private diagnostic display unavailable')
    rows = []
    def visit(window, depth):
        if len(rows) >= 256: return
        name = C.c_void_p()
        x.XFetchName(d, window, C.byref(name))
        title = b'' if not name.value else C.string_at(name)[:1024]
        if name.value: x.XFree(name)
        rows.append(dict(depth=depth, window=window, title=title.decode(errors='replace')))
        root, parent = C.c_ulong(), C.c_ulong()
        children, count = C.POINTER(C.c_ulong)(), C.c_uint()
        x.XQueryTree(d, window, C.byref(root), C.byref(parent), C.byref(children), C.byref(count))
        if depth < 8:
            for i in range(min(count.value, 256 - len(rows))): visit(children[i], depth + 1)
        if children: x.XFree(children)
    try:
        visit(x.XDefaultRootWindow(d), 0)
        return dict(windows=rows, destroyed_window_races=len(errors), truncated=len(rows) == 256)
    finally:
        x.XCloseDisplay(d)
        x.XSetErrorHandler(previous)
