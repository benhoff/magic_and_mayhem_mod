#!/usr/bin/env python3
"""Physical HUD, portrait tooltip, Space pause and Mini return on the public player."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import sys
import struct
import sqlite3
import subprocess
import time
from coverage_evidence import Reader
from rendering_pilot import require
from rendering_ui_acceptance import PREFIX,decode
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--plan',required=True);p.add_argument('--report',required=True)
    p.add_argument('--software',action='store_true');p.add_argument('--low',action='store_true');p.add_argument('--repair-of');p.add_argument('--repair-reason');a=p.parse_args()
    if os.environ.get('MNM_INTERACTION_BUS_OWNED')!='1':
        runtime=ROOT/'working/ui-runtime'/Path(a.report).stem;runtime.mkdir(parents=True,mode=0o700,exist_ok=False)
        env=dict(os.environ,XDG_RUNTIME_DIR=str(runtime),MNM_INTERACTION_BUS_OWNED='1');env.pop('AT_SPI_BUS_ADDRESS',None)
        os.execvpe('dbus-run-session',['dbus-run-session','--',sys.executable,__file__,*sys.argv[1:]],env)
    spec=importlib.util.spec_from_file_location('ui_base',ROOT/'tools/run-rendering-interaction.py');base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
    base.PREFIX,base.PLAN=PREFIX,a.plan
    # Charge before prefix admission: zero-game preparations must remain in
    # both the request count and the exact repair-ancestry graph.
    require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'Fresh UI request required')
    with Reader(ROOT) as reader:
        prospective=reader.json(a.plan)
        prior=[]
        for path in sorted((ROOT/PREFIX).glob('*-acquisition.json')):
            q=reader.json(path.relative_to(ROOT))
            require(q['kind']=='public_native_ui_acquisition' and type(q['original_game_runs']) is int,'Unresolved prior UI request')
            prior.append((str(path.relative_to(ROOT)),reader.digest(path.relative_to(ROOT)),q))
    budget=ROOT/'working/ui-requests.sqlite3'
    with sqlite3.connect(budget) as db,db:
        db.execute('CREATE TABLE IF NOT EXISTS requests(report TEXT PRIMARY KEY,plan TEXT NOT NULL,sha256 TEXT)')
        db.execute('BEGIN IMMEDIATE')
        for name,digest,q in prior:
            old=db.execute('SELECT sha256 FROM requests WHERE report=?',(name,)).fetchone()
            require(old is None or old[0] in (None,digest),'Prior UI request identity changed')
            db.execute('INSERT OR IGNORE INTO requests VALUES(?,?,?)',(name,q['prospective_plan'],digest))
            db.execute('UPDATE requests SET sha256=? WHERE report=?',(digest,name))
        known=list(db.execute('SELECT report FROM requests'))
        require(len(known)==len(prior) and {n for (n,) in known}=={n for n,h,q in prior},'Pending or unaccounted UI request')
        require(len(prior)<prospective['bounds']['maximum_requests'] and sum(q['original_game_runs'] for n,h,q in prior)<prospective['bounds']['maximum_original_games'],'Frozen UI request/game budget exhausted')
        db.execute('INSERT INTO requests VALUES(?,?,NULL)',(a.report,a.plan))
    original_borrow=base.borrow
    def borrow(template,target,environment):
        deadline=time.monotonic()+30
        while True:
            try:return original_borrow(template,target,environment)
            except BlockingIOError:
                require(not Path(target).exists() and not (Path(target).parent/('environment-'+Path(target).name+'.sqlite3')).exists(),'Partially acquired prefix cannot be retried')
                if time.monotonic()>=deadline:raise
                time.sleep(.5)
    base.borrow=borrow
    original_paths=base.native_runtime_source_paths
    base.native_runtime_source_paths=lambda root:original_paths(root)|{'tools/prepare-rendering-ui.py','tools/run-rendering-ui.py','tools/rendering_ui_acceptance.py'}
    original_configure=base.configure
    base.configure=lambda env,argv:dict(original_configure(env,argv),MNM_ACCEPTANCE_UI='1')
    old_write=base.write_report
    def write(root,name,raw):
        if name==a.report:
            q=json.loads(raw);q.update(kind='public_native_ui_acquisition',prospective_plan=a.plan,scope=__doc__)
            raw=(json.dumps(q,indent=2)+'\n').encode()
        return old_write(root,name,raw)
    base.write_report=write
    def admit(cases,repair,requests,games):
        with Reader(ROOT) as r:
            cases[:]=[dict(report=n,repair_of=q.get('repair_of'),result=q,proved=False,comparison_failed=False,preparation_fixed=False) for n,h,q in prior]
            for c in cases:
                candidates=list((ROOT/PREFIX).glob(Path(c['report']).name.replace('-acquisition.json','-comparison')+'*.json'))
                c['proved']=any((q:=r.json(n.relative_to(ROOT)))['success'] and q.get('ui_proof',{}).get('verified')
                    and q['acquisition']==dict(record=c['report'],sha256=r.digest(c['report'])) for n in candidates)
                c['comparison_failed']=any(not r.json(n.relative_to(ROOT))['success'] for n in candidates)
        return __import__('rendering_interaction_budget').admit(cases,repair,requests,games)
    base.admit=admit
    class Journey(base.InteractionJourney):
        def run(self):
            self.input.activate();self.set_scaling('sharp');self.click('Launch game');self.await_main()
            self.capture('main-ready-initial','Quick Battle');self.click('Preferences')
            preferences=self.wait(10)['preferences'];self.key('preferences-cancel',0xff1b);self.wait(3)
            self.click_id('mainMenuAction0');self.wait(18,timeout=60)
            self.click_id('regionEntryDifficulty1');self.wait(18)
            self.click_id('regionEntryEnter');self.wait_progress(timeout=60)
            self.set_scaling('integer');self.clear_dialogue('campaign-intro')
            self.interaction_schedule()
            self.open_mini();self.click_id('miniMenuButton4')
            deadline=time.monotonic()+12
            while time.monotonic()<deadline:
                events=base.module('test-menu-observer').decode((self.session/'events.bin').read_bytes())
                if any(v['event']==18 and v['argument'] for v in events):break
                time.sleep(.05)
            else:raise ValueError('Original campaign Quit confirmation absent')
            # Installed original 800x600 logical dialog scales with the frame.
            self.pointer([round(265*self.size[0]/800),round(395*self.size[1]/600)])
            for down in (1,0):
                require(self.input.xt.XTestFakeButtonEvent(self.input.display,1,down,0),'Physical Quit answer refused')
                self.input.x.XFlush(self.input.display);time.sleep(.15)
            self.wait(6,timeout=30);self.click_id('battleResultContinue');self.wait(3,timeout=30)
            self.capture('main-ready-returned','Quick Battle');self.click('Quit')
            return dict(preferences=preferences,campaign='fresh Celtic Forest, Adept',camera_actions=self.camera_actions)

        def open_mini(self):
            for attempt in range(8):
                self.key('physical-campaign-mini-'+str(attempt),0xff1b)
                deadline=time.monotonic()+1
                while time.monotonic()<deadline:
                    value=self.read()
                    if value and value[0]['ready'] and value[0]['screen']==17:
                        return self.wait(17)
                    time.sleep(.05)
            raise ValueError('Original campaign Mini admission absent after bounded Escape attempts')

        def clear_dialogue(self,label):
            from PIL import Image
            from campaign_input_admission import DialogueReadiness
            gate=DialogueReadiness(True,12,48)
            while True:
                shot=self.capture(label+'-'+str(gate.observations))
                rect=self.screen_geometry(self.viewport_geometry) if self.viewport_geometry else None
                if rect is None:
                    nodes=[v for _,v in self.access.nodes() if v.get('accessible_id','').endswith('.nativeGameViewport')]
                    require(len(nodes)==1,'Native dialogue viewport absent');rect=nodes[0]['rect']
                # Integer mode with DPR1: source image centred in the viewport.
                x=rect[0]+(rect[2]-self.size[0])//2;y=rect[1]+(rect[3]-self.size[1])//2
                crop=self.out/(label+'-'+str(gate.observations)+'-dialogue.png')
                Image.open(ROOT/shot['path']).crop((x+round(225*self.size[0]/800),y+round(90*self.size[1]/600),x+round(600*self.size[0]/800),y+round(270*self.size[1]/600))).save(crop)
                text=subprocess.run(['tesseract',str(crop),'stdout','--psm','6'],capture_output=True,text=True,check=True,timeout=10).stdout
                visible='hermes' in text.lower();decision=gate.observe({'native':visible,'original':visible},time.monotonic())
                self.actions.append(dict(action='dialogue-readiness',label=label,visible=visible,text=text,decision=decision,host_ns=time.monotonic_ns(),scope='Actual viewport OCR supplies an ordinary-input hint only; final state/output proof is independent.'))
                if decision=='ready':return
                if decision=='dismiss':self.left(label+'-dismiss-'+str(gate.dismissals),[self.size[0]//2,self.size[1]//2])
                time.sleep(3)
        def click_id(self,name):
            if name=='spellboxOK':
                deadline=time.monotonic()+3;value=None
                while time.monotonic()<deadline:
                    value=self.read()
                    if value and value[0]['ready'] and value[0]['screen']==7:break
                    time.sleep(.05)
                require(value and value[0]['ready'] and value[0]['screen']==7,'Stable original spellbox snapshot unavailable')
                raw=value[1];at=18000;offered=struct.unpack_from('<I',raw,at+8)[0];counts=struct.unpack_from('<3I',raw,at+16)
                items=[i for i in range(21) if offered&(1<<i)];assignments=[]
                for spell,alignment in [(71,1),(14,2)]:
                    choices=[i for i in items if struct.unpack_from('<I',raw,at+380+(i*3+alignment)*4)[0]==spell]
                    require(len(choices)==1 and counts[alignment]>0,'Original offered loadout lacks the documented Fireball/Zombie pair')
                    item=choices[0];index=items.index(item);slot=sum(counts[:alignment])
                    super().click_id('spellboxItem'+str(index));super().click_id('spellboxTalisman'+str(slot))
                    labels=[v['name'] for _,v in self.access.nodes() if v.get('accessible_id','').endswith('.spellboxTalisman'+str(slot))]
                    require(len(labels)==1 and ('Fireball' if spell==71 else 'Zombie') in labels[0],'Physical loadout placement did not reach the requested spell')
                    assignments.append(dict(item=item,spell=spell,alignment=alignment,slot=alignment*21))
                self.actions.append(dict(action='original-offered-loadout',offered=offered,assignments=assignments,host_ns=time.monotonic_ns()))
            return super().click_id(name)
        def ui(self):
            raw=(self.session/'ui-acceptance.bin').read_bytes();raw=raw[:16+(len(raw)-16)//512*512]
            return decode(raw,allow_pending=True)['rows']
        def dwell(self,name,seconds=2):
            self.mark(name,'begin');released=self.mark(name,'released');time.sleep(seconds)
            self.wait_progress(int(released['world']),minimum=3);self.mark(name,'settled')
        def left(self,name,point,button=1):
            self.mark(name,'begin');self.pointer(point)
            for down in (1,0):
                require(self.input.xt.XTestFakeButtonEvent(self.input.display,button,down,0),'Physical UI click refused')
                self.input.x.XFlush(self.input.display);time.sleep(.12)
            released=self.mark(name,'released');self.settle(name,released,.7)
        def mini_snapshot(self,label):
            for name in ('render-input.bin','native-media.bin','world-continuous.bin'):
                path=self.session/name;deadline=time.monotonic()+1
                while time.monotonic()<deadline:
                    raw=path.read_bytes()
                    if path.read_bytes()==raw:break
                else:raise ValueError('UI ownership snapshot did not settle: '+name)
                (self.out/(label+'-'+name)).write_bytes(raw)
            self.actions.append(dict(action=label,host_ns=time.monotonic_ns(),ui_row=len(self.ui())))
        def tooltip(self,suffix=''):
            self.mark('tooltip-open'+suffix,'begin');self.pointer([self.size[0]-70,self.size[1]-70]);released=self.mark('tooltip-open'+suffix,'released')
            deadline=time.monotonic()+8;samples=[]
            while time.monotonic()<deadline:
                require(self.child.poll() is None,'Player exited during tooltip dwell')
                samples=[r for r in self.ui() if r['kind']==1 and r['frame']['generation']>released['frame_generation'] and r['tooltip']['visible'] and r['tooltip']['text']]
                if len(samples)>=3 and samples[-1]['tick']-samples[-3]['tick']>=100:break
                time.sleep(.1)
            require(len(samples)>=3,'Original portrait tooltip did not become visible')
            self.mark('tooltip-open'+suffix,'settled')
            self.move('tooltip-close'+suffix,[40,100]);require(self.ui()[-1]['tooltip']['visible']==0,'Original tooltip remained visible')
        def interaction_schedule(self):
            self.set_scaling('integer')
            if self.ui()[-1]['mode']==1:self.held_key('initial-resume',0x20,.22,.6)
            for n in range(2):self.held_key('center-'+str(n),ord('w'),.22,.6)
            state,target=self.ready_target();self.actions.append(dict(action='hud-source-target',frame_input_row=state['ordinal'],**target))
            self.move('wizard-hover',target['point']);self.toggle('selection-toggle-1',target['point']);self.move('selection-away-1',[40,100])
            self.toggle('selection-toggle-2',target['point']);self.move('selection-away-2',[40,100])
            self.clear_dialogue('before-tooltip')
            self.tooltip()
            before_mana=self.ui()[-1]['wizard']['mana']
            for spell,point in [('fireball',[477,575]),('zombie',[523,575])]:
                self.left('select-'+spell,[point[0]+self.size[0]-800,point[1]+self.size[1]-600])
            self.move('summon-ground',[self.size[0]//2+40,self.size[1]//2+40])
            self.clear_dialogue('before-cast')
            # A modal may consume the earlier selection. Distinct original
            # spell icons re-admit casting before the first and only cast.
            for spell,point in [('fireball',[477,575]),('zombie',[523,575])]:
                self.left('readmit-'+spell,[point[0]+self.size[0]-800,point[1]+self.size[1]-600])
            self.move('summon-ground-ready',[self.size[0]//2+40,self.size[1]//2+40])
            require(self.latest()['mode']==2 and self.latest()['cursor_type']==5,'Original summon cursor was not admitted')
            self.left('summon-cast',[self.size[0]//2+40,self.size[1]//2+40],3)
            require(self.ui()[-1]['wizard']['mana']<before_mana,'Ordinary summon did not debit original current mana')
            self.left('cancel-summon',[523+self.size[0]-800,575+self.size[1]-600])
            self.tooltip('-after-cast')
            self.held_key('space-pause',0x20,.22,2)
            require(self.ui()[-1]['mode']==1,'Original Space pause mode absent')
            self.held_key('space-resume',0x20,.22,1)
            require(self.ui()[-1]['mode']==0,'Original Space resume mode absent')
            before_mini=int(self.viewport.poll()['world'])
            self.mark('mini-open','begin');self.open_mini()
            self.mini_snapshot('mini-paused-begin')
            time.sleep(1.5);self.capture('mini-paused-ready')
            self.mini_snapshot('mini-paused-end')
            self.click_id('miniMenuButton5');self.wait_progress(before_mini,minimum=5)
            self.mark('mini-resume','settled');self.dwell('hud-natural-update',4)
    base.InteractionJourney=Journey
    argv=[sys.argv[0],'--report',a.report]+(['--software'] if a.software else [])+(['--low'] if a.low else [])
    if a.repair_of:argv+=['--repair-of',a.repair_of,'--repair-reason',a.repair_reason]
    sys.argv=argv
    return base.main()


if __name__=='__main__':raise SystemExit(main())
