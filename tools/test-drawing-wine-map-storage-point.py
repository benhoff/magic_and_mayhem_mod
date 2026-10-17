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
    for name in ('tools/test-drawing-wine-map-storage-point.py', 'tests/drawing-wine-map-storage-point-test.c',
                 'runtime/drawing/wine_submission.h', 'runtime/drawing/source_byte_witness.h', 'runtime/drawing/wine_resource_mapping.h', 'runtime/drawing/session.c', 'runtime/drawing/wine_map_storage_point.h', 'tools/drawing_wine_map_storage_point.py'):
        sources[name] = sha((ROOT/name).read_bytes())
    if any(sha((ROOT/n).read_bytes()) != h for n, h in sources.items()):
        raise ValueError('Prospective source changed')
    parent = ROOT/'working/tests/drawing-wine-map-storage-point'
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
    subprocess.run(flags+['-c', str(ROOT/'tests/drawing-wine-map-storage-point-test.c'), '-o', str(run/'fixture.obj')], check=True)
    binary = run/'fixture.exe'
    subprocess.run(['lld-link', '/machine:x86', '/entry:start', '/subsystem:console', '/nodefaultlib',
                    '/safeseh:no', '/timestamp:0', '/out:'+str(binary), str(run/'fixture.obj'),str(dll.parent/'packet.obj'),
                    str(dll.parent/'kernel32.lib')], check=True)
    subprocess.run(['cp', '-a', '--reflink=auto', str(a.prefix_template.resolve()), str(run/'wineprefix')], check=True)
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(run/'wineprefix'), WINEDEBUG='-all', MNM_DRAW_SESSION_MODE='physical',
               MNM_DRAW02_PRODUCER_BIND='1', MNM_DRAW_WINE_SUBMISSION='1',MNM_DRAW_WINE_RESOURCE_MAP='1', MNM_DRAW_WINE_MAP_STORAGE='1', MNM_DRAW_SESSION_DIRECTORY='Z:'+str(capture).replace('/', '\\'))
    try:
        with (run/'wineboot.log').open('x') as out:
            subprocess.run(['wineboot', '-u'], env=env, stdout=out, stderr=subprocess.STDOUT, check=True, timeout=90)
        from drawing_wine_resource_mapping import decode
        from drawing_wine_map_storage_point import decode as storage_points
        cases=[]
        for name in ('normal','close-query'):
            target=capture if name=='normal' else run/name
            target.mkdir(exist_ok=True)
            case_env=dict(env,MNM_STORAGE_FIXTURE_CLOSE_QUERY='1' if name=='close-query' else '0',
                          MNM_DRAW_SESSION_DIRECTORY='Z:'+str(target).replace('/', '\\'))
            with (target/'fixture.log').open('x') as out:
                execution = subprocess.run(['wine', str(binary)], env=case_env, stdout=out, stderr=subprocess.STDOUT, timeout=60)
            execution.check_returncode()
            case_raw=(target/'wine-resource-map.bin').read_bytes();observed=decode(case_raw)
            if len(observed['calls'])!=12 or observed['open_map_intervals'] or observed['unknown_unmaps']:raise ValueError('Resource mapping fixture closure')
            point_raw=(target/'wine-map-storage-point.bin').read_bytes()
            if name=='close-query':
                if len(point_raw)!=160:raise ValueError('Closed point journal grew after outside-lock export')
                try:storage_points(point_raw,observed)
                except ValueError:case_points=dict(incomplete_refused=True)
                else:raise ValueError('Missing query journal was promoted to complete')
            else:
                points=storage_points(point_raw,observed);case_points=points
                if points['matched']!=1 or points['unknown']!=4:raise ValueError('Storage point fixture classification')
            cases.append(dict(case=name,exit=execution.returncode,journal_sha256=sha(case_raw),point_journal_sha256=sha(point_raw),storage_points=case_points))
        raw=(capture/'wine-resource-map.bin').read_bytes()
        controls=subprocess.run(['python3',str(ROOT/'tests/test-drawing-wine-map-storage-point.py')],capture_output=True,text=True,check=True)
        if any(sha((ROOT/n).read_bytes()) != h for n,h in sources.items()):
            raise ValueError('Fixture sources changed during execution')
        report = dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],
                      artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),
                      journal_sha256=sha(raw),calls=12,storage_points=points,unknown_unmaps=0,open_map_intervals=0,exact_begin_end_identity_verified=True,
                      cases=cases,closed_query_journal_unchanged=True,missing_query_trace_refused=True,
                      original_game_executed=False,async_egl_completion_covered=False,
                      scope='Actual PE32 once-only public map forwarding, exact linked points, failed map exclusion, known/unknown/ambiguous/extent/stopped metadata, LastError and original-return 512-byte FP preservation through an export that mutates both states. Forced close during outside-lock query preserves original callbacks, prevents closed-handle write/journal growth and keeps missing query trace explicitly incomplete. No source byte, ownership or full module/IAT quiescence claim.')
        a.report.parent.mkdir(parents=True,exist_ok=True)
        a.report.write_text(json.dumps(report,indent=2)+'\n')
        print(json.dumps(dict(success=True,calls=12,matched=1,unknown=4)))
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)


if __name__ == '__main__':
    main()
