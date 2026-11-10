#!/usr/bin/env python3
"""Physical fullscreen and same-process 800/640/800 provider observations."""
import argparse,importlib.util,json,os,sys,time
from pathlib import Path
from coverage_evidence import Reader
from rendering_pilot import require
from required_boundary_observation import PREFIX,CONTROLS
ROOT=Path(__file__).resolve().parents[1]

def main():
    p=argparse.ArgumentParser(add_help=False);p.add_argument('--plan',required=True)
    args,remaining=p.parse_known_args()
    if os.environ.get('MNM_INTERACTION_BUS_OWNED')!='1':
        runtime=ROOT/'working/required-boundaries/runtime'/str(os.getpid());runtime.mkdir(parents=True,mode=0o700)
        env=dict(os.environ,XDG_RUNTIME_DIR=str(runtime),MNM_INTERACTION_BUS_OWNED='1',ATSPI_DBUS_IMPLEMENTATION='dbus-daemon')
        env.pop('AT_SPI_BUS_ADDRESS',None)
        os.execvpe('dbus-run-session',['dbus-run-session','--',sys.executable,__file__,*sys.argv[1:]],env)
    spec=importlib.util.spec_from_file_location('required_session',ROOT/'tools/run-rendering-interaction.py')
    b=importlib.util.module_from_spec(spec);spec.loader.exec_module(b)
    b.PREFIX,b.PLAN=PREFIX,args.plan
    with Reader(ROOT) as reader:
        prior=[(v.relative_to(ROOT).as_posix(),reader.json(v.relative_to(ROOT)))
               for v in sorted((ROOT/PREFIX).glob('*-acquisition.json'))]
        plan=reader.json(args.plan)
        require(len(prior)<plan['bounds']['maximum_requests'] and
                sum(v['original_game_runs'] for _,v in prior)<plan['bounds']['maximum_original_games'],
                'Prospective boundary budget exhausted')
    original_admit=b.admit
    def admit(cases,repair,requests,games):
        known={c['report'] for c in cases}
        for name,result in prior:
            if name not in known:
                cases.append(dict(report=name,repair_of=result.get('repair_of'),result=result,
                                  proved=False,comparison_failed=False,preparation_fixed=False))
        return original_admit(cases,repair,requests,games)
    b.admit=admit
    original_paths=b.native_runtime_source_paths
    b.native_runtime_source_paths=lambda root:original_paths(root)|set(CONTROLS)
    configure=b.configure
    b.configure=lambda environment,flags:dict(configure(environment,flags),MNM_REQUIRED_BOUNDARIES='1')
    journeys=[];writer=b.write_report
    def write(root,path,raw):
        if path.endswith('-acquisition.json'):
            q=json.loads(raw);q['prospective_plan']=args.plan
            q['kind']='required_boundary_session_acquisition'
            if journeys:q['required_boundaries']=dict(transitions=journeys[0].transitions,rebuilds=journeys[0].rebuilds,geometry_samples=journeys[0].geometry_samples)
            raw=(json.dumps(q,indent=2)+'\n').encode()
        return writer(root,path,raw)
    b.write_report=write
    class Journey(b.InteractionJourney):
        def __init__(self,*a):
            super().__init__(*a);self.transitions=[];self.rebuilds=[];self.geometry_samples=[];self.viewport_node=None;journeys.append(self)
        def geometry(self):
            if self.viewport_node is None:
                nodes=[n for n,v in self.access.nodes() if v.get('accessible_id','').endswith('.nativeGameViewport')]
                require(len(nodes)==1,'Missing viewport geometry');self.viewport_node=nodes[0]
            r=self.viewport_node.get_component_iface().get_extents(self.access.a.CoordType.SCREEN)
            return dict(origin=self.window_origin(),viewport=[r.x,r.y,r.width,r.height],focus=self.input.focus())
        def transition(self,label,fullscreen,toolbar=False):
            before=self.mark(label,'begin');geometry=self.geometry()
            if toolbar:self.click_id('fullscreenButton')
            else:self.key(label,0xffc6) # X11 F9
            self.viewport_geometry=None;self.scaling_geometry=None
            deadline=time.monotonic()+30;paint=None
            while time.monotonic()<deadline:
                g=self.geometry();row=self.viewport.poll()
                self.geometry_samples.append(dict(action=label,host_ns=time.monotonic_ns(),geometry=g,paint=row))
                if row and int(row['world'])>int(before['world']) and g['viewport'][2:]==[row['width'],row['height']]:
                    if (g['viewport'][2:]==[1280,1024])==fullscreen:
                        paint=row;break
                time.sleep(.05)
            require(paint,'Fullscreen/window geometry not acknowledged')
            self.pointer([self.size[0]//2,self.size[1]//2])
            # Verify original input delivery and World progress at the new size.
            self.held_key(label+'-input',ord('w'),.22,.3)
            after=self.mark(label,'settled')
            self.transitions.append(dict(name=label,fullscreen=fullscreen,toolbar=toolbar,size=list(self.size),
                before=before,after=after,geometry_before=geometry,geometry_after=self.geometry(),paint=paint))
        def phase(self,label):
            self.transition(label+'-toolbar-enter',True,True)
            self.transition(label+'-shortcut-exit',False)
            self.transition(label+'-shortcut-enter',True)
            self.transition(label+'-shortcut-return',False)
        def rebuild(self,low):
            before=self.latest();old=list(self.size);world=int(self.viewport.poll()['world'])
            self.held_key('exit-rebuild-'+str(low),0xff1b,.3,.2,advance=False)
            self.wait(26);self.click_id('quickResultAction2');self.wait(22)
            self.key('return-main',0xff1b);self.wait(3)
            self.click('Preferences');preferences=self.wait(10)
            self.click_id('preferencesRadio'+str(2 if low else 1));self.click_id('preferencesOk');self.wait(3)
            self.click('Quick Battle');self.wait(22);self.click('Create Single Player Game');selected=self.wait(14)
            require(selected['setup']['map']==1,'Undeclared recreated map')
            self.click_id('singlePlayerStart');self.wait(7,timeout=60);self.click_id('spellboxOK')
            self.size=[640,480] if low else [800,600];self.viewport_geometry=None;self.scaling_geometry=None
            # Old-size paints can remain while the new World is being imported.
            deadline=time.monotonic()+30
            while time.monotonic()<deadline:
                paint=self.viewport.poll()
                if paint and [paint['frame_width'],paint['frame_height']]==self.size and int(paint['world'])>=world+5:break
                require(self.child.poll() is None,'Player exited rebuilding');time.sleep(.05)
            else:raise ValueError('Missing recreated World')
            self.rebuilds.append(dict(before=old,after=list(self.size),original_before=before,
                original_after=self.latest(),preferences=preferences,confirmed_paint=paint))
        def run(self):
            self.input.activate();self.set_scaling('sharp');self.click('Launch game');self.await_main()
            self.click('Preferences');preferences=self.wait(10)['preferences'];self.key('cancel',0xff1b);self.wait(3)
            self.click('Quick Battle');self.wait(22);self.click('Create Single Player Game');selected=self.wait(14)
            require(selected['setup']['map']==1,'Undeclared initial map')
            self.click_id('singlePlayerStart');self.wait(7,timeout=60);self.click_id('spellboxOK');self.wait_progress()
            self.phase('high-initial');self.rebuild(True);self.phase('low');self.rebuild(False);self.phase('high-returned')
            self.held_key('open-results',0xff1b,.3,.2,advance=False);self.wait(26)
            self.click_id('quickResultAction2');self.wait(22);self.key('quick-return',0xff1b);self.wait(3);self.click('Quit')
            return dict(preferences=preferences,map=selected['setup'],camera_actions=self.camera_actions)
    b.InteractionJourney=Journey;sys.argv=[sys.argv[0],*remaining];return b.main()
if __name__=='__main__':raise SystemExit(main())
