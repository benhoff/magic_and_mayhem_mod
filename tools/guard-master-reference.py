#!/usr/bin/env python3
"""Require integration admission before Git moves the local master reference."""
from pathlib import Path
import subprocess
import sys
from integration_queue import require_lease


def main():
    if sys.argv[1:] != ['prepared']:
        return 0
    records = [line.split() for line in sys.stdin]
    if any(len(row) == 3 and row[2] == 'refs/heads/master' and row[0] != row[1]
           for row in records):
        root = Path(subprocess.check_output(['git', 'rev-parse', '--show-toplevel'], text=True).strip())
        require_lease(root)
    return 0


if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        print(str(error), file=sys.stderr)
        raise SystemExit(1)
