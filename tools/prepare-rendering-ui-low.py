#!/usr/bin/env python3
"""Retain the source800 controller and freeze a separate source640 repair."""
import argparse
from copy import deepcopy
import json
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_ui_acceptance import PREFIX
from rendering_ui_low import CONTROL
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);args=p.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(),'Fresh prospective source640 plan required')
    with Reader(ROOT) as r:
        parent=PREFIX+'input-plan-08.json';q=deepcopy(r.json(parent))
        require(all(r.digest(n)==h for n,h in (q['sources']|q['runtime_sources']|q['compiler_sources']).items() if n not in CONTROL),'Preserved800/native/player source drift')
        require(r.json(PREFIX+'default640-10-acquisition.json')['error']=='Original ready menu deadline: (6, None)','Unreviewed Low failure')
        q['sources'].update({n:r.digest(n) for n in CONTROL})
        q['bounds'].update(maximum_requests=13,maximum_original_games=11,repair_games=7)
        q['supersedes']=dict(record=parent,sha256=r.digest(parent),reason='Retain all ten requests/eight game starts. Low10 completed the HUD, tooltip, Space and Mini checks but the final Quit motion/click preceded the viewport input handoff: retained input stayed at40,100. The Low-only adapter now waits for GUI ownership, public focus and read-only active input, then observes fresh target motion before the unchanged physical click. Preserve all800/native/compiler bytes. One additional repair revises cumulative ceilings12->13requests and10->11game starts. Three required accepted cases remain; no further repair starts reserved.')
        q['source640_variant']=dict(controller='tools/run-rendering-ui-low.py',native_changes=False,source800_controller_unchanged=True,failure=dict(record=PREFIX+'default640-10-acquisition.json',sha256=r.digest(PREFIX+'default640-10-acquisition.json')))
        print(write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode()))


if __name__=='__main__':main()
