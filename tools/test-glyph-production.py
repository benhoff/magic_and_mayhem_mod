#!/usr/bin/env python3
"""Exercise production borrowed glyph admission beyond the diagnostic call cap."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

from glyph_backend_launch import load, authorize
from glyph_live_capture import inspect

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--wine-prefix', type=Path, required=True)
    args = parser.parse_args()
    claims = json.loads(args.claims.read_text())
    if any(sha(ROOT/p)!=h for p,h in claims['sources'].items()):
        raise ValueError('Prospective sources changed')
    prefix = args.wine_prefix.resolve()
    if not prefix.is_relative_to(ROOT/'working/tests') or not prefix.is_dir():
        raise ValueError('Test prefix must be stopped and owned under working/tests')
    parent = ROOT/'working/tests/glyph-production'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    report = dict(schema=1, success=False, sources=claims['sources'], claims=claims['claims'],
        scope='Controlled fake process globals, valid disjoint two-pixel RLE/frame/table/canvas leases: actual production memory resolver and capture-disabled borrowed kernel,65545 admitted RGB565/555 requests past prior65536 diagnostic budget, no drawing-path WriteFile calls, statsV2 and resident authorization guards. No original EXE execution, installed hook/frame-return stream, World synchronization or live gameplay equivalence.',
        experiment=str(out.relative_to(ROOT)), live_replacement=False)
    env = {k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEDEBUG='-all', WINEPREFIX=str(prefix), WINEDLLOVERRIDES='ddraw=')
    commands = []

    def run(label, argv, extra=None):
        with (out/(label+'.log')).open('x') as log:
            r = subprocess.run([str(a) for a in argv], cwd=out, env={**env, **(extra or {})}, stdout=log, stderr=log, timeout=180)
        commands.append(dict(label=label, returncode=r.returncode, log=str((out/(label+'.log')).relative_to(ROOT))))
        r.check_returncode()
    try:
        run('scene-build', ['python3', ROOT/'tools/build-scene-observer.py', '--selftest'])
        game = out/'game'
        game.mkdir()
        shutil.copyfile(ROOT/'working/build/scene-observer-selftest/MnmScene.dll', game/'MnmScene.dll')
        build = json.loads((ROOT/'working/build/scene-observer-selftest/manifest.json').read_text())
        if set(build['sources'])-set(claims['sources']):
            raise ValueError('Undeclared native scene build sources: '+repr(sorted(set(build['sources'])-set(claims['sources']))))
        report['scene_build'] = build
        imports = {'LoadLibraryA':4,'GetProcAddress':8,'GetEnvironmentVariableA':12,'SetEnvironmentVariableA':8,'ExitProcess':4}
        definition = out/'kernel32.def'
        definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in imports.items()))
        run('imports', ['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
        run('probe-build', ['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector',
            '-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',ROOT/'tests/glyph-production-probe.c','-o',out/'probe.obj'])
        run('probe-link', ['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no',
            '/timestamp:0','/out:'+str(game/'probe.exe'),out/'probe.obj',out/'kernel32.lib'])
        directory = out/'glyph-backend'
        directory.mkdir()
        metadata = dict(glyph_backend_mode='takeover', glyph_capture=False, glyph_directory=str(directory),
                        glyph_dll_sha256=sha(game/'MnmScene.dll'),resident_world=True,native_draw=True,native_render=True)
        authorized = {}
        authorize(out, metadata, authorized)
        if authorized.get('MNM_GLYPH_CAPTURE')!='0':
            raise ValueError('Production capture policy was not authorized')
        refusals = 0
        for changed, environment in [(dict(metadata,glyph_capture=True),{}),(dict(metadata,glyph_backend_mode='shadow'),{}),
            (dict(metadata,native_draw=False),{}),(metadata,dict(MNM_WORD_SPRITES='takeover'))]:
            try:
                authorize(out, changed, environment)
            except ValueError:
                refusals += 1
        if refusals!=4:
            raise ValueError('Resident glyph authorization failed to refuse')
        # DLL initially loads with glyph mode unset; exported controlled test
        # configures production after its fake globals exist. No hook installs.
        run('production-admission', ['xvfb-run','-a','wine',game/'probe.exe'],
            {'MNM_TEST_GLYPH_DIR':'Z:'+str(directory).replace('/','\\')})
        result = inspect(directory,'takeover')
        if result['stats']['handled']!=65545 or result['stats']['budget'] or result['stats']['forwarded']:
            raise ValueError('Unbounded production admission accounting mismatch')
        report.update(stats=result['stats'], artifacts=result['artifacts'], production_policy_refusals=refusals,
                      source_snapshot_allocations=0, full_canvas_trial_copies=0, drawing_writefile_calls=0, success=True)
    except Exception as error:
        report['error'] = repr(error)
    finally:
        subprocess.run(['wineserver','-k'],env=env,timeout=10,check=False)
        subprocess.run(['wineserver','-w'],env=env,timeout=10,check=False)
        report['sources_stable'] = all(sha(ROOT/p)==h for p,h in claims['sources'].items())
        report['success'] = report['success'] and report['sources_stable']
        report['commands'] = commands
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
