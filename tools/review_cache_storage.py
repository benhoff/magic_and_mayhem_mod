"""Review-only cache migration; preserve accepted receipts and all draft bytes."""
from contextlib import closing
from pathlib import Path
import fcntl,hashlib,json,os,sqlite3
from coverage_bookkeeping import Workspace,DEFAULT_DB,encoded
from coverage_reviews import load_journal
from sqlite_snapshot_store import Store,archive,filehash,stamp,fsync_directory
from compact_snapshot_activity import busy

EXTENSIONS={"evidence_store_objects","evidence_store_chunks","evidence_store_snapshots",
            "evidence_store_paths","evidence_store_definitions","evidence_store_sources","evidence_store_claims",
            "commit_checks","integration_checks"}

def table_digest(db,name):
    keys=[row[1] for row in sorted(db.execute('pragma table_info("'+name+'")'),key=lambda row:row[5]) if row[5]]
    query='select * from "'+name+'"'+(' order by '+','.join('"'+key+'"' for key in keys) if keys else '')
    h=hashlib.sha256();count=0
    for row in db.execute(query):
        for value in row:
            data=b"B"+value if isinstance(value,bytes) else b"J"+json.dumps(value,ensure_ascii=True,separators=(",",":")).encode()
            h.update(str(len(data)).encode()+b":");h.update(data)
        h.update(b"\n");count+=1
    return count,h.hexdigest()

TABLES={"metadata","receipts","file_changes","behavior_changes","pending_gaps",
        "validation_evidence","drafts","imported_fingerprints"}

def compact(root,local,cold,scratch,local_floor=90*2**30,cold_floor=100*2**30,accept_current_order=False):
    root=Path(root).resolve();source=root/DEFAULT_DB
    if not source.is_file():raise ValueError("No review cache")
    with (source.parent/"bookkeeping.lock").open("a") as lock:
        fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
        before=stamp(source)
        if busy(source):raise ValueError("Review cache has an active consumer")
        journal,paths=load_journal(root)
        identities={p:filehash(root/p) for p in paths}
        reviews={r["id"]:r for r in journal["reviews"]}
        if len(reviews)!=len(journal["reviews"]):raise ValueError("Duplicate journal receipt")
        with closing(sqlite3.connect(source.as_uri()+"?mode=ro&immutable=1",uri=True)) as db:
            if db.execute("pragma user_version").fetchone()!=(1,):raise ValueError("Only legacy v1 caches need migration")
            names={r[0] for r in db.execute("select name from sqlite_master where type='table'")}
            if names-TABLES-EXTENSIONS:raise ValueError("Unknown cache tables need separate preservation review")
            if db.execute("select 1 from sqlite_master where type='view' limit 1").fetchone():
                raise ValueError("Source views require separate dependency review")
            metadata=dict(db.execute("select key,value from metadata"))
            if metadata.get("root")!=str(root):raise ValueError("Cache owner differs from this checkout")
            imported=json.loads(metadata.get("imported_ids","[]"))
            reordered=[r["id"] for r in journal["reviews"] if r["id"] in set(imported)]!=imported
            if reordered and (not accept_current_order or not set(imported)<=reviews.keys()):
                raise ValueError("Imported history removed or reordered")
            # An explicit migration may accept the authoritative merged order.
            # All previous IDs must remain and every common body is checked below.
            pending=[]
            for rid,body in db.execute("select id,record_json from receipts order by sequence"):
                item=json.loads(body)
                if rid!=item["id"]:raise ValueError("Receipt key mismatch")
                if rid in reviews:
                    if encoded(item)!=encoded(reviews[rid]):raise ValueError("Cached history differs from journal")
                else:pending.append(item)
            if "imported_fingerprints" in names:
                for rid,sha in db.execute("select id,sha256 from imported_fingerprints"):
                    if rid not in reviews or hashlib.sha256(encoded(reviews[rid])).hexdigest()!=sha:
                        raise ValueError("Cached fingerprint differs from journal")
            drafts=list(db.execute("select id,context_json from drafts"))
            extensions=[(name,sql) for name,sql in db.execute("select name,sql from sqlite_master where type='table'") if name in EXTENSIONS]
            extension_proof={name:table_digest(db,name) for name,sql in extensions}
            extension_indexes=[sql for name,table,sql in db.execute("select name,tbl_name,sql from sqlite_master where type='index'") if table in EXTENSIONS and sql]
            extension_triggers=[sql for table,sql in db.execute("select tbl_name,sql from sqlite_master where type='trigger'") if table in EXTENSIONS and sql]
            extension_schema_sha256=hashlib.sha256(json.dumps([extensions,extension_indexes,extension_triggers],sort_keys=True).encode()).hexdigest()

        prepared=source.with_name("bookkeeping-v2-prepared.sqlite3")
        if prepared.exists():raise ValueError("Preserve prior cache preparation before retrying")
        workspace=Workspace(root,prepared)
        try:
            workspace.db.execute("attach database ? as legacy",(str(source),))
            with workspace.db:
                for name,sql in extensions:
                    workspace.db.execute(sql)
                    workspace.db.execute('insert into "'+name+'" select * from legacy."'+name+'"')
                for sql in extension_indexes:workspace.db.execute(sql)
                for sql in extension_triggers:workspace.db.execute(sql)
                workspace.sync()
                for item in pending:workspace.insert(item)
                workspace.db.executemany("insert into drafts values(?,?)",drafts)
                for key,value in metadata.items():
                    if key not in ("root","imported_ids"):workspace.set_meta(key,value)
            if any(table_digest(workspace.db,name)!=proof for name,proof in extension_proof.items()):
                raise ValueError("Extension table migration mismatch")
            if workspace.pending(journal)!=pending:raise ValueError("Pending receipt migration mismatch")
            if list(workspace.db.execute("select id,context_json from drafts"))!=drafts:raise ValueError("Draft migration mismatch")
            if workspace.db.execute("pragma integrity_check").fetchone()!=("ok",) or workspace.db.execute("pragma foreign_key_check").fetchall():raise ValueError("Prepared cache integrity failure")
        finally:workspace.close()
        with prepared.open("rb") as out:os.fsync(out.fileno())
        if stamp(source)!=before:raise ValueError("Review cache changed while preparing")
        old_sha=filehash(source)
        result=archive(source,local,cold,scratch,local_floor,cold_floor,False,old_sha)
        proof=dict(result,prepared_sha256=filehash(prepared),prepared_bytes=prepared.stat().st_size,
                   imported_receipts=len(reviews),pending_receipts=len(pending),drafts=len(drafts),
                   accepted_current_journal_order=bool(reordered),
                   journal_files=identities,preserved_extensions=extension_proof,preserved_extension_schema_sha256=extension_schema_sha256,action="replace legacy materialized history with fingerprints")
        for path in (local,cold):
            store=Store(path)
            try:store.receipt(source,old_sha,"cache_replacement_prepared",proof)
            finally:store.close()
        if busy(source) or stamp(source)!=before or filehash(source)!=old_sha:raise ValueError("Review cache changed or acquired a consumer")
        if any(filehash(root/p)!=sha for p,sha in identities.items()):raise ValueError("Journal changed during cache migration")
        os.replace(prepared,source);fsync_directory(source.parent)
        result=dict(proof,success=True,reclaimed_allocated_bytes=before[5]-source.stat().st_blocks*512)
        for path in (local,cold):
            store=Store(path)
            try:store.receipt(source,old_sha,"cache_replacement_complete",result)
            finally:store.close()
        return result
