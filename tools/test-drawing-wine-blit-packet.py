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
    for name in ('tools/test-drawing-wine-blit-packet.py', 'tests/drawing-wine-blit-packet-test.c',
                 'runtime/drawing/wine_submission.h', 'runtime/drawing/wine_lifetime.h', 'runtime/drawing/wine_blit_worker.h', 'runtime/drawing/wine_blit_packet.h', 'runtime/drawing/wine_blit_packet.S', 'tests/drawing-wine-blit-packet-call.S', 'runtime/drawing/session.c'):
        sources[name] = sha((ROOT/name).read_bytes())
    if any(sha((ROOT/n).read_bytes()) != h for n, h in sources.items()):
        raise ValueError('Prospective source changed')
    parent = ROOT/'working/tests/drawing-wine-blit-packet'
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
    subprocess.run(flags+['-c', str(ROOT/'tests/drawing-wine-blit-packet-test.c'), '-o', str(run/'fixture.obj')], check=True)
    subprocess.run(flags+['-c',str(ROOT/'tests/drawing-wine-blit-packet-call.S'),'-o',str(run/'call.obj')],check=True)
    binary = run/'fixture.exe'
    subprocess.run(['lld-link', '/machine:x86', '/entry:start', '/subsystem:console', '/nodefaultlib',
                    '/safeseh:no', '/timestamp:0', '/out:'+str(binary), str(run/'fixture.obj'),str(run/'call.obj'),str(dll.parent/'packet.obj'),
                    str(dll.parent/'kernel32.lib')], check=True)
    subprocess.run(['cp', '-a', '--reflink=auto', str(a.prefix_template.resolve()), str(run/'wineprefix')], check=True)
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(run/'wineprefix'), WINEDEBUG='-all', MNM_DRAW_SESSION_MODE='physical',
               MNM_DRAW_WINE_SUBMISSION='1', MNM_DRAW_WINE_BLIT_WORKER='1', MNM_DRAW_WINE_BLIT_PACKET='1', MNM_DRAW_SESSION_DIRECTORY='Z:'+str(capture).replace('/', '\\'))
    try:
        with (run/'wineboot.log').open('x') as out:
            subprocess.run(['wineboot', '-u'], env=env, stdout=out, stderr=subprocess.STDOUT, check=True, timeout=90)
        with (run/'fixture.log').open('x') as out:
            execution = subprocess.run(['wine', str(binary)], env=env, stdout=out, stderr=subprocess.STDOUT, timeout=60)
        execution.check_returncode()
        raw=(capture/'wine-blit-packet.bin').read_bytes();rows=list(struct.iter_unpack('<16I',raw[64:]))
        if raw[:8]!=b'MNMWPK01' or len(rows)!=7:raise ValueError('Packet fixture envelope')
        for i in range(2):
            emitted,entered,returned=rows[i*3:i*3+3]
            if emitted[1:4]!=(1,i+1,17) or entered[1:4]!=(2,i+1,17) or returned[1:4]!=(3,i+1,17) or entered[7]!=i+1 or returned[7]!=i+1 or any(entered[8:]) or any(returned[8:]):raise ValueError('Packet exactbinding/retirement')
        if rows[-1][1:4]!=(4,1,2) or rows[-1][8]:raise ValueError('Packet fixture closure')
        if any(sha((ROOT/n).read_bytes()) != h for n,h in sources.items()):
            raise ValueError('Fixture sources changed during execution')
        report = dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],
                      artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),
                      journal_sha256=sha(raw),packet_bindings=2,assembly_shim_executed=True,
                      signature_refusal_verified=True,original_register_flags_stack_preserved=True,
                      last_error_preserved=True,pointer_not_transported=True,
                      original_game_executed=False,source_storage_owned=False,
                      scope='ActualPE32emittershim/expectedbytes/registerflagsstack/error andexactpacketworkerretirement fixture;nooriginalsource/EGLownership.')
        a.report.parent.mkdir(parents=True,exist_ok=True)
        a.report.write_text(json.dumps(report,indent=2)+'\n')
        print(json.dumps(dict(success=True,packet_bindings=2,assembly_shim_executed=True)))
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)


if __name__ == '__main__':
    main()
