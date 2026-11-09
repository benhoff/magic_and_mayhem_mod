"""Read only the explicitly assigned campaign owner's admitted sessions."""
import hashlib
import json
from pathlib import Path


def admitted_sessions(root, owner, prefix):
    root=Path(root).resolve();owner=Path(owner);prefix=Path(prefix).resolve()
    if not owner.is_absolute() or not owner.is_relative_to(root/'working') or owner.is_symlink():
        raise ValueError('Campaign owner must be an explicit owned working directory')
    if not prefix.is_relative_to(root/'working'):
        raise ValueError('Campaign prefix escaped owned working storage')
    if not owner.exists():return []
    if owner.resolve()!=owner or not owner.is_dir():raise ValueError('Aliased campaign owner')
    journal=owner/'sessions'
    if not journal.exists():return []
    if journal.is_symlink() or not journal.is_dir():raise ValueError('Aliased campaign session journal')
    files=sorted(journal.glob('*.json'))
    if len(files)>8:raise ValueError('Campaign generation journal exceeds bounded scope')
    result=[]
    for path in files:
        if path.is_symlink() or path.stat().st_size>2*1024**2:
            raise ValueError('Invalid campaign admission receipt')
        value=json.loads(path.read_bytes())
        declared=Path(value['prefix'])
        if not declared.is_absolute() or declared.resolve()!=prefix:
            raise ValueError('Campaign admission belongs to another prefix')
        session=Path(value['session'])
        if (not session.is_absolute() or session.parent!=root/'working/experiments/menu-observer'
                or not session.name.startswith('run-') or not session.is_dir()
                or session.resolve()!=session or path.name!=hashlib.sha256(str(session).encode()).hexdigest()+'.json'):
            raise ValueError('Campaign admission session escaped its exact identity')
        result.append(session)
    if len(set(result))!=len(result):raise ValueError('Repeated campaign session identity')
    return result


def single_session(root, owner, prefix):
    sessions=admitted_sessions(root,owner,prefix)
    if len(sessions)!=1:raise ValueError('Campaign requires one explicitly owned player generation')
    return sessions[0]
