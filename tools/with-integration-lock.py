#!/usr/bin/env python3
"""Preview integration admission, or explicitly run a whole integration under it."""
import argparse
import json
from pathlib import Path
import sys
import subprocess
from integration_queue import integration_lock, status


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--execute', action='store_true')
    parser.add_argument('--expected-master')
    parser.add_argument('command', nargs=argparse.REMAINDER)
    args = parser.parse_args()
    if not args.execute:
        print(json.dumps(status(args.root), indent=2))
        return 0
    command = args.command[1:] if args.command[:1] == ['--'] else args.command
    if not command:
        parser.error('--execute requires an integration command after --')
    with integration_lock(args.root, expected_master=args.expected_master) as lease:
        print(json.dumps(dict(integration_admitted=True, wait_seconds=lease.wait_seconds)), flush=True)
        return lease.run(command, cwd=args.root).returncode


if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        print(str(error), file=sys.stderr)
        raise SystemExit(2)
