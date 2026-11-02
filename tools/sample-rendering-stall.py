#!/usr/bin/env python3
"""Explicit diagnostic launch of the real Qt ELF under a parent debugger."""
import argparse
import os
from pathlib import Path
from rendering_stall_sampler import OwnedRing, run


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--repo', type=Path, required=True)
    parser.add_argument('--owner', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('arguments', nargs=argparse.REMAINDER)
    args = parser.parse_args()
    if any(os.environ.get(key) != '1' for key in
           ('MNM_ACCEPTANCE_PILOT', 'MNM_ACCEPTANCE_NORMAL', 'MNM_ACCEPTANCE_DIAGNOSTIC')):
        parser.error('Stall sampling requires an explicitly diagnostic normal collector')
    root = args.repo.resolve()
    if args.output.resolve() != args.output or not args.output.is_relative_to(root/'working/pilot'):
        parser.error('Sampler output must be a physical owned pilot path')
    arguments = args.arguments[1:] if args.arguments[:1] == ['--'] else args.arguments
    return run(args.binary, arguments, OwnedRing(root, args.owner), args.output,
               storage_main=os.environ.get('MNM_ACCEPTANCE_SAMPLE_STORAGE_MAIN')=='1')


if __name__ == '__main__':
    raise SystemExit(main())
