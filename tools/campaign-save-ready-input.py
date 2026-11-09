#!/usr/bin/env python3
"""Run retained800 input with current guarded hover proposals, then hand off to Save."""
import argparse,hashlib,importlib.util,json,os,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
from campaign_gameplay import read_rows,creatures
from campaign_portal_stage import PortalStage
from campaign_report import report_callback
from campaign_transition_checkpoint import retired_goal
from rendering_progression import require
PINNED={'campaign-outcome-input-v56.py':'4e0987b7443ed02d7419632ebc44a357a62c9248de3513df3c571579ba2235e5',
        'campaign_wizard_cast_v56.py':'a55090f16c2d62207c5d1c9272e434ac3ae7a6a470ee6912c8ebcbadffe0da95'}


class SaveReady(BaseException):pass


def main():
    p=argparse.ArgumentParser();p.add_argument('--case',choices=['victory'],required=True);p.add_argument('--stop-at-rearm',action='store_true');p.add_argument('--experiment',type=Path,required=True);p.add_argument('--seconds',type=int,required=True);p.add_argument('--rect',type=int,nargs=4,required=True);p.add_argument('--frame-size',type=int,nargs=2,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    require(os.environ.get('MNM_CAMPAIGN_CHECKPOINT_ROUTE')=='transition-ready' and os.environ.get('MNM_CAMPAIGN_CHECKPOINT')=='capture','Only explicit transition Save acquisition is allowed')
    require(a.experiment.resolve().is_relative_to(ROOT/'working') and a.output.resolve().is_relative_to(ROOT/'working'),'Owned input/output required')
    for name,h in PINNED.items():require(hashlib.sha256((ROOT/'tools'/name).read_bytes()).hexdigest()==h,'Retained input fixture differs')
    seed=json.loads(a.output.read_text()).get('seed_reference');require(seed and seed.get('save_name')==os.environ.get('MNM_CAMPAIGN_CHECKPOINT_SEED') and seed.get('save_sha256')==os.environ.get('MNM_CAMPAIGN_CHECKPOINT_SEED_SHA256'),'Actual original seed input required')
    def module(name,path):
        spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
    wizard=module('campaign_wizard_cast',ROOT/'tools/campaign_wizard_cast_v56.py');sys.modules['campaign_wizard_cast']=wizard
    base=module('retained_campaign_input_v56',ROOT/'tools/campaign-outcome-input-v56.py');original=base.retired_boss_return_candidate;stopped={}
    def candidate(actor,already=False):
        if original(actor,already):
            goal=retired_goal(actor);rows=read_rows(a.experiment/'gameplay-events.bin');actors=creatures(rows)
            heroes=[v for v in actors.values() if v['type']==0 and v['owner']==0 and v['active'] and v['health']>0]
            require(len(heroes)==1,'Save handoff needs a living sole Wizard')
            stopped.update(retired_boss=goal,wizard=heroes[0]);raise SaveReady()
        return False
    base.retired_boss_return_candidate=candidate
    # This is the actual seeded original Load. Compatibility flags affect only
    # this retained Python input process; the owning Qt flow remains capture.
    os.environ.update(MNM_CAMPAIGN_ENTRY_SAVE=seed['save_name'],MNM_CAMPAIGN_ENTRY_SAVE_SHA256=seed['save_sha256'],MNM_CAMPAIGN_CHECKPOINT='restore',MNM_CAMPAIGN_CHECKPOINT_PROBE='combat',MNM_CAMPAIGN_OUTCOME_POLICY='current')
    parent=a.output.with_name('checkpoint-driver-parent.json');args=sys.argv[:]
    current_probe=module('current_wizard_probe',ROOT/'tools/campaign_wizard_cast.py')
    def proposals(hero,boss,size):
        snapshot=json.loads(parent.read_text()) if parent.exists() else {}
        finishing=snapshot.get('finishing_pause',{})
        previous=snapshot.get('combat_pause',{}).get('actual_boss_point') if finishing and not finishing.get('released') else None
        return current_probe.probe_points(hero,boss,size,previous)
    base.wizard_probe_points=proposals
    sys.argv=[str(ROOT/'tools/campaign-outcome-input-v56.py'),'--case','victory','--resume-checkpoint','--experiment',str(a.experiment),'--seconds',str(a.seconds),'--rect',*map(str,a.rect),'--frame-size',*map(str,a.frame_size),'--output',str(parent)]
    try:
        try:base.main()
        except SaveReady:pass
        else:raise RuntimeError('Retained input ended before an original Boss retirement')
    finally:sys.argv=args;base.retired_boss_return_candidate=original
    raw=(a.experiment/'events.bin').read_bytes();import struct
    events=list(struct.iter_unpack('<16I',raw[16:16+(len(raw)-16)//64*64]));require(report_callback(events,events[0][2] if events else None) is None,'Checkpoint cannot substitute a primary outcome')
    pause=PortalStage(a.experiment,a.rect,a.frame_size,a.output.with_name('checkpoint-save-handoff-input.json'))
    try:pause.open();pause.key(0x20)
    finally:pause.close()
    result=dict(schema=1,kind='original_transition_ready_save_input',success=True,checkpoint_ready=True,checkpoint_domain='transition-ready',seed_reference=seed,retired_boss=stopped['retired_boss'],wizard=stopped['wizard'],ordinary_space_handed_to_save_ui=True,simulation_writes=False,semantic_outcome_accepted=False,source_fixtures=PINNED,adapted_probe_source=dict(path='tools/campaign_wizard_cast.py',sha256=hashlib.sha256((ROOT/'tools/campaign_wizard_cast.py').read_bytes()).hexdigest()),parent_input=str(parent),scope='Observed original retired Boss and living Wizard only. Stop ordinary input before any natural report; original Save/Load and all later map progression remain separate.')
    a.output.write_text(json.dumps(result)+'\n');return 0
if __name__=='__main__':raise SystemExit(main())
