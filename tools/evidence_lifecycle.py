"""Finite evidence acceptance and explicit retirement; never a cached gate pass.

Accepted retirement declarations are authored control records in Git. Temporary
plans and observations belong in SQLite. Full audits deliberately ignore these
exemptions and still require the original payloads.
"""
from datetime import datetime, timedelta, timezone
import hashlib
import json
from pathlib import Path
import re
import sqlite3
import subprocess

POLICY = 'tools/evidence-lifecycle.json'
RETIREMENTS = 'research/runtime/coverage/evidence-retirements.json'


def encoded(value):
    return (json.dumps(value, sort_keys=True, separators=(',', ':')) + '\n').encode()


def policy(root):
    path = Path(root) / POLICY
    if not path.exists():
        return None  # Historical checkouts retain their original finish behavior.
    return validate_policy(json.loads(path.read_bytes()))


def validate_policy(value):
    if (value.get('schema') != 1 or type(value.get('review_days')) is not int
            or value['review_days'] < 14
            or type(value.get('maximum_evidence_bytes')) is not int
            or value['maximum_evidence_bytes'] <= 0
            or type(value.get('finish_backups')) is not bool):
        raise ValueError('Invalid finite evidence lifecycle policy')
    return value


def timestamp(value):
    result = datetime.fromisoformat(value)
    if result.tzinfo is None:
        raise ValueError('Evidence lifecycle timestamps require a timezone')
    return result.astimezone(timezone.utc)


def now():
    return datetime.now(timezone.utc)


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args], stderr=subprocess.PIPE).decode().strip()


def database(root):
    # Shared across worktrees; never put the lifecycle clock in a retiring tree.
    common = Path(git(root, 'rev-parse', '--path-format=absolute', '--git-common-dir'))
    main = common.parent
    return main / 'working/coverage/evidence-lifecycle.sqlite3'


def connect(root):
    path = database(root)
    path.parent.mkdir(parents=True, exist_ok=True)
    db = sqlite3.connect(path)
    db.execute('CREATE TABLE IF NOT EXISTS integrations(commit_id TEXT PRIMARY KEY, observed_at TEXT NOT NULL)')
    db.execute('CREATE TABLE IF NOT EXISTS acceptances(commit_id TEXT PRIMARY KEY, record BLOB NOT NULL)')
    db.execute('CREATE TABLE IF NOT EXISTS plans(id TEXT PRIMARY KEY, record BLOB NOT NULL)')
    for table in ('integrations', 'acceptances', 'plans'):
        for operation in ('UPDATE', 'DELETE'):
            db.execute(f"CREATE TRIGGER IF NOT EXISTS {table}_no_{operation} BEFORE {operation} ON {table} BEGIN SELECT RAISE(ABORT,'Append-only lifecycle record'); END")
    return db


def integrated(root, commits):
    """Start clocks only after observing actual master inclusion, outside admission."""
    from integration_queue import _current, FD_VARIABLE
    import os
    if _current.get() is not None or os.environ.get(FD_VARIABLE):
        raise ValueError('Record integration after releasing promotion admission')
    pins = [git(root, 'rev-parse', '--verify', '--end-of-options', c + '^{commit}') for c in commits]
    master = git(root, 'rev-parse', 'master')
    for commit in pins:
        subprocess.run(['git', '-C', str(root), 'merge-base', '--is-ancestor', commit, master], check=True)
    with connect(root) as db:
        for commit in pins:
            db.execute('INSERT OR IGNORE INTO integrations VALUES(?,?)', (commit, now().isoformat()))
        result = {c: db.execute('SELECT observed_at FROM integrations WHERE commit_id=?', (c,)).fetchone()[0] for c in pins}
    db.close()
    return result


def acceptance(root, commit, tree, base, receipts, validation_reason, payloads, limitations=None):
    """Keep a compact acceptance record, not another snapshot of every database."""
    record = dict(schema=1, commit=commit, tree=tree, base=base, receipts=receipts,
                  validation_reason=validation_reason, payloads=payloads,
                  accepted_at=now().isoformat(), review_days=policy(root)['review_days'],
                  limitations=limitations or {},
                  scope='Exact source acceptance; supporting payloads remain required through integration and the review window.')
    with connect(root) as db:
        db.execute('INSERT INTO acceptances VALUES(?,?)', (commit, encoded(record)))
    db.close()
    return record


def declarations(read):
    """Parse an optional authored journal without resolving retired payloads."""
    value = json.loads(read(RETIREMENTS))
    if value.get('schema') != 1 or set(value) != {'schema', 'retirements'}:
        raise ValueError('Invalid evidence retirement journal')
    result = {}
    for row in value['retirements']:
        eid = row['evidence']
        if eid in result:
            raise ValueError('Duplicate evidence retirement: ' + eid)
        for key in ('sha256', 'definition_sha256'):
            if not re.fullmatch('[0-9a-f]{64}', row.get(key, '')):
                raise ValueError('Invalid retirement identity: ' + eid)
        for key in ('record', 'reason', 'owner', 'consumer_review', 'result', 'reproduction', 'lost_capability'):
            if not isinstance(row.get(key), str) or not row[key].strip():
                raise ValueError('Retirement needs ' + key + ': ' + eid)
        if not re.fullmatch('[0-9a-f]{40,64}', row.get('integration_commit', '')):
            raise ValueError('Retirement needs exact integration commit')
        if timestamp(row['retired_at']) < timestamp(row['integrated_at']) + timedelta(days=14):
            raise ValueError('Evidence retirement precedes minimum review window')
        if timestamp(row['retired_at']) > now():
            raise ValueError('Evidence retirement is dated in the future')
        result[eid] = row
    return result


def retired(view, register):
    if RETIREMENTS not in view.entries:
        return {}
    rows = declarations(view.raw)
    from coverage_gate import digest
    for ev in register['evidence']:
        row = rows.get(ev['id'])
        if row and (row['sha256'] != ev['sha256'] or row['record'] != ev['record']
                    or row['definition_sha256'] != digest(ev)):
            raise ValueError('Retirement definition differs: ' + ev['id'])
        if row and view.sha(ev['record']) != ev['sha256']:
            raise ValueError('Retired report reference identity changed: ' + ev['id'])
    return rows


def check_history(before, after):
    for eid, row in before.items():
        if after.get(eid) != row:
            raise ValueError('Historical retirement removed or rewritten: ' + eid)


def verify_clock(root, row, settings=None):
    from contextlib import closing
    with closing(connect(root)) as db:
        observed = db.execute('SELECT observed_at FROM integrations WHERE commit_id=?', (row['integration_commit'],)).fetchone()
    if not observed or observed[0] != row['integrated_at']:
        raise ValueError('New retirement must match the locally observed integration clock')
    settings = validate_policy(settings) if settings is not None else policy(root)
    if settings is None or timestamp(row['retired_at']) < timestamp(observed[0]) + timedelta(days=settings['review_days']):
        raise ValueError('New retirement precedes the configured review window')


def prepare_retirement(root, evidence_ids, commit, *, owner, reason, consumer_review, result, reproduction):
    """Verify original reports now; produce a reviewable SQLite plan, no deletion."""
    from coverage_scoped import View, dependency
    from coverage_gate import REGISTER, digest
    if not all(v.strip() for v in (owner, reason, consumer_review, result, reproduction)):
        raise ValueError('Concrete owner, reason, consumer review, result and reproduction are required')
    settings = policy(root)
    if settings is None:
        raise ValueError('Finite lifecycle policy is not installed')
    commit = git(root, 'rev-parse', '--verify', '--end-of-options', commit + '^{commit}')
    subprocess.run(['git', '-C', str(root), 'merge-base', '--is-ancestor', commit, 'master'], check=True)
    db = connect(root)
    try:
        row = db.execute('SELECT observed_at FROM integrations WHERE commit_id=?', (commit,)).fetchone()
        if row is None:
            raise ValueError('Integration clock missing; run integrated first (never infer it from commit dates)')
        integrated_at = row[0]
        if now() < timestamp(integrated_at) + timedelta(days=settings['review_days']):
            raise ValueError('Evidence review window has not expired')
        view = View(root, revision=commit)
        try:
            register = view.value(REGISTER)
            evidence = {e['id']: e for e in register['evidence']}
            rows = []
            for eid in sorted(set(evidence_ids)):
                ev = evidence[eid]
                dependency(ev, view)  # Verify the pinned payload and extract original provenance.
                from coverage_audit import pointer
                report = view.value(ev['record'])
                if any(pointer(report, a['pointer']) != a['equals'] for a in ev['assertions']):
                    raise ValueError('Evidence assertions do not match: ' + eid)
                rows.append(dict(evidence=eid, record=ev['record'], sha256=ev['sha256'],
                    definition_sha256=digest(ev), integration_commit=commit, integrated_at=integrated_at,
                    retired_at=now().isoformat(), owner=owner, reason=reason, consumer_review=consumer_review,
                    result=result, reproduction=reproduction,
                    lost_capability='Original supporting report may be deleted; replay, reanalysis and a complete evidence audit are no longer promised.'))
        finally:
            view.close()
        plan = dict(schema=1, retirements=rows)
        body = encoded(plan)
        identity = hashlib.sha256(body).hexdigest()
        with db:
            db.execute('INSERT OR IGNORE INTO plans VALUES(?,?)', (identity, body))
        return dict(plan=identity, database=str(database(root)), retirements=rows, deletion_performed=False)
    finally:
        db.close()


def export_retirement(root, plan_id):
    """Export accepted authored declarations for normal source review and hook."""
    from coverage_bookkeeping import atomic_replace
    with connect(root) as db:
        row = db.execute('SELECT record FROM plans WHERE id=?', (plan_id,)).fetchone()
    db.close()
    if row is None or hashlib.sha256(row[0]).hexdigest() != plan_id:
        raise ValueError('Missing or corrupt retirement plan')
    plan = json.loads(row[0])
    path = Path(root) / RETIREMENTS
    old = path.read_bytes() if path.exists() else None
    existing = declarations(lambda _: old) if old else {}
    for item in plan['retirements']:
        if item['evidence'] in existing and existing[item['evidence']] != item:
            raise ValueError('Retirement already declared with different terms')
        existing[item['evidence']] = item
    body = encoded(dict(schema=1, retirements=list(existing.values())))
    declarations(lambda _: body)
    atomic_replace(path, body, old)
    return dict(path=str(path), entries=len(existing), deletion_performed=False)


def verify_acceptance(root, base, evidence_ids):
    """Hash original staged/consumed payloads without making recovery snapshots."""
    from coverage_scoped import View
    from coverage_gate import REGISTER
    from coverage_evidence import identity
    from evidence_objects import Sink
    view = View(root, staged=True)
    try:
        settings = validate_policy(view.value(POLICY))
        reg = view.value(REGISTER)
        names = {e['record'] for e in reg['evidence'] if e['id'] in evidence_ids}
        for field in ('code_index', 'change_scope_index'):
            if field in reg:
                names.add(reg[field]['path'])
        changed = set(git(root, 'diff', '--cached', '--name-only', '--no-renames', base).splitlines())
        names.update(changed)
        result, total, hashes = [], 0, set()
        for name in sorted(names & view.entries.keys()):
            ref = identity(view.raw(name))
            if ref:
                # Reading an unchanged retained object allocates no new custody.
                if name in changed and ref[0] not in hashes:
                    total += ref[1]
                    hashes.add(ref[0])
                if total > settings['maximum_evidence_bytes']:
                    raise ValueError('Acceptance payload budget exceeded; narrow acquisition or review the finite budget')
                view.reader.resolver.copy(*ref, Sink())
                result.append(dict(path=name, sha256=ref[0], size=ref[1]))
        return result
    finally:
        view.close()


def prune_store(root, source, *, consumer_review, pin_review='', execute=False):
    """Retire declared report bytes in a stopped, plain pending Store.

    Shared pairs and hash-pinned release containers keep their separate migration
    protocols. Unknown SQLite tables cannot be discarded by this operation.
    """
    import os
    import tempfile
    from contextlib import closing
    from coverage_evidence import Store, digest_file, identity
    from coverage_scoped import View
    from coverage_gate import REGISTER
    from compact_snapshot_activity import busy
    from evidence_objects import Sink
    from integration_queue import _current, FD_VARIABLE
    if _current.get() is not None or os.environ.get(FD_VARIABLE):
        raise ValueError('Evidence expiry must run outside promotion admission')
    if not consumer_review.strip():
        raise ValueError('Review all writers, active branches and report consumers explicitly')
    root = Path(root).resolve()
    source = Path(source).absolute()
    main = Path(git(root, 'rev-parse', '--path-format=absolute', '--git-common-dir')).parent
    if (not source.is_relative_to(main / 'working') or not source.is_file()
            or any(p.is_symlink() for p in (source, *source.parents))
            or source.stat().st_nlink != 1 or source.stat().st_uid != os.geteuid()
            or re.fullmatch('[0-9a-f]{64}\\.sqlite3', source.name)):
        raise ValueError('Select a plain owned pending store, not links or a hash-pinned archive')
    for key in ('mm.evidencePrimary', 'mm.evidenceReplica'):
        configured = subprocess.run(['git', '-C', str(root), 'config', '--get', key], capture_output=True, text=True)
        if configured.returncode == 0 and Path(configured.stdout.strip()).resolve() == source:
            raise ValueError('Shared object stores require a separate source-specific retirement operation')
    if any((p / '.retain').exists() for p in source.parents if p.is_relative_to(main)) and not pin_review.strip():
        raise ValueError('Pinned store needs an explicit ownership and pin release review')

    def idle():
        if busy(source) or any(Path(str(source) + s).exists() for s in ('-wal', '-shm', '-journal')):
            raise ValueError('Evidence store is active; stop consumers before pruning')
    idle()
    original = digest_file(source)
    stat = source.stat()
    current = View(root, revision='master')
    try:
        declared = retired(current, current.value(REGISTER))
        allowed = {row['sha256'] for row in declared.values()}
        if not allowed:
            raise ValueError('No accepted retirements on master')
        # Every live checkout must have discharged its own claims. A master
        # declaration cannot silently retire another task's required evidence.
        for item in git(root, 'worktree', 'list', '--porcelain').split('\n\n'):
            if not item.strip():
                continue
            checkout = Path(item.splitlines()[0].removeprefix('worktree '))
            view = View(checkout)
            try:
                reg = view.value(REGISTER)
                local = retired(view, reg)
                for ev in reg['evidence']:
                    if ev['sha256'] in allowed and ev['id'] not in local:
                        allowed.discard(ev['sha256'])
                # Control payloads remain operational inputs until replaced.
                # All other objects are kept even if they have no known consumer.
                for field in ('code_index', 'change_scope_index'):
                    if field in reg:
                        allowed.discard(reg[field]['sha256'])
                for ref in reg.get('evidence_applicability', []):
                    allowed.discard(ref['sha256'])
                retired_paths = {e['record'] for e in reg['evidence'] if e['id'] in local}
                for name in view.entries:
                    if name.startswith('research/') and name.endswith('.json') and name not in retired_paths:
                        ref = identity(view.raw(name))
                        if ref:
                            allowed.discard(ref[0])
            finally:
                view.close()
        store = Store(source, readonly=True)
        try:
            tables = {r[0] for r in store.db.execute("SELECT name FROM sqlite_master WHERE type='table'")}
            if store.shared or tables != {'objects', 'chunks', 'versions', 'metadata'}:
                raise ValueError('Unknown/shared store schema requires a separate reviewed migration')
            store.verify()
            objects = list(store.db.execute('SELECT sha256,size,git_blob FROM objects ORDER BY sha256'))
            selected = [sha for sha, _, _ in objects if sha in allowed]
            plan = dict(source=str(source), original_sha256=original, retire=selected,
                        consumer_review=consumer_review, pin_review=pin_review,
                        loss='Selected original report payloads intentionally expire; their historical identities and results remain in Git.')
            plan_id = hashlib.sha256(encoded(plan)).hexdigest()
            with closing(connect(root)) as db, db:
                db.execute('INSERT OR IGNORE INTO plans VALUES(?,?)', (plan_id, encoded(plan)))
            if not execute or not selected:
                return dict(plan=plan_id, selected=len(selected), deletion_performed=False)
            fd, name = tempfile.mkstemp(prefix='expiry-', suffix='.sqlite3', dir=source.parent)
            os.close(fd)
            replacement = Path(name)
            try:
                output = Store(replacement)
                try:
                    with output.db:
                        for sha, size, blob in objects:
                            if sha in selected:
                                continue
                            with tempfile.SpooledTemporaryFile(max_size=8*1024**2) as payload:
                                store.copy(sha, size, payload)
                                payload.seek(0)
                                if output.put(payload, size, blob) != sha:
                                    raise ValueError('Surviving payload identity changed')
                        output.db.executemany('INSERT OR REPLACE INTO metadata VALUES(?,?)', store.db.execute('SELECT key,value FROM metadata'))
                        output.db.executemany('INSERT INTO versions VALUES(?,?,?)',
                            (r for r in store.db.execute('SELECT git_blob,path,sha256 FROM versions') if r[2] not in selected))
                    output.verify()
                finally:
                    output.close()
                idle()
                current_stat = source.stat()
                if digest_file(source) != original or any(getattr(current_stat, k) != getattr(stat, k)
                        for k in ('st_dev', 'st_ino', 'st_size', 'st_mtime_ns', 'st_ctime_ns', 'st_nlink')):
                    raise ValueError('Evidence store changed during expiry; retain it')
                os.chmod(replacement, stat.st_mode & 0o777)
                with replacement.open('rb') as stream:
                    os.fsync(stream.fileno())
                store.close()
                store = None
                os.replace(replacement, source)
                descriptor = os.open(source.parent, os.O_RDONLY | os.O_DIRECTORY)
                try:
                    os.fsync(descriptor)
                finally:
                    os.close(descriptor)
            finally:
                replacement.unlink(missing_ok=True)
        finally:
            if store is not None:
                store.close()
        receipt = dict(plan=plan_id, replacement_sha256=digest_file(source),
                       deletion_performed=True, retired_payloads=len(selected),
                       reclaimed_allocated_bytes=stat.st_blocks*512-source.stat().st_blocks*512,
                       replacement_copy_of_retired_payloads=False)
        with closing(connect(root)) as db, db:
            db.execute('INSERT INTO plans VALUES(?,?)', (hashlib.sha256(encoded(receipt)).hexdigest(), encoded(receipt)))
        return receipt
    finally:
        current.close()
