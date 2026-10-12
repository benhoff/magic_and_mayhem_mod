#!/usr/bin/env python3
"""Execute PE32 public resource map forwarding and machine-state guards."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
from drawing_api_session import sha

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--prefix-template', type=Path, required=True)
    p.add_argument('--claims', type=Path, required=True)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Refusing evidence overwrite')
    claims = json.loads(a.claims.read_text())
    sources = dict(claims['sources'])
    for name in ('tools/test-drawing-wine-retained-map-source.py', 'tests/drawing-wine-retained-map-source-test.c',
                 'runtime/drawing/wine_submission.h', 'runtime/drawing/source_byte_witness.h', 'runtime/drawing/wine_retained_map_source.h', 'runtime/drawing/wine_resource_mapping.h', 'tools/drawing_wine_retained_map_source.py', 'runtime/drawing/session.c'):
        sources[name] = sha((ROOT/name).read_bytes())
    if any(sha((ROOT/n).read_bytes()) != h for n, h in sources.items()):
        raise ValueError('Prospective source changed')
    parent = ROOT/'working/tests/drawing-wine-retained-map-source'
    parent.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    capture = run/'capture'
    capture.mkdir()
    spec = importlib.util.spec_from_file_location('drawing_build', ROOT/'tools/build-drawing-session.py')
    build = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(build)
    dll = build.build(True)
    flags = ['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding', '-fno-builtin',
             '-fno-stack-protector', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror', '-Wno-unused-function']
    subprocess.run(flags+['-c', str(ROOT/'tests/drawing-wine-retained-map-source-test.c'), '-o', str(run/'fixture.obj')], check=True)
    binary = run/'fixture.exe'
    subprocess.run(['lld-link', '/machine:x86', '/entry:start', '/subsystem:console', '/nodefaultlib',
                    '/safeseh:no', '/timestamp:0', '/out:'+str(binary), str(run/'fixture.obj'),str(dll.parent/'packet.obj'),
                    str(dll.parent/'kernel32.lib')], check=True)
    subprocess.run(['cp', '-a', '--reflink=auto', str(a.prefix_template.resolve()), str(run/'wineprefix')], check=True)
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(run/'wineprefix'), WINEDEBUG='-all', MNM_DRAW_SESSION_MODE='physical',
               MNM_DRAW02_PRODUCER_BIND='1', MNM_DRAW_WINE_SUBMISSION='1',MNM_DRAW_WINE_RESOURCE_MAP='1', MNM_DRAW_WINE_RETAINED_MAP='1', MNM_DRAW_SESSION_DIRECTORY='Z:'+str(capture).replace('/', '\\'))
    try:
        with (run/'wineboot.log').open('x') as out:
            subprocess.run(['wineboot', '-u'], env=env, stdout=out, stderr=subprocess.STDOUT, check=True, timeout=90)
        with (run/'fixture.log').open('x') as out:
            execution = subprocess.run(['wine', str(binary)], env=env, stdout=out, stderr=subprocess.STDOUT, timeout=60)
        execution.check_returncode()
        from drawing_wine_retained_map_source import decode
        raw=(capture/'wine-retained-map-source.bin').read_bytes();observed=decode(raw,(capture/'wine-resource-map.bin').read_bytes(),(capture/'wine-submission.bin').read_bytes(),(capture/'wine-lifetime.bin').read_bytes())
        if len(observed['points'])!=10 or observed['matched']!=5 or observed['unknown']!=5 or observed['matched_closed_maps']!=3:raise ValueError('Retained source fixture closure')
        if [p['status'] for p in observed['points']]!=[1,1,1,0,1,2,1,4,0,3]:raise ValueError('Retained source refusal state')
        controls=subprocess.run(['python3',str(ROOT/'tests/test-drawing-wine-retained-map-source.py')],capture_output=True,text=True,check=True)
        if any(sha((ROOT/n).read_bytes()) != h for n,h in sources.items()):
            raise ValueError('Fixture sources changed during execution')
        report = dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],
                      artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),
                      journal_sha256=sha(raw),calls=28,retained_map_observation=observed,exact_begin_end_identity_verified=True,
                      original_game_executed=False,async_egl_completion_covered=False,
                      scope='Actual PE32 13 resource forwards,10 source blits,5 lifetime forwards; open/closed/failed remap, generation reuse, map/lifetime changes during outside-lock query and ambiguity. FP/LastError preserved; closed map matching is not allocation ownership.')
        a.report.parent.mkdir(parents=True,exist_ok=True)
        a.report.write_text(json.dumps(report,indent=2)+'\n')
        print(json.dumps(dict(success=True,calls=28,matched=5,unknown=5,closed_matched=3)))
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)


if __name__ == '__main__':
    main()
