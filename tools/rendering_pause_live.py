"""External physical Campaign Mini controls and read-only SQLite sampling."""
import importlib.util
import ctypes as c
import struct
import subprocess
import json
import time
from pathlib import Path
from rendering_pause import Samples,validate_dwell
from pause_focus import cancel_focus,key_down
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('pause_interaction',ROOT/'tools/run-rendering-interaction.py')
interaction=importlib.util.module_from_spec(spec);spec.loader.exec_module(interaction)


class PauseJourney(interaction.InteractionJourney):
    def __init__(self,*args):
        super().__init__(*args);self.pause_actions=[];self.samples=None;self.dwells=[]
    def original(self):
        # An odd/changing seqlock is unpublished, not a changed menu owner.
        # Retry only absent publications; every stable publication is checked.
        for attempt in range(25):
            value=self.read()
            if value is not None:return value
            time.sleep(.001)
        raise ValueError('Original menu publication did not settle within25reads')
    def sample(self,phase):
        value=self.original()
        return self.samples.append(phase,self.session,value[1])
    def physical_key(self,label,symbol,hold=.22):
        code=self.input.x.XKeysymToKeycode(self.input.display,symbol);require(code>0,'No physical key')
        self.input.x.XQueryKeymap.argtypes=[c.c_void_p,c.c_void_p]
        bitmaps={}
        def pressed(phase):
            bitmap=c.create_string_buffer(32)
            require(self.input.x.XQueryKeymap(self.input.display,bitmap),'Physical keymap query refused')
            bitmaps[phase]=bitmap.raw.hex()
            return key_down(bitmap.raw,code)
        before=pressed('before');focus=self.input.focus()
        require(not before and focus['host_focused'],'Physical key admission requires released key and actual host focus')
        action=dict(action=label,host_ns=time.monotonic_ns(),symbol=symbol,held_ms=int(hold*1000),physical=dict(keycode=code,before_down=before,host_focus=focus,keymaps=bitmaps))
        self.actions.append(action);down=False
        try:
            require(self.input.xt.XTestFakeKeyEvent(self.input.display,code,1,0),'Physical key-down refused');down=True
            self.input.x.XFlush(self.input.display);action['physical']['pressed']=pressed('pressed')
            require(action['physical']['pressed'],'Physical key-down not acknowledged by X11')
            time.sleep(hold)
        finally:
            if down:
                require(self.input.xt.XTestFakeKeyEvent(self.input.display,code,0,0),'Physical key-up refused')
                self.input.x.XFlush(self.input.display);action['physical']['released']=not pressed('released')
                require(action['physical']['released'],'Physical key-up not acknowledged by X11')
    def admit_mini(self,label,ack):
        # Reuse the existing title-qualified Hermes ingress, without tutorial
        # gameplay commands. An input receipt never substitutes for original ACK.
        started=time.monotonic();escapes=0
        for attempt in range(12):
            require(time.monotonic()-started<120,'Qualified Mini ingress exceeded120seconds')
            self.focus_viewport();paint=self.viewport.poll()
            nodes=[v for _,v in self.access.nodes() if v.get('accessible_id','').endswith('.nativeGameViewport')]
            require(len(nodes)==1,'Missing viewport for physical Mini ingress')
            rect=nodes[0]['rect'];image=[rect[0]+paint['left'],rect[1]+paint['top'],paint['image_width'],paint['image_height']]
            output=self.out/('mini-ingress-'+str(label)+'-'+str(attempt)+'.json')
            subprocess.run(['python3',str(ROOT/'tools/menu-storage-input.py'),'--mode','mini','--skip-click','--dismiss-dialogue','--rect',*[str(x) for x in image],
                '--input-channel',str(self.session/'render-input.bin'),'--output',str(output)],cwd=ROOT,env=self.env,check=True,timeout=60)
            receipt=json.loads(output.read_text());require(receipt['success'] and receipt['skip_click'],'Physical Mini ingress failed')
            self.actions.append(dict(action='qualified-mini-ingress',label=str(label),attempt=attempt,host_ns=time.monotonic_ns(),record=str(output.relative_to(ROOT)),escape_sent=receipt['escape_sent'],tutorial_actions=receipt['tutorial_actions']))
            if receipt['tutorial_actions']:
                require(not receipt['escape_sent'] and all(v['action']=='dismiss' for v in receipt['tutorial_actions']),'Unclassified tutorial control')
                self.wait_progress(int(paint['world']),minimum=2,timeout=15)
                continue
            require(receipt['escape_sent'],'Neither qualified dismissal nor physical Escape')
            escapes+=1;require(escapes<=4,'Mini ingress exceeded4physical Escape controls')
            deadline=time.monotonic()+3
            while time.monotonic()<deadline:
                value=self.original()
                if value[0]['ready'] and value[0]['screen']==17 and value[0]['ack']==ack:return self.wait(17,ack)
                time.sleep(.05)
        raise ValueError('Bounded Hermes/Mini admission exhausted12qualified controls')
    def mini(self,number,ack):
        self.focus_viewport();time.sleep(.15);before=self.sample('world-before-mini-'+str(number))
        require(before['input']['active']==1 and not before['input']['held'],'No active released gameplay input before Mini')
        state=self.admit_mini(number,ack);raw=self.original()[1];mini=struct.unpack_from('<8I',raw,42000)
        require(mini[0]==0 and mini[1]==0 and mini[2]==5 and mini[3]==2
                and mini[4]==31 and mini[5:]==(5,2,0),'Wrong original Campaign Mini context/actions')
        self.capture('mini-'+str(number),'Load Game')
        require(self.input.focus()['host_focused'],'Host lost focus during Mini ownership')
        time.sleep(.35)
        rows=[];started=time.monotonic();deadline=started+3.0;sent=False
        while time.monotonic()<deadline or len(rows)<40:
            require(time.monotonic()-started<8.0,'Mini sample acquisition exceeded8second cap')
            value=self.original();require(value[0]['ready'] and value[0]['screen']==17
                and value[0]['ack']==ack and value[0]['thread']==state['thread'],'Original Mini ownership changed during dwell')
            rows.append(self.sample('mini-dwell-'+str(number)))
            if not sent and len(rows)>=8:
                self.physical_key('mini-blocked-gameplay-up-'+str(number),0xff52);sent=True
                require(self.input.focus()['host_focused'],'Physical Mini key lost host focus')
            time.sleep(.02)
        proof=validate_dwell(rows);proof.update(number=number,menu=state,first_sample=rows[0]['sample'],last_sample=rows[-1]['sample'],before=before,blocked_key_sent=sent)
        self.dwells.append(proof);self.pause_actions.append(dict(action='pause-dwell',number=number,host_ns=time.monotonic_ns(),first_sample=proof['first_sample'],last_sample=proof['last_sample']))
        return rows[-1]
    def resume(self,number,ack,by_key=False):
        if by_key:
            admitted=cancel_focus(self.access,self.input)
            self.actions.append(dict(action='focus-mini-cancel',host_ns=time.monotonic_ns(),rect=admitted['rect'],focus_admission=admitted))
            self.physical_key('mini-escape-cancel',0xff1b,.08)
            deadline=time.monotonic()+3
            while time.monotonic()<deadline:
                state,raw=self.original()
                if state['ack']==ack:
                    self.samples.append('escape-ack-'+str(number),self.session,raw);break
                time.sleep(.01)
            else:raise ValueError('Physical Mini Escape did not receive original Cancel ACK')
        else:self.click_id('miniMenuButton5')
        prior=self.viewport.poll();old=int(prior['world']);self.wait_progress(old,minimum=4,timeout=30)
        self.focus_viewport();time.sleep(.15);active=self.sample('resumed-'+str(number));state=self.original()[0]
        require(state['ack']==ack and active['input']['active']==1 and not active['input']['held'],
                'Acknowledged Mini resume did not restore gameplay input')
        require(active['original_world_return']>self.dwells[-1]['original_world_return'],
                'Original World did not advance after resume')
        baseline=active['input']['keys'];self.physical_key('resumed-gameplay-up-'+str(number),0xff52)
        self.wait_progress(int(self.viewport.poll()['world']),minimum=3)
        after=self.sample('resumed-input-control-'+str(number))
        require(after['input']['keys'][0x26]!=baseline[0x26] and not after['input']['held'],
                'Positive resumed key control failed to reach gameplay transport')
        self.dwells[-1]['resume']=dict(sample=active['sample'],control_sample=after['sample'],ack=ack,
            original_world_return=active['original_world_return'],by_key=by_key,
            fresh_source_generation=int(self.viewport.poll()['world']),key_generation_advanced=True)
    def run(self):
        self.input.activate();self.click('Launch game');self.await_main()
        self.capture('main-ready-initial','Quick Battle')
        self.click('Preferences');preferences=self.wait(10,1)['preferences']
        self.key('preferences-cancel',0xff1b);self.wait(3,2)
        self.click_id('mainMenuAction0');self.wait(18,3)
        self.click_id('regionEntryDifficulty2');self.wait(18,4)
        self.click_id('regionEntryEnter');self.wait_progress(minimum=5,timeout=60)
        self.samples=Samples(self.out/'pause-samples.sqlite3')
        self.mini(1,5);self.resume(1,6)
        self.mini(2,6);self.resume(2,7,by_key=True)
        # Normal Campaign Quit is a separate, existing public route. It gives
        # no spontaneous defeat/victory credit.
        self.admit_mini('normal-quit',7)
        self.click_id('miniMenuButton4')
        # Quit confirmation belongs to the original native viewport.
        deadline=time.monotonic()+15
        while time.monotonic()<deadline:
            state,raw=self.original()
            if state['ack']==8 and struct.unpack_from('<I',raw,42004)[0]==1:break
            time.sleep(.05)
        else:raise ValueError('Original Quit confirmation was not observed')
        self.focus_viewport();time.sleep(.3)
        point=[round(self.size[0]*265/800),round(self.size[1]*395/600)]
        self.pointer(point)
        for down in (1,0):
            require(self.input.xt.XTestFakeButtonEvent(self.input.display,1,down,0),'Original Quit Yes physical click refused')
            self.input.x.XFlush(self.input.display);time.sleep(.15)
        self.actions.append(dict(action='original-quit-confirm-yes',host_ns=time.monotonic_ns(),point=point))
        self.wait(6,8,timeout=30)
        self.click_id('battleResultContinue');self.wait(3,9,timeout=30)
        self.capture('main-returned','Quick Battle');self.click('Quit')
        self.samples.close();self.samples=None
        return dict(preferences=preferences,dwells=self.dwells,mini_actions=self.pause_actions,
                    no_compiled_test_driver=True,normal_quit_requested=True)
