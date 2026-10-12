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
    p.add_argument('--shared-fixture-prefix',type=Path)
    p.add_argument('--claims', type=Path, required=True)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Refusing evidence overwrite')
    claims = json.loads(a.claims.read_text())
    sources = dict(claims['sources']);sources['tools/drawing_fixture_prefix.py']=sha((ROOT/'tools/drawing_fixture_prefix.py').read_bytes())
    for name in ('tools/test-drawing-source-held-snapshot.py', 'tests/drawing-source-held-snapshot-test.c',
                 'runtime/drawing/wine_submission.h', 'runtime/drawing/source_byte_witness.h', 'runtime/drawing/source_held_snapshot.h','tools/drawing_source_held_snapshot.py','tools/drawing_source_pending_admission.py','tools/drawing_source_pending_refusal.py', 'runtime/drawing/source_worker_lease.h', 'runtime/drawing/source_readonly_lease.h', 'runtime/drawing/session.c', 'tools/drawing_source_held_refusal.py', 'tests/test-drawing-source-held-refusal.py'):
        sources[name] = sha((ROOT/name).read_bytes())
    if any(sha((ROOT/n).read_bytes()) != h for n, h in sources.items()):
        raise ValueError('Prospective source changed')
    parent = ROOT/'working/tests/drawing-source-held-snapshot'
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
    subprocess.run(flags+['-c', str(ROOT/'tests/drawing-source-held-snapshot-test.c'), '-o', str(run/'fixture.obj')], check=True)
    binary = run/'fixture.exe'
    subprocess.run(['lld-link', '/machine:x86', '/entry:start', '/subsystem:console', '/nodefaultlib',
                    '/safeseh:no', '/timestamp:0', '/out:'+str(binary), str(run/'fixture.obj'),str(dll.parent/'packet.obj'),
                    str(dll.parent/'kernel32.lib')], check=True)
    from drawing_fixture_prefix import prepare
    prefix,prefix_lock=prepare(ROOT,run,a.prefix_template,a.shared_fixture_prefix)
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(prefix), WINEDEBUG='-all', MNM_DRAW_SESSION_MODE='physical',
               MNM_DRAW02_PRODUCER_BIND='1', MNM_DRAW_SOURCE_BYTES='1',MNM_DRAW_SOURCE_LEASE='1', MNM_DRAW_WINE_SUBMISSION='1', MNM_DRAW_WINE_TEXTURE_ENTRY='1', MNM_DRAW_WINE_BLIT_WORKER='1', MNM_DRAW_WINE_BLIT_PACKET='1', MNM_DRAW_SOURCE_WORKER_LEASE='1', MNM_DRAW_SOURCE_HELD_SNAPSHOT='1', MNM_DRAW_SESSION_DIRECTORY='Z:'+str(capture).replace('/', '\\'))
    try:
        with (run/'wineboot.log').open('x') as out:
            subprocess.run(['wineboot', '-u'], env=env, stdout=out, stderr=subprocess.STDOUT, check=True, timeout=90)
        from drawing_source_held_snapshot import analyze
        from drawing_source_held_refusal import decode as rejection
        cases=[]
        for mode,expected in [('g',0),('c',99),('m',99),('r',99),('b',99),('s',99),('p',0),('e',99)]:
            target=run/mode;target.mkdir()
            if mode in ('p','e'):env['MNM_DRAW_SOURCE_PENDING_COPY']='1'
            else:env.pop('MNM_DRAW_SOURCE_PENDING_COPY',None)
            env.update(MNM_FIXTURE_MODE=mode,MNM_DRAW_SESSION_DIRECTORY='Z:'+str(target).replace('/', '\\'))
            with (target/'fixture.log').open('x') as out:
                execution=subprocess.run(['wine',str(binary)],env=env,stdout=out,stderr=subprocess.STDOUT,timeout=20)
            if execution.returncode!=expected:raise ValueError(f'{mode}: {execution.returncode}, expected {expected}')
            case=dict(mode=mode,returncode=execution.returncode)
            if not expected:
                raw=(target/'source-held-snapshot.bin').read_bytes();observed=analyze(target)
                if observed['stable_snapshot_pairs']!=1 or observed['admitted_planes']!=2:raise ValueError('Protected snapshot closure')
                expected=b''.join(struct.pack('<H',i&65535) for i in range(480000))
                if any((target/f['path']).read_bytes()!=expected for f in observed['frames']):raise ValueError('Protected snapshot pixels')
                case.update(journal_sha256=sha(raw),observed=observed)
            else:
                diagnostic=rejection((target/'source-held-refusal.bin').read_bytes())
                if diagnostic['reason_code']!={'c':5,'m':4,'r':1,'b':3,'s':2,'e':11}[mode]:raise ValueError('Wrong held rejection guard')
                case['rejection']=diagnostic
            cases.append(case)
        if any(sha((ROOT/n).read_bytes()) != h for n,h in sources.items()):
            raise ValueError('Fixture sources changed during execution')
        report = dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],
                      artifacts=str(run.relative_to(ROOT)),shared_fixture_prefix=str(prefix.relative_to(ROOT)) if a.shared_fixture_prefix else None,binary_sha256=sha(binary.read_bytes()),
                      cases=cases,protected_snapshot_pairs=2,exact_begin_end_identity_verified=True,
                      original_game_executed=False,async_egl_completion_covered=False,
                      scope='Eight actual PE32 explicit rejection and protected source cases; protected source pages, ordered copies after protection and before restore, exact V1/V2 plane identity and pending admission file joins; mismatch, changed canvas and copy refusal. No original source ownership or hardware completion.')
        a.report.parent.mkdir(parents=True,exist_ok=True)
        a.report.write_text(json.dumps(report,indent=2)+'\n')
        print(json.dumps(dict(success=True,cases=len(cases))))
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)


if __name__ == '__main__':
    main()
