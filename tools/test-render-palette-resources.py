#!/usr/bin/env python3
"""Record native and sanitized palette wire/GPU validation without original input."""
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
    parser.add_argument('build',type=Path)
    parser.add_argument('sanitized',type=Path)
    args=parser.parse_args()
    paths=[ROOT/p for p in ['renderer/CMakeLists.txt','renderer/commands.cpp','renderer/commands.hpp',
        'renderer/command_consumer.cpp','renderer/command_state.hpp','renderer/blit.cpp','renderer/blit.hpp',
        'tests/render-palette-resources-test.cpp','apps/qt-shell/gl_viewport.cpp','apps/qt-shell/gl_viewport.hpp',
        'protocols/schemas/render_stream-v2.json','protocols/include/mnm/render_stream_v2.h',
        'tools/test-render-palette-resources.py']]
    sources={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    parent=ROOT/'working/tests/render-palette-resources';parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));results=[]
    for name,build in [('native',args.build),('asan-ubsan',args.sanitized)]:
        env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')
        if name=='asan-ubsan':env.update(ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
        proc=subprocess.run(['xvfb-run','-a',str(build.resolve()/'render-palette-resources-test')],env=env,text=True,capture_output=True,timeout=60)
        (run/(name+'.log')).write_text(proc.stdout+proc.stderr)
        assert proc.returncode==0,(name,proc.stdout,proc.stderr)
        result=json.loads(proc.stdout);assert result['success'] and result['full_frames']==21 and result['rejections']==15 and not result['ordinary_readbacks']
        results.append(dict(mode=name,**result))
    assert all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in sources.items())
    report=dict(schema=1,success=True,sources=sources,cases=results,scope='Independent synthetic indexed pixels and palette expectations; shared update fanout, rebinding, retirement/recreation, fragmented stream v2 and stale/version/capacity refusal. No original-game or driver equivalence.')
    (run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json')

if __name__=='__main__':main()
