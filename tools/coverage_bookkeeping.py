"""SQLite workspace for exact reviews; Git exports remain the durable authority.

The database is disposable and worktree-local. No cached audit or snapshot can
approve a later tree: recording always recomputes both sides with today's gate.
"""
from contextlib import contextmanager
import fcntl
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import sqlite3
import subprocess
import tempfile

from coverage_gate import (REGISTER, check, changes, digest, draft, review_errors,
                           snapshot, source_index)
from coverage_merge_history import inherited_history
from coverage_reviews import REVIEWS, load_journal
from coverage_evidence import read_bytes as evidence_bytes, write_report

DEFAULT_DB = 'working/coverage/bookkeeping.sqlite3'


def encoded(value):
    return (json.dumps(value, sort_keys=True, separators=(',', ':'), ensure_ascii=True) + '\n').encode()


def atomic_replace(path, data, expected):
    """Refuse changed inputs and replace a complete, fsynced file on one volume."""
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.is_symlink():
        raise ValueError(f'Refusing symlink output: {path}')
    current = path.read_bytes() if path.exists() else None
    if current != expected:
        raise ValueError(f'Output changed concurrently; retry: {path}')
    if current == data:
        return
    fd, name = tempfile.mkstemp(prefix=path.name + '.', suffix='.tmp', dir=path.parent)
    try:
        with os.fdopen(fd, 'wb') as stream:
            if path.exists():
                os.fchmod(stream.fileno(), path.stat().st_mode & 0o777)
            stream.write(data)
            stream.flush()
            os.fsync(stream.fileno())
        if (path.read_bytes() if path.exists() else None) != expected:
            raise ValueError(f'Output changed concurrently; retry: {path}')
        os.replace(name, path)
        directory = os.open(path.parent, os.O_RDONLY | os.O_DIRECTORY)
        try:
            os.fsync(directory)
        finally:
            os.close(directory)
    finally:
        if os.path.exists(name):
            os.unlink(name)


class Workspace:
    def __init__(self, root, database=None):
        self.root = Path(root).resolve()
        path = self.root/Path(database) if database else self.root/DEFAULT_DB
        self.path = path.resolve()
        # Mutable binary state never belongs among the durable coverage records
        # or immutable originals. Keep explicit alternate databases in working/.
        if not self.path.is_relative_to(self.root/'working'):
            raise ValueError('Database must be inside this worktree\'s ignored working/ directory')
        self.path.parent.mkdir(parents=True, exist_ok=True)
        self.db = sqlite3.connect(self.path, timeout=30)
        try:
            self.db.execute('PRAGMA foreign_keys=ON')
            version = self.db.execute('PRAGMA user_version').fetchone()[0]
            if version not in (0, 1):
                raise ValueError(f'Unsupported bookkeeping database schema: {version}')
            self.db.executescript('''
                CREATE TABLE IF NOT EXISTS metadata (key TEXT PRIMARY KEY, value TEXT NOT NULL);
                CREATE TABLE IF NOT EXISTS receipts (
                    sequence INTEGER PRIMARY KEY, id TEXT UNIQUE NOT NULL,
                    reason TEXT NOT NULL, validation_status TEXT NOT NULL,
                    validation_reason TEXT NOT NULL, record_json TEXT NOT NULL);
                CREATE TABLE IF NOT EXISTS file_changes (
                    receipt TEXT NOT NULL REFERENCES receipts(id), ordinal INTEGER NOT NULL,
                    path TEXT NOT NULL, before_sha256 TEXT, after_sha256 TEXT,
                    PRIMARY KEY(receipt, ordinal));
                CREATE INDEX IF NOT EXISTS file_lookup ON file_changes(path);
                CREATE TABLE IF NOT EXISTS behavior_changes (
                    receipt TEXT NOT NULL REFERENCES receipts(id), ordinal INTEGER NOT NULL,
                    behavior TEXT NOT NULL, before_sha256 TEXT, after_sha256 TEXT,
                    PRIMARY KEY(receipt, ordinal));
                CREATE INDEX IF NOT EXISTS behavior_lookup ON behavior_changes(behavior);
                CREATE TABLE IF NOT EXISTS pending_gaps (
                    receipt TEXT NOT NULL REFERENCES receipts(id), ordinal INTEGER NOT NULL,
                    gap TEXT NOT NULL, PRIMARY KEY(receipt, ordinal));
                CREATE TABLE IF NOT EXISTS validation_evidence (
                    receipt TEXT NOT NULL REFERENCES receipts(id), ordinal INTEGER NOT NULL,
                    evidence TEXT NOT NULL, PRIMARY KEY(receipt, ordinal));
                CREATE TABLE IF NOT EXISTS drafts (id TEXT PRIMARY KEY, context_json TEXT NOT NULL);
                CREATE TRIGGER IF NOT EXISTS immutable_receipts_update BEFORE UPDATE ON receipts
                    BEGIN SELECT RAISE(ABORT, 'Receipts are append-only'); END;
                CREATE TRIGGER IF NOT EXISTS immutable_receipts_delete BEFORE DELETE ON receipts
                    BEGIN SELECT RAISE(ABORT, 'Receipts are append-only'); END;
                PRAGMA user_version=1;
            ''')
            with self.db:
                owner = self.meta('root')
                if owner is not None and owner != str(self.root):
                    raise ValueError('Database belongs to another worktree; rebuild it from that tree\'s exports')
                self.set_meta('root', str(self.root))
        except BaseException:
            self.db.close()
            raise

    def close(self):
        self.db.close()

    def meta(self, key):
        row = self.db.execute('SELECT value FROM metadata WHERE key=?', (key,)).fetchone()
        return row[0] if row else None

    def set_meta(self, key, value):
        self.db.execute('INSERT INTO metadata VALUES (?,?) ON CONFLICT(key) DO UPDATE SET value=excluded.value', (key, value))

    @contextmanager
    def locked(self):
        """Serialize all workflow writers in this worktree, even alternate DBs."""
        lock = self.root/'working/coverage/bookkeeping.lock'
        lock.parent.mkdir(parents=True, exist_ok=True)
        with lock.open('a') as stream:
            fcntl.flock(stream, fcntl.LOCK_EX)
            try:
                with self.db:
                    self.db.execute('BEGIN IMMEDIATE')
                    yield
            finally:
                fcntl.flock(stream, fcntl.LOCK_UN)

    def insert(self, review):
        row = self.db.execute('SELECT record_json FROM receipts WHERE id=?', (review['id'],)).fetchone()
        value = encoded(review).decode()
        if row:
            if row[0] != value:
                raise ValueError('Receipt ID reused with different content: ' + review['id'])
            return
        v = review['validation']
        self.db.execute('INSERT INTO receipts(id,reason,validation_status,validation_reason,record_json) VALUES (?,?,?,?,?)',
                        (review['id'], review['reason'], v['status'], v['reason'], value))
        for table, field, key in (('file_changes', 'files', 'path'), ('behavior_changes', 'behaviors', 'id')):
            self.db.executemany(f'INSERT INTO {table} VALUES (?,?,?,?,?)',
                                [(review['id'], i, item[key], item['before_sha256'], item['after_sha256'])
                                 for i, item in enumerate(review.get(field, []))])
        for table, items in (('pending_gaps', review.get('pending_gaps', [])),
                             ('validation_evidence', v.get('evidence', []))):
            self.db.executemany(f'INSERT INTO {table} VALUES (?,?,?)',
                                [(review['id'], i, item) for i, item in enumerate(items)])

    def sync(self):
        """Import exact values, rejecting rewritten/removed/reordered history.

        Caller holds locked(). External exports can add receipts, including an
        identical local receipt whose previous export completed before a crash.
        """
        journal, _ = load_journal(self.root)
        if journal.get('schema') != 1:
            raise ValueError('Unsupported review journal schema')
        ids = [r['id'] for r in journal['reviews']]
        if len(set(ids)) != len(ids):
            raise ValueError('Duplicate journal receipt ID')
        previous = json.loads(self.meta('imported_ids') or '[]')
        prior = set(previous)
        if [rid for rid in ids if rid in prior] != previous:
            raise ValueError('Imported receipt history removed or reordered; use the correct worktree database')
        for review in journal['reviews']:
            self.insert(review)
        self.set_meta('imported_ids', json.dumps(ids))
        return journal

    def pending(self, journal):
        ids = {r['id'] for r in journal['reviews']}
        return [json.loads(row[0]) for row in self.db.execute('SELECT record_json FROM receipts ORDER BY sequence')
                if json.loads(row[0])['id'] not in ids]

    def export(self):
        """Append local accepted receipts; no-op exports preserve every byte."""
        journal = self.sync()
        pending = self.pending(journal)
        if not pending:
            return []
        path = self.root/REVIEWS
        previous = path.read_bytes()
        active = json.loads(previous)
        # Archives remain byte-for-byte untouched and in their existing order.
        active['reviews'].extend(pending)
        atomic_replace(path, encoded(active), previous)
        self.sync()
        return [r['id'] for r in pending]

    def find(self, *, path=None, behavior=None, status=None, limit=25):
        where, params = [], []
        if path:
            where.append('EXISTS (SELECT 1 FROM file_changes f WHERE f.receipt=r.id AND f.path=?)')
            params.append(path)
        if behavior:
            where.append('EXISTS (SELECT 1 FROM behavior_changes b WHERE b.receipt=r.id AND b.behavior=?)')
            params.append(behavior)
        if status:
            where.append('r.validation_status=?')
            params.append(status)
        sql = 'SELECT r.record_json FROM receipts r' + (' WHERE ' + ' AND '.join(where) if where else '')
        return [json.loads(row[0]) for row in self.db.execute(sql + ' ORDER BY r.sequence DESC LIMIT ?', params + [limit])]


def refresh_index(root):
    """Explicitly acknowledged indexing review, never a behavior mapping review."""
    root = Path(root)
    path = root/REGISTER
    previous = path.read_bytes()
    register = json.loads(previous)
    data = (json.dumps(source_index.index(root, register), indent=2) + '\n').encode()
    sha = hashlib.sha256(data).hexdigest()
    name = f'research/runtime/coverage/source-index-{sha}.json'
    target = root/name
    if target.exists():
        if evidence_bytes(target, root=root) != data:
            raise ValueError('Existing source census content mismatch')
    else:
        write_report(root, target, data)
    register['code_index'] = {'path': name, 'sha256': sha}
    atomic_replace(path, (json.dumps(register, indent=2) + '\n').encode(), previous)
    return name


def gate_cli():
    spec = importlib.util.spec_from_file_location('coverage_workflow_gate', Path(__file__).with_name('check-re-coverage.py'))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def capture(root, base):
    from evidence_local import local_evidence
    with local_evidence(root):
        return _capture(root, base)


def _capture(root, base):
    cli = gate_cli()
    # Resolve a moving branch name once. This is deliberately not a pass cache.
    head = cli.git(root, 'rev-parse', '--verify', 'HEAD^{commit}').decode().strip()
    before, commit = cli.git_snapshot(root, base)
    after, audit_report = snapshot(root)
    journal = load_journal(root)[0]
    history = inherited_history(root, commit, after, journal)
    return before, after, audit_report, journal, history, commit, head


def tree_digest(value):
    # The journal is independently append-only checked/imported; new unrelated
    # receipts may arrive without invalidating an unchanged source draft.
    return digest({k: v for k, v in value.items() if k != 'historical_reviews'})


def select_delta(delta, paths=(), behaviors=()):
    if not paths and not behaviors:
        return delta
    files = {f['path']: f for f in delta['files']}
    contracts = {b['id']: b for b in delta['behaviors']}
    unknown = set(paths) - files.keys() | set(behaviors) - contracts.keys()
    if unknown:
        raise ValueError('Selectors must name changed files/behaviors: ' + ', '.join(sorted(unknown)))
    selected = [f for f in delta['files'] if f['path'] in paths]
    bids = set(behaviors) | {bid for f in selected for bid in f['behaviors']}
    # A shared file brings every affected contract with it. Exact gap identities
    # remain explicit and can be inspected before the receipt is accepted.
    return {'files': selected, 'behaviors': [b for b in delta['behaviors'] if b['id'] in bids],
            'new_gaps': delta['new_gaps']}


def prepare(workspace, base='HEAD', paths=(), behaviors=(), index_reason=None):
    with workspace.locked():
        workspace.sync()
        index = None
        if index_reason is not None:
            if not index_reason.strip():
                raise ValueError('Index review reason cannot be empty')
            index = refresh_index(workspace.root)
        before, after, audit_report, journal, history, commit, head = capture(workspace.root, base)
        delta = select_delta(changes(before, after), paths, behaviors)
        if not delta['files'] and not delta['behaviors']:
            raise ValueError('No selected changes to record')
        review = draft(delta)['reviews'][0]
        context = {'review': review, 'base': commit, 'head': head,
                   'before_sha256': tree_digest(before), 'after_sha256': tree_digest(after),
                   'paths': list(paths), 'behaviors': list(behaviors), 'index_review_reason': index_reason,
                   'index': index}
        workspace.db.execute('INSERT INTO drafts VALUES (?,?) ON CONFLICT(id) DO UPDATE SET context_json=excluded.context_json',
                             (review['id'], encoded(context).decode()))
        workspace.set_meta('latest_draft', review['id'])
        report = check(before, after, journal, audit_report, history)
        return {'draft': review, 'base': commit, 'head': head, 'index': index,
                'gate_errors': report['errors'], 'counts': report['counts']}


def record(workspace, draft_id, reason, validation_reason, status='pending', evidence=(), stages=(), export=True):
    if not reason.strip() or not validation_reason.strip():
        raise ValueError('Concrete review and validation reasons are required')
    with workspace.locked():
        journal = workspace.sync()
        draft_id = draft_id or workspace.meta('latest_draft')
        row = workspace.db.execute('SELECT context_json FROM drafts WHERE id=?', (draft_id,)).fetchone()
        if row is None:
            raise ValueError('Unknown draft; run prepare first')
        context = json.loads(row[0])
        before, after, audit_report, current_journal, history, commit, head = capture(workspace.root, context['base'])
        if (head != context['head'] or tree_digest(before) != context['before_sha256']
                or tree_digest(after) != context['after_sha256']):
            raise ValueError('Draft is stale: source, contract, census or HEAD changed; run prepare again')
        if current_journal['reviews'] != journal['reviews']:
            raise ValueError('Journal changed during recording; retry')
        review = context['review']
        review['reason'] = reason
        review['validation'] = {'status': status, 'reason': validation_reason, 'evidence': list(evidence)}
        if stages:
            review['validation']['stages'] = list(stages)
        if context['index_review_reason']:
            review['index_review_reason'] = context['index_review_reason']
        # Include accepted but not yet exported local receipts, preserving order.
        local = workspace.pending(journal)
        candidate = dict(journal, reviews=journal['reviews'] + local + [review])
        existing = {r['id']: r for r in journal['reviews'] + local}
        if review['id'] in existing:
            if existing[review['id']] != review:
                raise ValueError('Receipt already recorded with different reasons/validation; prepare a different change')
            candidate = dict(journal, reviews=journal['reviews'] + local)
        delta = changes(before, after)
        problems, _, _ = review_errors(review, delta, before, after, audit_report)
        if problems:
            raise ValueError('; '.join(problems))
        report = check(before, after, candidate, audit_report, history)
        # Partial receipts are useful in a shared dirty checkout. They cannot
        # introduce a new gate failure; unresolved failures remain explicit.
        prior = check(before, after, dict(journal, reviews=journal['reviews'] + local), audit_report, history)
        introduced = set(report['errors']) - set(prior['errors'])
        if introduced:
            raise ValueError('; '.join(sorted(introduced)))
        workspace.insert(review)
    # Commit the DB first: a failed export is retryable without losing a review.
    exported = []
    if export:
        with workspace.locked():
            exported = workspace.export()
    return {'receipt': review['id'], 'exported': exported, 'gate_passed': not report['errors'],
            'gate_errors': report['errors'], 'counts': report['counts']}


def promotion_reviews(before, after, audit_report, delta, prefix, reason):
    """Account existing declarations; never infer, set or promote a milestone."""
    ranks = {'implementation': {'none': 0, 'partial': 1, 'scoped': 2},
             'comparison': {'none': 0, 'recorded': 1},
             'integration': {'none': 0, 'headless': 1, 'preview': 2,
                             'live_observation': 3, 'live_equivalence': 4},
             'replacement': {'none': 0, 'scoped_live': 1}}
    audited = {b['id']: b for b in audit_report['behaviors']}
    rows = {b['id']: b for b in delta['behaviors']}
    groups = {}
    for bid, current in after['behaviors'].items():
        status = current['record']['status']
        old = before['behaviors'].get(bid, {}).get('record', {}).get('status', {})
        stages = tuple(stage for stage, levels in ranks.items()
                       if levels.get(status.get(stage), 0) > levels.get(old.get(stage), 0)
                       and (stage != 'implementation' or status[stage] == 'scoped'))
        if not stages:
            continue
        evidence = set()
        for stage in stages:
            support = audited[bid]['current_validation'][stage]
            if support['state'] != 'current' or not support['evidence']:
                raise ValueError(f'Current execution evidence required for {bid}/{stage}')
            evidence.update(support['evidence'])
        group = groups.setdefault(stages, {'behaviors': [], 'evidence': set()})
        group['behaviors'].append(rows[bid])
        group['evidence'].update(evidence)
    return [{'id': prefix + '-proof-' + '-'.join(stages),
             'reason': 'Reviewed current scope-bound execution for the explicitly declared promoted dimensions.',
             'files': [], 'behaviors': value['behaviors'], 'pending_gaps': [],
             'validation': {'status': 'recorded', 'stages': list(stages),
                            'reason': reason, 'evidence': sorted(value['evidence'])}}
            for stages, value in sorted(groups.items())]


def finish(workspace, base, reason, validation_reason, message, index_reason=None,
           record_promotions=False):
    from evidence_local import local_evidence
    with local_evidence(workspace.root):
        return _finish(workspace, base, reason, validation_reason, message,
                       index_reason, record_promotions)


def _finish(workspace, base, reason, validation_reason, message, index_reason=None,
            record_promotions=False):
    """One owned batch: fresh review, local backups, staging and normal commit.

    Nothing is published. The normal pre-commit hook rechecks the actual index;
    a failed check/commit leaves source, accepted receipts and backups intact.
    """
    from evidence_local import backup_sqlite, local_evidence
    if not all(value.strip() for value in (reason, validation_reason, message)):
        raise ValueError('Concrete review/validation reasons and commit message are required')
    with workspace.locked():
        workspace.sync()
        if index_reason is not None:
            if not index_reason.strip():
                raise ValueError('Index review reason cannot be empty')
            refresh_index(workspace.root)
        before, after, audit_report, journal, history, commit, head = capture(workspace.root, base)
        if commit != head:
            raise ValueError('Finish must review against the current HEAD')
        delta = changes(before, after)
        if not delta['files'] and not delta['behaviors']:
            raise ValueError('No changes to finish')
        review = draft(delta)['reviews'][0]
        review['reason'] = reason
        review['validation'] = {'status': 'pending', 'reason': validation_reason, 'evidence': []}
        if index_reason:
            review['index_review_reason'] = index_reason
        additions = [review]
        if record_promotions:
            additions += promotion_reviews(before, after, audit_report, delta, review['id'], validation_reason)
        local = workspace.pending(journal)
        existing = {r['id']: r for r in journal['reviews'] + local}
        additions = [r for r in additions if r['id'] not in existing or r != existing[r['id']]]
        for item in additions:
            errors, _, _ = review_errors(item, delta, before, after, audit_report)
            if errors:
                raise ValueError('; '.join(errors))
        candidate = dict(journal, reviews=journal['reviews'] + local + additions)
        result = check(before, after, candidate, audit_report, history)
        if result['errors']:
            raise ValueError('; '.join(result['errors']))
        workspace.db.execute('INSERT OR REPLACE INTO drafts VALUES (?,?)',
                             (review['id'], encoded({'base': commit, 'head': head, 'review': review}).decode()))
        for item in additions:
            workspace.insert(item)
    with workspace.locked():
        exported = workspace.export()
    directory = (workspace.path.parent/'backups').resolve()
    if not directory.is_relative_to(workspace.root/'working'):
        raise ValueError('SQLite backups must remain in this worktree working/ directory')
    backups = [backup_sqlite(workspace.path, directory)]
    pending = workspace.root/'working/coverage/evidence-pending.sqlite3'
    if pending.is_file():
        backups.append(backup_sqlite(pending, directory))
    git = ['git', '-C', str(workspace.root)]
    if subprocess.check_output(git + ['rev-parse', 'HEAD'], text=True).strip() != head:
        raise ValueError('HEAD changed during completion; review the new base')
    subprocess.run(git + ['add', '--all', '--', '.'], check=True)
    # Hooks remain mandatory and execute against this worktree's staged tree.
    with local_evidence(workspace.root):
        subprocess.run(git + ['commit', '-m', message], check=True)
    committed = subprocess.check_output(git + ['rev-parse', 'HEAD'], text=True).strip()
    return {'commit': committed, 'receipts': exported, 'backups': backups,
            'publication': 'local only; evidence publication is optional'}
