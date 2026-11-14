#!/usr/bin/env python3
"""Plan or execute explicit hybrid custody for an inactive evidence mirror."""
import argparse,json
from pathlib import Path
from evidence_mirror_compaction import prepare,apply

def main():
    p=argparse.ArgumentParser(description=__doc__);sub=p.add_subparsers(dest='command',required=True)
    a=sub.add_parser('plan');a.add_argument('--primary',type=Path,required=True);a.add_argument('--replica',type=Path,required=True);a.add_argument('--output',type=Path,required=True)
    a=sub.add_parser('apply');a.add_argument('plan',type=Path);a.add_argument('--expected-plan-sha256',required=True);a.add_argument('--primary-minimum-free-gib',type=float,default=100);a.add_argument('--replica-minimum-free-gib',type=float,default=90)
    args=p.parse_args()
    result=prepare(args.primary,args.replica,args.output)if args.command=='plan'else apply(args.plan,args.expected_plan_sha256,minimum_free=(args.primary_minimum_free_gib*2**30,args.replica_minimum_free_gib*2**30))
    print(json.dumps(result,indent=2))

if __name__=='__main__':main()
