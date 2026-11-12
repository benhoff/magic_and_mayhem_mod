"""Admit a physical Mini Escape only after public focus is confirmed."""
import time
from rendering_pilot import require

def cancel_focus(access,host,timeout=2):
    host.activate()
    matches=[(n,v) for n,v in access.nodes() if v.get('accessible_id','').endswith('.miniMenuButton5')]
    require(len(matches)==1 and matches[0][1]['showing'] and matches[0][1]['enabled'],'Visible enabled Campaign Mini Cancel required')
    require(matches[0][0].get_component_iface().grab_focus(),'Native Mini Cancel focus request refused')
    deadline=time.monotonic()+timeout
    while time.monotonic()<deadline:
        matches=[v for _,v in access.nodes() if v.get('accessible_id','').endswith('.miniMenuButton5')]
        window=host.focus()
        if len(matches)==1 and matches[0]['showing'] and matches[0]['enabled'] and matches[0]['focused'] and window['host_focused']:
            return dict(accessible_id=matches[0]['accessible_id'],focused=True,enabled=True,showing=True,rect=matches[0]['rect'],host_focus=window,confirmed_host_ns=time.monotonic_ns())
        time.sleep(.01)
    raise ValueError('Native Mini Cancel/host focus did not become acknowledged')

def key_down(bitmap,code):
    require(len(bitmap)==32 and type(code) is int and 8<=code<256,'Invalid physical X11 keymap')
    return bool(bitmap[code//8]&(1<<(code%8)))
