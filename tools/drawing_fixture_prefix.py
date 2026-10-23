"""Private Wine fixture prefixes, with explicit serial suite reuse to bound storage."""
from coverage_evidence import read_text as evidence_text
import fcntl
import json
import os
from pathlib import Path
import subprocess


def borrow(root, shared):
    """Borrow a marked prefix already exclusively leased by the supervising runner.

    The supervisor owns idle admission and final process retirement. Child tools
    must stop their clients before returning and must not clone this prefix.
    """
    prefix = shared.resolve()
    marker = json.loads(evidence_text(prefix.parent/'fixture-prefix-owner.json'))
    owner = Path(marker['root']).resolve()
    if (marker != {'schema': 1, 'root': str(owner), 'scope': 'serial-owned-fixtures-only'} or
        prefix.name != 'wineprefix' or not prefix.is_dir() or
        not prefix.is_relative_to((owner/'working/tests/drawing-fixture-prefix-suite').resolve())):
        raise ValueError('Invalid borrowed fixture prefix owner')
    def common(path):
        value = subprocess.check_output(['git', '-C', str(path), 'rev-parse', '--git-common-dir'], text=True).strip()
        return (path/value).resolve()
    if common(owner) != common(root):
        raise ValueError('Borrowed prefix must share this Git checkout')
    try:
        probe = (prefix.parent/'fixture-prefix.lock').open('rb')
    except FileNotFoundError as error:
        raise ValueError('Borrowed prefix requires a supervising exclusive lease') from error
    with probe:
        try:
            fcntl.flock(probe, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            return prefix, None
    raise ValueError('Borrowed prefix requires a supervising exclusive lease')


def prepare(root,run,template,shared=None):
    if shared is None:
        prefix=run/'wineprefix'
        subprocess.run(['cp','-a','--reflink=auto',str(template.resolve()),str(prefix)],check=True)
        return prefix,None
    prefix=shared.resolve()
    allowed=(root/'working/tests/drawing-fixture-prefix-suite').resolve()
    if prefix.name!='wineprefix' or not prefix.is_relative_to(allowed) or not prefix.is_dir():
        raise ValueError('Shared prefix must belong to the private fixture suite')
    marker=json.loads(evidence_text(prefix.parent/'fixture-prefix-owner.json'))
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
