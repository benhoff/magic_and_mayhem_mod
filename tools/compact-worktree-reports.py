#!/usr/bin/env python3
"""Plan/apply verified replacement of expanded reports by their Git pointers."""
import argparse,json
from pathlib import Path
import worktree_report_compaction as compaction

ROOT=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--main',type=Path,required=True)
parser.add_argument('--own',type=Path,default=ROOT)
parser.add_argument('--restore-directory',type=Path,required=True)
commands=parser.add_subparsers(dest='action',required=True)
p=commands.add_parser('plan');p.add_argument('root',type=Path);p.add_argument('--output',type=Path,required=True);p.add_argument('--reason',required=True)
p.add_argument('--retain-missing-copies',action='store_true',help='Explicitly retain exact Git-bound reports in the configured independent pair when two existing copies are unavailable')
p=commands.add_parser('apply');p.add_argument('plan',type=Path);p.add_argument('--expected-plan-sha256',required=True)
args=parser.parse_args()
if args.action=='plan':result=compaction.prepare(args.root,args.main,args.own,args.output,args.reason,args.restore_directory,args.retain_missing_copies)
else:result=compaction.apply(args.plan,args.expected_plan_sha256,args.main,args.own,args.restore_directory)
print(json.dumps(result,indent=2))
