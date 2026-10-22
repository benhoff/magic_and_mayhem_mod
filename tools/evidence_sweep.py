"""Reviewed JSON inventory and lossless compaction into SQLite-backed references.

Only generated research payloads move. Configuration, schemas, fixtures, live
registers and source locks remain editable files. A verified cold SQLite copy
and two durable receipts precede any working-file replacement; Git's index is
never changed. Source identity checks preserve concurrent work.
"""
import hashlib
import io
import json
import os
from pathlib import Path
import sqlite3
import stat
import subprocess
import tempfile

from coverage_evidence import (CONFIG, KEEP, Reader, Store, artifact, digest_file,
                               config, identity, pointer, safe_path)


def role(name):
    if artifact(name):
        return 'generated_research'
    if name in KEEP or name.endswith(('register.json', '-coverage.json')):
        return 'editable_accounting'
    if name.startswith('protocols/schemas/'):
        return 'wire_schema'
    if name.startswith('tests/fixtures/'):
        return 'test_fixture'
    if name.startswith('apps/coverage-ui/'):
        return 'editable_ui_policy'
    return 'configuration'


def json_paths(root):
    root = Path(root)
    result = subprocess.run(['git', '-C', str(root), 'ls-files', '-z', '--cached',
                             '--others', '--exclude-standard'], capture_output=True)
    if result.returncode == 0:
        names = {os.fsdecode(x) for x in result.stdout.split(b'\0') if x}
    else:
        names = set()
        for directory, children, files in os.walk(root, followlinks=False):
            children[:] = [name for name in children if name not in
                           {'original', 'working', '.git', '.agents', '.codex', 'node_modules'}
                           and not (Path(directory)/name).is_symlink()]
            names.update((Path(directory)/name).relative_to(root).as_posix() for name in files)
    return sorted(name for name in names if name.endswith('.json') and
                  Path(name).parts[0] not in {'original', 'working', '.git', '.agents', '.codex'})


def inventory(root):
    root = Path(root).resolve()
    files, totals = [], {}
    for name in json_paths(root):
        path = root/name
        if path.is_symlink():
            files.append({'path': name, 'role': 'preserved_symlink', 'bytes': 0})
            continue
        if not path.is_file():
            continue
        info = path.stat()
        with path.open('rb') as source:
            header = source.read(161)
        ident = identity(header) if len(header) <= 160 else None
        kind = role(name)
        row = {'path': name, 'role': kind, 'bytes': info.st_size,
               'representation': 'sqlite_reference' if ident else 'expanded_json'}
        if ident:
            row.update(sha256=ident[0], payload_bytes=ident[1])
        files.append(row)
        group = totals.setdefault(kind, {'files': 0, 'bytes': 0, 'references': 0})
        group['files'] += 1
        group['bytes'] += info.st_size
        group['references'] += int(ident is not None)
    return {'schema': 1, 'root': str(root), 'groups': totals, 'files': files}


def durable_json(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, name = tempfile.mkstemp(prefix=path.name+'.', dir=path.parent)
    try:
        with os.fdopen(fd, 'wb') as output:
            output.write((json.dumps(value, indent=2, sort_keys=True)+'\n').encode())
            output.flush()
            os.fsync(output.fileno())
        os.replace(name, path)
        directory = os.open(path.parent, os.O_RDONLY | os.O_DIRECTORY)
        try:
            os.fsync(directory)
        finally:
            os.close(directory)
    finally:
        if os.path.exists(name):
            os.unlink(name)


def file_identity(path):
    info = Path(path).lstat()
    if not stat.S_ISREG(info.st_mode):
        raise ValueError('Compaction requires a regular file: '+str(path))
    return {'device': info.st_dev, 'inode': info.st_ino, 'size': info.st_size,
            'mtime_ns': info.st_mtime_ns, 'mode': stat.S_IMODE(info.st_mode),
            'allocated_bytes': info.st_blocks*512}


def prepare(root, output, database=None):
    root, output = Path(root).resolve(), Path(output).resolve()
    if output.is_relative_to(root/'research') or output.is_relative_to(root/'original'):
        raise ValueError('Plan and cold backup must be outside research/ and original/')
    output.mkdir(parents=True, exist_ok=False)
    report = inventory(root)
    database = Path(database or root/'working/coverage/evidence-pending.sqlite3').resolve()
    if not database.is_relative_to(root/'working'):
        raise ValueError('Mutable evidence database must remain under ignored working/')
    store = Store(database)
    rows, documents = [], []
    try:
        with Reader(root) as reader:
            pinned = {entry['sha256'] for entry in (reader.resolver.spec or {}).get('archives', [])}
            for item in report['files']:
                if item['role'] != 'generated_research' or item['representation'] == 'sqlite_reference':
                    continue
                name = item['path']
                path = safe_path(root, name)
                before = file_identity(path)
                data = path.read_bytes()
                sha = hashlib.sha256(data).hexdigest()
                origin = None
                try:
                    existing = reader.resolver.find(sha, len(data))
                except ValueError as exc:
                    if not str(exc).startswith('Evidence unavailable;'):
                        raise
                else:
                    class Sink:
                        def write(self, chunk):
                            pass
                    existing.copy(sha, len(data), Sink())
                    if existing.path.stem in pinned:
                        origin = existing.path.stem
                if origin is None:
                    with store.db:
                        if store.put(io.BytesIO(data), len(data)) != sha:
                            raise ValueError('Evidence changed during import: '+name)
                if file_identity(path) != before or hashlib.sha256(path.read_bytes()).hexdigest() != sha:
                    raise ValueError('Source changed during import; bytes retained in SQLite: '+name)
                rows.append({'path': name, 'sha256': sha, 'bytes': len(data), 'before': before,
                             'storage': origin or 'cold_backup',
                             'reference': pointer(sha, len(data)).decode()})
                try:
                    document = json.loads(data)
                except (ValueError, UnicodeError):
                    document = {}
                if not isinstance(document, dict):
                    document = {}
                documents.append((name, sha, len(data),
                    int(document['success']) if isinstance(document.get('success'), bool) else None,
                    document.get('scope') if isinstance(document.get('scope'), str) else None,
                    json.dumps(document.get('schema'))))
            archive_manifest = reader.resolver.spec
        content = json.dumps([(x['path'], x['sha256'], x['bytes']) for x in rows], separators=(',', ':')).encode()
        sweep = hashlib.sha256(content).hexdigest()
        store.db.executescript('''
            CREATE TABLE IF NOT EXISTS json_sweeps (
                id TEXT PRIMARY KEY, root TEXT NOT NULL);
            CREATE TABLE IF NOT EXISTS json_documents (
                sweep TEXT NOT NULL REFERENCES json_sweeps(id), path TEXT NOT NULL,
                sha256 TEXT NOT NULL, size INTEGER NOT NULL, success INTEGER,
                scope TEXT, schema_json TEXT, PRIMARY KEY(sweep,path));
            CREATE INDEX IF NOT EXISTS json_documents_path ON json_documents(path);
        ''')
        with store.db:
            store.db.execute('INSERT OR IGNORE INTO json_sweeps VALUES (?,?)', (sweep, str(root)))
            store.db.executemany('INSERT OR IGNORE INTO json_documents VALUES (?,?,?,?,?,?,?)',
                                 ((sweep, *row) for row in documents))
        backup = output/'evidence-backup.sqlite3'
        destination = sqlite3.connect(backup)
        try:
            store.db.backup(destination)
        finally:
            destination.close()
    finally:
        store.close()
    with backup.open('rb') as stream:
        os.fsync(stream.fileno())
    verified = Store(backup, readonly=True)
    try:
        count = verified.verify()
        sample = next((row for row in rows if row['storage'] == 'cold_backup'), None)
        if sample:
            restored = output/'verified-restore.payload'
            with restored.open('xb') as stream:
                verified.copy(sample['sha256'], sample['bytes'], stream)
                stream.flush()
                os.fsync(stream.fileno())
            if digest_file(restored) != sample['sha256']:
                raise ValueError('Cold backup restore differs from source')
    finally:
        verified.close()
    local = root/'working/retention'/('json-sqlite-sweep-'+sweep+'.json')
    plan = {'schema': 1, 'sweep': sweep, 'root': str(root), 'database': str(database),
            'backup': str(backup), 'backup_sha256': digest_file(backup),
            'backup_objects_verified': count, 'archive_manifest': archive_manifest,
            'inventory': report, 'files': rows,
            'receipts': [str(output/'receipt.json'), str(local)]}
    durable_json(output/'plan.json', plan)
    receipt = {'schema': 1, 'state': 'prepared', 'plan': str(output/'plan.json'),
               'plan_sha256': digest_file(output/'plan.json'), 'backup': str(backup),
               'backup_sha256': plan['backup_sha256'], 'objects_verified': count,
               'files': len(rows), 'completed': 0, 'reclaimed_allocated_bytes': 0}
    for path in plan['receipts']:
        durable_json(path, receipt)
    return plan


def apply(plan_path):
    plan_path = Path(plan_path).resolve()
    plan = json.loads(plan_path.read_text())
    root = Path(plan['root']).resolve()
    backup = Path(plan['backup'])
    if digest_file(backup) != plan['backup_sha256']:
        raise ValueError('Cold backup changed; preserve the plan and investigate')
    current = {item['sha256']: item for item in (config(root) or {}).get('archives', [])}
    if any(current.get(item['sha256']) != item
           for item in (plan['archive_manifest'] or {}).get('archives', [])):
        raise ValueError('Pinned evidence manifest changed since the compaction plan')
    expected = digest_file(plan_path)
    receipts = [json.loads(Path(name).read_text()) for name in plan['receipts']]
    if any(item['plan_sha256'] != expected or item['backup_sha256'] != plan['backup_sha256']
           for item in receipts):
        raise ValueError('Both durable catalog receipts must match before replacement')
    # Verify every original identity before the first replacement. Completed
    # references permit a failed/interrupted run to resume without recopying.
    for row in plan['files']:
        path = safe_path(root, row['path'])
        if path.read_bytes() == row['reference'].encode():
            continue
        if file_identity(path) != row['before'] or digest_file(path) != row['sha256']:
            raise ValueError('Source changed since the reviewed plan: '+row['path'])
    completed, reclaimed, directories = 0, 0, set()
    try:
        for row in plan['files']:
            path = safe_path(root, row['path'])
            reference = row['reference'].encode()
            if path.read_bytes() != reference:
                if file_identity(path) != row['before'] or digest_file(path) != row['sha256']:
                    raise ValueError('Source changed during compaction: '+row['path'])
                fd, temporary = tempfile.mkstemp(prefix=path.name+'.sqlite-', dir=path.parent)
                try:
                    with os.fdopen(fd, 'wb') as output:
                        os.fchmod(output.fileno(), row['before']['mode'])
                        output.write(reference)
                        output.flush()
                        os.fsync(output.fileno())
                    if file_identity(path) != row['before'] or digest_file(path) != row['sha256']:
                        raise ValueError('Source changed before atomic replacement: '+row['path'])
                    os.replace(temporary, path)
                finally:
                    if os.path.exists(temporary):
                        os.unlink(temporary)
            reclaimed += max(0, row['before']['allocated_bytes'] - path.stat().st_blocks*512)
            completed += 1
            directories.add(path.parent)
    finally:
        for directory in directories:
            fd = os.open(directory, os.O_RDONLY | os.O_DIRECTORY)
            try:
                os.fsync(fd)
            finally:
                os.close(fd)
        receipt = dict(receipts[0], state='complete' if completed == len(plan['files']) else 'partial',
                       completed=completed, reclaimed_allocated_bytes=reclaimed,
                       git_index_changed=False)
        for name in plan['receipts']:
            durable_json(name, receipt)
    return receipt
