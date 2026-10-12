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
    for name in ('tools/test-drawing-wine-blit-worker.py', 'tests/drawing-wine-blit-worker-test.c',
                 'runtime/drawing/wine_submission.h', 'runtime/drawing/wine_lifetime.h', 'runtime/drawing/wine_blit_worker.h', 'runtime/drawing/session.c'):
        sources[name] = sha((ROOT/name).read_bytes())
    if any(sha((ROOT/n).read_bytes()) != h for n, h in sources.items()):
        raise ValueError('Prospective source changed')
    parent = ROOT/'working/tests/drawing-wine-blit-worker'
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
    subprocess.run(flags+['-c', str(ROOT/'tests/drawing-wine-blit-worker-test.c'), '-o', str(run/'fixture.obj')], check=True)
    binary = run/'fixture.exe'
    subprocess.run(['lld-link', '/machine:x86', '/entry:start', '/subsystem:console', '/nodefaultlib',
                    '/safeseh:no', '/timestamp:0', '/out:'+str(binary), str(run/'fixture.obj'),
                    str(dll.parent/'kernel32.lib')], check=True)
    subprocess.run(['cp', '-a', '--reflink=auto', str(a.prefix_template.resolve()), str(run/'wineprefix')], check=True)
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(run/'wineprefix'), WINEDEBUG='-all', MNM_DRAW_SESSION_MODE='physical',
               MNM_DRAW_WINE_SUBMISSION='1', MNM_DRAW_WINE_BLIT_WORKER='1', MNM_DRAW_SESSION_DIRECTORY='Z:'+str(capture).replace('/', '\\'))
    try:
        with (run/'wineboot.log').open('x') as out:
            subprocess.run(['wineboot', '-u'], env=env, stdout=out, stderr=subprocess.STDOUT, check=True, timeout=90)
        with (run/'fixture.log').open('x') as out:
            execution = subprocess.run(['wine', str(binary)], env=env, stdout=out, stderr=subprocess.STDOUT, timeout=60)
        execution.check_returncode()
        raw=(capture/'wine-blit-worker.bin').read_bytes();header=struct.unpack_from('<16I',raw);rows=list(struct.iter_unpack('<48I',raw[64:]))
        if raw[:8]!=b'MNMWBK01' or header[2:7]!=(1,192,0x40209ca7,131072,42) or len(rows)!=7:raise ValueError('Worker fixture envelope')
        for i in range(3):
            entry,returned=rows[i*2:i*2+2]
            if entry[1:4]!=(1,i+1,42) or returned[1:4]!=(2,i+1,42) or returned[6]!=0xcafe or entry[7:]!=returned[7:] or entry[9]!=1 or entry[11]!=2 or any(entry[34:]):raise ValueError('Worker fixture identity/generation/return')
            if entry[12:14]!=(4,5) or entry[14:20]!=tuple(i*7 for i in range(3,9)) or entry[20:26]!=tuple(i*11 for i in range(11,17)) or entry[26:34]!=tuple(i*11 for i in range(17,25)):raise ValueError('Worker fixture command scalar forwarding')
        if rows[-1][1:4]!=(4,1,3):raise ValueError('Worker fixture closure')
        if any(sha((ROOT/n).read_bytes()) != h for n,h in sources.items()):
            raise ValueError('Fixture sources changed during execution')
        report = dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],
                      artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),
                      journal_sha256=sha(raw),worker_calls=3,reentrant_threads=3,
                      owned_command_scalars_verified=True,known_generations_verified=True,
                      last_error_x87_preserved=True,pending_worker_closure=True,
                      original_game_executed=False,source_submission_identity_bound=False,
                      scope='ActualPE32synthetic opcode42workerforward/scalar/generation/reentry/error/FP/pending fixture;nooriginalqueue/source/EGLidentity orownership.')
        a.report.parent.mkdir(parents=True,exist_ok=True)
        a.report.write_text(json.dumps(report,indent=2)+'\n')
        print(json.dumps(dict(success=True,worker_calls=3,reentrant_threads=3)))
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)


if __name__ == '__main__':
    main()
