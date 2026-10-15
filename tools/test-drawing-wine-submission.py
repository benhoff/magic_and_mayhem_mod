#!/usr/bin/env python3
"""Execute PE32 cdecl submission-forwarding guards in a private Wine prefix."""
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
    for name in ('tools/test-drawing-wine-submission.py', 'tests/drawing-wine-submission-test.c',
                 'runtime/drawing/wine_submission.h', 'runtime/drawing/wine_lifetime.h', 'runtime/drawing/session.c'):
        sources[name] = sha((ROOT/name).read_bytes())
    if any(sha((ROOT/n).read_bytes()) != h for n, h in sources.items()):
        raise ValueError('Prospective source changed')
    parent = ROOT/'working/tests/drawing-wine-submission'
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
    subprocess.run(flags+['-c', str(ROOT/'tests/drawing-wine-submission-test.c'), '-o', str(run/'fixture.obj')], check=True)
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
        raw = (capture/'wine-submission.bin').read_bytes()
        header = struct.unpack_from('<16I', raw)
        rows = list(struct.iter_unpack('<48I', raw[64:]))
        if raw[:8] != b'MNMWIN02' or header[2:8] != (2,192,0x40209ca7,131072,32,2048) or any(header[8:]) or len(rows) != 19:
            raise ValueError('Fixture journal envelope/count')
        if [r[0] for r in rows] != list(range(1,20)) or rows[-1][1:4] != (4,1,9):
            raise ValueError('Fixture journal sequence/terminal')
        for i in range(9):
            entry, returned = rows[i*2:i*2+2]
            method = 1 if i < 4 else 2
            fault = (0,0x80004005,0x887601c2,0x887601ae)[i%4] if i < 8 else 0
            if entry[1:4] != (1,i+1,method) or returned[1:4] != (2,i+1,method) or returned[8:10] != (fault,0xcafe):
                raise ValueError('Fixture callback/result forwarding')
            if entry[5:7] != ((42,2) if i < 8 else (0,0)) or returned[5:7] != entry[5:7]:
                raise ValueError('Nested/unbound enclosing API identity')
            if entry[15] != 3 or entry[16:24] != (0xfffffffe,3,8,9,1,2,11,12) or any(entry[30:]) or any(returned[30:]):
                raise ValueError('Owned rectangles/reserved fields')
            if entry[27:30] != ((4,2,10) if method==1 else (3,0,10)) or returned[27:30]!=entry[27:30]:raise ValueError('V2generation metadata changed')
        from drawing_wine_submission import decode
        observed=decode(raw,lifetime=(capture/'wine-lifetime.bin').read_bytes())
        if len(observed['calls'])!=9:raise ValueError('Strict V2fixture admission')
        if any(sha((ROOT/n).read_bytes()) != h for n,h in sources.items()):
            raise ValueError('Fixture sources changed during execution')
        report = dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],
                      artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),
                      journal_sha256=sha(raw),cdecl_cases=9,driver_reentrant_threads=9,
                      nested_api_identity_verified=True,unbound_submission_retained=True,
                      result_last_error_x87_preserved=True,original_game_executed=False,
                      source_submission_identity_bound=False,physical_driver_validated=False,
                      scope='Actual PE32 synthetic cdecl callback argument/single-forward/result/error/x87 and recorder-lock release checks; no original game/private Wine texture layout/EGL association or driver proof.')
        a.report.parent.mkdir(parents=True,exist_ok=True)
        a.report.write_text(json.dumps(report,indent=2)+'\n')
        print(json.dumps(dict(success=True,cdecl_cases=9,driver_reentrant_threads=9)))
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)


if __name__ == '__main__':
    main()
