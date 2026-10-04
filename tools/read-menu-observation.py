#!/usr/bin/env python3
"""Decode a bounded MNMMENU1 log; a trace is observation, not replacement evidence."""
import argparse
import importlib.util
import json
from pathlib import Path
REPO=Path(__file__).resolve().parents[1]
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('log',type=Path);args=parser.parse_args()
    if args.log.stat().st_size>16+256*64:raise ValueError('Observation exceeds record bound')
    spec=importlib.util.spec_from_file_location('menu_decoder',REPO/'tools/test-menu-observer.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    rows=module.decode(args.log.read_bytes())
    print(json.dumps({'scope':'Observation only','records':rows},indent=2))
