#!/usr/bin/env python3
"""Run every frozen picking comparison check with explicit ANI target replay."""
import argparse
from functools import partial
import importlib.util
from pathlib import Path
import sys
from broader_picking_animation_target import load_index, projected_targets
from coverage_evidence import Reader
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]

def main():
    p=argparse.ArgumentParser(add_help=False);p.add_argument('--acquisition',required=True)
    args,_=p.parse_known_args()
    with Reader(ROOT)as reader:
        a=reader.json(args.acquisition)
        phase=reader.json(a['broader_phase']['record'])
        require(reader.digest(a['broader_phase']['record'])==a['broader_phase']['sha256'],
                'Picking phase changed')
        reference=phase['picking_animation_review']
        require(a['provider_adaptation']['picking_animation_target']['review']==reference,
                'Acquired picking target policy differs')
        index=load_index(reader,reference,a['installation_inputs'])
        for name in ['tools/broader_picking_animation_target.py','tools/broader_picking_animation_launch.py',
                     'tools/run-broader-picking-animation-world.py','tools/check-broader-picking-animation.py']:
            require(a['sources'][name]==reader.digest(name),'Picking policy source changed')
    import rendering_picking_resize_assessment as assessment
    previous=assessment.targets
    assessment.targets=partial(projected_targets,index=index)
    try:
        path=ROOT/'tools/check-rendering-picking-closeout.py'
        spec=importlib.util.spec_from_file_location('animation_frozen_picking_check',path)
        core=importlib.util.module_from_spec(spec);spec.loader.exec_module(core)
        return core.main()
    finally:assessment.targets=previous

if __name__=='__main__':raise SystemExit(main())
