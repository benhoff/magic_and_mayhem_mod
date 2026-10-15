#!/usr/bin/env python3
"""Execute PE32 selected texture/swapchain lifecycle guards in a private Wine prefix."""
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
    for name in ('tools/test-drawing-wine-lifetime.py', 'tests/drawing-wine-lifetime-test.c',
                 'runtime/drawing/wine_submission.h', 'runtime/drawing/wine_lifetime.h', 'runtime/drawing/session.c'):
        sources[name] = sha((ROOT/name).read_bytes())
    if any(sha((ROOT/n).read_bytes()) != h for n, h in sources.items()):
        raise ValueError('Prospective source changed')
    parent = ROOT/'working/tests/drawing-wine-lifetime'
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
    subprocess.run(flags+['-c', str(ROOT/'tests/drawing-wine-lifetime-test.c'), '-o', str(run/'fixture.obj')], check=True)
    binary = run/'fixture.exe'
    subprocess.run(['lld-link', '/machine:x86', '/entry:start', '/subsystem:console', '/nodefaultlib',
                    '/safeseh:no', '/timestamp:0', '/out:'+str(binary), str(run/'fixture.obj'),
                    str(dll.parent/'kernel32.lib')], check=True)
    subprocess.run(['cp', '-a', '--reflink=auto', str(a.prefix_template.resolve()), str(run/'wineprefix')], check=True)
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(run/'wineprefix'), WINEDEBUG='-all', MNM_DRAW_SESSION_MODE='physical',
               MNM_DRAW_WINE_SUBMISSION='1', MNM_DRAW_SESSION_DIRECTORY='Z:'+str(capture).replace('/', '\\'))
    try:
        with (run/'wineboot.log').open('x') as out:
            subprocess.run(['wineboot', '-u'], env=env, stdout=out, stderr=subprocess.STDOUT, check=True, timeout=90)
        with (run/'fixture.log').open('x') as out:
            execution = subprocess.run(['wine', str(binary)], env=env, stdout=out, stderr=subprocess.STDOUT, timeout=60)
        execution.check_returncode()
        raw = (capture/'wine-lifetime.bin').read_bytes()
        header = struct.unpack_from('<16I', raw)
        rows = list(struct.iter_unpack('<24I', raw[64:]))
        if raw[:8] != b'MNMWLF01' or header[2:7] != (1,96,0x40209ca7,131072,2048) or any(header[7:]) or len(rows) != 29:
            raise ValueError('Lifetime journal envelope/count')
        if [r[0] for r in rows] != list(range(1,30)) or rows[-1][1:4] != (4,1,14) or rows[-1][11] != 4:
            raise ValueError('Lifetime journal terminal/generations')
        active={};created=[]
        for i in range(14):
            entry,returned=rows[i*2:i*2+2]
            if entry[1:3]!=(1,i+1) or returned[1:3]!=(2,i+1) or entry[3:7]!=returned[3:7] or returned[9]!=0xcafe or any(entry[13:]) or any(returned[13:]):
                raise ValueError('Lifetime paired ABI/result/error')
            if entry[3] in (1,3) and returned[8]==0:
                if not returned[11] or returned[11] in created:raise ValueError('Lifetime reused generation')
                created.append(returned[11]);active[returned[10]]=returned[11]
            elif entry[3] in (2,4):
                if active.get(entry[10])!=entry[11]:raise ValueError('Lifetime release generation')
                if returned[8]==0:
                    if returned[12]!=2:raise ValueError('Lifetime missing retire flag')
                    active.pop(entry[10])
        if active or created!=[1,2,3,4]:raise ValueError('Lifetime create/release/reuse closure')
        if any(sha((ROOT/n).read_bytes()) != h for n,h in sources.items()):
            raise ValueError('Fixture sources changed during execution')
        report = dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],
                      artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),
                      journal_sha256=sha(raw),cdecl_cases=14,observed_generations=4,
                      creation_failure_preserved=True,nonfinal_release_retained=True,
                      final_release_retires=True,address_reuse_new_generation=True,
                      result_last_error_preserved=True,original_game_executed=False,
                      source_storage_owned=False,all_internal_lifetimes_observed=False,
                      scope='ActualPE32synthetic selected create/decref ABI and failure/nonzero/zero/reuse metadata; no original game/internal Wine lifetime completeness/storage ownership or EGL assignment.')
        a.report.parent.mkdir(parents=True,exist_ok=True)
        a.report.write_text(json.dumps(report,indent=2)+'\n')
        print(json.dumps(dict(success=True,cdecl_cases=14,observed_generations=4)))
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)


if __name__ == '__main__':
    main()
