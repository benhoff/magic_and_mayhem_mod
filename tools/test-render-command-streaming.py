#!/usr/bin/env python3
"""Record bounded-memory native v2 sustained GPU integration and regressions."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('build',type=Path);args=parser.parse_args()
    sources=['renderer/commands.hpp','renderer/commands.cpp','renderer/command_state.hpp','renderer/command_consumer.cpp','apps/qt-shell/command_channel.hpp','apps/qt-shell/command_channel.cpp','apps/qt-shell/live_command_renderer.hpp','apps/qt-shell/live_command_renderer.cpp','tests/live-render-channel-test.cpp','tools/test-render-command-streaming.py','protocols/include/mnm/render_command_ring.h']
    sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
    fingerprints={p:sha(ROOT/p) for p in sources}
    parent=ROOT/'working/tests/render-command-streaming';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')
    binary=args.build.resolve()/'live-render-channel-test'
    result=subprocess.run([str(binary)],capture_output=True,text=True,env=env,timeout=60);(run/'native.log').write_text(result.stdout+result.stderr);assert result.returncode==0,result.stderr
    native=json.loads(result.stdout);assert native['success'] and native['sustained_bytes']>64*1024*1024 and native['sustained_commands']>4096 and native['sustained_frames']==4 and native['full_retries']>0
    result=subprocess.run(['ctest','--test-dir',str(args.build),'-R','opengl-incremental-consumer|opengl-command-stream','--output-on-failure'],capture_output=True,text=True,env=env,timeout=90);(run/'regression.log').write_text(result.stdout+result.stderr);assert result.returncode==0,result.stdout+result.stderr
    assert all(sha(ROOT/p)==h for p,h in fingerprints.items()),'sources changed'
    report=dict(success=True,sources=fingerprints,native=native,regressions_passed=True,binary_sha256=sha(binary),scope='Native v2 mapped adapter/decoder/GPU synthetic sustained integration; independent complete displayed uniform colors. Bounded v1/offline regressions. No original-driver comparison or sustained original owned-capture claim.')
    (run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)
if __name__=='__main__':main()
