#!/usr/bin/env python3
"""Run the unchanged required-boundary pixel/geometry checker in the owned namespace."""
import argparse,importlib.util,sys
from pathlib import Path
from coverage_evidence import Reader
from rendering_pilot import require
import required_boundary_observation as observation
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser(add_help=False);p.add_argument('--plan',required=True);args,remaining=p.parse_known_args()
 with Reader(ROOT)as r:
  plan=r.json(args.plan);require(plan['success']and plan['family']=='boundary','Unbound provider boundary comparison')
 observation.PREFIX=plan['collector_prefix'];filename=ROOT/'tools/check-required-boundary-session.py';spec=importlib.util.spec_from_file_location('broader_unchanged_boundary_comparison',filename);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);sys.argv=[str(filename),*remaining];return module.main()
if __name__=='__main__':raise SystemExit(main())
