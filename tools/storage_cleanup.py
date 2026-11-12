"""Dry-first working-data cleanup, using the existing verified workspace handoff.

Inventory is a review aid. Only explicitly selected, completed, unpinned execution
directories can enter a hash-bound plan. No evidence garbage collection, database
compaction, source-checkout removal, pin override or unarchived deletion exists.
"""
from collections import Counter, defaultdict
from contextlib import closing
import fcntl
from functools import lru_cache
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import sqlite3
import stat
import subprocess
import tempfile
import time

from evidence_objects import configured, settings, storage_policy
from evidence_workspace import MARKER, idle, safe, scan, seal

GIB = 1024 ** 3
TOOLS = Path(__file__).resolve().parent
SOURCE_NAMES = ('cleanup-working.py', 'storage_cleanup.py', 'evidence_workspace.py',
                'evidence_objects.py', 'evidence_blocks.py', 'coverage_evidence.py',
                'experiment_environment.py', 'working-retention.py')
PROTECTED = {'original', '.git', 'coverage', 'retention', 'evidence-shared',
             'environment-pool', 'fixtures', 'check-inputs', 'user-saves',
             'game-clean', 'game-nocd', 'source-disc', 'wineprefix', 'wineprefix-x86_64',
             'toolchain', 'manifests', 'runtime', 'logs'}
SOURCE_SUFFIXES = {'.py', '.sh', '.c', '.cpp', '.h', '.hpp', '.S', '.patch', '.diff'}
LIMITATIONS = [
    'Inventory and timestamps are not proof of completion or inactivity.',
    'Literal tracked metadata references are hints; SQLite report payloads, other branches, '
    'dynamic consumers and historical restore dependencies require the recorded dependency review.',
    'Host /proc observation is best effort; unrelated writers must remain stopped during handoff.',
    'Held data is never automatically reclassified. Existing pins require their own reviewed handoff.',
    'Allocated-byte totals are upper bounds before hardlink/dedup/shared-store costs; '
    'filesystem free-space change is measured separately.',
]


def encoded(value):
    return json.dumps(value, sort_keys=True, separators=(',', ':'), allow_nan=False).encode()


def digest(value):
    return hashlib.sha256(encoded(value)).hexdigest()


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args], stderr=subprocess.PIPE)


@lru_cache(maxsize=1)
def retention_module():
    spec = importlib.util.spec_from_file_location('cleanup_retention', TOOLS / 'working-retention.py')
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def rules_for(root):
    value = retention_module().policy(root / 'tools/working-retention.json')
    # The coordinator never weakens the repository's prospective retention floors.
    value['recent_hours'] = max(24, value['recent_hours'])
    value['keep_latest_runs'] = max(2, value['keep_latest_runs'])
    value['archive_quiet_minutes'] = max(60, value.get('archive_quiet_minutes', 60))
    return value


def checkout_roots(root, include_worktrees):
    trees = retention_module().worktrees(root)
    return list(dict.fromkeys([root, *trees])) if include_worktrees else [root]


class ReferenceHints(dict):
    """Index descendants once, rather than comparing every reference per unit."""
    def __init__(self, values):
        super().__init__(values)
        self.descendants = defaultdict(set)
        for name, sources in self.items():
            p = Path(name)
            for ancestor in (p, *p.parents):
                self.descendants[ancestor.as_posix()].update(sources)

    def overlapping(self, relative):
        result = set(self.descendants.get(relative, ()))
        for parent in Path(relative).parents:
            result.update(self.get(parent.as_posix(), ()))
        return sorted(result)


def reference_hints(root):
    return ReferenceHints(retention_module().references(root))


def _present(path):
    return path.exists() or path.is_symlink()


def _stamp(path):
    s = path.lstat()
    return [s.st_dev, s.st_ino, s.st_size, s.st_mtime_ns, s.st_ctime_ns]


def host_process_view():
    """Conservative Linux workstation admission; private agent /proc is insufficient."""
    return Path('/proc/1/comm').read_text().strip() in ('systemd', 'init')


def newest_runs(path, count):
    if not path.name.startswith('run-'):
        return set()
    siblings = [p for p in path.parent.iterdir() if p.name.startswith('run-')
                and p.is_dir() and not p.is_symlink()]
    return set(sorted(siblings, key=lambda p: (p.stat().st_mtime_ns, str(p)), reverse=True)[:count])


class GitOwnership:
    """One tracked/ignored query per checkout, including force-added working files."""
    def __init__(self, root, paths):
        self.tracked = set()
        for raw in git(root, 'ls-files', '-z', '--', 'working/').split(b'\0'):
            if raw:
                p = Path(os.fsdecode(raw))
                self.tracked.update(q.as_posix() for q in (p, *p.parents))
        names = [p.relative_to(root).as_posix() for p in paths]
        value = subprocess.run(['git', '-C', str(root), 'check-ignore', '-z', '--stdin'],
                               input=b''.join(os.fsencode(n)+b'\0' for n in names), capture_output=True)
        if value.returncode not in (0, 1):
            raise ValueError('Cannot establish ignored payload ownership')
        self.ignored = {os.fsdecode(n) for n in value.stdout.split(b'\0') if n}

    def reasons(self, relative):
        return (['tracked files are never cleanup payloads'] if relative in self.tracked else []) + (
            ['payload is not ignored by Git'] if relative not in self.ignored else [])


def inspect(root, path, rules, refs, trees, *, max_entries=100000, now=None, ownership=None):
    """Stat-only inspection. Never follow aliases or descend into original/source trees."""
    now = time.time() if now is None else now
    root, path = Path(root).absolute(), Path(path).absolute()
    row = dict(root=str(root), path=str(path), allocated_bytes=0, logical_bytes=0,
               entries=0, measurement_complete=True, reasons=[], references=[])
    reasons = row['reasons']
    try:
        safe(root, path)
        if not path.is_dir():
            raise ValueError('Select a real execution directory')
        relative = path.relative_to(root).as_posix()
        parts = path.relative_to(root / 'working').parts
        if any(n in PROTECTED or n in rules['protected_names'] for n in parts):
            reasons.append('protected storage, input, source or bookkeeping path')
        if any(retention_module().overlap(path, tree) for tree in trees if tree != root and tree not in root.parents):
            reasons.append('registered source worktree overlap')
        ownership = ownership if ownership is not None else GitOwnership(root, [path])
        reasons.extend(ownership.reasons(relative))
        for ancestor in (path, *path.parents):
            if ancestor == root.parent:
                break
            if _present(ancestor / '.retain'):
                reasons.append('retention pin at ' + str(ancestor))
        if path in newest_runs(path, rules['keep_latest_runs']):
            reasons.append('one of the newest runs in its group')
        if isinstance(refs, ReferenceHints):
            row['references'] = refs.overlapping(relative)
        else:
            for name, sources in refs.items():
                if retention_module().overlap(path, root / name):
                    row['references'].extend(sorted(sources))
        if row['references']:
            row['references'] = sorted(set(row['references']))
            reasons.append('tracked metadata references this path')
        # Policy exemptions are reported without opening large stores or fixtures.
        if reasons:
            row['measurement_complete'] = False
        else:
            pending = [path]
            device = path.lstat().st_dev
            while pending:
                p = pending.pop()
                row['entries'] += 1
                if row['entries'] > max_entries:
                    reasons.append('bounded inventory entry limit reached')
                    row['measurement_complete'] = False
                    break
                s = p.lstat()
                row['allocated_bytes'] += s.st_blocks * 512
                if s.st_dev != device:
                    raise ValueError('Nested filesystem requires separate review')
                if p.name in ('.retain', MARKER):
                    raise ValueError('Pinned member or existing archive restore stub')
                if p.name in ('.git', 'AGENTS.md', 'original'):
                    raise ValueError('Nested source checkout or original directory')
                if stat.S_ISLNK(s.st_mode):
                    raise ValueError('Symlink requires separate handoff review')
                if stat.S_ISDIR(s.st_mode):
                    if p.name.startswith('run-') and p in newest_runs(p, rules['keep_latest_runs']):
                        raise ValueError('Selection contains one of the newest runs in its group')
                    if _present(p / '.git') or _present(p / 'AGENTS.md') or _present(p / 'original'):
                        raise ValueError('Nested source checkout or original directory')
                    if now - s.st_mtime < rules['archive_quiet_minutes'] * 60:
                        raise ValueError('Directory active within quiet interval')
                    pending.extend(sorted(p.iterdir(), reverse=True))
                elif stat.S_ISREG(s.st_mode):
                    row['logical_bytes'] += s.st_size
                    if s.st_nlink != 1:
                        raise ValueError('Multiply linked file requires separate review')
                    if now - s.st_mtime < rules['recent_hours'] * 3600:
                        raise ValueError('File modified within retention interval')
                    if p.suffix.lower() in {'.sqlite3', '.sqlite', '.db'} or p.name.endswith(('-wal', '-shm', '-journal')):
                        raise ValueError('Database/catalog requires its dedicated checkpoint workflow')
                    if p.suffix in SOURCE_SUFFIXES:
                        raise ValueError('Unique source or reconstruction recipe requires separate review')
                else:
                    raise ValueError('Special file requires separate preservation')
            if not reasons:
                # Includes open files, cwd, executables, mappings and Wine-prefix environments.
                idle(path)
                prefix = os.fsencode(str(path))
                for process in Path('/proc').iterdir():
                    if not process.name.isdecimal() or int(process.name) == os.getpid():
                        continue
                    try:
                        arguments = (process / 'cmdline').read_bytes().split(b'\0')
                    except (FileNotFoundError, ProcessLookupError, PermissionError):
                        continue
                    if any(a == prefix or a.startswith(prefix + b'/') or a.startswith(prefix + b'=') for a in arguments):
                        raise ValueError('Active process names the execution directory')
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        reasons.append(str(error))
        row['measurement_complete'] = False
    row['state'] = 'held' if reasons else 'review-required'
    return row


def inventory(root, *, include_worktrees=False, max_entries=100000):
    root = Path(root).resolve()
    trees = retention_module().worktrees(root)
    rows, errors = [], []
    for owner in checkout_roots(root, include_worktrees):
        try:
            if owner.is_symlink() or not owner.is_dir():
                errors.append(dict(root=str(owner), reason='missing or aliased checkout retained'))
                continue
            rules = rules_for(owner)
            refs = reference_hints(owner)
            working = owner / 'working'
            if not working.is_dir() or working.is_symlink():
                errors.append(dict(root=str(owner), reason='working directory absent or aliased; retained'))
                continue
            # Include files/aliases too, as held entries rather than silently ignoring them.
            selected = []
            units = list(retention_module().units(working))
            for child in sorted(working.iterdir()):
                if child.name in ('tests', 'experiments', 'build') and child.is_dir() and not child.is_symlink():
                    paths = [p for p, kind, group in units if p.is_relative_to(child)]
                    if not paths:
                        paths = [child]
                else:
                    paths = [child]
                selected.extend(paths)
            ownership = GitOwnership(owner, selected)
            for path in selected:
                rows.append(inspect(owner, path, rules, refs, trees, max_entries=max_entries, ownership=ownership))
        except (OSError, ValueError, subprocess.SubprocessError) as error:
            errors.append(dict(root=str(owner), reason=str(error)))
    return dict(schema=1, kind='working_cleanup_inventory', created_ns=time.time_ns(),
                root=str(root), host_process_view=host_process_view(), rows=rows, errors=errors, limitations=LIMITATIONS)


def summary(data):
    rows = data.get('rows', [])
    return dict(kind=data['kind'], states=dict(Counter(r['state'] for r in rows)),
                measured_allocated_bytes=sum(r['allocated_bytes'] for r in rows),
                review_allocated_bytes=sum(r['allocated_bytes'] for r in rows if r['state'] == 'review-required'),
                partially_measured_units=sum(not r['measurement_complete'] for r in rows),
                errors=data.get('errors', []), limitations=LIMITATIONS)


def make_plan(root, selections, completion_reason, dependency_review, *, max_entries=100000):
    root = Path(root).resolve()
    if not completion_reason.strip() or not dependency_review.strip():
        raise ValueError('Record completion and dependency-review reasons before planning handoff')
    if not selections:
        raise ValueError('Select at least one exact completed execution directory')
    paths = [Path(p).absolute() if Path(p).is_absolute() else root / p for p in selections]
    overlap = retention_module().overlap
    if any(overlap(p, q) for i, p in enumerate(paths) for q in paths[i+1:]):
        raise ValueError('Selections must be distinct and nonoverlapping')
    rules, refs = rules_for(root), reference_hints(root)
    trees = retention_module().worktrees(root)
    rows = []
    for path in paths:
        row = inspect(root, path, rules, refs, trees, max_entries=max_entries)
        if row['state'] != 'review-required':
            raise ValueError('Selected directory is held: ' + str(path) + ': ' + '; '.join(row['reasons']))
        members, identity, allocated = scan(path)
        row.update(members=members, identities={k:list(v) for k,v in identity.items()}, allocated_bytes=allocated)
        rows.append(row)
    return dict(schema=1, kind='working_cleanup_plan', created_ns=time.time_ns(), root=str(root),
                head=git(root, 'rev-parse', 'HEAD').decode().strip(), policy=rules,
                tool_sources={n:hashlib.sha256((TOOLS/n).read_bytes()).hexdigest() for n in SOURCE_NAMES},
                completion_reason=completion_reason, dependency_review=dependency_review,
                max_entries=max_entries, rows=rows, limitations=LIMITATIONS)


def write_database(path, data):
    """Exclusive creation; immutable plan bytes and append-only execution events."""
    path = Path(path).absolute()
    if (path.suffix != '.sqlite3' or any(n in path.parts for n in ('original', '.git', '..'))
            or not any(path.parts[i:i+2] == ('working', 'retention') for i in range(len(path.parts)-1))
            or any(p.is_symlink() for p in (path, *path.parents))):
        raise ValueError('Report requires a new real .sqlite3 path under working/retention/')
    path.parent.mkdir(parents=True, exist_ok=True)
    fd = os.open(path, os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o600)
    os.close(fd)
    body = encoded(data)
    sha = hashlib.sha256(body).hexdigest()
    with closing(sqlite3.connect(path)) as db:
        db.executescript('''PRAGMA synchronous=FULL;
            CREATE TABLE plan(sha256 TEXT PRIMARY KEY, payload BLOB NOT NULL);
            CREATE TABLE events(sequence INTEGER PRIMARY KEY, observed_ns INTEGER, payload BLOB NOT NULL);
            CREATE TRIGGER immutable_plan_update BEFORE UPDATE ON plan BEGIN SELECT RAISE(ABORT,'Immutable plan'); END;
            CREATE TRIGGER immutable_plan_delete BEFORE DELETE ON plan BEGIN SELECT RAISE(ABORT,'Immutable plan'); END;
            CREATE TRIGGER immutable_event_update BEFORE UPDATE ON events BEGIN SELECT RAISE(ABORT,'Immutable event'); END;
            CREATE TRIGGER immutable_event_delete BEFORE DELETE ON events BEGIN SELECT RAISE(ABORT,'Immutable event'); END;''')
        with db:
            db.execute('INSERT INTO plan VALUES(?,?)', (sha, body))
    return sha


def read_database(path):
    path = Path(path).absolute()
    if any(n in path.parts for n in ('original', '.git', '..')) or any(p.is_symlink() for p in (path, *path.parents)):
        raise ValueError('Plan must not traverse aliases')
    with closing(sqlite3.connect(path.as_uri() + '?mode=ro', uri=True)) as db:
        values = db.execute('SELECT sha256,payload FROM plan').fetchall()
    if len(values) != 1 or hashlib.sha256(values[0][1]).hexdigest() != values[0][0]:
        raise ValueError('Plan payload integrity failure')
    return values[0][0], json.loads(values[0][1])


def event(path, value):
    with closing(sqlite3.connect(path)) as db, db:
        db.execute('INSERT INTO events(observed_ns,payload) VALUES(?,?)', (time.time_ns(), encoded(value)))


def revalidate(plan, row):
    root = Path(plan['root'])
    if rules_for(root) != plan['policy']:
        raise ValueError('Retention policy changed; create a new plan')
    current = inspect(root, Path(row['path']), plan['policy'], reference_hints(root),
                      retention_module().worktrees(root), max_entries=plan['max_entries'])
    if current['state'] != 'review-required':
        raise ValueError('Current directory is held: ' + '; '.join(current['reasons']))
    members, identities, allocated = scan(Path(row['path']))
    if members != row['members'] or {k:list(v) for k,v in identities.items()} != row['identities']:
        raise ValueError('Execution membership, metadata or bytes changed; create a new plan')


class GuardedPair:
    """Recheck external consumers/policy after seal's real restore, before retirement.

    seal emits workspace_verified immediately before its final idle/identity checks
    and quarantine. The exact seal source is bound to the reviewed plan.
    """
    def __init__(self, pair, check):
        self.pair, self.check = pair, check

    def __getattr__(self, name):
        return getattr(self.pair, name)

    def root(self, kind, key, data):
        if kind == 'workspace_verified':
            self.check()
        return self.pair.root(kind, key, data)


def apply_plan(database, *, expected_sha=None, execute=False, scratch=None):
    sha, plan = read_database(database)
    if plan.get('schema') != 1 or plan.get('kind') != 'working_cleanup_plan':
        raise ValueError('Only a selected execution plan can be applied')
    if expected_sha is not None and sha != expected_sha:
        raise ValueError('Reviewed plan SHA256 differs')
    if execute and (expected_sha is None or scratch is None):
        raise ValueError('Execution requires --plan-sha256 and an explicit --scratch directory')
    if not 0 <= time.time_ns() - plan['created_ns'] <= 24 * 3600 * 10**9:
        raise ValueError('Plan expired or has a future timestamp; create a new plan')
    if any(hashlib.sha256((TOOLS/n).read_bytes()).hexdigest() != h for n,h in plan['tool_sources'].items()):
        raise ValueError('Cleanup implementation changed; create a new plan')
    root = Path(plan['root'])
    for row in plan['rows']:
        revalidate(plan, row)
    result = dict(success=True, dry_run=not execute, plan_sha256=sha, results=[],
                  selected_allocated_bytes=sum(r['allocated_bytes'] for r in plan['rows']))
    if not execute:
        return result
    if not host_process_view():
        raise ValueError('Execution requires the host process view; private/unrecognized PID namespace is held')
    paths = settings(root)
    if paths is None:
        raise ValueError('Two independent evidence stores must be configured')
    scratch = Path(scratch).absolute()
    if not scratch.is_dir() or any(p.is_symlink() for p in (scratch, *scratch.parents)):
        raise ValueError('Scratch must be an existing real directory')
    if any(retention_module().overlap(scratch, Path(r['path'])) for r in plan['rows']):
        raise ValueError('Restore scratch must be disjoint from selections')
    filesystem = subprocess.check_output(['findmnt', '-n', '-o', 'FSTYPE', '--target', str(scratch)], text=True).strip()
    floors = {root.stat().st_dev: max(90, plan['policy']['minimum_free_gib']) * GIB}
    storage = storage_policy(root)
    for p, key in zip(paths, ('primary_minimum_free_gib', 'replica_minimum_free_gib')):
        floors[p.parent.stat().st_dev] = max(floors.get(p.parent.stat().st_dev, 0), storage[key] * GIB)
    # Unknown disk/archive mounts retain a conservative100GiB floor, which also
    # protects the local SATA10GiB tier. tmpfs scratch uses a1GiB RAM headroom.
    scratch_floor = floors.get(scratch.stat().st_dev, GIB if filesystem == 'tmpfs' else 100*GIB)
    required = max(r['logical_bytes'] for r in plan['rows']) + scratch_floor + 64*1024**2
    if shutil.disk_usage(scratch).free < required:
        raise ValueError('Restore scratch would cross its free-space reserve')
    if os.statvfs(scratch).f_favail < max(r['entries'] for r in plan['rows']) + 1024:
        raise ValueError('Restore scratch has insufficient free inodes')
    # Cooperative coordinator lock; the object pair separately serializes archive writes.
    with Path(database).open('rb') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        before = shutil.disk_usage(root).free
        previous_tempdir = tempfile.tempdir
        tempfile.tempdir = str(scratch)
        try:
            with configured(root) as pair:
                pair.root('working_cleanup_plan', sha, plan)
                for row in plan['rows']:
                    revalidate(plan, row)
                    event(database, dict(state='started', path=row['path'], plan_sha256=sha))
                    value = seal(root, Path(row['path']), GuardedPair(pair, lambda: revalidate(plan, row)),
                                 retire=True, restore_budget=row['logical_bytes'])
                    result['results'].append(value)
                    event(database, dict(state='completed', path=row['path'], result=value))
                result.update(filesystem_free_before=before, filesystem_free_after=shutil.disk_usage(root).free)
                result['reclaimed_allocated_bytes'] = sum(r['reclaimed_allocated_bytes'] for r in result['results'])
                pair.root('working_cleanup_result', sha, result)
        except BaseException as error:
            event(database, dict(state='failed', error=str(error), completed=result['results']))
            raise
        finally:
            tempfile.tempdir = previous_tempdir
    event(database, dict(state='finished', result=result))
    return result
