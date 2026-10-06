#!/usr/bin/env python3
"""Compare native GL clipping against retained independent Wine Surface2 outputs."""
import argparse
import gzip
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def corpus():
    parent=ROOT/'tests/fixtures/surfaces';meta=json.loads((parent/'driver-clipping-surface2-corpus.json').read_text())
    assert meta['path']=='driver-clipping-surface2-original.json.gz'
    p=parent/meta['path'];assert sha(p)==meta['sha256'];raw=gzip.decompress(p.read_bytes())
    assert hashlib.sha256(raw).hexdigest()==meta['raw_sha256'];rows=json.loads(raw);assert len(rows)==meta['cases']==408;return rows

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build',type=Path,default=ROOT/'working/build/renderer');p.add_argument('--report',type=Path);args=p.parse_args()
    if args.report and args.report.exists():p.error('Refusing to overwrite evidence')
    parent=ROOT/'working/tests/native-surface-clipping';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent))
    if not args.report:args.report=run/'report.json'
    paths=['renderer/blit.hpp','renderer/blit.cpp','renderer/surface_copy.hpp','renderer/surface_copy.cpp','renderer/CMakeLists.txt','tests/render-surface-copy-fixtures.cpp','tools/check-native-surface-clipping.py','tests/fixtures/surfaces/driver-clipping-surface2-corpus.json','tests/fixtures/surfaces/driver-clipping-surface2-original.json.gz']
    sources={name:sha(ROOT/name) for name in paths};rows=corpus()
    # Expected outputs and HRESULTs are deliberately absent from native inputs.
    inputs=[dict(r['input'],source_pixels=r['source_before'],destination_pixels=r['destination_before']) for r in rows]
    (run/'inputs.json').write_text(json.dumps(inputs));binary=args.build.resolve()/'render-surface-copy-fixtures'
    env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')
    result=subprocess.run([str(binary),str(run/'inputs.json'),str(run/'outputs.json')],env=env,capture_output=True,text=True,timeout=60)
    (run/'native.log').write_text(result.stdout+result.stderr);assert result.returncode==0,result.stdout+result.stderr
    native=json.loads((run/'outputs.json').read_text());actual=native.pop('cases');assert len(actual)==len(rows)
    mismatches=[r['input']['id'] for r,a in zip(rows,actual) if (a['id'],a['hresult'],a['source_after'],a['destination_after'])!=(r['input']['id'],r['hresult'],r['source_after'],r['destination_after'])]
    assert not mismatches,mismatches
    assert all(sha(ROOT/name)==h for name,h in sources.items()),'Sources changed during comparison'
    report=dict(schema=1,success=True,sources=sources,cases=len(rows),matched=len(actual),destination_pixels=len(rows)*48,partial_failure_cases=sum(bool(r['hresult']) and r['destination_before']!=r['destination_after'] for r in rows),native=native,binary_sha256=sha(binary),artifacts=str(run.relative_to(ROOT)),
        scope='Native RGB565 GL Surface2-policy API versus408 independent retained Wine driver HRESULT/full native output pairs. Busy cases use explicit admission observations; no native lock/loss lifecycle, original game execution, Windows-driver equivalence, command protocol or live replacement.')
    args.report.parent.mkdir(parents=True,exist_ok=True)
    with args.report.open('x') as out:json.dump(report,out,indent=2);out.write('\n')
    print(json.dumps({k:report[k] for k in ['success','cases','matched','partial_failure_cases']}))
if __name__=='__main__':main()
