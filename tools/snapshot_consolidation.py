"""Union legacy snapshot catalogs without losing source catalogs or raw snapshots.

The source SQLite page layouts are explicitly retired. Original snapshot bytes,
source schema and every source row remain independently restorable.
"""
from contextlib import closing
import hashlib
import json
import os
from pathlib import Path
import shutil
import sqlite3
import tempfile
import zlib

from compact_snapshot_activity import busy
from evidence_objects import _sync
from sqlite_snapshot_store import Store, Sink, CHUNK, stamp, filehash, fsync_directory


def source_catalog(path):
    path = Path(path).absolute()
    if any(p.is_symlink() for p in (path, *path.parents)) or any(p in path.parts for p in ('original', '.git')):
        raise ValueError('Unsafe source catalog')
    if busy(path) or any(Path(str(path) + suffix).exists() for suffix in ('-wal', '-shm', '-journal')):
        raise ValueError('Active source catalog')
    if path.stat().st_uid != os.geteuid():
        raise ValueError('Source catalog belongs to another owner')
    with closing(sqlite3.connect(path.as_uri() + '?mode=ro', uri=True)) as db:
        tables = {r[0] for r in db.execute("SELECT name FROM sqlite_master WHERE type='table'")}
        if tables != {'chunks', 'snapshots', 'parts', 'receipts'}:
            raise ValueError('Unknown catalog schema requires a separate preservation adapter')
        result = dict(source=str(path), identity=stamp(path), container_sha256=filehash(path),
                      schema=[list(r) for r in db.execute("SELECT type,name,tbl_name,sql FROM sqlite_master WHERE sql IS NOT NULL ORDER BY type,name")],
                      snapshots=list(db.execute('SELECT * FROM snapshots ORDER BY sha256')),
                      receipts=[[v.hex() if isinstance(v, bytes) else v for v in row] for row in db.execute('SELECT * FROM receipts ORDER BY id')],
                      receipt_types=[[type(v).__name__ for v in row] for row in db.execute('SELECT * FROM receipts ORDER BY id')],
                      pragmas={k: db.execute('PRAGMA ' + k).fetchone()[0] for k in ('user_version', 'application_id')},
                      byte_exact_container_restore=False)
    return result


def restore_catalog(union, source, output):
    """Recreate source logical tables, retaining raw snapshot identities exactly."""
    output = Path(output).absolute()
    if output.exists() or any(p.is_symlink() for p in (output, *output.parents)) or 'original' in output.parts:
        raise ValueError('Catalog restore requires a new safe file')
    with closing(sqlite3.connect(Path(union).as_uri() + '?mode=ro', uri=True)) as src:
        row = src.execute('SELECT payload FROM source_catalogs WHERE source=?', (str(source),)).fetchone()
        if row is None:
            raise ValueError('Unknown source catalog')
        plan = json.loads(zlib.decompress(row[0]))
        if shutil.disk_usage(output.parent).free < plan['identity'][2] * 1.1 + 2**20:
            raise ValueError('Insufficient source catalog restore capacity')
        with closing(sqlite3.connect(output)) as dst:
            for kind, name, table, sql in plan['schema']:
                if kind == 'table':
                    dst.execute(sql)
            with dst:
                dst.executemany('INSERT INTO snapshots VALUES(?,?,?)', plan['snapshots'])
                dst.executemany('INSERT INTO parts VALUES(?,?,?)', src.execute('SELECT snapshot,ordinal,chunk FROM source_parts WHERE source=? ORDER BY snapshot,ordinal', (str(source),)))
                dst.executemany('INSERT INTO chunks VALUES(?,?,?)', src.execute('SELECT c.sha256,c.size,c.payload FROM chunks c JOIN source_chunks s ON s.chunk=c.sha256 WHERE s.source=? ORDER BY c.sha256', (str(source),)))
                for values, types in zip(plan['receipts'], plan['receipt_types']):
                    dst.execute('INSERT INTO receipts VALUES(?,?,?,?,?)', [bytes.fromhex(v) if t == 'bytes' else v for v, t in zip(values, types)])
            for kind, name, table, sql in plan['schema']:
                if kind != 'table':
                    dst.execute(sql)
            for key, value in plan['pragmas'].items():
                dst.execute('PRAGMA ' + key + '=' + str(value))
            dst.commit()
            if dst.execute('PRAGMA integrity_check').fetchone() != ('ok',) or dst.execute('PRAGMA foreign_key_check').fetchall():
                raise ValueError('Restored source catalog integrity failure')
    _sync(output)
    return output


def consolidate(sources, destinations, pair, scratch, *, floors=(100 * 2**30, 90 * 2**30), retire=False, resume=False):
    sources = [Path(p).absolute() for p in sources]
    destinations = [Path(p).absolute() for p in destinations]
    if len(destinations) != 2 or destinations[0].parent.stat().st_dev == destinations[1].parent.stat().st_dev:
        raise ValueError('Two independent destination filesystems required')
    if len(set(sources + destinations)) != len(sources) + 2:
        raise ValueError('Distinct source and destination paths required')
    plans = [source_catalog(p) for p in sources]
    stores = []
    try:
        for path, floor in zip(destinations, floors):
            if any(p.is_symlink() for p in (path, *path.parents)) or any(p in path.parts for p in ('original', '.git')):
                raise ValueError('Union output must be a safe file')
            if path.exists():
                if not resume or path.stat().st_uid != os.geteuid() or path.stat().st_nlink != 1 or busy(path):
                    raise ValueError('Existing union requires explicit idle owned resume')
                # SQLite recovers its own interrupted rollback journal when the
                # recognized private union is reopened; no source is retired.
                with closing(sqlite3.connect(path)) as previous:
                    names = {r[0] for r in previous.execute("SELECT name FROM sqlite_master WHERE type='table'")}
                    if names != {'chunks', 'snapshots', 'parts', 'receipts', 'source_catalogs', 'source_chunks', 'source_parts'}:
                        raise ValueError('Unknown existing union schema')
                    expected = {p['source']: p for p in plans}
                    for source, payload in previous.execute('SELECT source,payload FROM source_catalogs'):
                        if source not in expected or json.loads(zlib.decompress(payload)) != json.loads(json.dumps(expected[source])):
                            raise ValueError('Resumed source identity differs')
            if shutil.disk_usage(path.parent).free < floor + 2**20:
                raise ValueError('Union destination reserve crossed')
            s = Store(path, minimum_free=floor)
            s.db.executescript('''
              CREATE TABLE IF NOT EXISTS source_catalogs(source TEXT PRIMARY KEY,payload BLOB NOT NULL);
              CREATE TABLE IF NOT EXISTS source_chunks(source TEXT,chunk TEXT,PRIMARY KEY(source,chunk));
              CREATE TABLE IF NOT EXISTS source_parts(source TEXT,snapshot TEXT,ordinal INTEGER,chunk TEXT,PRIMARY KEY(source,snapshot,ordinal));
            ''')
            stores.append(s)
        for plan in plans:
            source = Path(plan['source'])
            if all(s.db.execute('SELECT 1 FROM source_catalogs WHERE source=?', (str(source),)).fetchone() for s in stores):
                # Completed source metadata was one atomic transaction. Full
                # union hashes and real original-catalog comparisons still run.
                continue
            with closing(sqlite3.connect(source.as_uri() + '?mode=ro', uri=True)) as db:
                # Bound each transaction and retain original compressed encodings.
                cursor = db.execute('SELECT sha256,size,payload FROM chunks ORDER BY sha256')
                while True:
                    rows = cursor.fetchmany(256)
                    if not rows:
                        break
                    for sha, size, payload in rows:
                        dec = zlib.decompressobj()
                        raw = dec.decompress(payload, CHUNK + 1)
                        if not 0 < size <= CHUNK or len(raw) != size or not dec.eof or dec.unused_data or dec.unconsumed_tail or hashlib.sha256(raw).hexdigest() != sha:
                            raise ValueError('Corrupt source chunk')
                    for store in stores:
                        need = sum(len(r[2]) + 12288 for r in rows) + 2**20
                        if shutil.disk_usage(store.path.parent).free - need < store.minimum_free:
                            raise ValueError('Union write would breach reserve')
                        with store.db:
                            for row in rows:
                                old = store.db.execute('SELECT sha256,size,payload FROM chunks WHERE sha256=?', (row[0],)).fetchone()
                                if old is not None and old != row:
                                    raise ValueError('Different original chunk encoding requires explicit preservation')
                                store.db.execute('INSERT OR IGNORE INTO chunks VALUES(?,?,?)', row)
                                store.db.execute('INSERT OR IGNORE INTO source_chunks VALUES(?,?)', (str(source), row[0]))
                for store in stores:
                    with store.db:
                        store.db.execute('INSERT OR IGNORE INTO source_catalogs VALUES(?,?)', (str(source), zlib.compress(json.dumps(plan, separators=(',', ':')).encode(), 6)))
                        for sha, size, manifest in plan['snapshots']:
                            old = store.db.execute('SELECT * FROM snapshots WHERE sha256=?', (sha,)).fetchone()
                            if old is not None and old != (sha, size, manifest):
                                raise ValueError('Conflicting snapshot identity')
                            store.db.execute('INSERT OR IGNORE INTO snapshots VALUES(?,?,?)', (sha, size, manifest))
                        cursor = db.execute('SELECT snapshot,ordinal,chunk FROM parts ORDER BY snapshot,ordinal')
                        while True:
                            rows = cursor.fetchmany(8192)
                            if not rows:
                                break
                            store.db.executemany('INSERT OR IGNORE INTO parts VALUES(?,?,?)', rows)
                            store.db.executemany('INSERT OR IGNORE INTO source_parts VALUES(?,?,?,?)', ((str(source), *r) for r in rows))
                        if store.db.execute('SELECT 1 FROM source_parts s JOIN parts p USING(snapshot,ordinal) WHERE s.source=? AND s.chunk!=p.chunk LIMIT 1', (str(source),)).fetchone():
                            raise ValueError('Conflicting snapshot part')
                        if store.db.execute('SELECT count(*) FROM source_chunks WHERE source=?', (str(source),)).fetchone()[0] != db.execute('SELECT count(*) FROM chunks').fetchone()[0] or store.db.execute('SELECT count(*) FROM source_parts WHERE source=?', (str(source),)).fetchone()[0] != db.execute('SELECT count(*) FROM parts').fetchone()[0]:
                            raise ValueError('Resumed source membership differs')
            if stamp(source) != plan['identity']:
                raise ValueError('Source catalog changed during consolidation')
        for store in stores:
            _sync(store.path)
            if store.db.execute('PRAGMA integrity_check').fetchone() != ('ok',) or store.db.execute('PRAGMA foreign_key_check').fetchall():
                raise ValueError('Union integrity failure')
            for sha, in store.db.execute('SELECT sha256 FROM snapshots ORDER BY sha256'):
                store.copy(sha, Sink())
        # Actually restore the largest snapshot and every snapshot unique to a source.
        counts = {}
        for plan in plans:
            for sha, size, _ in plan['snapshots']:
                counts[sha] = counts.get(sha, 0) + 1
        witnesses = {max((r for p in plans for r in p['snapshots']), key=lambda r: r[1])[0]}
        witnesses.update(sha for sha, n in counts.items() if n == 1)
        # Distinct sources may be independent copies of the same catalog. This
        # bounded witness set is sufficient alongside full per-snapshot hashes.
        restored = []
        with tempfile.TemporaryDirectory(prefix='mm-union-restore-', dir=scratch) as tmp:
            for sha in sorted(witnesses):
                path = Path(tmp) / (sha + '.sqlite3')
                stores[1].restore(sha, path)
                path.unlink()
                restored.append(sha)
            # A real reconstruction of each distinct source catalog must retain
            # its exact schema and all logical rows, including auxiliary receipts.
            checked = set()
            for plan in plans:
                if plan['container_sha256'] in checked:
                    continue
                checked.add(plan['container_sha256'])
                path = Path(tmp) / 'source-catalog.sqlite3'
                restore_catalog(destinations[1], plan['source'], path)
                with closing(sqlite3.connect(Path(plan['source']).as_uri() + '?mode=ro', uri=True)) as a, closing(sqlite3.connect(path.as_uri() + '?mode=ro', uri=True)) as b:
                    for table, order in [('chunks', 'sha256'), ('snapshots', 'sha256'), ('parts', 'snapshot,ordinal'), ('receipts', 'id')]:
                        left, right = a.execute('SELECT * FROM ' + table + ' ORDER BY ' + order), b.execute('SELECT * FROM ' + table + ' ORDER BY ' + order)
                        while True:
                            x, y = left.fetchmany(256), right.fetchmany(256)
                            if x != y:
                                raise ValueError('Restored source catalog rows differ: ' + table)
                            if not x:
                                break
                    schema = lambda d: d.execute("SELECT type,name,tbl_name,sql FROM sqlite_master WHERE sql IS NOT NULL ORDER BY type,name").fetchall()
                    if schema(a) != schema(b):
                        raise ValueError('Restored source catalog schema differs')
                path.unlink()
        result = dict(schema=1, kind='snapshot_catalog_union', sources=plans, resumed=resume,
                      destinations=list(map(str, destinations)), snapshots=stores[0].db.execute('SELECT count(*) FROM snapshots').fetchone()[0],
                      all_snapshot_hashes_verified_in_both_copies=True, actual_restores=restored,
                      all_distinct_source_catalogs_restored_and_compared=True,
                      original_container_encodings='retired only after explicit source-specific logical handoff',
                      reclaimed_allocated_bytes=0, retired=[])
        proof = pair.root('snapshot_catalog_union_verified', ';'.join(map(str, sources)), result)
        for store in stores:
            store.receipt(';'.join(map(str, sources)), proof['sha256'], 'union_verified', result)
        if retire:
            # Original catalog receipts and metadata are in each union, and the
            # replacement explicitly promises logical rather than page restoration.
            for plan in plans:
                path = Path(plan['source'])
                if busy(path) or stamp(path) != plan['identity'] or filehash(path) != plan['container_sha256']:
                    raise ValueError('Source changed or acquired a consumer before retirement')
                alias = path.with_name(path.name + '.union-alias')
                if alias.exists() or alias.is_symlink():
                    raise ValueError('Unreviewed prior alias')
                # Keep primary and replica historical paths on their own device.
                target = next((p for p in destinations if p.stat().st_dev == path.stat().st_dev), destinations[0])
                alias.symlink_to(target)
                os.replace(alias, path)
                fsync_directory(path.parent)
                result['retired'].append(str(path))
                result['reclaimed_allocated_bytes'] += plan['identity'][5]
        result['proof'] = proof
        result['receipt'] = pair.root('snapshot_catalog_union_completed', ';'.join(map(str, sources)), result)
        return result
    finally:
        for store in stores:
            store.close()
