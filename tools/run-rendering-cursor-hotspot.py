#!/usr/bin/env python3
"""Bounded physical nonzero-hotspot matrix, separate from exhausted resize plans."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import sys
import time
from coverage_evidence import Reader,write_report
from cursor_hotspot import ROOT,PREFIX,PLAN,MAX_REQUESTS,MAX_GAMES,acceptance_points as points
from rendering_pilot import require


def main():
    parser=argparse.ArgumentParser(add_help=False)
    parser.add_argument('--report',required=True);parser.add_argument('--plan',default=PLAN)
    args,remaining=parser.parse_known_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts,'Owned hotspot report required')
    if os.environ.get('MNM_INTERACTION_BUS_OWNED')!='1':
        runtime=ROOT/'working/hotspot-runtime'/Path(args.report).stem
        runtime.mkdir(parents=True,mode=0o700,exist_ok=False)
        env=dict(os.environ,XDG_RUNTIME_DIR=str(runtime),MNM_INTERACTION_BUS_OWNED='1',
                 ATSPI_DBUS_IMPLEMENTATION='dbus-daemon')
        env.pop('AT_SPI_BUS_ADDRESS',None)
        os.execvpe('dbus-run-session',['dbus-run-session','--',sys.executable,__file__,*sys.argv[1:]],env)
    spec=importlib.util.spec_from_file_location('hotspot_collector',ROOT/'tools/run-rendering-interaction.py')
    base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
    base.PREFIX=PREFIX;base.PLAN=args.plan;base.__doc__=__doc__
    original_paths=base.native_runtime_source_paths
    base.native_runtime_source_paths=lambda root: original_paths(root)|{
        'tools/run-rendering-cursor-hotspot.py','tools/prepare-rendering-cursor-hotspot.py','tools/cursor_hotspot.py',
        'tools/run-rendering-interaction.py','tools/rendering_interaction_budget.py','tools/rendering_interaction.py',
        'tools/rendering_interaction_live.py','tools/rendering_interaction_stability.py'}
    old_configure=base.configure
    def configure(env,flags):
        result=old_configure(env,flags)
        out=ROOT/'working/interaction-acceptance'/Path(args.report).stem
        result.update(MNM_NATIVE_COPY_DIAGNOSTICS=str(out/'copy-diagnostics'),
            MNM_NATIVE_COPY_DIAGNOSTIC_SCOPE='all',MNM_CURSOR_COPY_STATE='1',
            MNM_NATIVE_COPY_CALLERS='1',MNM_NATIVE_CURSOR_COPY_CALLERS='1')
        return result
    base.configure=configure
    old_admit=base.admit
    def admit(cases,repair,requests,games):
        require((requests,games)==(MAX_REQUESTS,MAX_GAMES),'Hotspot budget changed')
        # A failure before the shared collector reaches its attempt INSERT is
        # still a request. Include every immutable failed report in this owned
        # question; no preflight failure may disappear from the eight-request cap.
        known={c['report'] for c in cases}
        with Reader(ROOT) as reader:
            for path in sorted((ROOT/PREFIX).glob('*-acquisition.json')):
                name=str(path.relative_to(ROOT))
                if name in known:continue
                data=reader.json(name)
                require(data['success'] is False,'Uncatalogued successful request needs review')
                cases.append(dict(report=name,repair_of=data.get('repair_of'),result=data,
                    proved=False,comparison_failed=False,preparation_fixed=False))
        require(all(c['report'].startswith(PREFIX) for c in cases) and
                (repair is None or repair.startswith(PREFIX)),'Foreign/exhausted batch cannot reset into hotspot accounting')
        return old_admit(cases,repair,requests,games)
    base.admit=admit
    with Reader(ROOT) as r:
        plan_ref=dict(record=args.plan,sha256=r.digest(args.plan))
        zero_hotspot_classes={int(n) for n,d in r.json(args.plan)['assets']['descriptors'].items()
                              if d['hotspot']==[0,0]}
    old_writer=base.write_report
    def writer(root,path,data):
        if str(path)==args.report:
            result=json.loads(data);result['prospective_plan']=plan_ref
            # A discovered original session is charged even when its exit
            # recorder is absent or malformed. Failure cannot refund a start.
            if result.get('session') or result.get('states'):
                result['start_accounting']=dict(base_count=result.get('original_game_runs'),
                    policy='Conservatively charge every discovered original session')
                result['original_game_runs']=1
            result['configuration']['cursor_hotspot']=dict(physical_actions_only=True,
                required_nonzero_classes=[6,7,9],copy_census='all',cursor_state_version=1,
                cursor_caller_handoff_version=1,synthetic_coordinate_exercise=False)
            result['source_commit']=__import__('subprocess').check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()
            data=(json.dumps(result,indent=2)+'\n').encode()
        return old_writer(root,path,data)
    base.write_report=writer

    class HotspotJourney(base.InteractionJourney):
        def settle(self,action,released,delay=.55):
            point=next(v['logical'] for v in reversed(self.actions) if v['action']=='pointer')
            started=time.monotonic();seen={}
            while time.monotonic()-started<=10:
                require(self.child.poll() is None,'Player exited during hotspot observation')
                paint=self.viewport.poll()
                if paint and int(paint['world'])>released['frame_generation']:
                    raw=(self.session/'frame-input.bin').read_bytes()
                    raw=raw[:16+(len(raw)-16)//384*384]
                    rows=base.decode(raw,allow_pending=True)['rows']
                    row=next((r for r in reversed(rows) if r['kind']==1 and r['frame']
                              and r['frame']['generation']==int(paint['world'])),None)
                    if (row and row['cursor'] and row['cursor']['point']==point and
                            (row['tick']-row['cursor']['tick'])%2**32<=500):
                        seen[row['frame']['generation']]=dict(generation=row['frame']['generation'],
                            ordinal=row['ordinal'],cursor_class=row['cursor_type'],saved=row['saved_cursor'],
                            paint=paint['paint'],poll=row['cursor']['poll'])
                last=[seen[g] for g in sorted(seen)[-3:]]
                wanted={6,7,9} if '-hotspot-frame-' in action else zero_hotspot_classes
                if (len(last)==3 and len({r['cursor_class'] for r in last})==1
                        and last[-1]['cursor_class'] in wanted
                        and all(r['saved'] and r['saved'][0]==1 for r in last)
                        and time.monotonic()-started>=.3):
                    self.actions.append(dict(action='hotspot-readiness',name=action,point=point,
                        observations=[seen[g] for g in sorted(seen)],host_ns=time.monotonic_ns()))
                    self.mark(action,'settled');return
                time.sleep(.04)
            raise ValueError('Three fresh displayed original hotspot observations missing')

        def interaction_schedule(self):
            for mode in ('sharp','integer'):
                self.set_scaling(mode)
                if self.latest()['mode']==1:self.held_key(mode+'-resume',0x20,.22,.6)
                for label,point in points(self.size):
                    self.move(mode+'-hotspot-'+label,point)
                    # Explicit return after each edge observes background restoration
                    # at the old location; no synthetic coordinates or memory writes.
                    if label!='center':self.move(mode+'-restore-'+label,points(self.size)[0][1])
                for repeat in range(2):self.held_key(mode+'-center-after-edges-'+str(repeat),ord('w'),.22,.6)

    base.InteractionJourney=HotspotJourney
    sys.argv=[sys.argv[0],'--report',args.report,*remaining]
    return base.main()


if __name__=='__main__':raise SystemExit(main())
