#!/usr/bin/env python3
"""Preserve stopped execution trees in two stores, then retire or restore copies."""
import argparse
import json
from pathlib import Path
from evidence_objects import configured
from evidence_workspace import seal,restore

ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--root',type=Path,default=ROOT)
    sub=p.add_subparsers(dest='command',required=True)
    s=sub.add_parser('seal');s.add_argument('path',type=Path);s.add_argument('--retire',action='store_true');s.add_argument('--restore-budget-gib',type=float,default=16)
    r=sub.add_parser('restore');r.add_argument('path',type=Path)
    a=p.parse_args()
    path=a.path if a.path.is_absolute() else a.root/a.path
    with configured(a.root) as pair:
        result=seal(a.root,path,pair,retire=a.retire,restore_budget=a.restore_budget_gib*1024**3) if a.command=='seal' else dict(success=True,path=str(restore(a.root,path,pair)))
    print(json.dumps(result,indent=2))


if __name__=='__main__':main()
