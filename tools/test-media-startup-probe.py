#!/usr/bin/env python3
"""Execute the unchanged pinned startup FilterGraph availability check."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
from drawing_census import ROOT, BUILD_SHA256

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable',type=Path,required=True)
    parser.add_argument('--claims',type=Path,required=True)
    args=parser.parse_args();executable=args.executable.resolve()
    if sha(executable)!=BUILD_SHA256:raise ValueError('Unsupported original executable')
    declaration=json.loads(args.claims.read_text())
    if any(sha(ROOT/path)!=digest for path,digest in declaration['sources'].items()):raise ValueError('Declaration source drift')
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    parent=ROOT/'working/tests/media-startup-probe';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(schema=1,success=False,original_sha256=BUILD_SHA256,sources=declaration['sources'],claims=declaration['claims'],scope='Unchanged original0x483f30..483f7b availability caller with49 signed HRESULT combinations and exact scripted CoInitialize/FilterGraph CoCreateInstance/Release/CoUninitialize order. Actual Wine COM/Quartz/codec/device and native policy equivalence excluded.')
    try:
        with (out/'build.log').open('x') as log:
            subprocess.run(['g++','-m32','-std=c++17','-O2','-fno-pie','-no-pie','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror',str(ROOT/'tests/media-startup-probe-reference.cpp'),'-o',str(out/'original')],stdout=log,stderr=subprocess.STDOUT,check=True,timeout=90)
        result=subprocess.run([str(out/'original'),str(executable)],capture_output=True,text=True,timeout=30)
        (out/'original.log').write_text(result.stdout+result.stderr)
        if result.returncode:raise ValueError('Original reference failed: '+str(result.returncode))
        report['original']=json.loads(result.stdout)
        assert report['original']['cases']==49 and report['original']['caller_bytes']==75
        report['input_unchanged']=sha(executable)==BUILD_SHA256
        report['sources_stable']=all(sha(ROOT/path)==digest for path,digest in declaration['sources'].items())
        assert report['input_unchanged'] and report['sources_stable']
        report['original_control_instructions_unchanged']=True;report['success']=True
    except Exception as error:report['error']=str(error);raise
    finally:
        subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
        report['original_manifest_verified_before_after']=True
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
if __name__=='__main__':main()
