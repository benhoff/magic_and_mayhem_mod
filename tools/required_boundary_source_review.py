"""Exact retained-source review for two host-analysis corrections, never native code."""
import hashlib
from pathlib import Path
from rendering_pilot import require
PREFIX='research/runtime/required-boundaries-results-20261110/'
CHECKER='tools/check-required-boundary-session.py'
PROVIDER='tools/check-required-provider-session.py'

def corrected(name,old):
    if name==CHECKER:
        require(old.count(b"and p['paint'] in matches")==1
                and old.count(b"matches[p['paint']]")==1,'Unexpected old paint checker')
        return old.replace(b"and p['paint'] in matches",b"and int(p['paint']) in matches").replace(
            b"matches[p['paint']]",b"matches[int(p['paint'])]")
    require(name==PROVIDER and old.count(b'all(r[57:63])')==1,'Unexpected provider analyzer')
    return old.replace(b'all(r[57:63])',b'all(r[57:62]) and r[62]==0')

def review(reader,sources):
    exceptions=[];current={}
    for name,expected in sources.items():
        actual=reader.digest(name);current[name]=actual
        if actual==expected:continue
        require(name in (CHECKER,PROVIDER),'Native/compiler/other source drift: '+name)
        snapshot=PREFIX+'live-plan-04-source-'+hashlib.sha256(name.encode()).hexdigest()[:16]+'.bin'
        require(reader.digest(snapshot)==expected,'Missing exact old host-analysis source')
        old=reader.read_bytes(snapshot);new=reader.read_bytes(name)
        require(corrected(name,old)==new,'Host correction exceeded the reviewed change')
        exceptions.append(dict(path=name,old_sha256=expected,current_sha256=actual,old_source=snapshot,
            native_code_changed=False,acquisition_executed_this_analyzer=False,
            reason='Normalize captured paint IDs before the existing exact lookup' if name==CHECKER else
                'Main allocates six controls, then explicitly destroys and clears the sixth before prepare returns; require five live and one null'))
    return current,exceptions
