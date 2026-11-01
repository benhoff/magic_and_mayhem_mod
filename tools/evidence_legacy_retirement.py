"""Replace redundant physical backup containers after a logical handoff review.

This is deliberately limited to evidence-store and review-cache snapshots whose
entire contents match supplied, independently restored logical roots. Historical
snapshot hashes/receipts remain recorded; their physical-page encoding is marked
retired, never silently presented as a byte-exact reconstruction.
"""
from contextlib import closing
import hashlib
import json
import os
from pathlib import Path
import sqlite3
import tempfile
import uuid
from evidence_objects import configured,_sqlite_catalog,_sync
from compact_snapshot_activity import busy
from sqlite_snapshot_store import Store as Snapshots,stamp,filehash,Sink
from coverage_evidence import Store as Evidence
from evidence_consolidation import description


def _json_value(v):
    return {'bytes_hex':v.hex()} if isinstance(v,bytes) else v


def retire(root,catalogs,evidence_manifest,cache_manifest):
    catalogs=[Path(p).absolute() for p in catalogs]
    if len(catalogs)!=2 or catalogs[0].parent.stat().st_dev==catalogs[1].parent.stat().st_dev:
        raise ValueError('Review exactly two independent legacy snapshot catalogs')
    with configured(root) as pair:
        expected_evidence=json.loads(pair.read(evidence_manifest['sha256'],evidence_manifest['size'],1))
        expected_cache=json.loads(pair.read(cache_manifest['sha256'],cache_manifest['size'],1))
        if expected_evidence['kind']!='legacy_evidence_catalog' or expected_cache['kind']!='sqlite_logical_checkpoint':
            raise ValueError('Explicit evidence and cache replacement roots required')
        plans=[];already=[]
        for path in catalogs:
            if path.name!='catalog.sqlite3' or path.parent.name!='snapshots' or not (path.parent/'.retain').is_file() or any(p.is_symlink() for p in (path,*path.parents)) or any(p in path.parts for p in ('original','.git')) or path.resolve() in [s.path.resolve() for s in pair.stores] or path.stat().st_uid!=os.geteuid():
                raise ValueError('Unsafe legacy catalog retirement')
            if busy(path) or any(Path(str(path)+s).exists() for s in ('-wal','-shm','-journal')):
                raise ValueError('Legacy catalog is active')
            before=stamp(path);sha=filehash(path)
            with closing(sqlite3.connect(path.as_uri()+'?mode=ro',uri=True)) as db:
                tables={r[0] for r in db.execute("SELECT name FROM sqlite_master WHERE type='table'")}
                if tables=={'retired_checkpoint'}:
                    previous=json.loads(db.execute('SELECT payload FROM retired_checkpoint').fetchone()[0])
                    if previous['replacements']!=dict(evidence=evidence_manifest,cache=cache_manifest):
                        raise ValueError('Legacy retirement already names different replacements')
                    already.append(dict(path=str(path),already_retired=True));continue
                if tables!={'chunks','snapshots','parts','receipts'}:
                    raise ValueError('Unknown legacy catalog tables require separate preservation')
                snapshots=list(db.execute('SELECT sha256,size,manifest_sha256 FROM snapshots ORDER BY sha256'))
                if len(snapshots)!=2:raise ValueError('Expected exactly the reviewed evidence/cache snapshots')
                metadata=dict(schema=[list(r) for r in db.execute("SELECT type,name,tbl_name,sql FROM sqlite_master WHERE sql IS NOT NULL ORDER BY type,name")],snapshots=snapshots,
                    parts=list(db.execute('SELECT snapshot,ordinal,chunk FROM parts ORDER BY snapshot,ordinal')),
                    receipts=[[ _json_value(v) for v in row] for row in db.execute('SELECT id,path,sha256,state,metadata FROM receipts ORDER BY id')],
                    chunk_encodings=[[h,n,hashlib.sha256(payload).hexdigest()] for h,n,payload in db.execute('SELECT sha256,size,payload FROM chunks ORDER BY sha256')])
            matches=[];archive=Snapshots(path,readonly=True)
            try:
                for snapshot,size,_ in snapshots:
                    archive.copy(snapshot,Sink())
                    with tempfile.TemporaryDirectory(prefix='mm-legacy-handoff-') as tmp:
                        restored=Path(tmp)/'legacy.sqlite3';archive.restore(snapshot,restored)
                        with closing(sqlite3.connect(restored.as_uri()+'?mode=ro',uri=True)) as db:
                            names={r[0] for r in db.execute("SELECT name FROM sqlite_master WHERE type='table'")}
                            if {'objects','chunks','versions','metadata'}<=names:
                                store=Evidence(restored,readonly=True)
                                try:
                                    if json.loads(json.dumps(description(store)))!=expected_evidence['catalog'] or [list(row) for row in store.db.execute("SELECT type,name,tbl_name,sql FROM sqlite_master WHERE sql IS NOT NULL ORDER BY type,name")]!=expected_evidence['source_schema']:
                                        raise ValueError('Legacy evidence snapshot differs from replacement')
                                finally:store.close()
                                matches.append(dict(snapshot=snapshot,kind='legacy_evidence',replacement=evidence_manifest))
                            else:
                                if _sqlite_catalog(db,pair)!=expected_cache['catalog']:
                                    raise ValueError('Legacy cache snapshot differs from replacement')
                                matches.append(dict(snapshot=snapshot,kind='sqlite_logical',replacement=cache_manifest))
            finally:archive.close()
            if {m['kind'] for m in matches}!={'legacy_evidence','sqlite_logical'}:
                raise ValueError('Incomplete legacy replacement review')
            plans.append(dict(source=str(path),identity=before,original_catalog_sha256=sha,metadata=metadata,matches=matches,
                replacements=dict(evidence=evidence_manifest,cache=cache_manifest),source_images_restored_and_logically_compared=True,
                physical_page_images='retired_after_verified_logical_handoff',report_bytes_and_all_source_records_preserved=True))
        prepared=pair.root('legacy_snapshot_retirement_prepared',';'.join(map(str,catalogs)),dict(schema=1,plans=plans,already_retired=already))
        # All sources are reviewed before any catalog replacement.
        results=list(already)
        for plan in plans:
            path=Path(plan['source'])
            if busy(path) or stamp(path)!=plan['identity'] or filehash(path)!=plan['original_catalog_sha256']:
                raise ValueError('Legacy catalog changed before retirement')
            replacement=path.with_name(path.name+'.handoff-'+uuid.uuid4().hex)
            payload=dict(schema=1,kind='retired_snapshot_catalog',prepared=prepared,
                original_catalog_sha256=plan['original_catalog_sha256'],replacements=plan['replacements'],matches=plan['matches'],
                byte_exact_container_restore_available=False,logical_restore_command='tools/evidence-objects.py restore-retired --catalog <this-file> --snapshot <old-sha> --output <new-path>')
            with closing(sqlite3.connect(replacement)) as db:
                db.execute('CREATE TABLE retired_checkpoint(payload TEXT NOT NULL)')
                db.execute('INSERT INTO retired_checkpoint VALUES(?)',(json.dumps(payload,sort_keys=True),));db.commit()
            os.chmod(replacement,0o600);_sync(replacement)
            if busy(path) or stamp(path)!=plan['identity']:raise ValueError('Legacy catalog acquired a consumer')
            os.replace(replacement,path);_sync(path)
            results.append(dict(path=str(path),retired=True,reclaimed_allocated_bytes=plan['identity'][5]-path.stat().st_blocks*512))
        receipt=pair.root('legacy_snapshot_retirement_completed',';'.join(map(str,catalogs)),dict(schema=1,prepared=prepared,results=results))
    return dict(success=True,results=results,receipt=receipt,report_hashes_unchanged=True,physical_page_images_retired=True)
