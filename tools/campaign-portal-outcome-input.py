#!/usr/bin/env python3
"""Explicit selected combat policy with observed-portal decision adapter."""
import argparse
import importlib.util
import sys
import os
from pathlib import Path
from campaign_retained_policies import policy_script,ADAPTED_POLICIES
from campaign_portal_stage import PortalStage,is_portal_choice


def run(base,stage):
    original=base.combat_choice
    def choose(case,hero,target,*args,**kwargs):
        choice=original(case,hero,target,*args,**kwargs)
        if is_portal_choice(choice,case,hero,target):
            stage.approach(hero,target)
            return 'idle'
        return choice
    base.combat_choice=choose
    early=hasattr(base,'portal_approach_adapter');previous=getattr(base,'portal_approach_adapter',None)
    if early:base.portal_approach_adapter=stage.approach
    try:return base.main()
    finally:
        base.combat_choice=original
        if early:base.portal_approach_adapter=previous
        stage.close()


def main():
    root=Path(__file__).resolve().parents[1]
    selected=os.environ.get('MNM_CAMPAIGN_OUTCOME_POLICY','observed-v34-portal')
    if selected not in ADAPTED_POLICIES:raise ValueError('Explicit observed portal policy required')
    parent=ADAPTED_POLICIES[selected][2]
    path=root/'tools/campaign-outcome-input.py' if parent=='current' else policy_script(root,parent)
    # The selected module must not recursively redirect to this wrapper.
    os.environ['MNM_CAMPAIGN_OUTCOME_POLICY']='current'
    spec=importlib.util.spec_from_file_location('campaign_retained_v34',path)
    base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
    if '--help' in sys.argv[1:]:return base.main()
    p=argparse.ArgumentParser(add_help=False);p.add_argument('--experiment',type=Path,required=True)
    p.add_argument('--rect',type=int,nargs=4,required=True);p.add_argument('--frame-size',type=int,nargs=2,required=True)
    p.add_argument('--output',type=Path,required=True);a,_=p.parse_known_args()
    if not a.experiment.resolve().is_relative_to(root/'working') or not a.output.resolve().is_relative_to(root/'working'):
        raise ValueError('Portal adapter requires owned working paths')
    stage=PortalStage(a.experiment,a.rect,a.frame_size,a.output.with_name('portal-'+a.output.name),legacy_output=a.output)
    if '--resume-checkpoint' in sys.argv[1:]:
        if not os.environ.get('MNM_CAMPAIGN_ENTRY_SAVE') or os.environ.get('MNM_CAMPAIGN_CHECKPOINT'):
            raise ValueError('Loaded progression requires its explicit original input binding')
        # Compatibility guards in the immutable legacy Python driver only;
        # the Qt/original processes keep their real progression entry mode.
        os.environ['MNM_CAMPAIGN_CHECKPOINT']='restore';os.environ['MNM_CAMPAIGN_CHECKPOINT_PROBE']='combat'
    return run(base,stage)


if __name__=='__main__':raise SystemExit(main())
