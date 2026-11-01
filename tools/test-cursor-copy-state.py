#!/usr/bin/env python3
"""Run bounded cursor companion controls and compile the production PE32 bridge."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
from coverage_evidence import write_report
from rendering_operations_history import sha

ROOT=Path(__file__).resolve().parents[1]
SOURCES=['tools/test-cursor-copy-state.py','tools/cursor_copy_state.py',
         'tools/native_copy_routes.py','tools/native_gpu_copy_diagnostics.py',
         'tools/rendering_operations_history.py','tests/cursor-copy-state-test.c',
         'tests/test-cursor-copy-state.py','tests/test-native-copy-routes.py',
         'tests/test-native-copy-output.py','protocols/include/mnm/cursor_copy_state_v1.h',
         'runtime/render/cursor_copy_state.h','runtime/render/copy_route_census.h',
         'tools/build-render-bridge.py']


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
    if (ROOT/a.report).exists():p.error('New immutable report required')
    result=dict(schema=1,success=False,sources={n:sha((ROOT/n).read_bytes()) for n in SOURCES},commands=[],
                scope='Python integrity/binding controls, native writer capacity/overflow controls with sanitizers and a production PE32 bridge build. No game, original binary or native rendering execution.')
    try:
        with tempfile.TemporaryDirectory(prefix='mnm-cursor-copy-controls-') as directory:
            directory=Path(directory)
            def run(command):
                done=subprocess.run([str(v) for v in command],cwd=ROOT,capture_output=True,text=True,timeout=120,
                                    env=dict(os.environ,TMPDIR=str(directory)))
                result['commands'].append(dict(command=[str(v) for v in command],returncode=done.returncode,
                                                output=(done.stdout+done.stderr)[-32000:]))
                if done.returncode:raise ValueError('Control command failed: '+str(command[0]))
            for test in ['test-cursor-copy-state.py','test-native-copy-routes.py','test-native-copy-output.py']:
                run([sys.executable,'-B',ROOT/'tests'/test])
            for label,flags in [('optimized',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
                binary=directory/label
                run(['cc','-std=c11','-Wall','-Wextra','-Werror',*flags,ROOT/'tests/cursor-copy-state-test.c','-o',binary]);run([binary])
            bridge=directory/'bridge'
            run([sys.executable,'-B',ROOT/'tools/build-render-bridge.py','--output',bridge])
            manifest=json.loads((bridge/'manifest.json').read_text())
            for field in ['sources','compiled_sources','protocol_sources']:
                for name,digest in manifest.get(field,{}).items():
                    if '/' not in name:name='runtime/render/'+name
                    if name in result['sources'] and result['sources'][name]!=digest:raise ValueError('Build source changed: '+name)
                    result['sources'][name]=digest
            result.update(success=True,production_pe32_build_sha256=manifest['sha256'],
                          optimized_writer_passed=True,sanitized_writer_passed=True)
    except Exception as error:result['error']=str(error)
    result['sources_stable']=all(sha((ROOT/n).read_bytes())==h for n,h in result['sources'].items())
    result['success'] &= result['sources_stable']
    write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps({k:result.get(k) for k in ['success','error','sources_stable','production_pe32_build_sha256']}),flush=True)
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
