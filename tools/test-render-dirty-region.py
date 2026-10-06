#!/usr/bin/env python3
"""Owned dirty rectangles: independent reconstruction and minimal-edge checks."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
def main():
    paths=['runtime/render/dirty_region.h','runtime/render/owned_session.h','tests/render-dirty-region-test.c','tools/test-render-dirty-region.py']
    sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in paths}
    parent=ROOT/'working/tests/render-dirty-region';parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));report=dict(success=True,sources=sources,cases=[],scope='Native storage-byte delta envelope, independent reconstruction and minimality across indexed8/RGB16/24/32. No original pixels or driver equivalence.')
    for label,flags in [('native',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
        binary=run/label
        subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',*flags,str(ROOT/'tests/render-dirty-region-test.c'),'-o',str(binary)],check=True)
        result=subprocess.run([str(binary)],env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1'),capture_output=True,text=True,timeout=30)
        (run/(label+'.log')).write_text(result.stdout+result.stderr);assert result.returncode==0,result.stderr
        report['cases'].append(dict(label=label,**json.loads(result.stdout)))
    assert all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in sources.items())
    (run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)
if __name__=='__main__':main()
