#!/usr/bin/env python3
"""Preserve immutable evidence in two stores and replace one idle copy with an alias."""
from pathlib import Path
import argparse,json
from evidence_consolidation import consolidate,restore

def main():
    p=argparse.ArgumentParser(description=__doc__)
    sub=p.add_subparsers(dest="command",required=True)
    c=sub.add_parser("consolidate")
    for key in ("source","local","cold","local-catalog","cold-catalog","scratch"):
        c.add_argument("--"+key,type=Path,required=True)
    r=sub.add_parser("restore")
    for key in ("catalog","source","cold","output"):r.add_argument("--"+key,type=Path,required=True)
    a=p.parse_args()
    if a.command=="restore":print(restore(a.catalog,a.source,a.output,a.cold))
    else:
        if a.local.parent.stat().st_dev==a.cold.parent.stat().st_dev:p.error("Independent filesystems required")
        print(json.dumps(consolidate(a.source,a.local,a.cold,[a.local_catalog,a.cold_catalog],a.scratch),sort_keys=True))
if __name__=="__main__":main()
