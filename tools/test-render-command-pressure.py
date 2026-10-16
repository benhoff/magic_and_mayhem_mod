#!/usr/bin/env python3
"""Real C queue/admission controls with deterministic independent draining."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    args=parser.parse_args();declaration=json.loads(args.claims.read_text());sources=declaration['sources']
    def stable():return all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in sources.items())
    if not stable():raise ValueError('Prospective sources changed')
    parent=ROOT/'working/tests/render-command-pressure';parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    report=dict(schema=1,success=False,claims=declaration['claims'],sources=sources,cases={},scope='Actual C queue and copy admission with deterministic Win32/worker/reader fakes; no original game or actual OS timing guarantee.')
    for label,flags in [('native',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
        binary=run/label
        subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',*flags,str(ROOT/'tests/render-command-pressure-test.c'),'-o',str(binary)],check=True)
        result=subprocess.run([str(binary)],capture_output=True,text=True,timeout=60,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1'))
        (run/(label+'.log')).write_text(result.stdout+result.stderr);result.check_returncode()
        report['cases'][label]=[json.loads(line) for line in result.stdout.splitlines()]
        assert all(r['success'] for r in report['cases'][label])
    report['source_stable']=stable();assert report['source_stable']
    report['success']=True;(run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)

if __name__=='__main__':main()
