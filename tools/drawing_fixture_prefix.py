"""Private Wine fixture prefixes, with explicit serial suite reuse to bound storage."""
import fcntl
import json
import os
from pathlib import Path
import subprocess


def prepare(root,run,template,shared=None):
    if shared is None:
        prefix=run/'wineprefix'
        subprocess.run(['cp','-a','--reflink=auto',str(template.resolve()),str(prefix)],check=True)
        return prefix,None
    prefix=shared.resolve()
    allowed=(root/'working/tests/drawing-fixture-prefix-suite').resolve()
    if prefix.name!='wineprefix' or not prefix.is_relative_to(allowed) or not prefix.is_dir():
        raise ValueError('Shared prefix must belong to the private fixture suite')
    marker=json.loads((prefix.parent/'fixture-prefix-owner.json').read_text())
    if marker!={'schema':1,'root':str(root.resolve()),'scope':'serial-owned-fixtures-only'}:
        raise ValueError('Shared prefix owner marker')
    lock=(prefix.parent/'fixture-prefix.lock').open('a+b')
    try:
        fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
        # Refuse another live client; do not kill a foreign running session.
        subprocess.run(['wineserver','-w'],env=dict(os.environ,WINEPREFIX=str(prefix),WINEDEBUG='-all'),check=True,timeout=2)
    except BaseException:
        lock.close()
        raise
    return prefix,lock
