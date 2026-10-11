#!/usr/bin/env python3
"""Retain an independently registered original presented session as portable data."""
import argparse
import gzip
import json
from pathlib import Path
from drawing_corpus import ROOT, local, sha
from drawing_session import admit


def retain(source_root,evidence_id,destination):
    from rendering_coverage import evidence_record
    source_root=source_root.resolve();destination=destination.resolve()
    if destination.exists() or not destination.is_relative_to((ROOT/'tests/fixtures').resolve()):
        raise ValueError('Choose a new directory beneath tests/fixtures')
    register=json.loads((source_root/'research/runtime/coverage/register.json').read_text())
    ev=next(e for e in register['evidence'] if e['id']==evidence_id)
    if ev['kind'] not in ('isolated_original','live_equivalence','live_replacement'):
        raise ValueError('Retention requires independent original execution evidence')
    report=evidence_record(source_root,ev,{b['id']:b for b in register['builds']})
    import copy
    catalog=copy.deepcopy(report['drawing_session'])
    catalog.update(schema=1,build=ev['build'],source_sha256=report['source_executable_sha256'],
                   evidence=evidence_id,historical_report_sha256=ev['sha256'])
    prepared={}
    for entry in [i['artifact'] for i in catalog['inputs']]+[f['oracle'] for f in catalog['frames']]:
        original_path=entry['historical_path'];data=local(source_root,original_path).read_bytes()
        if report['inputs'].get(original_path)!=sha(data) or entry['raw_sha256']!=sha(data):
            raise ValueError('Original session input/output changed: '+original_path)
        packed=gzip.compress(data,mtime=0);path=destination/(sha(data)+'.gz');prepared[path]=packed
        entry.update(path=path.relative_to(ROOT).as_posix(),encoding='gzip',sha256=sha(packed))
    # Prepare in a temporary mirror for full independent admission before
    # publishing any output. This also tests the exact re-encoded artifact bytes.
    import tempfile,shutil
    with tempfile.TemporaryDirectory(prefix='draw02-session-retention-') as tmp:
        mirror=Path(tmp)
        record=local(source_root,ev['record']);target=mirror/ev['record'];target.parent.mkdir(parents=True,exist_ok=True)
        shutil.copyfile(record,target)
        for path,data in prepared.items():
            target=mirror/path.relative_to(ROOT);target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data)
        admit(mirror,catalog,register)
    destination.mkdir(parents=True)
    for path,data in prepared.items():
        with path.open('xb') as out:out.write(data)
    with (destination/'session.json').open('x') as out:json.dump(catalog,out,indent=2);out.write('\n')
    return catalog


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--source-root',type=Path,default=ROOT);p.add_argument('--evidence',required=True)
    p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    try:
        result=retain(a.source_root,a.evidence,a.output)
        print(json.dumps({'session':result['id'],'scenario':result['scenario'],'frames':len(result['frames'])}))
    except (ValueError,OSError,KeyError,TypeError,StopIteration) as e:p.exit(2,str(e)+'\n')

if __name__=='__main__':raise SystemExit(main())
