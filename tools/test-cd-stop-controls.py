#!/usr/bin/env python3
"""Compare bounded native CD control models with unchanged original x86 bodies."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
from drawing_census import ROOT,BUILD_SHA256

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def run(executable,claims):
    if sha(executable)!=BUILD_SHA256:raise ValueError('Unsupported original CD executable')
    declaration=json.loads(claims.read_text())
    sources=declaration['sources']
    if any(sha(ROOT/path)!=digest for path,digest in sources.items()):raise ValueError('CD declaration source drift')
    parent=ROOT/'working/tests/cd-stop-controls';parent.mkdir(parents=True,exist_ok=True)
    output=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(output,flush=True)
    report=dict(schema=1,success=False,sources=sources,claims=declaration['claims'],original_sha256=BUILD_SHA256,
                scope='Whole unchanged original CD stop caller versus native model:62208 authored receiver/scripted MCI, first-mixer, volume and nonwrapping positive clock cases, exact return/event ordering/receiver and guard bytes. Gates, MODE4 early returns, fade raw0..65535 conversion, descending levels/clock polling, volume/query failures, ignored STOP/restore returns and paused clearing. Callee implementations, nested original setter stop/enable semantics, clock wrap, real driver timing, notifications/playlist and live playback excluded.')
    try:
        programs=['tests/cd-stop-controls-reference.cpp','reconstruction/audio/cd_stop_controls.cpp']
        flags=['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror','-I'+str(ROOT/'reconstruction/audio')]
        with (output/'build.log').open('x') as log:
            for native in (True,False):
                target=output/('native' if native else 'original')
                command=flags+([] if native else ['-m32','-fno-pie','-no-pie','-mno-sse','-mno-mmx','-DORIGINAL_REFERENCE'])
                subprocess.run(command+[str(ROOT/path) for path in programs]+['-o',str(target)],stdout=log,stderr=subprocess.STDOUT,check=True,timeout=90)
                result=subprocess.run([str(target)]+([] if native else [str(executable)]),stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,check=False,timeout=30)
                (output/(target.name+'.log')).write_text(result.stdout+result.stderr)
                report[target.name+'_returncode']=result.returncode
                if result.returncode:raise RuntimeError('CD '+target.name+' reference failed: '+str(result.returncode))
                report[target.name]=json.loads(result.stdout)
        if report['native']['cases']!=62208 or report['original']['cases']!=62208 or report['original']['guard_bytes']!=62208*1056:
            raise ValueError('Incomplete CD comparison matrix/guards')
        for field in ('success','cases','events'):
            if report['native'][field]!=report['original'][field]:raise ValueError('Native/original CD summary differs')
        report['input_unchanged']=sha(executable)==BUILD_SHA256
        report['sources_stable']=all(sha(ROOT/path)==digest for path,digest in sources.items())
        if not report['input_unchanged'] or not report['sources_stable']:raise ValueError('CD experiment input/source changed')
        report['original_control_instructions_unchanged']=True
        report['success']=True
    except Exception as error:
        report['error']=str(error);raise
    finally:
        report['artifacts']={str(path.relative_to(ROOT)):dict(sha256=sha(path),bytes=path.stat().st_size) for path in output.iterdir() if path.is_file()}
        subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
        report['original_manifest_verified_before_after']=True
        (output/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    return report
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable',type=Path,required=True);parser.add_argument('--claims',type=Path,required=True)
    args=parser.parse_args();subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    run(args.executable.resolve(),args.claims.resolve())
if __name__=='__main__':main()
