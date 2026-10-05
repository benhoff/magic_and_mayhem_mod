#!/usr/bin/env python3
"""Install the repository-local coverage hook, preserving existing hook setups."""
import argparse
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def install(root, uninstall=False):
    def git(*args):
        return subprocess.run(['git', '-C', str(root), *args], capture_output=True, text=True)
    value = git('config', '--local', '--get', 'core.hooksPath')
    effective = git('config', '--get', 'core.hooksPath')
    if uninstall:
        if value.returncode or value.stdout.strip() != '.githooks':
            raise ValueError('Coverage hook is not installed locally; nothing changed')
        result = git('config', '--local', '--unset', 'core.hooksPath')
    else:
        if effective.returncode == 0 and effective.stdout.strip() != '.githooks':
            raise ValueError('Existing core.hooksPath must be integrated manually; nothing changed')
        hook = root/'.githooks/pre-commit'
        if not hook.is_file() or not hook.stat().st_mode & 0o111:
            raise ValueError('Missing executable .githooks/pre-commit')
        hooks = git('rev-parse', '--git-path', 'hooks')
        if hooks.returncode:
            raise ValueError(hooks.stderr.strip())
        directory = Path(hooks.stdout.strip())
        if not directory.is_absolute():
            directory = root/directory
        if effective.returncode != 0 and directory.exists():
            active = [p.name for p in directory.iterdir() if p.is_file() and
                      not p.name.endswith('.sample') and p.stat().st_mode & 0o111]
            if active:
                raise ValueError('Existing active hooks must be integrated manually: ' + ', '.join(sorted(active)))
        result = git('config', '--local', 'core.hooksPath', '.githooks')
    if result.returncode:
        raise OSError(result.stderr.strip())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--uninstall', action='store_true', help='Remove only our local hooksPath setting')
    args = parser.parse_args()
    try:
        install(ROOT, args.uninstall)
    except (ValueError, OSError) as exc:
        parser.exit(1, str(exc) + '\n')
    print('Coverage pre-commit hook ' + ('disabled' if args.uninstall else 'enabled') + ' for this repository.')


if __name__ == '__main__':
    main()
