#!/usr/bin/env python3
"""Retain blocked-sink, FIFO, sanitizer and real GL copy-witness controls."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
import sys
from coverage_evidence import write_report
ROOT=Path(__file__).resolve().parents[1]


def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);a=parser.parse_args()
    if not a.report.startswith('research/runtime/storage-main-stall-results-20261101/') or '..' in Path(a.report).parts or (ROOT/a.report).exists():parser.error('New owned immutable report required')
    out=ROOT/'working/copy-worker-controls'/Path(a.report).stem;out.mkdir(parents=True,exist_ok=False);(out/'.retain').write_text('Preserve worker controls, including failures.\n')
    build=ROOT/'working/build/qt-shell';library=build/'renderer/libmnm-renderer.a'
    names={'tools/test-copy-diagnostic-worker.py','tests/diagnostic-worker-test.cpp','tests/render-copy-diagnostics-test.cpp','renderer/diagnostic_worker.hpp','renderer/command_copy_diagnostics.cpp','renderer/command_copy_diagnostics.hpp'}
    for dep in (build/'renderer/CMakeFiles/mnm-renderer.dir').rglob('*.o.d'):
        for word in shlex.split(dep.read_text().replace('\\\n',' ')):
            path=Path(word)
            if not path.is_absolute():continue
            path=path.resolve()
            if path.is_file() and path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):names.add(path.relative_to(ROOT).as_posix())
    result=dict(schema=1,kind='copy_diagnostic_worker_controls',success=False,sources={n:sha(ROOT/n) for n in sorted(names)},library=dict(path=str(library),sha256=sha(library)),controls=[],original_artifacts_consumed=False,live_game_validated=False)
    env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe',LP_NUM_THREADS='4')
    def execute(label,cmd):
        result['controls'].append(dict(label=label,command=list(map(str,cmd))))
        run=subprocess.run(list(map(str,cmd)),cwd=ROOT,env=env,capture_output=True,text=True,timeout=120)
        result['controls'][-1].update(returncode=run.returncode,output=run.stdout+run.stderr)
        if run.returncode:raise ValueError('Control failed: '+label)
        return run.stdout
    try:
        execute('renderer-build',['cmake','--build',build,'--target','mnm-renderer','--parallel','4'])
        result['library']['sha256']=sha(library)
        for label,flags in [('queue',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            binary=out/label;execute(label+'-build',['g++','-std=c++17','-pthread','-Wall','-Wextra','-Werror',*flags,ROOT/'tests/diagnostic-worker-test.cpp','-o',binary]);execute(label,[binary])
        flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Gui','Qt6OpenGL'],text=True))
        binary=out/'copy-gpu';execute('gpu-build',['g++','-std=c++17','-O2','-fPIC','-no-pie','-pthread','-Wall','-Wextra','-Werror','-pedantic','-I'+str(ROOT/'renderer'),ROOT/'tests/render-copy-diagnostics-test.cpp',library,*flags,'-o',binary])
        result['gpu']=json.loads(execute('gpu',['xvfb-run','-a',binary]).splitlines()[-1])
        result['sources_stable']=all(sha(ROOT/n)==h for n,h in result['sources'].items()) and sha(library)==result['library']['sha256']
        result['success']=result['sources_stable'] and result['gpu']['success'] is True
    except Exception as error:result['error']=str(error)
    result['artifacts']=[]
    for path in out.iterdir():
        if not path.is_file() or path.name=='.retain':continue
        raw=path.read_bytes();name=str(Path(a.report).with_suffix(''))+'-'+path.name+'.bin'
        result['artifacts'].append(dict(path=path.name,record=name,sha256=write_report(ROOT,name,raw)))
    write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps({k:result.get(k) for k in ('success','error','sources_stable','gpu')}),flush=True)
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
