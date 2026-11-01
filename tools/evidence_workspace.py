"""Seal stopped execution trees into shared objects and restore them on demand.

Every file, symlink, mode, timestamp and xattr is retained. Directory pins move
into the immutable manifest; a small pinned stub keeps the historical pathname.
Retirement is a separate explicit action after a real, verified restore.
"""
import hashlib
import json
import os
from pathlib import Path
import shutil
import stat
import tempfile
import uuid

MARKER='.evidence-workspace'


def safe(root, path):
    root=Path(root).resolve();path=Path(path).absolute()
    if '..' in path.parts or path==root/'working' or not path.is_relative_to(root/'working'):
        raise ValueError('Execution handoff requires a descendant of this checkout working/')
    if any(p.is_symlink() for p in (path,*path.parents) if p==root or root in p.parents):
        raise ValueError('Execution handoff must not traverse symlinks')
    if (path/'.git').exists() or (path/'AGENTS.md').exists():
        raise ValueError('Source checkouts cannot be execution handoffs')
    return path


def idle(path):
    from experiment_environment import _busy
    if _busy(path):
        raise ValueError('Workspace has an active host/Wine consumer')
    prefix=os.fsencode(str(path))
    for p in Path('/proc').iterdir():
        if not p.name.isdecimal():continue
        try:
            for value in (p/'environ').read_bytes().split(b'\0'):
                if value.startswith(b'WINEPREFIX=') and (value[11:]==prefix or value[11:].startswith(prefix+b'/')):
                    raise ValueError('Workspace has an active Wine prefix')
        except (PermissionError,FileNotFoundError,ProcessLookupError):pass


def scan(path, pair=None):
    rows=[];identities={};allocated=0
    def visit(p):
        nonlocal allocated
        s=p.lstat();rel='.' if p==path else p.relative_to(path).as_posix()
        if p.name in ('.git','AGENTS.md') or (rel==MARKER):
            raise ValueError('Workspace contains a source checkout or existing handoff')
        identities[rel]=(s.st_dev,s.st_ino,s.st_size,s.st_mtime_ns,s.st_ctime_ns)
        allocated+=s.st_blocks*512
        row=dict(path=rel,mode=stat.S_IMODE(s.st_mode),mtime_ns=s.st_mtime_ns,
                 uid=s.st_uid,gid=s.st_gid,
                 xattrs={k:os.getxattr(p,k,follow_symlinks=False).hex() for k in sorted(os.listxattr(p,follow_symlinks=False))})
        if stat.S_ISLNK(s.st_mode):row.update(kind='symlink',target=os.readlink(p))
        elif stat.S_ISDIR(s.st_mode):
            row['kind']='directory'
        elif stat.S_ISREG(s.st_mode):
            if s.st_nlink!=1:raise ValueError('Multiply linked workspace file requires separate review: '+rel)
            row.update(kind='file',size=s.st_size)
            with p.open('rb') as f:
                row['sha256']=pair.put(f,s.st_size) if pair else hashlib.file_digest(f,'sha256').hexdigest()
                a=os.fstat(f.fileno())
                if (a.st_dev,a.st_ino,a.st_size,a.st_mtime_ns,a.st_ctime_ns)!=identities[rel]:
                    raise ValueError('Workspace changed while reading: '+rel)
        else:raise ValueError('Special workspace file requires separate preservation: '+rel)
        rows.append(row)
        if row['kind']=='directory':
            for child in sorted(p.iterdir()):visit(child)
    visit(path)
    return rows,identities,allocated


def _validate(rows):
    names={}
    for row in rows:
        name=row['path'];p=Path(name)
        if name in names or p.is_absolute() or '..' in p.parts or (name!='.' and p.as_posix()!=name):
            raise ValueError('Unsafe or duplicate workspace member')
        if name!='.' and p.parent.as_posix() not in names:
            raise ValueError('Workspace parent order is invalid')
        if name!='.' and names[p.parent.as_posix()]!='directory':
            raise ValueError('Workspace member traverses a symlink')
        if row['kind'] not in ('file','directory','symlink') or row['mode']&~0o7777:
            raise ValueError('Unsupported workspace metadata')
        names[name]=row['kind']
    if not rows or rows[0]['path']!='.' or rows[0]['kind']!='directory':
        raise ValueError('Workspace root missing')


def materialize(pair, manifest, output, replica=1):
    data=json.loads(pair.read(manifest['sha256'],manifest['size'],replica));rows=data['members']
    if data['kind']!='execution_workspace':raise ValueError('Wrong workspace root')
    _validate(rows);output=Path(output)
    if output.exists() or output.is_symlink() or any(p.is_symlink() for p in output.parents):
        raise ValueError('Workspace restore requires a new real directory')
    need=sum(r.get('size',0) for r in rows)
    if shutil.disk_usage(output.parent).free<need+1024**2:
        raise ValueError('Insufficient workspace restore capacity')
    output.mkdir()
    try:
        for row in rows:
            p=output if row['path']=='.' else output/row['path']
            if row['kind']=='directory':
                if row['path']!='.':p.mkdir()
            elif row['kind']=='symlink':p.symlink_to(row['target'])
            else:
                with p.open('xb') as f:
                    pair.stores[replica].copy(row['sha256'],row['size'],f)
                    f.flush();os.fsync(f.fileno())
        for row in reversed(rows):
            p=output if row['path']=='.' else output/row['path']
            if (p.lstat().st_uid,p.lstat().st_gid)!=(row['uid'],row['gid']):
                os.chown(p,row['uid'],row['gid'],follow_symlinks=False)
            if row['kind']!='symlink':os.chmod(p,row['mode'])
            for k,v in row['xattrs'].items():os.setxattr(p,k,bytes.fromhex(v),follow_symlinks=False)
            os.utime(p,ns=(row['mtime_ns'],row['mtime_ns']),follow_symlinks=False)
        if scan(output)[0]!=rows:raise ValueError('Restored workspace differs from its immutable manifest')
    except BaseException:
        # Failed restores remain available for inspection, never the only copy.
        raise
    return output


def seal(root, path, pair, *, retire=False, restore_budget=16*1024**3):
    path=safe(root,path);idle(path)
    with pair.batch():
        rows,identity,allocated=scan(path,pair)
        if scan(path)[1]!=identity:raise ValueError('Workspace changed during sealing')
        data=dict(schema=1,kind='execution_workspace',source=str(path),members=rows,
                  pin_scope='Every member and original .retain contents; restore before historical physical consumers')
        manifest=pair.root('execution_workspace',path,data)
    need=sum(r.get('size',0) for r in rows)
    if need>restore_budget:raise ValueError('Real restore exceeds declared scratch budget; sealed source retained')
    with tempfile.TemporaryDirectory(prefix='mm-workspace-restore-') as tmp:
        materialize(pair,manifest,Path(tmp)/'restored')
    result=dict(success=True,source=str(path),manifest=manifest,files=sum(r['kind']=='file' for r in rows),
                logical_bytes=need,allocated_bytes=allocated,real_restore_verified=True,replicas_verified=2,retired=False)
    pair.root('workspace_verified',path,result)
    if retire:
        idle(path)
        if scan(path)[1]!=identity:raise ValueError('Workspace changed before retirement')
        quarantine=path.with_name(path.name+'.handoff-'+uuid.uuid4().hex)
        path.rename(quarantine)
        created=False
        try:
            idle(path);idle(quarantine)
            moved=scan(quarantine)
            if moved[0]!=rows:raise ValueError('Quarantined workspace changed')
            path.mkdir()
            created=True
            (path/MARKER).write_text(json.dumps(manifest,separators=(',',':'))+'\n')
            (path/'.retain').write_text('Original workspace and pins retained in two verified object stores.\nManifest: '+manifest['sha256']+'\nRestore before historical consumers.\n')
            from evidence_objects import _sync
            _sync(path/MARKER);_sync(path/'.retain')
        except BaseException:
            if created and sorted(p.name for p in path.iterdir()) in ([],[MARKER],[MARKER,'.retain']):
                for p in path.iterdir():p.unlink()
                path.rmdir()
            if not path.exists():quarantine.rename(path)
            raise
        shutil.rmtree(quarantine)
        result.update(retired=True,reclaimed_allocated_bytes=allocated-sum(p.stat().st_blocks*512 for p in path.iterdir()))
        pair.root('workspace_retired',path,result)
    return result


def restore(root,path,pair):
    path=safe(root,path);idle(path)
    if sorted(p.name for p in path.iterdir())!=['.evidence-workspace','.retain']:
        raise ValueError('Workspace stub has unexpected members')
    manifest=json.loads((path/MARKER).read_text())
    if json.loads(pair.read(manifest['sha256'],manifest['size']))['source']!=str(path):
        raise ValueError('Workspace stub belongs to another pathname')
    before={p.name:p.read_bytes() for p in path.iterdir()}
    target=path.with_name(path.name+'.restore-'+uuid.uuid4().hex)
    materialize(pair,manifest,target)
    idle(path)
    if {p.name:p.read_bytes() for p in path.iterdir()}!=before:
        raise ValueError('Workspace stub changed during restoration')
    # Preserve the original stub if switching the verified restoration fails.
    old=path.with_name(path.name+'.stub-'+uuid.uuid4().hex)
    path.rename(old)
    try:target.rename(path)
    except BaseException:old.rename(path);raise
    if {p.name:p.read_bytes() for p in old.iterdir()}==before:shutil.rmtree(old)
    return path


def register_completed(root,paths,report):
    """Queue handoff only; launch never archives or removes execution data."""
    from evidence_objects import configured
    from coverage_evidence import Reader
    root=Path(root).resolve()
    with Reader(root) as reader:
        data=reader.json(report['record'])
        if reader.digest(report['record'])!=report['sha256'] or data.get('private_wine_stopped') is not True:
            raise ValueError('Completed execution lacks stopped producer proof')
    if data.get('prefix_pool') and data['prefix_pool'].get('returned') is not True:
        raise ValueError('Held prefix state requires review before execution handoff')
    sources=dict(data['sources'])
    sources.update(data.get('native_admission',{}).get('native_only',{}).get('sources',{}))
    with configured(root) as pair:
        snapshot={}
        with Reader(root) as reader:
            for name,sha in sources.items():
                p=root/name
                if not p.resolve().is_relative_to(root) or name.startswith('original/'):
                    raise ValueError('Execution source snapshot escapes repository code')
                body=reader.read_bytes(name)
                actual=pair.bytes(body)
                if actual!=sha:raise ValueError('Execution source changed before sealing: '+name)
                snapshot[name]=dict(sha256=sha,size=len(body))
                file_bytes=p.read_bytes()
                if file_bytes!=body:snapshot[name]['worktree_file']=dict(sha256=pair.bytes(file_bytes),size=len(file_bytes))
        source_root=pair.root('execution_sources',report['record'],dict(schema=1,sources=snapshot))
        for path in paths:
            path=safe(root,path)
            pair.root('completed_execution',path,dict(schema=1,owner_root=str(root),path=str(path),report=report,source_snapshot=source_root))


def finish_completed(root):
    from evidence_objects import configured,storage_policy
    root=Path(root).resolve();results=[]
    budget=storage_policy(root).get('workspace_restore_budget_gib',16)
    if type(budget) not in (int,float) or not 0<budget<=16:raise ValueError('Invalid workspace restore budget')
    with configured(root) as pair:
        if not pair.stores[0].db.execute("SELECT 1 FROM sqlite_master WHERE name='retention_roots'").fetchone():
            return []
        rows=list(pair.stores[0].db.execute("SELECT sha256,size FROM retention_roots WHERE kind='completed_execution'"))
        selected={}
        for sha,size in rows:
            data=json.loads(pair.read(sha,size))
            if data['owner_root']==str(root):selected[data['path']]=data
        for name in sorted(selected):
            path=Path(name)
            if (path/MARKER).is_file():continue
            results.append(seal(root,path,pair,retire=True,restore_budget=int(budget*1024**3)))
    return results
