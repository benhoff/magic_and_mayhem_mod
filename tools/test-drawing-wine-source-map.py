#!/usr/bin/env python3
"""Execute owned PE32 public mapped source snapshots and original blit guards."""
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
    for name in ('tools/test-drawing-wine-source-map.py', 'tests/drawing-wine-source-map-test.c',
                 'runtime/drawing/wine_submission.h', 'runtime/drawing/source_byte_witness.h', 'runtime/drawing/wine_resource_mapping.h','runtime/drawing/wine_source_descriptor.h','runtime/drawing/wine_source_map.h','tools/drawing_wine_source_map.py','tools/drawing_fixture_prefix.py', 'runtime/drawing/session.c'):
        sources[name] = sha((ROOT/name).read_bytes())
    if any(sha((ROOT/n).read_bytes()) != h for n, h in sources.items()):
        raise ValueError('Prospective source changed')
    parent = ROOT/'working/tests/drawing-wine-source-map'
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
    subprocess.run(flags+['-c', str(ROOT/'tests/drawing-wine-source-map-test.c'), '-o', str(run/'fixture.obj')], check=True)
    binary = run/'fixture.exe'
    subprocess.run(['lld-link', '/machine:x86', '/entry:start', '/subsystem:console', '/nodefaultlib',
                    '/safeseh:no', '/timestamp:0', '/out:'+str(binary), str(run/'fixture.obj'),str(dll.parent/'packet.obj'),
                    str(dll.parent/'kernel32.lib')], check=True)
    from drawing_fixture_prefix import prepare
    prefix,prefix_lock=prepare(ROOT,run,a.prefix_template,a.shared_fixture_prefix)
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(prefix), WINEDEBUG='-all', MNM_DRAW_SESSION_MODE='physical',
               MNM_DRAW02_PRODUCER_BIND='1', MNM_DRAW_WINE_SUBMISSION='1',MNM_DRAW_WINE_RESOURCE_MAP='1',MNM_DRAW_WINE_TEXTURE_ENTRY='1',MNM_DRAW_WINE_SOURCE_DESCRIPTOR='1',MNM_DRAW_WINE_SOURCE_MAP='1', MNM_DRAW_SESSION_DIRECTORY='Z:'+str(capture).replace('/', '\\'))
    try:
        with (run/'wineboot.log').open('x') as out:
            subprocess.run(['wineboot', '-u'], env=env, stdout=out, stderr=subprocess.STDOUT, check=True, timeout=90)
        with (run/'fixture.log').open('x') as out:
            execution = subprocess.run(['wine', str(binary)], env=env, stdout=out, stderr=subprocess.STDOUT, timeout=60)
        execution.check_returncode()
        from drawing_wine_source_map import decode
        raw=(capture/'wine-source-map.bin').read_bytes();observed=decode(raw,capture)
        statuses=[0,2,2,4,5,6,6,6,6,7,0,0,2]+[0]*8+[3,1,1,1,1]
        if len(observed['samples'])!=26 or observed['retained_payloads']!=11 or observed['map_attempts']!=[16,1] or [r['status'] for r in observed['samples']]!=statuses:raise ValueError('Source map fixture closure')
        forwards=(capture/'fixture-forwards.bin').read_bytes()
        if list(struct.iter_unpack('<I',forwards))!=[(i,) for i in range(1,28)]:raise ValueError('Original forward count')
        for payload in observed['payloads']:
            expected=bytes((x*3+y*5+7)&255 for y in range(payload['height']) for x in range(payload['width']*2))
            if (capture/payload['path']).read_bytes()!=expected:raise ValueError('Owned dense rows differ after unmap mutation')
        negatives=[]
        for fatal,status in [('1',9),('2',10)]:
            target=run/('fatal-'+fatal);target.mkdir()
            trial=dict(env,MNM_TEST_SOURCE_MAP_FATAL=fatal,MNM_DRAW_SESSION_DIRECTORY='Z:'+str(target).replace('/', chr(92)))
            with (run/('fatal-'+fatal+'.log')).open('x') as log:
                failed=subprocess.run(['wine',str(binary)],env=trial,stdout=log,stderr=subprocess.STDOUT,timeout=60)
            prefix=(target/'wine-source-map.bin').read_bytes()
            if failed.returncode!=99 or (target/'fixture-forwards.bin').read_bytes() or len(prefix)!=320 or struct.unpack_from('<64I',prefix,64)[10]!=status:raise ValueError('Fatal map must stop before original forwarding')
            try:decode(prefix,target)
            except ValueError:pass
            else:raise ValueError('Incomplete fatal map accepted')
            negatives.append(dict(exit_code=99,status=status,original_forwards=0,raw_sha256=sha(prefix)))
        controls=subprocess.run(['python3',str(ROOT/'tests/test-drawing-wine-source-map.py')],capture_output=True,text=True,check=True)
        if any(sha((ROOT/n).read_bytes()) != h for n,h in sources.items()):
            raise ValueError('Fixture sources changed during execution')
        report=dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),journal_sha256=sha(raw),original_blits=27,map_attempts=17,unmaps=16,retained_payloads=11,retained_bytes=sum(p['size'] for p in observed['payloads']),default_disabled_queries=0,original_entry_and_return_fp_preserved=True,original_last_error_preserved=True,owned_copies_survive_unmap_mutation=True,fatal_guards=negatives,analysis=observed,original_game_executed=False,scope='Owned PE32 read-only mapping/copy/unmap/original forwarding and default-off/range/format/access/active-map/budget/generation refusals. Failed unmap and reentrant diagnostics abort before forwarding; driver/real source ownership and whole acquisition remain pending.')
        a.report.parent.mkdir(parents=True,exist_ok=True)
        a.report.write_text(json.dumps(report,indent=2)+'\n')
        print(json.dumps(dict(success=True,original_blits=27,retained_payloads=11)))
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        if prefix_lock is not None:prefix_lock.close()


if __name__ == '__main__':
    main()
