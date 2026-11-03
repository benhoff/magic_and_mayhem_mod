"""One fingerprinted input set per Git project; private mutable game/prefix loans."""
from contextlib import closing
import hashlib
import json
from pathlib import Path
import shutil
import sqlite3
import uuid
from experiment_environment import _owner, _scan, _digest, _idle, _lock, _copy


def paths(root):
    root=Path(root).resolve();database=root/'working/check-inputs.sqlite3'
    if not database.exists():return {}
    if database.is_symlink():raise ValueError('Shared input catalog is redirected')
    with closing(sqlite3.connect(database.as_uri()+'?mode=ro',uri=True)) as db:
        rows=list(db.execute('SELECT kind,path,manifest,sha256 FROM inputs'))
    result={}
    for kind,name,body,sha in rows:
        if hashlib.sha256(body).hexdigest()!=sha:raise ValueError('Shared input catalog hash differs')
        path=Path(name)
        if not path.is_relative_to(_owner(root)/'working/check-inputs') or path.is_symlink():
            raise ValueError('Shared input escapes the owned input store')
        if _scan(path)[0]!=json.loads(body):raise ValueError('Shared input changed: '+kind)
        result[kind]=path
    return result


def provision(root,sources,*,aliases=True):
    root=Path(root).resolve();owner=_owner(root);base=owner/'working/check-inputs'
    if base.is_symlink():raise ValueError('Shared input store is redirected')
    base.mkdir(parents=True,exist_ok=True)
    (base/'.retain').write_text('Shared game/assets and admitted prefix inputs. Preserve catalogs, sources and pool consumers.\n')
    database=root/'working/check-inputs.sqlite3';database.parent.mkdir(parents=True,exist_ok=True)
    if database.is_symlink():raise ValueError('Shared input catalog is redirected')
    result={}
    with _lock(base/'setup.lock'),closing(sqlite3.connect(database)) as db:
        db.execute('CREATE TABLE IF NOT EXISTS inputs(kind TEXT PRIMARY KEY,path TEXT,manifest BLOB,sha256 TEXT)')
        for kind,source in sources.items():
            if kind not in ('game','assets','prefix'):raise ValueError('Unknown shared input role')
            source=Path(source).resolve()
            if not source.is_relative_to(owner/'working') or 'original' in source.parts:
                raise ValueError('Shared inputs require owned working sources')
            _idle(source,'wineprefix' if kind=='prefix' else 'game')
            before,_=_scan(source);signature=_digest(before)
            target=source if source.parent==base and source.name.startswith(kind+'-') else base/(kind+'-'+signature[:24])
            old=db.execute('SELECT path FROM inputs WHERE kind=?',(kind,)).fetchone()
            if old and old!=(str(target),):raise ValueError('Existing shared input binding differs; retain it')
            if not target.exists():
                stage=base/('.staging-'+uuid.uuid4().hex)
                _copy(source,stage,root)
                if _scan(source)[0]!=before or _scan(stage)[0]!=before:
                    raise ValueError('Input changed while provisioning; staging retained')
                # Only the copy's ownership marker changes; original prefix stays exact.
                if kind=='prefix':
                    from directplay_dx6_launch import prefix_identity,check_prefix
                    marker=source/'.mnm-directplay-dx6.json'
                    if json.loads(marker.read_text())!=prefix_identity(source):
                        raise ValueError('Prefix source has no matching admission')
                    check_prefix(stage)
                    (stage/'.mnm-directplay-dx6.json').write_text(json.dumps(prefix_identity(target),indent=2)+'\n')
                stage.rename(target)
            current,_=_scan(target)
            if kind!='prefix' and current!=before:raise ValueError('Shared input target differs')
            if kind=='prefix':
                from directplay_dx6_launch import prefix_identity,check_prefix
                check_prefix(target)
                expected=dict(before);actual=dict(current)
                expected.pop('.mnm-directplay-dx6.json');actual.pop('.mnm-directplay-dx6.json')
                if actual!=expected or json.loads((target/'.mnm-directplay-dx6.json').read_text())!=prefix_identity(target):
                    raise ValueError('Shared prefix copy differs')
            body=json.dumps(current,sort_keys=True,separators=(',',':')).encode();sha=hashlib.sha256(body).hexdigest()
            row=(kind,str(target),body,sha)
            previous=db.execute('SELECT kind,path,manifest,sha256 FROM inputs WHERE kind=?',(kind,)).fetchone()
            if previous and previous!=row:raise ValueError('Prior shared input manifest differs')
            db.execute('INSERT OR IGNORE INTO inputs VALUES(?,?,?,?)',row);db.commit()
            result[kind]=dict(source=str(source),path=str(target),source_signature=signature,manifest_sha256=sha,
                              files=sum(v[0]=='file' for v in current.values()))
        bindings=[('game','game-nocd'),('assets','assets/native-cd-audio-20261012'),('prefix','fixtures/prefix-seed')] if aliases else []
        for kind,relative in bindings:
            if kind not in result:continue
            alias=root/'working'/relative;alias.parent.mkdir(parents=True,exist_ok=True);target=Path(result[kind]['path'])
            if alias.is_symlink() and alias.resolve()==target:continue
            if alias.exists() or alias.is_symlink():raise ValueError('Existing input alias preserved: '+str(alias))
            alias.symlink_to(target,target_is_directory=True)
    paths(root)
    return result


if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser(description='Validate an explicitly catalogued shared game source')
    p.add_argument('--game',type=Path,required=True)
    a=p.parse_args()
    if paths(Path(__file__).resolve().parents[1]).get('game')!=a.game.resolve():
        raise SystemExit('Source is not the verified shared game input')
