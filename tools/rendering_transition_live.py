"""Public Campaign Mini failure/restart actions; only the declared ring fault writes."""
import ctypes as c,json,sqlite3,struct,subprocess,time
from pathlib import Path
from rendering_pause_live import PauseJourney
from rendering_pause_low_live import PauseJourney as LowPauseJourney
from rendering_pause import Samples,input_state,stable
from rendering_pilot import require
from native_only_contract import process_identity,outcome
ROOT=Path(__file__).resolve().parents[1]

class TransitionJourney(PauseJourney):
    def __init__(self,child,out,env,size,fault_binary):
        super().__init__(child,out,env,size);self.current_epoch=0;self.epochs=[];self.epoch_roots=[];self.fault_binary=fault_binary;self.epoch_lower_ns=time.monotonic_ns();self.terminal_samples=[]
    def discover(self):
        admissions=[]
        for f in (self.out/'owner/sessions').glob('*.json'):
            row=json.loads(f.read_bytes());path=Path(row['session']);require(path.is_relative_to(ROOT/'working/experiments/menu-observer'),'Unowned producer admission');admissions.append(path)
        require(len(set(admissions))==len(admissions) and len(admissions)<=2,'Unexpected original launch count')
        candidates=[p for p in admissions if p not in self.epoch_roots]
        if self.session is None and candidates:
            require(len(candidates)==1,'Ambiguous new original generation');self.session=candidates[0]
        return self.session
    def read(self):
        value=super().read()
        if self.current_epoch==0 and self.session is None:
            require(not any(v.get('name','').startswith('Native rendering stopped.') for _,v in self.access.nodes()),'Native staging failed before original session admission')
        return value
    def wait(self,*args,**kwargs):
        row=super().wait(*args,**kwargs);row['epoch']=self.current_epoch;return row
    def wait_progress(self,after=0,minimum=5,timeout=30):
        end=time.monotonic()+timeout
        while time.monotonic()<end:
            require(self.child.poll() is None,'Player exited before fresh World progress');row=self.viewport.poll()
            if row and int(row['host_us'])*1000>=self.epoch_lower_ns:
                require([row['frame_width'],row['frame_height']]==self.size,'Wrong actual recovery extent')
                if int(row['world'])>=after+minimum:return row
            time.sleep(.05)
        raise ValueError('New epoch source/presentation progress deadline')
    def admit_mini(self,label,ack):
        return (LowPauseJourney.admit_mini if self.size==[640,480] else PauseJourney.admit_mini)(self,label,ack)
    def enter_world(self,label):
        self.await_main();self.capture(label+'-main','Quick Battle')
        self.click('Preferences');self.wait(10,1);self.key(label+'-preferences-cancel',0xff1b);self.wait(3,2)
        self.click_id('mainMenuAction0');self.wait(18,3);self.click_id('regionEntryDifficulty2');self.wait(18,4);self.click_id('regionEntryEnter')
        row=self.wait_progress(minimum=8,timeout=60);self.samples=Samples(self.out/('epoch-'+str(self.current_epoch)+'-samples.sqlite3'))
        sample=self.sample(label+'-world-active');require(sample['input']['active']==1 and not sample['input']['held'],'World input is not active/released')
        self.epochs.append(dict(root=str(self.session),start_host_ns=self.epoch_lower_ns,first_world=dict(row),first_sample=sample));return sample
    def restart_key(self):
        self.input.activate();require(self.input.focus()['host_focused'],'Public Hard Restart lacks actual host focus')
        self.input.x.XQueryKeymap.argtypes=[c.c_void_p,c.c_void_p]
        symbols=[0xffe3,0xffe1,ord('r')];codes=[self.input.x.XKeysymToKeycode(self.input.display,v) for v in symbols];require(all(codes),'Hard Restart keys unavailable')
        def bitmap():
            b=c.create_string_buffer(32);require(self.input.x.XQueryKeymap(self.input.display,b),'Keymap unavailable');return b.raw
        def bit(raw,code):return bool(raw[code//8]&(1<<(code%8)))
        before=bitmap();require(not any(bit(before,k) for k in codes),'Hard Restart modifier/key already held')
        action=dict(action='physical-hard-restart',host_ns=time.monotonic_ns(),symbols=symbols,keycodes=codes,keymaps=dict(before=before.hex()),host_focus=self.input.focus());self.actions.append(action);down=[]
        try:
            for code in codes:
                require(self.input.xt.XTestFakeKeyEvent(self.input.display,code,1,0),'Hard Restart key-down failed');down.append(code);self.input.x.XFlush(self.input.display);time.sleep(.04)
            pressed=bitmap();require(all(bit(pressed,k) for k in codes),'Hard Restart chord not physically acknowledged');action['keymaps']['pressed']=pressed.hex()
        finally:
            for code in reversed(down):
                require(self.input.xt.XTestFakeKeyEvent(self.input.display,code,0,0),'Hard Restart release failed');self.input.x.XFlush(self.input.display);time.sleep(.03)
            released=bitmap();action['keymaps']['released']=released.hex();require(not any(bit(released,k) for k in codes),'Hard Restart key leaked')
    def terminal(self):
        end=time.monotonic()+30;old=self.session
        while time.monotonic()<end:
            require(self.child.poll() is None,'Native host exited instead of offering recovery')
            path=old/'native-terminal-state.json'
            if path.exists():
                value=json.loads(path.read_bytes());buttons=[v for _,v in self.access.nodes() if v.get('accessible_id','').endswith('.hardRestartGame')]
                if buttons and buttons[0]['enabled']:
                    require(value['session']==str(old) and value['error'] and all(v is True for v in value['state'].values()),'Incomplete actual native terminal snapshot');break
            time.sleep(.05)
        else:raise ValueError('Terminal retirement/public restart deadline')
        self.capture('terminal-error-visible');started=time.monotonic();last=None
        db=sqlite3.connect(self.out/'terminal-samples.sqlite3');db.execute('CREATE TABLE terminal(ordinal INTEGER PRIMARY KEY,host_ns INTEGER,input BLOB,native BLOB,viewport BLOB)')
        while time.monotonic()-started<.65 or len(self.terminal_samples)<8:
            self.viewport.poll();raw_input=stable(old/'render-input.bin',1088,(16,));inp=input_state(raw_input);require(not inp['active'] and not inp['held'],'Terminal input remained active/held')
            raw_state=(old/'native-only-session.json').read_bytes();state=json.loads(raw_state);require(state['state']=='failed','Failed epoch changed class before restart')
            require(not any(v.get('accessible_id','').endswith('.nativeGameViewport') for _,v in self.access.nodes()),'Terminal retained visible game viewport')
            row=dict(host_ns=time.monotonic_ns(),input=inp,native=state,last_world_paint=dict(self.viewport.last) if self.viewport.last else None)
            if last:require(row['last_world_paint']==last['last_world_paint'],'Old World advanced during terminal stability')
            self.terminal_samples.append(row);db.execute('INSERT INTO terminal VALUES(?,?,?,?,?)',(len(self.terminal_samples),row['host_ns'],raw_input,raw_state,json.dumps(row['last_world_paint']).encode()));db.commit();last=row;time.sleep(.05)
        db.close()
        self.epochs[0]['terminal']=value;self.epochs[0]['terminal_samples']=self.terminal_samples
    def normal_quit(self):
        self.admit_mini('fresh-normal-quit',5);self.click_id('miniMenuButton4');end=time.monotonic()+15
        while time.monotonic()<end:
            state,raw=self.original()
            if state['ack']==6 and struct.unpack_from('<I',raw,42004)[0]==1:break
            time.sleep(.05)
        else:raise ValueError('Fresh original Quit confirmation absent')
        self.focus_viewport();time.sleep(.3);point=[round(self.size[0]*265/800),round(self.size[1]*395/600)];self.pointer(point)
        for down in (1,0):
            require(self.input.xt.XTestFakeButtonEvent(self.input.display,1,down,0),'Fresh original Quit Yes refused');self.input.x.XFlush(self.input.display);time.sleep(.15)
        self.actions.append(dict(action='fresh-original-quit-yes',point=point,host_ns=time.monotonic_ns()));self.wait(6,6);self.click_id('battleResultContinue');self.wait(3,7);self.capture('fresh-main-returned','Quick Battle');self.click('Quit')
    def inject_fault(self,channel,header):
        with (self.out/'fault-injection.stderr').open('xb') as err:
            run=subprocess.run([self.fault_binary,str(ROOT),str(channel),str(header[4])],cwd=ROOT,capture_output=False,stdout=subprocess.PIPE,stderr=err,check=True,timeout=5)
        fault=json.loads(run.stdout);(self.out/'fault-injection.json').write_bytes(run.stdout)
        self.actions.append(dict(action='declared-terminal-fault',host_ns=time.monotonic_ns(),producer=header[4],record='fault-injection.json'))
        return fault
    def run(self):
        self.input.activate();self.click('Launch game');self.enter_world('old')
        self.mini(1,5);self.capture('old-mini-before-fault');self.epochs[0]['mini']=self.dwells[0]
        boundary=self.sample('mini-fault-boundary');state=self.original()[0]
        require(state['ready'] and state['screen']==17 and state['ack']==5 and not boundary['input']['active'] and not boundary['input']['held'] and boundary['world']['pending_world_jobs']==0 and boundary['original_world_return']==self.dwells[0]['original_world_return'],'Fault boundary lost real Mini ownership/quiescence')
        self.epochs[0]['fault_boundary']=boundary
        admission=json.loads((self.session/'native-output-admission.json').read_bytes());self.epochs[0]['admission']=admission
        channel=self.session/'render-commands.bin';end=time.monotonic()+2
        while time.monotonic()<end:
            h=struct.unpack('<16I',channel.read_bytes()[:64])
            if h[6]==1 and h[5]==h[9] and not h[7] and not h[8]:break
            time.sleep(.02)
        else:raise ValueError('Mini command transport did not drain')
        self.epochs[0]['fault']=self.inject_fault(channel,h)
        self.samples.close();self.samples=None;self.terminal();self.epoch_roots.append(self.session)
        self.current_epoch=1;self.epoch_lower_ns=time.monotonic_ns();self.restart_key();self.session=None;self.last=None;self.viewport.last=None;self.viewport.paint=None;self.viewport_geometry=None
        self.enter_world('fresh');require(self.session!=self.epoch_roots[0],'Hard Restart reused the original namespace')
        old_outcome=outcome(self.epoch_roots[0]);require(old_outcome['state']=='restarted' and old_outcome['rendering_accepted'] is False,'Failed epoch gained normal credit');self.epochs[0]['outcome']=old_outcome
        current=json.loads((self.session/'native-output-admission.json').read_bytes());require(current['runner_identity']!=admission['runner_identity'],'Fresh launch reused the original runner identity');self.epochs[1]['admission']=current
        before=self.sample('fresh-before-gameplay-control');self.focus_viewport();self.physical_key('fresh-gameplay-up',0xff52);self.wait_progress(int(self.viewport.poll()['world']),minimum=3)
        after=self.sample('fresh-after-gameplay-control');require(after['input']['active']==1 and not after['input']['held'] and after['input']['keys'][0x26]!=before['input']['keys'][0x26] and after['original_world_return']>before['original_world_return'],'Fresh generation input/original progress failed')
        self.epochs[1]['positive_input']=dict(before=before,after=after);self.normal_quit();self.samples.close();self.samples=None
        return dict(epochs=self.epochs,controlled_faults=1,physical_restart=True,no_compiled_test_driver=True,failed_epoch_normal_credit=False)
