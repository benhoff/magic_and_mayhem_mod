#!/usr/bin/env python3
"""Review, back up, stage and commit one owned batch without publication."""
import argparse
import json
import os
import re
from pathlib import Path
import subprocess
import sys

from coverage_bookkeeping import Workspace, finish
from integration_queue import finish_lock


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--db', type=Path)
    parser.add_argument('--base', default='HEAD')
    parser.add_argument('--reason', required=True)
    parser.add_argument('--validation-reason', required=True)
    parser.add_argument('--index-review-reason')
    parser.add_argument('--record-current-promotions', action='store_true',
                        help='Record only declared promoted stages with current complete-source execution proof')
    parser.add_argument('--message', required=True)
    parser.add_argument('--handoff-completed-workspaces',action='store_true',help='Explicit storage operation: seal and retire queued owned execution trees')
    parser.add_argument('--result-token',help='Invocation nonce for the checked candidate result on stdout')
    parser.add_argument('--timings', action='store_true', help='Print finish phase durations and return timings_seconds')
    cache=parser.add_mutually_exclusive_group()
    cache.add_argument('--evict-review-cache',action='store_true',help='Explicit storage operation: verify a real restore before evicting the cache')
    cache.add_argument('--keep-review-cache',action='store_true',help='Keep a materialized cache after shared-store completion')
    args = parser.parse_args()
    if args.result_token is not None and not re.fullmatch(r'[0-9a-f]{32}',args.result_token):
        raise ValueError('Invalid integration result token')
    if args.timings:
        os.environ['MM_COVERAGE_TIMINGS'] = '1'
    branch=subprocess.check_output(['git','-C',str(args.root),'symbolic-ref','--quiet','--short','HEAD'],text=True).strip()
    if branch=='master':raise ValueError('Finish in an owned task branch; promote the checked candidate separately')
    from integration_queue import FD_VARIABLE,_current
    if os.environ.get(FD_VARIABLE) or _current.get()is not None:
        raise ValueError('Finish validation must run outside promotion admission')
    with finish_lock(args.root):
        return run(args)


def run(args):
    from evidence_objects import settings
    handoffs=[]
    if settings(args.root) and args.handoff_completed_workspaces:
        from evidence_workspace import finish_completed
        handoffs=finish_completed(args.root)
    workspace = Workspace(args.root, args.db)
    try:
        result = finish(workspace, args.base, args.reason, args.validation_reason,
                        args.message, args.index_review_reason, args.record_current_promotions, args.timings)
    finally:
        workspace.close()
    result['workspace_handoffs']=handoffs
    if settings(args.root) and args.evict_review_cache and not args.keep_review_cache and workspace.path == args.root.resolve()/'working/coverage/bookkeeping.sqlite3':
        from evidence_cache import evict
        result['review_cache']=evict(args.root,workspace.path)
    print(json.dumps(result, indent=2))
    if args.result_token:
        print('MM_FINISH_CHECKED',args.result_token,result['commit'],result['checked_tree'],result['base'],flush=True)


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, subprocess.CalledProcessError) as exc:
        print(str(exc), file=sys.stderr)
        sys.exit(1)
