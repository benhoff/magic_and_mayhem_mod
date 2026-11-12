#!/usr/bin/env python3
"""Initialize, replicate, verify and migrate shared immutable evidence objects."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import tempfile
from evidence_objects import configured, restore_legacy, checkpoint_objects, logical_checkpoint, verify_pair

ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root',type=Path,default=ROOT)
    sub=p.add_subparsers(dest='command',required=True)
    for name in ('init','status','verify','sync'):
        sub.add_parser(name)
    imp=sub.add_parser('import');imp.add_argument('--source',type=Path,required=True);imp.add_argument('--retire',action='store_true')
    restore=sub.add_parser('restore-source');restore.add_argument('--manifest',required=True);restore.add_argument('--size',type=int,required=True);restore.add_argument('--output',type=Path,required=True)
    checkpoint=sub.add_parser('checkpoint');checkpoint.add_argument('--source',type=Path,required=True)
    cache=sub.add_parser('cache-evict');cache.add_argument('--source',type=Path,required=True)
    retired=sub.add_parser('retire-snapshots');retired.add_argument('--catalog',type=Path,action='append',required=True);retired.add_argument('--evidence-manifest',required=True);retired.add_argument('--evidence-size',type=int,required=True);retired.add_argument('--cache-manifest',required=True);retired.add_argument('--cache-size',type=int,required=True)
    retired_restore=sub.add_parser('restore-retired');retired_restore.add_argument('--catalog',type=Path,required=True);retired_restore.add_argument('--snapshot',required=True);retired_restore.add_argument('--output',type=Path,required=True)
    a=p.parse_args();root=a.root.resolve()
    if a.command=='retire-snapshots':
        from evidence_legacy_retirement import retire
        result=retire(root,a.catalog,dict(sha256=a.evidence_manifest,size=a.evidence_size),dict(sha256=a.cache_manifest,size=a.cache_size))
    elif a.command=='restore-retired':
        import sqlite3
        from contextlib import closing
        from evidence_objects import restore_logical
        with closing(sqlite3.connect(a.catalog.resolve().as_uri()+'?mode=ro',uri=True)) as db:
            data=json.loads(db.execute('SELECT payload FROM retired_checkpoint').fetchone()[0])
        match=next(m for m in data['matches'] if m['snapshot']==a.snapshot)
        with configured(root) as pair:
            (restore_legacy if match['kind']=='legacy_evidence' else restore_logical)(pair,match['replacement'],a.output)
        result=dict(success=True,logical_restore_verified=True,byte_exact_container=False,path=str(a.output))
    elif a.command=='sync':
        result=checkpoint_objects(root)
    elif a.command=='checkpoint':
        result=logical_checkpoint(root,a.source)
    elif a.command=='cache-evict':
        from evidence_cache import evict
        result=evict(root,a.source)
    else:
        with configured(root) as pair:
            if a.command=='restore-source':
                result=dict(success=True,path=str(restore_legacy(pair,dict(sha256=a.manifest,size=a.size),a.output)))
            elif a.command=='import':
                from compact_snapshot_activity import busy
                from sqlite_snapshot_store import stamp,filehash
                source=a.source.absolute()
                if 'working' not in source.parts or 'original' in source.parts or source.is_symlink():
                    raise ValueError('Import requires a separate physical ignored evidence store')
                if source.resolve() in [s.path.resolve() for s in pair.stores] or busy(source):
                    raise ValueError('Import source is active or is an authoritative replica')
                before=stamp(source);source_sha=filehash(source)
                result=pair.import_legacy(source)
                with tempfile.TemporaryDirectory(prefix='mm-evidence-source-restore-') as tmp:
                    restore_legacy(pair,result['manifest'],Path(tmp)/'restored.sqlite3')
                result.update(logical_restore_verified=True,source_sha256=source_sha,byte_exact_container=False)
                if a.retire:
                    result['source_identity']=before
                    pair.root('legacy_handoff_prepared',source,result)
                    if busy(source) or stamp(source)!=before or filehash(source)!=source_sha:
                        raise ValueError('Source changed or acquired a consumer before retirement')
                    # The old pathname remains a readable legacy object view.
                    # Source-specific auxiliary tables restore from its root.
                    link=source.with_name(source.name+'.shared-new')
                    if link.exists() or link.is_symlink():raise ValueError('Prior retirement needs review')
                    link.symlink_to(pair.paths[0]);os.replace(link,source)
                    from evidence_objects import _sync
                    _sync(source)
                    result.update(retired=True,reclaimed_allocated_bytes=before[5],alias=str(source))
                    pair.root('legacy_handoff_retired',source,result)
            else:
                if a.command=='verify':
                    verify_pair(pair)
                result=dict(success=True,replicas=[dict(path=str(s.path),allocated=s.path.stat().st_blocks*512,objects=s.db.execute('SELECT count(*) FROM objects').fetchone()[0],blocks=s.db.execute('SELECT count(*) FROM blocks').fetchone()[0]) for s in pair.stores])
    print(json.dumps(result,indent=2))


if __name__=='__main__':
    main()
