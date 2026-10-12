#!/usr/bin/env python3
"""Execute scoped native pitch/seek/generated-cursor ramp and lifecycle cases."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--claims',type=Path,required=True);args=p.parse_args();decl=json.loads(args.claims.read_text())
    if any(sha(ROOT/path)!=digest for path,digest in decl['sources'].items()):raise ValueError('Audio control declaration drift')
    parent=ROOT/'working/tests/native-audio-controls';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(schema=1,success=False,sources=decl['sources'],claims=decl['claims'],inputs={'pcm_s16le_hex':'0000e803d007b80b','source_rate':1000,'output_rate':2000},scope='Native static pitch/seek/generated-cursor ramp, phase continuity, duplicate independence, strict bounds, active/completed lifecycle and primary/released refusal; no original or physical consumed-cursor equivalence.')
    try:
        build=ROOT/'working/build/audio'
        with (out/'build.log').open('x') as log:
            subprocess.run(['cmake','-S',str(ROOT/'audio'),'-B',str(build)],check=True,stdout=log,stderr=subprocess.STDOUT,timeout=90)
            subprocess.run(['cmake','--build',str(build),'--target','audio-controls-test','-j','4'],check=True,stdout=log,stderr=subprocess.STDOUT,timeout=90)
        with (out/'test.log').open('x') as log:subprocess.run([str(build/'audio-controls-test')],check=True,stdout=log,stderr=subprocess.STDOUT,timeout=10)
        report['sources_stable']=all(sha(ROOT/path)==digest for path,digest in decl['sources'].items());report['inputs_stable']=True
        if not report['sources_stable']:raise ValueError('Audio control sources changed during execution')
        report['success']=True
    except Exception as error:report['error']=str(error);raise
    finally:(out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
if __name__=='__main__':main()
