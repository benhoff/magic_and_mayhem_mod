#!/usr/bin/env python3
"""Owned PE32 source/worker version races, aliases, reuse and ABI guards."""
import argparse,importlib.util,json,os,struct,subprocess,tempfile
from pathlib import Path
from drawing_api_session import sha
from drawing_source_version import decode
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--prefix-template',type=Path,required=True);p.add_argument('--shared-fixture-prefix',type=Path)
    p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    claims=json.loads(a.claims.read_text());sources=claims['sources']
    def stable():
        if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective fixture sources changed')
    stable();parent=ROOT/'working/tests/drawing-source-version';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(run/'.retain').write_text('Owned source-version guard fixture, preserve binaries and refusal evidence.\n')
    spec=importlib.util.spec_from_file_location('drawing_build',ROOT/'tools/build-drawing-session.py');build=importlib.util.module_from_spec(spec);spec.loader.exec_module(build);dll=build.build(True)
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-Wno-unused-function']
    subprocess.run(flags+['-c',str(ROOT/'tests/drawing-source-version-test.c'),'-o',str(run/'fixture.obj')],check=True)
    binary=run/'fixture.exe';subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),str(run/'fixture.obj'),str(dll.parent/'packet.obj'),str(dll.parent/'kernel32.lib')],check=True)
    from drawing_fixture_prefix import prepare
    prefix,lock=prepare(ROOT,run,a.prefix_template,a.shared_fixture_prefix)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(prefix),WINEDEBUG='-all',MNM_DRAW_SESSION_MODE='physical',MNM_DRAW_WINE_SUBMISSION='1',MNM_DRAW_WINE_TEXTURE_ENTRY='1',MNM_DRAW_WINE_RESOURCE_MAP='1',MNM_DRAW_WINE_RESOURCE_COPY='1',MNM_DRAW_WINE_SOURCE_DESCRIPTOR='1',MNM_DRAW_WINE_SOURCE_MAP='2',MNM_DRAW_WINE_BLIT_WORKER='1',MNM_DRAW_WINE_BLIT_PACKET='1',MNM_DRAW_WINE_SOURCE_VERSION='1')
    try:
        with (run/'wineboot.log').open('x') as out:subprocess.run(['wineboot','-u'],env=env,stdout=out,stderr=subprocess.STDOUT,check=True,timeout=90)
        results=[]
        for mode,reason,forwards in [('0',None,1),('7',None,1),('8',None,1),('1',8,0),('2',8,0),('3',3,0),('4',9,1),('5',8,0),('6',7,0),('9',2,0),('a',None,1),('b',None,1),('off',None,1)]:
            target=run/('case-'+mode);target.mkdir();trial=dict(env,MNM_TEST_VERSION='0' if mode=='off' else mode,MNM_DRAW_SESSION_DIRECTORY='Z:'+str(target).replace('/',chr(92)))
            if mode=='off':trial.pop('MNM_DRAW_WINE_SOURCE_VERSION')
            with (target/'wine.log').open('x') as out:x=subprocess.run(['wine',str(binary)],env=trial,stdout=out,stderr=subprocess.STDOUT,timeout=30)
            raw_forwards=(target/'fixture-worker-forwards.bin').read_bytes()
            if raw_forwards!=struct.pack('<I',1)*forwards:raise ValueError('Original worker forward count: '+mode+' exit'+str(x.returncode))
            observed=None;raw=b''
            if mode=='off':
                if x.returncode or (target/'wine-source-version.bin').exists():raise ValueError('Default-off version guard changed forwarding')
            else:
                raw=(target/'wine-source-version.bin').read_bytes()
                if reason is None:
                    if x.returncode:raise ValueError('Owned positive failed: '+mode+' exit'+str(x.returncode))
                    observed=decode(raw)
                    if observed['checked_workers']!=1:raise ValueError('Owned worker closure')
                    if mode=='b' and (len(observed['open_write_leases'])!=1 or observed['all_global_write_leases_closed']):raise ValueError('Open boundary lease hidden')
                    if mode=='a':
                        snapshot=(target/'wine-source-map-000001.565').read_bytes()[0]
                        consumed=(target/'fixture-consumed-byte.bin').read_bytes()[0]
                        if snapshot!=3 or consumed!=2 or observed['original_source_atomicity_proved']:raise ValueError('Unobserved direct-write limitation hidden')
                else:
                    last=struct.unpack_from('<32I',raw,len(raw)-128)
                    if x.returncode!=99 or last[1]!=8 or last[16]!=reason:raise ValueError('Owned refusal differs: '+mode+' exit'+str(x.returncode)+' reason'+str(last[16]))
                    try:decode(raw)
                    except ValueError:pass
                    else:raise ValueError('Incomplete refused version journal accepted')
            results.append(dict(mode=mode,exit_code=x.returncode,refusal_reason=reason,original_worker_forwards=forwards,journal_sha256=sha(raw) if raw else None,analysis=observed))
        controls=subprocess.run(['python3',str(ROOT/'tests/test-drawing-source-version.py')],capture_output=True,text=True,check=True);stable()
        report=dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),cases=results,parser_checks=controls.stderr,original_game_executed=False,worker_entry_return_fp_and_last_error_preserved=True,rebind_entry_return_fp_and_last_error_preserved=True,unobserved_direct_write_can_escape=True,scope='Owned actual PE32 two-thread alias write/restore and open write lease, generation reuse, write during worker, conservative failed rebind, wrong subresource and mutation during read/unmap refusal. Positive normal/self-alias/unrelated write and default-off forwarding. Direct unhooked write deliberately changes the consumed byte while this observed-version guard accepts: preserved proof of its limitation, not atomicity. No original source atomicity/unobserved writes/GPU completion.')
        a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=True,cases=len(results))))
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        if lock is not None:lock.close()


if __name__=='__main__':main()
