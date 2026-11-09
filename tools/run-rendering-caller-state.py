#!/usr/bin/env python3
"""Bounded public caller-state/pixel acquisition; first pilot must pass before expansion."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time
from coverage_evidence import Reader,write_report
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/caller-state-installed-results-20261108/'
PLAN=PREFIX+'input-plan-01.json'
MAX_REQUESTS=9
MAX_GAMES=7
CONTROLS={'tools/run-rendering-caller-state.py','tools/prepare-rendering-caller-state.py',
          'tools/run-rendering-interaction.py','tools/rendering_interaction.py',
          'tools/rendering_interaction_budget.py','tools/rendering_interaction_live.py'}


def main():
    p=argparse.ArgumentParser(add_help=False);p.add_argument('--report',required=True);p.add_argument('--plan',default=PLAN)
    args,rest=p.parse_known_args()
    require(args.report.startswith(PREFIX)and '..'not in Path(args.report).parts and not(ROOT/args.report).exists(),'Fresh owned caller-state request required')
    if os.environ.get('MNM_INTERACTION_BUS_OWNED')!='1':
        runtime=ROOT/'working/caller-state-runtime'/Path(args.report).stem
        runtime.mkdir(parents=True,mode=0o700,exist_ok=False)
        env=dict(os.environ,XDG_RUNTIME_DIR=str(runtime),MNM_INTERACTION_BUS_OWNED='1',ATSPI_DBUS_IMPLEMENTATION='dbus-daemon')
        env.pop('AT_SPI_BUS_ADDRESS',None)
        os.execvpe('dbus-run-session',['dbus-run-session','--',sys.executable,__file__,*sys.argv[1:]],env)
    spec=importlib.util.spec_from_file_location('caller_state_public_collector',ROOT/'tools/run-rendering-interaction.py')
    base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
    base.PREFIX=PREFIX;base.PLAN=args.plan;base.__doc__=__doc__
    old_paths=base.native_runtime_source_paths
    base.native_runtime_source_paths=lambda root:old_paths(root)|CONTROLS
    old_configure=base.configure
    ram=Path(tempfile.mkdtemp(prefix='mnm-caller-capture-',dir='/tmp'));(ram/'capture').mkdir()
    def configure(env,flags):
        result=old_configure(env,flags);out=ROOT/'working/interaction-acceptance'/Path(args.report).stem
        require(not(out/'capture').exists()and not(out/'worlds.bin').exists(),'Owned capture targets already exist')
        (out/'capture').symlink_to(ram/'capture',target_is_directory=True);(out/'worlds.bin').symlink_to(ram/'worlds.bin')
        result.update(MNM_DRAW_CALLER_STATE='1',MNM_NATIVE_COPY_CALLERS='1',
            MNM_NATIVE_COPY_DIAGNOSTICS=str(out/'copy-diagnostics'),MNM_NATIVE_COPY_DIAGNOSTIC_SCOPE='all')
        return result
    base.configure=configure
    old_admit=base.admit
    def admit(cases,repair,requests,games):
        require((requests,games)==(MAX_REQUESTS,MAX_GAMES),'Caller-state prospective budget changed')
        require(repair is None or repair.startswith(PREFIX),'Foreign repair cannot reset caller-state accounting')
        cases[:]=[c for c in cases if c['report'].startswith(PREFIX)]
        known={c['report']for c in cases}
        with Reader(ROOT)as reader:
            for path in sorted((ROOT/PREFIX).glob('*-acquisition.json')):
                name=path.relative_to(ROOT).as_posix()
                if name not in known:
                    data=reader.json(name)
                    cases.append(dict(report=name,repair_of=data.get('repair_of'),result=data,proved=False,comparison_failed=False,preparation_fixed=False))
            for case in cases:
                a=case['result'];same=a.get('player_binary',{}).get('sha256')==plan['player_binary']['sha256']and all(a.get('sources',{}).get(n)==h for n,h in plan['runtime_sources'].items())
                comparisons=[]
                for path in sorted((ROOT/PREFIX).glob(Path(case['report']).name.replace('-acquisition.json','-comparison')+'*.json')):
                    c=reader.json(path.relative_to(ROOT))
                    if c.get('acquisition')==dict(record=case['report'],sha256=reader.digest(case['report'])):comparisons.append(dict(c,_record=path.relative_to(ROOT).as_posix(),_sha256=reader.digest(path.relative_to(ROOT))))
                if not same and plan.get('capacity_repair'):
                    review=reader.json(plan['capacity_repair']['record'])
                    allowed={p['record']:p['sha256']for p in review['retained_parent_proofs']}
                    parent=reader.json(review['parent_plan']['record'])
                    require(reader.digest(review['parent_plan']['record'])==review['parent_plan']['sha256'],'Changed retained source phase')
                    same=(a.get('player_binary',{}).get('sha256')==plan['player_binary']['sha256']and a.get('compiler_sources')==plan['compiler_sources']and all(a.get('sources',{}).get(n)==h for n,h in plan['runtime_sources'].items()if n not in review['allowed_post_retirement_controls'])and any(allowed.get(c.get('_record'))==c.get('_sha256')for c in comparisons))
                case['proved']=same and a['success']and any(c.get('kind')=='public_native_caller_state_comparison'and c['success']and c['sources_stable']and c['comparison']['mismatching_pixels']==0 and c['caller_state']['verified']for c in comparisons)
                case['comparison_failed']=any(not c['success']for c in comparisons)
        return old_admit(cases,repair,requests,games)
    base.admit=admit
    with Reader(ROOT)as reader:
        plan=reader.json(args.plan);plan_ref=dict(record=args.plan,sha256=reader.digest(args.plan))
    old_writer=base.write_report
    def writer(root,path,raw):
        if str(path)==args.report:
            result=json.loads(raw);result['prospective_plan']=plan_ref
            result['source_commit']=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()
            result['source_merge_parent']=subprocess.check_output(['git','rev-parse','MERGE_HEAD'],cwd=ROOT,text=True).strip()
            if result.get('session')or result.get('states'):result['original_game_runs']=1
            result['configuration'].update(caller_state_version=5,source_tail_version=1,source_only_observation=True,acquisition_focus='physical viewport centre; no picking or sprite-target proof')
            result['temporary_classification']='Capture files and source history are held in RAM during this bounded run and retained below in SQLite before closeout. Compiler caches/resettable environments are temporary; no automatic cleanup of pins, sources or unique observations.'
            if ram is not None:
                result['ram_capture_root']=str(ram);out=ROOT/'working/interaction-acceptance'/Path(args.report).stem
                existing={a['path']:a for a in result.get('artifacts',[])}
                for physical in sorted(ram.rglob('*')):
                    if not physical.is_file():continue
                    relative=physical.relative_to(ram);logical=(out/relative).relative_to(ROOT).as_posix()
                    require(physical.stat().st_size<=(2*1024**3 if relative.as_posix()=='worlds.bin' else 1024**3),'Bounded RAM capture exceeded')
                    data=physical.read_bytes();digest=hashlib.sha256(data).hexdigest()
                    if logical in existing:require(existing[logical]['sha256']==digest,'Different duplicate capture')
                    else:
                        record=args.report.removesuffix('.json')+'-raw-'+hashlib.sha256(logical.encode()).hexdigest()[:20]+'.bin'
                        result.setdefault('artifacts',[]).append(dict(path=logical,record=record,sha256=write_report(ROOT,record,data),bytes=len(data)))
            raw=(json.dumps(result,indent=2)+'\n').encode()
        return old_writer(root,path,raw)
    base.write_report=writer
    class CallerJourney(base.InteractionJourney):
        def set_scaling(self,mode):
            # This row needs actual state and pixels, not sprite picking. Avoid
            # decoding a growing live sprite corpus merely to focus the widget.
            wanted={'sharp':0,'integer':2}[mode];live=self.viewport.poll()
            if self.scaling_geometry is None:
                nodes=[v for _,v in self.access.nodes()if v.get('accessible_id','').endswith('.viewportScaling')]
                require(len(nodes)==1 and nodes[0]['enabled'],'Public scaling unavailable')
                origin=self.window_origin();r=nodes[0]['rect'];self.scaling_geometry=[r[0]-origin[0],r[1]-origin[1],r[2],r[3]]
            if not live or live['scaling']!=wanted:
                self.input.click(self.screen_geometry(self.scaling_geometry));self.key('scaling-'+mode,0xff50 if mode=='sharp'else 0xff57);self.key('scaling-confirm',0xff0d)
            if live:
                deadline=time.monotonic()+5
                while time.monotonic()<deadline and self.viewport.poll()['scaling']!=wanted:time.sleep(.05)
                require(self.viewport.poll()['scaling']==wanted,'Actual viewport scaling differs')
            self.actions.append(dict(action='scaling',mode=mode,host_ns=time.monotonic_ns(),control_rect=self.screen_geometry(self.scaling_geometry)))
            if self.session and self.viewport.poll():
                self.pointer([self.size[0]//2,self.size[1]//2])
                for down in(1,0):
                    require(self.input.xt.XTestFakeButtonEvent(self.input.display,1,down,0),'Physical viewport focus refused')
                    self.input.x.XFlush(self.input.display);time.sleep(.04)
                self.actions.append(dict(action='viewport-centre-focus',mode=mode,host_ns=time.monotonic_ns(),scope='Physical focus only; no picking/target proof'))
                self.wait_progress(int(self.viewport.poll()['world']),minimum=1)
        def settle(self,action,released,delay=.12):
            time.sleep(delay);self.wait_progress(int(released['world']),minimum=1);self.mark(action,'settled')
        def interaction_schedule(self):
            for mode in('sharp','integer'):
                self.set_scaling(mode)
                if self.latest()['mode']==1:self.held_key(mode+'-resume',0x20,.08,.1)
                self.held_key(mode+'-centre',ord('w'),.08,.1)
                self.move(mode+'-centre-pointer',[self.size[0]//2,self.size[1]//2])
                self.held_key(mode+'-pan',0xff52,.08,.1)
                self.wait_progress(int(self.viewport.poll()['world']),minimum=2)
    base.InteractionJourney=CallerJourney
    sys.argv=[sys.argv[0],'--report',args.report,*rest]
    # QFile::NewOnly must receive the real, absent RAM file. The logical
    # symlink is for stopped readers and retention, never the exclusive writer.
    return base.main(source_capture_path=ram/'worlds.bin')


if __name__=='__main__':raise SystemExit(main())
