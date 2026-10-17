#!/usr/bin/env python3
"""Execute owned PE32 worker source-prefix guards without original game execution."""
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
    for name in ('tools/test-drawing-worker-producer-points.py', 'tests/drawing-worker-producer-points-test.c',
                 'runtime/drawing/wine_submission.h', 'runtime/drawing/wine_lifetime.h', 'runtime/drawing/wine_blit_worker.h', 'runtime/drawing/session.c', 'runtime/drawing/worker_producer_points.h', 'protocols/include/mnm/drawing_worker_producer_points_v1.h', 'protocols/include/mnm/drawing_source_point_v1.h', 'tools/drawing_worker_producer_points.py', 'tests/test-drawing-worker-producer-points.py'):
        sources[name] = sha((ROOT/name).read_bytes())
    if any(sha((ROOT/n).read_bytes()) != h for n, h in sources.items()):
        raise ValueError('Prospective source changed')
    parent = ROOT/'working/tests/drawing-worker-producer-points'
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
    subprocess.run(flags+['-c', str(ROOT/'tests/drawing-worker-producer-points-test.c'), '-o', str(run/'fixture.obj')], check=True)
    binary = run/'fixture.exe'
    subprocess.run(['lld-link', '/machine:x86', '/entry:start', '/subsystem:console', '/nodefaultlib',
                    '/safeseh:no', '/timestamp:0', '/out:'+str(binary), str(run/'fixture.obj'),
                    str(dll.parent/'kernel32.lib')], check=True)
    subprocess.run(['cp', '-a', '--reflink=auto', str(a.prefix_template.resolve()), str(run/'wineprefix')], check=True)
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(run/'wineprefix'), WINEDEBUG='-all', MNM_DRAW_SESSION_MODE='physical',
               MNM_DRAW02_PRODUCER_BIND='1', MNM_DRAW_WINE_SUBMISSION='1', MNM_DRAW_WINE_BLIT_WORKER='1', MNM_DRAW_SESSION_DIRECTORY='Z:'+str(capture).replace('/', '\\'))
    try:
        with (run/'wineboot.log').open('x') as out:
            subprocess.run(['wineboot', '-u'], env=env, stdout=out, stderr=subprocess.STDOUT, check=True, timeout=90)
        from drawing_worker_producer_points import decode
        cases=[]
        for name,flag,expected in [('enabled','1',0),('disabled',None,0),('unavailable','1',0),('refused','1',0),('mid-close','1',0),('budget','1',99),('invalid','x',0)]:
            target=run/name;target.mkdir()
            case_env=dict(env,MNM_WORKER_POINT_CASE=name,MNM_DRAW_SESSION_DIRECTORY='Z:'+str(target).replace('/', '\\'))
            if flag is not None:case_env['MNM_DRAW_WINE_WORKER_PRODUCER']=flag
            with (target/'fixture.log').open('x') as out:
                execution=subprocess.run(['wine',str(binary)],env=case_env,stdout=out,stderr=subprocess.STDOUT,timeout=60)
            if execution.returncode!=expected:raise ValueError(name+' fixture exit:'+str(execution.returncode))
            path=target/'wine-worker-producer-points.bin'
            if name in ('disabled','invalid'):
                if path.exists():raise ValueError('Disabled/invalid mode wrote a point journal')
                observation=dict(no_journal=True)
            elif name in ('mid-close','budget'):
                try:decode(path.read_bytes(),(target/'wine-blit-worker.bin').read_bytes())
                except ValueError:observation=dict(incomplete_refused=True,journal_sha256=sha(path.read_bytes()))
                else:raise ValueError('Incomplete worker prefix trace was admitted')
            else:
                observation=decode(path.read_bytes(),(target/'wine-blit-worker.bin').read_bytes())
                if len(observation['calls'])!=3 or observation['unavailable_pairs']!=(3 if name in ('unavailable','refused') else 0):raise ValueError('Fixture prefix counts differ')
                if name=='enabled' and observation['producer_changes_during_worker']!=3:raise ValueError('Original body prefix changes not retained')
            cases.append(dict(case=name,exit=execution.returncode,observation=observation))
        subprocess.run(['python3','-B',str(ROOT/'tests/test-drawing-worker-producer-points.py')],check=True)
        if any(sha((ROOT/n).read_bytes()) != h for n,h in sources.items()):
            raise ValueError('Fixture sources changed during execution')
        report = dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],
                      artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),
                      cases=cases,worker_calls=3,postclose_original_forwards=1,
                      getter_and_callback_reentrant_threads_verified=True,full_512byte_fp_preserved=True,
                      original_game_executed=False,source_submission_identity_bound=False,
                      scope='Seven owned PE32 worker producer-prefix configurations: six source getter queries around three once-only original callbacks with command/error/512-byte FP preserved, outside-lock getter/original reentry, postclose forwarding, disabled/invalid modes, unavailable/refused metadata, forced mid-close and row-budget refusal. No original game, byte ownership, actual source/driver order repair or full hook quiescence.')
        a.report.parent.mkdir(parents=True,exist_ok=True)
        a.report.write_text(json.dumps(report,indent=2)+'\n')
        print(json.dumps(dict(success=True,worker_calls=3,cases=len(cases))))
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)


if __name__ == '__main__':
    main()
