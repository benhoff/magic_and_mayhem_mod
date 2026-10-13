#!/usr/bin/env python3
"""Inventory working storage and guard runs against the free-space reserve.

Inventory is non-destructive. Archival verifies cold copies before optional pruning.
"""
import argparse
from collections import defaultdict
from datetime import datetime, timezone
import json
import math
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[1]
GIB = 1024 ** 3


def policy(path):
    value = json.loads(path.read_text())
    if value.get('schema') != 1:
        raise ValueError('Unsupported retention policy')
    for key in ('minimum_free_gib', 'recent_hours', 'keep_latest_runs'):
        if type(value.get(key)) not in (int, float) or not math.isfinite(value[key]) or value[key] < 0:
            raise ValueError('Invalid policy value: ' + key)
    maximum = value.get('max_working_gib')
    if maximum is not None and (type(maximum) not in (int, float) or not math.isfinite(maximum) or maximum <= 0):
        raise ValueError('max_working_gib must be null or a positive finite number')
    if type(value['keep_latest_runs']) is not int:
        raise ValueError('Require an integral retained-run count')
    if not isinstance(value.get('protected_names'), list) or any(
            not isinstance(n, str) or not n or '/' in n or n in ('.', '..')
            for n in value['protected_names']):
        raise ValueError('Invalid protected directory names')
    if value.get('auto_archive', False) is not False and not isinstance(value.get('auto_archive'), bool):
        raise ValueError('auto_archive must be boolean')
    if 'archive_root' in value and not Path(value['archive_root']).is_absolute():
        raise ValueError('archive_root must be absolute')
    for key in ('minimum_archive_free_gib', 'archive_quiet_minutes', 'archive_minimum_file_mib',
                'monitor_seconds', 'stop_grace_seconds'):
        if key in value and (type(value[key]) not in (int, float) or not math.isfinite(value[key]) or value[key] <= 0):
            raise ValueError('Invalid policy value: ' + key)
    return value


def usage(working, depth=None):
    if working.is_symlink() or not working.is_dir():
        raise ValueError('working must be a real directory')
    command = ['du', '-x', '-B1', '--null']
    command += ['-s'] if depth is None else ['--max-depth=' + str(depth)]
    result = subprocess.run(command + ['--', str(working)], capture_output=True,
                            env=dict(os.environ, LC_ALL='C'))
    if result.returncode:
        # Active captures unlink replies while du traverses them. Accept only
        # those disappeared descendants; permission, I/O and all other failures
        # still refuse measurement. du must also provide its final root total.
        diagnostics = result.stderr.decode(errors='replace').splitlines()
        vanished = [re.fullmatch(r"du: cannot access '(.+)': No such file or directory", line)
                    for line in diagnostics]
        if result.returncode != 1 or not vanished or any(
                match is None or working not in Path(match[1]).parents for match in vanished):
            raise subprocess.CalledProcessError(result.returncode, result.args,
                                                output=result.stdout, stderr=result.stderr)
        print('Storage inventory: temporary descendants disappeared during scan.', file=sys.stderr)
    sizes = {os.fsdecode(p): int(n) for row in result.stdout.split(b'\0') if row
             for n, p in [row.split(b'\t', 1)]}
    if str(working) not in sizes:
        raise ValueError('Storage inventory did not produce a root total')
    return sizes


def pressure(total, free, rules, reserve=0):
    reasons = []
    maximum = rules.get('max_working_gib')
    if maximum is not None and total + reserve > maximum * GIB:
        reasons.append(f"working usage plus reservation exceeds the {maximum:g} GiB policy")
    if free - reserve < rules['minimum_free_gib'] * GIB:
        reasons.append('reservation would leave less than the minimum free disk space')
    return reasons


def references(root):
    """Conservative literal references in tracked research and fixture metadata.

    These are retention hints, not proof that unmentioned files are disposable.
    Source-code output-directory literals are not dependency declarations.
    """
    paths = subprocess.check_output(['git', '-C', str(root), 'ls-files', '-z'])
    result = defaultdict(set)
    pattern = re.compile(r'(?:' + re.escape(str(root)) + r'/)?working/[^\s\"\'`<>\\]+')
    for raw in paths.split(b'\0'):
        if not raw:
            continue
        name = os.fsdecode(raw)
        if not name.startswith(('research/', 'tests/fixtures/')) or Path(name).suffix not in ('.json', '.md', '.tsv', '.txt'):
            continue
        path = root / name
        if path.is_symlink():
            raise ValueError('Tracked metadata symlink requires manual review: ' + name)
        for match in pattern.finditer(path.read_text(errors='replace')):
            token = match.group().removeprefix(str(root) + '/').rstrip('.,;:)]}')
            parts = Path(token).parts
            if '..' not in parts and len(parts) > 1:
                result[Path(*parts).as_posix()].add(name)
    return result


def worktrees(root):
    output = subprocess.check_output(['git', '-C', str(root), 'worktree', 'list', '--porcelain', '-z'])
    return [Path(os.fsdecode(row[9:])) for row in output.split(b'\0') if row.startswith(b'worktree ')]


def overlap(a, b):
    return a == b or a in b.parents or b in a.parents


def units(working):
    """Nonoverlapping review units; never follow symlinks into other trees."""
    for child in sorted(working.iterdir()):
        if child.is_symlink() or not child.is_dir():
            continue
        if child.name == 'build':
            for item in sorted(child.iterdir()):
                if item.is_dir() and not item.is_symlink():
                    yield item, 'build', None
        elif child.name in ('tests', 'experiments'):
            for group in sorted(child.iterdir()):
                if group.is_symlink() or not group.is_dir():
                    continue
                runs = [p for p in group.iterdir() if p.name.startswith('run-') and p.is_dir() and not p.is_symlink()]
                if runs:
                    for run in sorted(runs):
                        yield run, child.name, group
                else:
                    yield group, 'unclassified', None
        else:
            yield child, 'unclassified', None


def inventory(root, rules, now=None):
    now = time.time() if now is None else now
    working = root / 'working'
    sizes = usage(working, 3)
    refs = references(root)
    descendants = defaultdict(set)
    for target, sources in refs.items():
        p = Path(target)
        for ancestor in (p, *p.parents):
            descendants[ancestor.as_posix()].update(sources)
    trees = worktrees(root)
    items = list(units(working))
    newest = set()
    groups = defaultdict(list)
    for path, kind, group in items:
        if group:
            groups[group].append(path)
    for paths in groups.values():
        newest.update(sorted(paths, key=lambda p: (p.stat().st_mtime_ns, str(p)), reverse=True)[:rules['keep_latest_runs']])
    rows = []
    for path, kind, group in items:
        relative = path.relative_to(root).as_posix()
        reasons = []
        if any(overlap(path, tree) for tree in trees if tree != root):
            reasons.append('registered Git worktree')
        if path.relative_to(working).parts[0] in rules['protected_names']:
            reasons.append('policy exemption')
        if (path / '.retain').exists() or (group and (group / '.retain').exists()):
            reasons.append('.retain marker')
        linked_sources = set(descendants.get(relative, ()))
        for ancestor in Path(relative).parents:
            linked_sources.update(refs.get(ancestor.as_posix(), ()))
        linked = sorted(linked_sources)
        if linked:
            reasons.append('tracked evidence/research reference')
        if path in newest:
            reasons.append('one of the newest runs in its group')
        # Shallow times are deliberately not represented as a process/liveness check.
        latest = max([path.stat().st_mtime] + [p.lstat().st_mtime for p in path.iterdir()])
        if now - latest < rules['recent_hours'] * 3600:
            reasons.append('recent immediate-directory contents')
        if kind == 'unclassified':
            reasons.append('unclassified: explicit review needed')
        rows.append({'path': relative, 'kind': kind, 'allocated_bytes': sizes.get(str(path), 0),
                     'classification': 'retain' if reasons else 'review_for_cleanup',
                     'reasons': reasons, 'reference_documents': linked,
                     'immediate_latest_mtime': latest})
    rows.sort(key=lambda r: (-r['allocated_bytes'], r['path']))
    total = sizes[str(working)]
    free = shutil.disk_usage(working).free
    candidates = sum(r['allocated_bytes'] for r in rows if r['classification'] == 'review_for_cleanup')
    return {'schema': 1, 'created_utc': datetime.fromtimestamp(now, timezone.utc).isoformat(),
            'root': str(root), 'policy': rules, 'working_allocated_bytes': total,
            'filesystem_free_bytes': free,
            'over_budget_bytes': (max(0, total - rules['max_working_gib'] * GIB)
                                  if rules.get('max_working_gib') is not None else None),
            'free_space_shortfall_bytes': max(0, rules['minimum_free_gib'] * GIB - free),
            'cleanup_review_bytes': candidates, 'projected_bytes_if_all_review_candidates_removed': total - candidates,
            'pressure': pressure(total, free, rules), 'units': rows,
            'limitations': ['Dry run only. Review candidates are not authorized or proven safe deletions.',
                           'Literal current-tree metadata references do not include all historical branches or dynamic inputs.',
                           'Recent shallow timestamps and newest-run retention do not prove a run is inactive.',
                           'Registered worktrees are protected as whole trees; inspect their ignored outputs separately.',
                           'du is a non-atomic allocated-block snapshot on one filesystem, without following symlinks.',
                           'The run command monitors filesystem free space for its own child process group; it is not a filesystem quota.']}


def stop_group(child, grace):
    """Stop only the isolated process group created by this guarded launch."""
    def send(sig):
        try:
            os.killpg(child.pid, sig)
        except ProcessLookupError:
            pass

    send(signal.SIGTERM)
    try:
        child.wait(timeout=grace)
    except subprocess.TimeoutExpired:
        pass
    # Also stop any descendants still running after their leader has exited.
    send(signal.SIGKILL)
    child.wait()


def guarded_run(command, root, rules):
    child = subprocess.Popen(command, cwd=root, start_new_session=True)
    try:
        while child.poll() is None:
            free = shutil.disk_usage(root / 'working').free
            if free < rules['minimum_free_gib'] * GIB:
                print(f"Stopping guarded command: {free / GIB:.2f} GiB free is below "
                      f"the {rules['minimum_free_gib']:g} GiB reserve.", file=sys.stderr, flush=True)
                stop_group(child, rules.get('stop_grace_seconds', 5))
                return 2
            try:
                return child.wait(timeout=rules.get('monitor_seconds', 2))
            except subprocess.TimeoutExpired:
                continue
        return child.returncode
    except KeyboardInterrupt:
        stop_group(child, rules.get('stop_grace_seconds', 5))
        return 130
    except OSError:
        stop_group(child, rules.get('stop_grace_seconds', 5))
        raise


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--policy', type=Path, default=ROOT / 'tools/working-retention.json')
    sub = parser.add_subparsers(dest='action', required=True)
    plan = sub.add_parser('inventory', help='Write a new dry-run JSON report')
    plan.add_argument('--output', type=Path, required=True)
    check = sub.add_parser('check', help='Refuse reservations that would violate the free-space policy')
    check.add_argument('--reserve-gib', type=float, default=0)
    run = sub.add_parser('run', help='Preflight a command and monitor free disk space until it exits')
    run.add_argument('--reserve-gib', type=float, required=True)
    run.add_argument('command', nargs=argparse.REMAINDER)
    archive = sub.add_parser('archive', help='Verify completed-run payload copies on the second NVMe')
    archive.add_argument('--prune', action='store_true', help='Remove payloads only after verified durable archive receipts')
    archive.add_argument('--include-worktree-output', action='store_true', help='Also consider completed ignored runs in registered worktrees; preserve source trees')
    archive.add_argument('--max-files', type=int, help='Bound an initial archive experiment')
    restore = sub.add_parser('restore', help='Restore an archived payload to its exact original path')
    restore.add_argument('path', help='Repository-relative working path from the archive catalog')
    args = parser.parse_args()
    try:
        rules = policy(args.policy)
        if args.action in ('archive', 'restore'):
            from working_archive import archive_lock, maintain, restore as restore_file
            if args.action == 'restore':
                with archive_lock(Path(rules['archive_root'])):
                    restore_file(ROOT, Path(rules['archive_root']), args.path)
                print('Restored: ' + args.path)
                return 0
            if args.max_files is not None and args.max_files <= 0:
                raise ValueError('--max-files must be positive')
            result = maintain(ROOT, rules, args.prune, args.include_worktree_output, args.max_files)
            return 1 if result['failures'] else 0
        if args.action == 'inventory':
            if args.output.exists():
                raise ValueError('Refusing to overwrite an inventory')
            report = inventory(ROOT, rules)
            args.output.parent.mkdir(parents=True, exist_ok=True)
            with args.output.open('x') as out:
                json.dump(report, out, indent=2)
                out.write('\n')
            print(json.dumps({k: v for k, v in report.items() if k not in ('units', 'limitations', 'policy')}, indent=2))
            return 0
        if not 0 <= args.reserve_gib < float('inf'):
            raise ValueError('Reservation must be finite and nonnegative')
        command = args.command if args.action == 'run' else []
        if command[:1] == ['--']:
            command = command[1:]
        if args.action == 'run' and not command:
            raise ValueError('Supply a command after --')
        working = ROOT / 'working'
        # Free-space-only launches need no expensive, non-atomic working-tree
        # scan. Inventory remains available for allocated-size accounting.
        total = usage(working)[str(working)] if rules.get('max_working_gib') is not None else 0
        free = shutil.disk_usage(working).free
        reasons = pressure(total, free, rules, args.reserve_gib * GIB)
        if reasons and args.action == 'run' and rules.get('auto_archive', False):
            from working_archive import maintain
            # Reserve room for this particular launch, then reassess actual usage.
            archive_rules = dict(rules, minimum_free_gib=rules['minimum_free_gib'] + args.reserve_gib)
            if rules.get('max_working_gib') is not None:
                archive_rules['max_working_gib'] = max(0, rules['max_working_gib'] - args.reserve_gib)
            maintain(ROOT, archive_rules, prune=True)
            total = usage(working)[str(working)]
            free = shutil.disk_usage(working).free
            reasons = pressure(total, free, rules, args.reserve_gib * GIB)
        print(json.dumps({'working_gib': total / GIB if rules.get('max_working_gib') is not None else None,
                          'free_gib': free / GIB, 'minimum_free_gib': rules['minimum_free_gib'],
                          'reserved_gib': args.reserve_gib, 'refusal_reasons': reasons}), flush=True)
        if reasons:
            return 2
        return guarded_run(command, ROOT, rules) if command else 0
    except (ValueError, OSError, subprocess.CalledProcessError) as exc:
        parser.exit(1, str(exc) + '\n')


if __name__ == '__main__':
    sys.exit(main())
