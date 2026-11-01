"""Evict and lazily restore a checkpointed local review cache.

Pending drafts, accepted receipts and unknown auxiliary tables are backed up as
logical records. Eviction never authorizes a cached audit to approve a tree.
"""
import json
import fcntl
import os
from pathlib import Path
import uuid
import sqlite3
from evidence_objects import configured,logical_checkpoint,restore_logical


def marker(path):
    return Path(str(path)+'.evidence-cache')


def restore_if_evicted(root,path):
    path=Path(path);pin=marker(path)
    if path.exists() or not pin.exists():return False
    if pin.is_symlink():raise ValueError('Unsafe cache checkpoint marker')
    data=json.loads(pin.read_text())
    if data['source']!=str(path.resolve()):raise ValueError('Foreign cache checkpoint marker')
    target=path.with_name(path.name+'.restore-'+uuid.uuid4().hex)
    with configured(root) as pair:
        value=json.loads(pair.read(data['manifest']['sha256'],data['manifest']['size']))
        if value['source']!=str(path.resolve()):raise ValueError('Foreign logical cache checkpoint')
        restore_logical(pair,data['manifest'],target)
    # Do not replace a cache concurrently recreated by another process.
    try:os.link(target,path)
    except FileExistsError:raise ValueError('Cache was recreated during restoration')
    finally:target.unlink(missing_ok=True)
    from evidence_objects import _sync
    _sync(path)
    return True


def connect_cache(root,path):
    from evidence_objects import settings
    if not settings(root) and not marker(path).exists():
        return sqlite3.connect(path,timeout=30)
    lock=Path(root)/'working/coverage/bookkeeping.lock';lock.parent.mkdir(parents=True,exist_ok=True)
    with lock.open('a') as handle:
        try:fcntl.flock(handle,fcntl.LOCK_EX|fcntl.LOCK_NB)
        except BlockingIOError:raise ValueError('Review cache is owned by another workflow operation') from None
        restore_if_evicted(root,path)
        return sqlite3.connect(path,timeout=30)


def evict(root,path):
    lock=Path(root)/'working/coverage/bookkeeping.lock'
    with lock.open('a') as handle:
        fcntl.flock(handle,fcntl.LOCK_EX|fcntl.LOCK_NB)
        return _evict(root,path)


def _evict(root,path):
    from compact_snapshot_activity import busy
    from sqlite_snapshot_store import stamp,filehash
    root=Path(root).resolve();path=Path(path).absolute()
    if path!=root/'working/coverage/bookkeeping.sqlite3' or path.is_symlink():
        raise ValueError('Only the declared per-worktree review cache can be evicted')
    if busy(path) or any(Path(str(path)+s).exists() for s in ('-wal','-shm','-journal')):
        raise ValueError('Review cache is active; retain it')
    before=stamp(path);sha=filehash(path)
    result=logical_checkpoint(root,path)
    result.update(source_identity=before,source_sha256=sha,role='derived_review_cache')
    with configured(root) as pair:
        pair.root('cache_handoff_prepared',path,result)
        if busy(path) or stamp(path)!=before or filehash(path)!=sha:
            raise ValueError('Review cache changed before eviction')
        pin=marker(path)
        from coverage_bookkeeping import atomic_replace
        atomic_replace(pin,(json.dumps(result,sort_keys=True)+'\n').encode(),pin.read_bytes() if pin.exists() else None)
        path.unlink()
        from evidence_objects import _sync
        _sync(pin)
        result['evicted']=True
        pair.root('cache_handoff_evicted',path,result)
    return result
