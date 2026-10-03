#!/usr/bin/env python3
"""Compare every ordered candidate byte from a logging-only neighbor expansion."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys
REPO=Path(__file__).resolve().parent.parent

def compare(expected,record):
    if len(record)<126 or record[:8]!=b'MNMEXP01':raise ValueError('Invalid expansion record')
    current,before_budget,after_budget,before_count,after_count=struct.unpack_from('<5I',record,8)
    if before_count>4096 or not before_count<=after_count<=before_count+26 or len(record)!=126+(before_count+after_count)*36:
        raise ValueError('Invalid expansion lengths')
    before=record[126:126+before_count*36];actual=record[126+before_count*36:]
    model=bytes.fromhex(expected['candidate_hex'])
    if len(model)!=expected['candidate_count']*36:raise ValueError('Invalid model candidate lengths')
    differences=[]
    for index in range(max(len(actual),len(model))):
        if index>=len(actual) or index>=len(model) or actual[index]!=model[index]:
            differences.append({'candidate':index//36,'byte':index%36,
                                'original':actual[index] if index<len(actual) else None,
                                'model':model[index] if index<len(model) else None})
    budget=expected['budget_remaining'] & 0xffffffff
    preserved=actual[:len(before)]==before
    status='match' if not differences and budget==after_budget and preserved else 'mismatch'
    return {'status':status,'current':current,'budget_before':before_budget,'budget_after':after_budget,
            'model_budget_after':budget,'prefix_preserved':preserved,'prefix_candidates':before_count,
            'original_candidates':after_count,'model_candidates':expected['candidate_count'],
            'differences':differences}

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('evidence',type=Path)
    args=parser.parse_args();root=args.evidence
    if (root/'game/shadow').is_dir():root=root/'game/shadow'
    records=sorted(root.glob('expansion-*.bin'))
    if not records:raise ValueError('No completed expansion captures; no equivalence claim')
    build=REPO/'working/build/pathfinding'
    subprocess.run(['cmake','-S',str(REPO/'reconstruction/pathfinding'),'-B',str(build)],check=True,stdout=sys.stderr)
    subprocess.run(['cmake','--build',str(build),'--target','neighbor-replay','-j','4'],check=True,stdout=sys.stderr)
    results=[]
    for path in records:
        world=path.with_name(path.name.replace('expansion-','world-'))
        output=subprocess.run([str(build/'neighbor-replay'),str(world),str(path)],check=True,capture_output=True,text=True,timeout=30)
        result=compare(json.loads(output.stdout),path.read_bytes())
        result['files']={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in (path,world)}
        results.append(result)
    manifest=root.parent.parent/'manifest.json'
    metadata=json.loads(manifest.read_text()) if manifest.is_file() else {}
    origin=metadata.get('origin','unverified')
    if origin=='runtime_shadow':
        for name,key in [('Chaos.exe','staged_sha256'),('MnmShadow.dll','dll_sha256')]:
            if hashlib.sha256((root.parent/name).read_bytes()).hexdigest()!=metadata[key]:
                raise ValueError(f'Staged {name} hash mismatch')
    report={'origin':origin,'provenance':metadata,'replay_sha256':hashlib.sha256((build/'neighbor-replay').read_bytes()).hexdigest(),'scope':'ordered candidate bytes and expansion budget','results':results,
            'matched':sum(r['status']=='match' for r in results),'mismatched':sum(r['status']=='mismatch' for r in results)}
    (root/'comparison.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))
    if report['mismatched']:raise SystemExit(1)
if __name__=='__main__':main()
