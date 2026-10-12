"""Own original-format saves across disposable native-menu installations.

The original engine serializes only into its isolated game/Save directory.
This store seeds once from the initial installation and becomes authoritative
for subsequent native sessions. Each exit merges only actual changes against
that session's baseline under a lock; conflicting external edits are preserved.
It does not decode, validate compatibility, or replace original serialization.
"""
from contextlib import contextmanager
import fcntl
import hashlib
import json
import os
from pathlib import Path
import tempfile

BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
MAX_FILES = 100
MAX_BYTES = 256 * 1024 * 1024

def _name(name):
    if not name.lower().endswith('.sav') or len(name) > 255 or any(ord(c)<32 or c in '/\\:"<>|?*' for c in name):
        raise ValueError('Unsupported save filename: '+repr(name))
    return name

def _files(directory):
    if directory.is_symlink(): raise ValueError('Save directory is a symlink')
    files = {}
    if not directory.exists(): return files
    for p in directory.iterdir():
        if p.suffix.lower() != '.sav': continue
        _name(p.name)
        if p.is_symlink() or not p.is_file(): raise ValueError('Save is not a regular owned file: '+str(p))
        size = p.stat().st_size
        if not 0 < size <= MAX_BYTES: raise ValueError('Save file size is outside the bounded store policy: '+p.name)
        if p.name.casefold() in {n.casefold() for n in files}: raise ValueError('Case-conflicting save names')
        files[p.name] = hashlib.sha256(p.read_bytes()).hexdigest()
    if len(files) > MAX_FILES: raise ValueError('Too many saves for original menu enumeration')
    return files

def _atomic(path, data):
    fd, temporary = tempfile.mkstemp(prefix='.save-store-', dir=path.parent)
    try:
        with os.fdopen(fd, 'wb') as out:
            out.write(data); out.flush(); os.fsync(out.fileno())
        os.replace(temporary, path)
    finally:
        if os.path.exists(temporary): os.unlink(temporary)

@contextmanager
def _lock(store):
    if store.is_symlink(): raise ValueError('Save store is a symlink')
    store.mkdir(parents=True, exist_ok=True)
    lock=store/'.lock'
    if lock.is_symlink(): raise ValueError('Save store lock is a symlink')
    with lock.open('a+b') as held:
        fcntl.flock(held.fileno(),fcntl.LOCK_EX)
        yield

def stage(game, store):
    """Initialize/seed once, then copy only the authoritative store into a session."""
    game,store=Path(game),Path(store)
    target=game/'Save'
    if target.is_symlink(): raise ValueError('Staged Save directory is a symlink')
    target.mkdir(parents=True,exist_ok=True)
    if target.resolve()==store.resolve(): raise ValueError('Staged saves alias the persistent store')
    with _lock(store):
        metadata=store/'store.json'
        if metadata.is_symlink(): raise ValueError('Save metadata is a symlink')
        if not metadata.exists():
            initial=_files(target)
            if _files(store): raise ValueError('Uninitialized nonempty store; preserve files for explicit import')
            for name in initial: _atomic(store/name,(target/name).read_bytes())
            _atomic(metadata,(json.dumps({'schema':1,'build':BUILD,'initial_files':initial},indent=2)+'\n').encode())
            _atomic(store/'.retain',b'Native menu user saves; preserve until explicit user cleanup.\n')
        config=json.loads(metadata.read_text())
        if config.get('schema')!=1 or config.get('build')!=BUILD: raise ValueError('Save store belongs to another schema/build')
        files=_files(store)
        for name in _files(target):
            if name not in files: (target/name).unlink()
        for name in files: _atomic(target/name,(store/name).read_bytes())
        return {'schema':1,'build':BUILD,'store':str(store.resolve()),'game':str(game.resolve()),'baseline':files}

def commit(state):
    """Merge a completed session without replacing concurrent store changes."""
    if state.get('schema')!=1 or state.get('build')!=BUILD: raise ValueError('Invalid save session baseline')
    store=Path(state['store']);target=Path(state['game'])/'Save';baseline=state['baseline']
    final=_files(target)
    changed={n for n in set(baseline)|set(final) if baseline.get(n)!=final.get(n)}
    with _lock(store):
        current=_files(store)
        conflicts=[n for n in changed if current.get(n)!=baseline.get(n)]
        if conflicts: raise ValueError('Concurrent save changes preserved; session files retained at '+str(target)+': '+', '.join(sorted(conflicts)))
        # Validate the complete merge and case namespace before changing any file.
        merged={**current,**{n:final[n] for n in changed if n in final}}
        for n in changed:
            if n not in final: merged.pop(n,None)
        if len(merged)>MAX_FILES or len({n.casefold() for n in merged})!=len(merged): raise ValueError('Merged save namespace exceeds original menu limits')
        for name in sorted(changed):
            if name in final: _atomic(store/name,(target/name).read_bytes())
            elif (store/name).exists(): (store/name).unlink()
        fd=os.open(store,os.O_RDONLY|os.O_DIRECTORY)
        try: os.fsync(fd)
        finally: os.close(fd)
    return {'changed':sorted(changed),'files':merged,'scope':'Atomic per-file copying and locked conflict checks; not a multi-file crash transaction or original serializer/durability equivalence'}
