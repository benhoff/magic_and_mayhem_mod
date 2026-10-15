#!/usr/bin/env python3
"""Execute owned PE32 extra public source descriptor and original blit guards."""
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
    p.add_argument('--shared-fixture-prefix',type=Path)
    p.add_argument('--claims', type=Path, required=True)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Refusing evidence overwrite')
    claims = json.loads(a.claims.read_text())
    sources = dict(claims['sources'])
    for name in ('tools/test-drawing-wine-source-descriptor.py', 'tests/drawing-wine-source-descriptor-test.c',
                 'runtime/drawing/wine_submission.h', 'runtime/drawing/source_byte_witness.h', 'runtime/drawing/wine_resource_mapping.h','runtime/drawing/wine_source_descriptor.h','tools/drawing_wine_source_descriptor.py','tools/drawing_fixture_prefix.py', 'runtime/drawing/session.c'):
        sources[name] = sha((ROOT/name).read_bytes())
    if any(sha((ROOT/n).read_bytes()) != h for n, h in sources.items()):
        raise ValueError('Prospective source changed')
    parent = ROOT/'working/tests/drawing-wine-source-descriptor'
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
    subprocess.run(flags+['-c', str(ROOT/'tests/drawing-wine-source-descriptor-test.c'), '-o', str(run/'fixture.obj')], check=True)
    binary = run/'fixture.exe'
    subprocess.run(['lld-link', '/machine:x86', '/entry:start', '/subsystem:console', '/nodefaultlib',
                    '/safeseh:no', '/timestamp:0', '/out:'+str(binary), str(run/'fixture.obj'),str(dll.parent/'packet.obj'),
                    str(dll.parent/'kernel32.lib')], check=True)
    from drawing_fixture_prefix import prepare
    prefix,prefix_lock=prepare(ROOT,run,a.prefix_template,a.shared_fixture_prefix)
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(prefix), WINEDEBUG='-all', MNM_DRAW_SESSION_MODE='physical',
               MNM_DRAW02_PRODUCER_BIND='1', MNM_DRAW_WINE_SUBMISSION='1',MNM_DRAW_WINE_RESOURCE_MAP='1',MNM_DRAW_WINE_TEXTURE_ENTRY='1',MNM_DRAW_WINE_SOURCE_DESCRIPTOR='1', MNM_DRAW_SESSION_DIRECTORY='Z:'+str(capture).replace('/', '\\'))
    try:
        with (run/'wineboot.log').open('x') as out:
            subprocess.run(['wineboot', '-u'], env=env, stdout=out, stderr=subprocess.STDOUT, check=True, timeout=90)
        with (run/'fixture.log').open('x') as out:
            execution = subprocess.run(['wine', str(binary)], env=env, stdout=out, stderr=subprocess.STDOUT, timeout=60)
        execution.check_returncode()
        from drawing_wine_source_descriptor import decode
        raw=(capture/'wine-source-descriptor.bin').read_bytes();observed=decode(raw)
        if len(observed['samples'])!=6 or observed['known_descriptors']!=1 or observed['extra_public_queries']!=7 or [r['status'] for r in observed['samples']]!=[0,1,2,3,4,1]:raise ValueError('Source descriptor fixture closure')
        controls=subprocess.run(['python3',str(ROOT/'tests/test-drawing-wine-source-descriptor.py')],capture_output=True,text=True,check=True)
        if any(sha((ROOT/n).read_bytes()) != h for n,h in sources.items()):
            raise ValueError('Fixture sources changed during execution')
        report = dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],
                      artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),
                      journal_sha256=sha(raw),original_blits=7,extra_resource_getters=4,extra_descriptor_queries=3,descriptor_samples=6,known_descriptors=1,default_disabled_queries=0,original_entry_and_return_fp_preserved=True,original_last_error_preserved=True,analysis=observed,
                      original_game_executed=False,async_egl_completion_covered=False,
                      scope='Owned PE32 extra public source getter/descriptor queries, disabled/untrusted/null/failed/changed-generation cases, exactly seven original blit forwards with entry/return512BFP and LastError preservation. Synthetic objects only; no original source/driver bytes, ownership or full acquisition proof.')
        a.report.parent.mkdir(parents=True,exist_ok=True)
        a.report.write_text(json.dumps(report,indent=2)+'\n')
        print(json.dumps(dict(success=True,original_blits=7,descriptor_samples=6)))
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        if prefix_lock is not None:prefix_lock.close()


if __name__ == '__main__':
    main()
