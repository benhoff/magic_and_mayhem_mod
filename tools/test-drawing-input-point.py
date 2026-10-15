#!/usr/bin/env python3
"""Cross-DLL PE32 original cursor forwarding and read-only point query fixture."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
from drawing_api_session import inspect,sha
from drawing_input_point import decode
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--prefix-template',type=Path,required=True);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    claims=json.loads(a.claims.read_text());sources=dict(claims['sources'])
    names=['tools/test-drawing-input-point.py','tests/drawing-input-point-test.c','runtime/drawing/input_point.h','protocols/include/mnm/drawing_input_point_v1.h','tools/drawing_input_point.py','tools/build-drawing-session.py','runtime/drawing/session.c','runtime/shadow/win32_min.h']
    sources.update({n:sha((ROOT/n).read_bytes()) for n in names})
    if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective sources changed')
    parent=ROOT/'working/tests/drawing-input-point';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));capture=run/'capture';capture.mkdir();print(run,flush=True)
    spec=importlib.util.spec_from_file_location('build',ROOT/'tools/build-drawing-session.py');build=importlib.util.module_from_spec(spec);spec.loader.exec_module(build);dll=build.build(True)
    for n,h in json.loads((dll.parent/'manifest.json').read_text())['sources'].items():
        if n in sources and sources[n]!=h:raise ValueError('Compiled sources differ')
        sources[n]=h
    shutil.copy2(dll,run/dll.name)
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
    subprocess.run(flags+['-c',str(ROOT/'tests/drawing-input-point-test.c'),'-o',str(run/'fixture.obj')],check=True)
    binary=run/'fixture.exe';subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),str(run/'fixture.obj'),str(dll.parent/'drawing.lib'),str(dll.parent/'kernel32.lib')],check=True)
    subprocess.run(['cp','-a','--reflink=auto',str(a.prefix_template.resolve()),str(run/'wineprefix')],check=True)
    point_file=run/'points.bin';env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(run/'wineprefix'),WINEDEBUG='-all',MNM_DRAW_SESSION_MODE='inputs',MNM_DRAW_INPUT_POINT='1',MNM_DRAW_SESSION_DIRECTORY='Z:'+str(capture).replace('/','\\'),MNM_DRAW_POINT_FIXTURE_FILE='Z:'+str(point_file).replace('/','\\'))
    try:
        with (run/'wineboot.log').open('x') as out:subprocess.run(['wineboot','-u'],env=env,stdout=out,stderr=subprocess.STDOUT,check=True,timeout=90)
        with (run/'fixture.log').open('x') as out:process=subprocess.run(['wine',str(binary)],cwd=run,env=env,stdout=out,stderr=subprocess.STDOUT,timeout=30)
        if process.returncode:raise ValueError('Cursor point fixture exit:'+str(process.returncode))
        raw=point_file.read_bytes()
        if len(raw)!=320:raise ValueError('Fixture point count')
        points=[decode(raw[i:i+64]) for i in range(0,len(raw),64)];observed=inspect(capture)
        if len(observed['inputs'])!=18 or observed['frames'] or observed['calls'] or any(x['method']!=3 for x in observed['inputs']):raise ValueError('Cursor callback count/type differs')
        by_record={x['ordinal']:x for x in observed['inputs']}
        for point in points:
            if not point['cursor_record']:continue
            row=by_record.get(point['cursor_record'])
            if row is None or row['thread']!=point['owner_thread'] or row['cursor']!=point['cursor']:raise ValueError('Exact point owner/result/journal link differs')
        if [p['cursor_record'] for p in points]!=[1,0,2,3,2] or not points[0]['current_cursor_admissible'] or points[2]['current_cursor_admissible']:raise ValueError('Fixture cursor validity sequence differs')
        if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Fixture sources changed')
        a.report.write_text(json.dumps(dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),dll_sha256=sha((run/dll.name).read_bytes()),points_sha256=sha(raw),points=points,api_observation=observed,original_cursor_callbacks=18,query_added_cursor_calls=0,original_result_error_fp_preserved=True,query_error_fp_preserved=True,read_only_output_refused=True,overflow_refused=True,original_game_executed=False,scope='Actual cross-DLL PE32 original cursor stand-in forwarded once with error/FP preservation; five exact journal-linked points, owner isolation, failed cursor clearing, invalid/read-only output and17live-owner capacity refusal. No original game or semantic click/selection proof.'),indent=2)+'\n')
        print(json.dumps(dict(success=True,original_cursor_callbacks=18,points=5)))
    finally:subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)


if __name__=='__main__':main()
