#!/usr/bin/env python3
"""Explicit private SSH backup of verified SQLite snapshots; never run by a commit hook."""
import argparse
from contextlib import closing
import json
import os
from pathlib import Path, PurePosixPath
import re
import shlex
import sqlite3
import subprocess
import sys
import uuid

from coverage_evidence import cache_root, config, digest_file
from evidence_local import backup_sqlite

REMOTE = r'''
import hashlib,json,os,pathlib,re,sqlite3,sys
os.umask(0o077)
root=pathlib.Path(sys.argv[1]);req=json.load(sys.stdin)
assert root.is_absolute() and '..' not in root.parts and not root.is_symlink()
for name in ('objects','incoming','sets'):
 p=root/name;assert not p.is_symlink();p.mkdir(parents=True,exist_ok=True,mode=0o700)
def valid(item,path):
 assert not path.is_symlink() and path.is_file(),str(path)
 assert path.stat().st_size==item['bytes'],'Size mismatch: '+str(path)
 with path.open('rb') as f:assert hashlib.file_digest(f,'sha256').hexdigest()==item['sha256'],'Checksum mismatch: '+str(path)
def target(item,kind='objects'):
 assert re.fullmatch('[0-9a-f]{64}',item['sha256'])
 return root/kind/(item['sha256']+'.sqlite3')
missing=[]
for item in req['files']:
 p=target(item)
 if p.exists():valid(item,p)
 else:missing.append(item['sha256'])
if req['action']=='complete':
 assert re.fullmatch('[0-9a-f]{32}',req['id'])
 for item in req['files']+[req['manifest']]:
  kind='sets' if item is req['manifest'] else 'objects';p=target(item,kind)
  incoming=root/'incoming'/(req['id']+'-'+item['sha256']+'.sqlite3')
  if p.exists():
   valid(item,p)
   if incoming.exists():valid(item,incoming);incoming.unlink()
   continue
  valid(item,incoming)
  with sqlite3.connect(incoming.as_uri()+'?mode=ro',uri=True) as db:
   assert db.execute('PRAGMA integrity_check').fetchone()==('ok',),'SQLite integrity failure'
   assert not db.execute('PRAGMA foreign_key_check').fetchall(),'Foreign-key failure'
  os.chmod(incoming,0o600)
  with incoming.open('rb') as f:os.fsync(f.fileno())
  try:os.link(incoming,p)
  except FileExistsError:valid(item,p)
  incoming.unlink()
 for name in ('objects','sets'):
  fd=os.open(root/name,os.O_RDONLY|os.O_DIRECTORY);os.fsync(fd);os.close(fd)
 missing=[]
elif req['action']!='probe':raise ValueError('Unknown backup action')
print(json.dumps({'missing':missing,'verified':req['action']=='complete'}))
'''


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args], stderr=subprocess.PIPE).decode().strip()


def main_root(root):
    return Path(git(root, 'rev-parse', '--path-format=absolute', '--git-common-dir')).parent


def destination(root):
    values = []
    for key in ('mm.evidenceBackupHost', 'mm.evidenceBackupDirectory'):
        p = subprocess.run(['git', '-C', str(root), 'config', '--get', key], capture_output=True, text=True)
        values.append(p.stdout.strip() if p.returncode == 0 else '')
    host, directory = values
    if not re.fullmatch(r'[A-Za-z0-9_][A-Za-z0-9_.@-]*', host):
        raise ValueError('Configure an SSH alias in mm.evidenceBackupHost')
    path = PurePosixPath(directory)
    if not path.is_absolute() or '..' in path.parts or str(path) == '/':
        raise ValueError('Configure an absolute project directory in mm.evidenceBackupDirectory')
    return host, str(path)


def state_directory(root):
    path = main_root(root)/'working/coverage/private-backups'
    path.mkdir(parents=True, exist_ok=True)
    if not (path/'.retain').exists():
        (path/'.retain').write_text('Private backup snapshots, set manifests and pending retries. No automatic pruning.\n')
    return path


def catalog(root):
    db = sqlite3.connect(state_directory(root)/'catalog.sqlite3', timeout=30)
    db.execute('CREATE TABLE IF NOT EXISTS sets (id TEXT PRIMARY KEY, commit_id TEXT NOT NULL, '
               'manifest TEXT NOT NULL, host TEXT NOT NULL, directory TEXT NOT NULL, '
               "state TEXT NOT NULL DEFAULT 'pending', error TEXT)")
    return db


def snapshots(root, review=None):
    main = main_root(root)
    sources = [main/'working/coverage/evidence-pending.sqlite3', main/'working/coverage/bookkeeping.sqlite3',
               root/'working/coverage/bookkeeping.sqlite3']
    if review:
        sources.append(Path(review))
    paths = list(dict.fromkeys(p.resolve() for p in sources if p.is_file()))
    if not paths:
        raise ValueError('No local databases found')
    return [backup_sqlite(path, state_directory(root)/'snapshots') for path in paths]


def prepare(root, commit, backups):
    """Persist an immutable commit-linked set before any network access."""
    where = destination(root)
    commit = git(root, 'rev-parse', '--verify', commit+'^{commit}')
    rows = []
    for item in backups:
        path = Path(item['path']).resolve()
        if digest_file(path) != item['sha256']:
            raise ValueError('Backup changed since verification: '+str(path))
        rows.append((item['sha256'], path.stat().st_size, str(path), 'snapshot', item.get('source','')))
    for archive in (config(root) or {}).get('archives', []):
        path = cache_root(root)/(archive['sha256']+'.sqlite3')
        if not path.is_file() or digest_file(path) != archive['sha256']:
            raise ValueError('Pinned archive unavailable locally: '+archive['sha256'])
        rows.append((archive['sha256'], path.stat().st_size, str(path), 'pinned_archive', str(path)))
    if not rows:
        raise ValueError('No databases to back up')
    sid = uuid.uuid4().hex
    manifest = state_directory(root)/(sid+'.sqlite3')
    with closing(sqlite3.connect(manifest)) as db, db:
        db.execute('CREATE TABLE backup (commit_id TEXT NOT NULL, created_utc TEXT NOT NULL)')
        db.execute("INSERT INTO backup VALUES (?,datetime('now'))", (commit,))
        db.execute('CREATE TABLE files (sha256 TEXT PRIMARY KEY, bytes INTEGER NOT NULL, '
                   'local_path TEXT NOT NULL, role TEXT NOT NULL, source TEXT NOT NULL)')
        db.executemany('INSERT OR IGNORE INTO files VALUES (?,?,?,?,?)', rows)
    with manifest.open('rb') as f:os.fsync(f.fileno())
    with closing(catalog(root)) as db, db:
        db.execute('INSERT INTO sets(id,commit_id,manifest,host,directory) VALUES (?,?,?,?,?)',
                   (sid, commit, str(manifest), *where))
    return sid


def remote(host, directory, request):
    command = 'python3 -c '+shlex.quote(REMOTE)+' '+shlex.quote(directory)
    p = subprocess.run(['ssh', '-o', 'BatchMode=yes', '-o', 'ConnectTimeout=10', host, command],
                       input=json.dumps(request), capture_output=True, text=True, timeout=300)
    if p.returncode:
        raise ValueError('Private verification failed: '+p.stderr.strip())
    return json.loads(p.stdout)


def upload(root, sid):
    with closing(catalog(root)) as db:
        row = db.execute('SELECT manifest,host,directory FROM sets WHERE id=?', (sid,)).fetchone()
    if row is None:raise ValueError('Unknown backup set: '+sid)
    manifest, host, directory = row
    if destination(root) != (host, directory):
        raise ValueError('Pending set has a different destination; review before retrying')
    try:
        with closing(sqlite3.connect(Path(manifest).as_uri()+'?mode=ro', uri=True)) as db:
            files = [dict(sha256=s, bytes=n, path=p) for s,n,p in db.execute('SELECT sha256,bytes,local_path FROM files')]
        for item in files:
            if digest_file(item['path']) != item['sha256']:
                raise ValueError('Retained snapshot changed: '+item['path'])
        needed = set(remote(host, directory, dict(action='probe', files=files))['missing'])
        manifest_item = dict(sha256=digest_file(manifest), bytes=Path(manifest).stat().st_size, path=manifest)
        for item in [f for f in files if f['sha256'] in needed]+[manifest_item]:
            target = host+':'+directory+'/incoming/'+sid+'-'+item['sha256']+'.sqlite3'
            subprocess.run(['rsync', '--protect-args', '--chmod=D700,F600', '-e',
                            'ssh -o BatchMode=yes -o ConnectTimeout=10', '--', item['path'], target],
                           check=True, timeout=1800, stdout=subprocess.DEVNULL)
        result = remote(host, directory, dict(action='complete', id=sid, files=files, manifest=manifest_item))
        if not result.get('verified'):raise ValueError('Remote did not confirm this backup set')
        with closing(catalog(root)) as db, db:
            db.execute("UPDATE sets SET state='verified',error=NULL WHERE id=?", (sid,))
        return dict(state='verified', id=sid, destination=host+':'+directory,
                    manifest=manifest_item['sha256']+'.sqlite3', databases=len(files),
                    uploaded_databases=len(needed), bytes=sum(f['bytes'] for f in files))
    except (OSError, ValueError, sqlite3.Error, subprocess.SubprocessError) as exc:
        with closing(catalog(root)) as db, db:
            db.execute("UPDATE sets SET state='pending',error=? WHERE id=?", (str(exc), sid))
        raise


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    mode = p.add_mutually_exclusive_group()
    mode.add_argument('--retry', action='store_true')
    mode.add_argument('--status', action='store_true')
    p.add_argument('--review-db', type=Path)
    args = p.parse_args();root = args.root.resolve()
    if args.status or args.retry:
        with closing(catalog(root)) as db:
            rows = db.execute('SELECT id,commit_id,state,error FROM sets ORDER BY rowid').fetchall()
        if args.status:
            print(json.dumps([dict(zip(('id','commit','state','error'),r)) for r in rows], indent=2));return
        for sid,_,state,_ in rows:
            if state=='pending':print(json.dumps(upload(root,sid)),flush=True)
        return
    destination(root)
    backups=snapshots(root,args.review_db)
    # Name the source separately from its immutable snapshot for restoration.
    with closing(sqlite3.connect(state_directory(root)/'snapshots/catalog.sqlite3')) as db:
        for item in backups:
            item['source']=db.execute('SELECT source FROM backups WHERE sha256=?',(item['sha256'],)).fetchone()[0]
    print(json.dumps(upload(root,prepare(root,'HEAD',backups)),indent=2))

if __name__=='__main__':
    try:main()
    except (OSError,ValueError,sqlite3.Error,subprocess.SubprocessError) as exc:
        print(str(exc),file=sys.stderr);sys.exit(1)
