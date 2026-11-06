"""External observation/input for the existing public Qt menus; no game writes."""
import ctypes as c
import os
import time


class HostInput:
    def __init__(self):
        self.x=c.CDLL('libX11.so.6');self.xt=c.CDLL('libXtst.so.6')
        self.x.XOpenDisplay.argtypes=[c.c_char_p];self.x.XOpenDisplay.restype=c.c_void_p
        self.x.XDefaultRootWindow.argtypes=[c.c_void_p];self.x.XDefaultRootWindow.restype=c.c_ulong
        self.x.XQueryTree.argtypes=[c.c_void_p,c.c_ulong,c.POINTER(c.c_ulong),c.POINTER(c.c_ulong),c.POINTER(c.POINTER(c.c_ulong)),c.POINTER(c.c_uint)]
        self.x.XFetchName.argtypes=[c.c_void_p,c.c_ulong,c.POINTER(c.c_char_p)]
        self.x.XFree.argtypes=[c.c_void_p]
        self.x.XSetInputFocus.argtypes=[c.c_void_p,c.c_ulong,c.c_int,c.c_ulong]
        self.x.XGetInputFocus.argtypes=[c.c_void_p,c.POINTER(c.c_ulong),c.POINTER(c.c_int)]
        self.x.XKeysymToKeycode.argtypes=[c.c_void_p,c.c_ulong];self.x.XKeysymToKeycode.restype=c.c_uint
        self.x.XFlush.argtypes=self.x.XCloseDisplay.argtypes=[c.c_void_p]
        self.x.XInternAtom.argtypes=[c.c_void_p,c.c_char_p,c.c_int];self.x.XInternAtom.restype=c.c_ulong
        self.x.XSendEvent.argtypes=[c.c_void_p,c.c_ulong,c.c_int,c.c_long,c.c_void_p]
        self.xt.XTestFakeMotionEvent.argtypes=[c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong]
        self.xt.XTestFakeButtonEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
        self.xt.XTestFakeKeyEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
        self.display=self.x.XOpenDisplay(os.environ['DISPLAY'].encode())
        if not self.display:raise ValueError('Missing isolated host display')
        self.window=0

    def find_window(self):
        root=self.x.XDefaultRootWindow(self.display);parent=c.c_ulong();returned=c.c_ulong();children=c.POINTER(c.c_ulong)();count=c.c_uint()
        if not self.x.XQueryTree(self.display,root,c.byref(returned),c.byref(parent),c.byref(children),c.byref(count)):return 0
        found=[]
        try:
            for i in range(count.value):
                title=c.c_char_p()
                if self.x.XFetchName(self.display,children[i],c.byref(title)) and title:
                    name=title.value.decode(errors='replace');self.x.XFree(title)
                    if name=='Magic & Mayhem Workshop':found.append(int(children[i]))
        finally:
            if children:self.x.XFree(children)
        if len(found)>1:raise ValueError('Ambiguous public Qt host')
        if found:self.window=found[0]
        return self.window

    def activate(self):
        if not self.find_window():raise ValueError('Public Qt host absent')
        self.x.XSetInputFocus(self.display,self.window,2,0);self.x.XFlush(self.display)

    def focus(self):
        window=c.c_ulong();revert=c.c_int();self.x.XGetInputFocus(self.display,c.byref(window),c.byref(revert))
        return dict(window=window.value,host_window=self.window,host_focused=window.value==self.window)

    def key(self,symbol):
        code=self.x.XKeysymToKeycode(self.display,symbol)
        if not code:raise ValueError('Physical key unavailable')
        for down in (1,0):
            if not self.xt.XTestFakeKeyEvent(self.display,code,down,0):raise ValueError('Physical key rejected')
            self.x.XFlush(self.display);time.sleep(.06)
        return dict(symbol=symbol,keycode=code,down_up=True,host_focus=self.focus())

    def click(self,rect):
        x,y,w,h=rect
        if min(w,h)<=0:raise ValueError('Invisible physical target')
        if not self.xt.XTestFakeMotionEvent(self.display,-1,x+w//2,y+h//2,0):raise ValueError('Pointer motion rejected')
        for down in (1,0):
            if not self.xt.XTestFakeButtonEvent(self.display,1,down,0):raise ValueError('Physical click rejected')
            self.x.XFlush(self.display);time.sleep(.06)
        return dict(rect=list(rect),down_up=True,host_focus=self.focus())

    def close_window(self):
        class Message(c.Structure):
            _fields_=[('type',c.c_int),('serial',c.c_ulong),('send_event',c.c_int),('display',c.c_void_p),('window',c.c_ulong),('message_type',c.c_ulong),('format',c.c_int),('data',c.c_long*5)]
        class Event(c.Union):_fields_=[('message',Message),('padding',c.c_long*24)]
        e=Event();e.message.type=33;e.message.display=self.display;e.message.window=self.window;e.message.format=32
        e.message.message_type=self.x.XInternAtom(self.display,b'WM_PROTOCOLS',0);e.message.data[0]=self.x.XInternAtom(self.display,b'WM_DELETE_WINDOW',0)
        if not self.x.XSendEvent(self.display,self.window,0,0,c.byref(e)):raise ValueError('Host close request failed')
        self.x.XFlush(self.display)

    def close(self):
        if self.display:self.x.XCloseDisplay(self.display);self.display=None


class AccessibleMenus:
    def __init__(self,pid):
        import gi
        gi.require_version('Atspi','2.0')
        from gi.repository import Atspi
        self.a=Atspi;self.pid=pid;Atspi.init()

    def nodes(self):
        desktop=self.a.get_desktop(0);apps=[]
        for i in range(desktop.get_child_count()):
            app=desktop.get_child_at_index(i)
            if app and app.get_process_id()==self.pid:apps.append(app)
        if len(apps)>1:raise ValueError('Ambiguous accessibility process')
        out=[];pending=apps[:]
        while pending:
            node=pending.pop()
            if len(out)>4096:raise ValueError('Accessibility tree budget')
            state=node.get_state_set();visible=state.contains(self.a.StateType.SHOWING)
            if visible:
                component=node.get_component_iface();rect=None
                if component:
                    r=component.get_extents(self.a.CoordType.SCREEN);rect=[r.x,r.y,r.width,r.height]
                item=dict(name=node.get_name(),role=node.get_role_name(),focused=state.contains(self.a.StateType.FOCUSED),enabled=state.contains(self.a.StateType.ENABLED),showing=True,rect=rect)
                if hasattr(node,'get_accessible_id'):
                    try:item['accessible_id']=node.get_accessible_id()
                    except Exception:pass
                value=node.get_value_iface()
                if value:item['value']=dict(current=value.get_current_value(),minimum=value.get_minimum_value(),maximum=value.get_maximum_value())
                out.append((node,item))
            for i in range(node.get_child_count()-1,-1,-1):
                child=node.get_child_at_index(i)
                if child:pending.append(child)
        return out

    def find(self,name,role=None):
        objects={'Launch game':'launchGame','Quick Battle':'mainMenuAction2','Preferences':'mainMenuAction3','Quit':'mainMenuAction4','Create Single Player Game':'quickBattleAction2','Map':'singlePlayerMap'}
        nodes=self.nodes();key=objects.get(name)
        found=[(n,v) for n,v in nodes if key and v.get('accessible_id','').endswith('.'+key) and (role is None or v['role']==role)]
        if not found:found=[(n,v) for n,v in nodes if v['name']==name and (role is None or v['role']==role)]
        if len(found)!=1:raise ValueError('Accessible target missing/ambiguous: '+name+' '+str(role))
        return found[0]

    def launch_log(self):
        desktop=self.a.get_desktop(0);pending=[desktop.get_child_at_index(i) for i in range(desktop.get_child_count()) if desktop.get_child_at_index(i).get_process_id()==self.pid];out=[];seen=0
        while pending:
            node=pending.pop();seen+=1
            if seen>4096:raise ValueError('Accessibility log budget')
            if node.get_role_name()=='text':
                iface=node.get_text_iface()
                if iface:out.append(self.a.Text.get_text(iface,0,min(iface.get_character_count(),40000)))
            for i in range(node.get_child_count()):
                child=node.get_child_at_index(i)
                if child:pending.append(child)
        return out
