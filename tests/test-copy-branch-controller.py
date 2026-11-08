#!/usr/bin/env python3
"""Retry a genuinely stale accessibility node without hiding other failures."""
import importlib.util
from pathlib import Path
import unittest
import sys
import tempfile
import os,shutil,subprocess,ctypes as c
from unittest.mock import patch
root=Path(__file__).resolve().parents[1];sys.path.insert(0,str(root/'tools'))
spec=importlib.util.spec_from_file_location('branch_control',root/'tools/run-rendering-copy-branches.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)


class Controls(unittest.TestCase):
    def test_retired_host_needs_no_window_request(self):
        class Child:
            def wait(self,timeout):return 0
        class Input:
            def close_window(self):raise AssertionError('Unnecessary window request')
        m.close_after_retirement(Child(),Input())

    def test_failed_host_is_not_successful_retirement(self):
        class Child:
            def wait(self,timeout):return 1
        with self.assertRaisesRegex(ValueError,'Native host failed'):m.close_after_retirement(Child(),None)

    @unittest.skipUnless(shutil.which('Xvfb'),'Requires isolated X11 client control')
    def test_destroyed_private_window_does_not_abort_collector(self):
        with tempfile.TemporaryDirectory(prefix='copy-host-control-') as tmp:
            host=Path(tmp)/'host';host.mkdir()
            server,info,_=m.menus.module('run-native-owned.py').start_display(host)
            input=None
            try:
                with patch.dict(os.environ,DISPLAY=info['display'],XAUTHORITY=info['authority']):input=m.menus.HostInput()
                x=input.x
                x.XCreateSimpleWindow.argtypes=[c.c_void_p,c.c_ulong,c.c_int,c.c_int,c.c_uint,c.c_uint,c.c_uint,c.c_ulong,c.c_ulong]
                x.XCreateSimpleWindow.restype=c.c_ulong
                window=x.XCreateSimpleWindow(input.display,x.XDefaultRootWindow(input.display),0,0,32,32,0,0,0)
                self.assertTrue(window);input.window=window
                x.XDestroyWindow.argtypes=[c.c_void_p,c.c_ulong];x.XSync.argtypes=[c.c_void_p,c.c_int]
                x.XDestroyWindow(input.display,window);x.XSync(input.display,0)
                class Child:
                    calls=0
                    def wait(self,timeout):
                        self.calls+=1
                        if self.calls==1:raise subprocess.TimeoutExpired('control',timeout)
                        return 0
                child=Child();m.close_after_retirement(child,input);self.assertEqual(child.calls,2)
            finally:
                if input:input.close()
                server.terminate();server.wait(timeout=10)

    def physical_journey(self,geometry):
        class Input:
            def activate(self):pass
            def focus(self):return {'host_focused':True}
            def click(self,rect):return {'rect':rect}
        class Image:
            def save(self,path):pass
        class Fake:
            input=Input();states=[];actions=[];screenshots=[];keys=[];ids=[];captures=[]
            def click(self,name):self.actions.append({'name':name})
            def await_main(self):self.states.append({'screen':3,'ack':0})
            def capture(self,label,*args):
                self.captures.append(label)
                return {'nodes':[{'accessible_id':'ui.mainMenuAction4','enabled':True,'rect':[1,2,3,4]}]}
            def click_id(self,name):self.ids.append(name)
            def wait(self,screen,ack):self.states.append({'screen':screen,'ack':ack})
            def key(self,name,symbol):self.keys.append(symbol)
        f=Fake();f.out=root/'working/controller-control';f.env={'DISPLAY':':control'}
        with patch.object(m,'host_geometry',side_effect=geometry),patch.object(m.time,'sleep'),patch.object(m,'sha',return_value='unused'),patch('PIL.ImageGrab.grab',return_value=Image()):
            m.Journey.run(f)
        return f

    def test_navigation_uses_public_keys_after_dynamic_page(self):
        f=self.physical_journey([[1,0,0,800,600,0,24]]*2)
        self.assertEqual(f.captures,['main-ready','book-open'])
        self.assertEqual(f.keys,[0xff56,0xff56,0xff55,0xff1b,0xff1b])
        self.assertEqual([(s['screen'],s['ack']) for s in f.states],[(3,0),(18,1),(27,2),(27,3),(27,4),(27,5),(27,6),(18,7),(3,8)])
        self.assertEqual(f.actions[-1]['action'],'quit-at-retained-main-target')

    def test_changed_main_geometry_refuses_retained_target(self):
        with self.assertRaisesRegex(ValueError,'geometry/focus changed'):
            self.physical_journey([[1,0,0,800,600,0,24],[1,2,0,800,600,0,24]])

    def test_destroyed_node_reacquires_fresh_tree(self):
        calls=[]
        def read():
            calls.append(1)
            if len(calls)==1:raise RuntimeError("atspi_error: No such object path '/org/a11y/stale' (1)")
            return ['fresh']
        resets=[]
        self.assertEqual(m.fresh_nodes(read,lambda:True,sleep=lambda _:None,reset=lambda:resets.append(1)),['fresh'])
        self.assertEqual(len(calls),2)
        self.assertEqual(resets,[1])

    def test_uncache_only_owned_application(self):
        class Node:
            def __init__(self,pid):self.pid=pid;self.calls=[]
            def clear_cache(self):self.calls.append('clear')
            def get_process_id(self):return self.pid
            def set_cache_mask(self,mask):self.calls.append(('mask',mask))
        own,foreign=Node(17),Node(19)
        class Desktop(Node):
            def get_child_count(self):return 2
            def get_child_at_index(self,i):return [own,foreign][i]
        desktop=Desktop(0)
        class Api:
            Cache=int
            def get_desktop(self,i):return desktop
        class Access:pid=17;a=Api()
        m.uncache_accessibility(Access())
        self.assertEqual(desktop.calls,['clear'])
        self.assertEqual(own.calls,[('mask',0),'clear'])
        self.assertEqual(foreign.calls,[])

    def test_other_errors_are_not_retried(self):
        def read():raise RuntimeError('Access denied')
        with self.assertRaisesRegex(RuntimeError,'Access denied'):m.fresh_nodes(read,lambda:True)

    def test_optional_ids_do_not_invent_a_target(self):
        items=[(None,{'accessible_id':None}),(None,{}),(None,{'accessible_id':'actual.grimoireNext'})]
        self.assertEqual([v['accessible_id'] for _,v in m.optional_accessible_ids(items)],['','','actual.grimoireNext'])
        self.assertEqual(sum(v['accessible_id'].endswith('.grimoireNext') for _,v in items),1)

    def test_deadline_preserves_stale_error(self):
        now=iter([0,4])
        def read():raise RuntimeError('atspi_error: No such object path')
        with self.assertRaisesRegex(RuntimeError,'No such object'):m.fresh_nodes(read,lambda:True,clock=lambda:next(now))

    def test_exited_host_refuses_before_read(self):
        with self.assertRaisesRegex(ValueError,'Host exited'):m.fresh_nodes(lambda:self.fail('read after exit'),lambda:False)


if __name__=='__main__':unittest.main()
