#!/usr/bin/env python3
"""Owned PE32 read reservations, pre-write refusal and exact fenced retirement."""
import argparse,importlib.util,json,os,struct,subprocess,tempfile
from pathlib import Path
from drawing_api_session import sha
from drawing_source_reservations import decode
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--prefix-template',type=Path,required=True);p.add_argument('--shared-fixture-prefix',type=Path)
    p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    claims=json.loads(a.claims.read_text());sources=claims['sources']
    def stable():
        if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective fixture sources changed')
    stable();parent=ROOT/'working/tests/drawing-source-reservations';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(run/'.retain').write_text('Owned source-version guard fixture, preserve binaries and refusal evidence.\n')
    spec=importlib.util.spec_from_file_location('drawing_build',ROOT/'tools/build-drawing-session.py');build=importlib.util.module_from_spec(spec);spec.loader.exec_module(build);dll=build.build(True)
    flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-Wno-unused-function']
    subprocess.run(flags+['-c',str(ROOT/'tests/drawing-source-reservations-test.c'),'-o',str(run/'fixture.obj')],check=True)
    binary=run/'fixture.exe';subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),str(run/'fixture.obj'),str(dll.parent/'packet.obj'),str(dll.parent/'kernel32.lib')],check=True)
    from drawing_fixture_prefix import prepare
    prefix,lock=prepare(ROOT,run,a.prefix_template,a.shared_fixture_prefix)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(prefix),WINEDEBUG='-all',MNM_DRAW_SESSION_MODE='physical',MNM_DRAW_WINE_SUBMISSION='1',MNM_DRAW_WINE_TEXTURE_ENTRY='1',MNM_DRAW_WINE_RESOURCE_MAP='1',MNM_DRAW_WINE_RESOURCE_COPY='1',MNM_DRAW_WINE_SOURCE_DESCRIPTOR='1',MNM_DRAW_WINE_SOURCE_MAP='2',MNM_DRAW_WINE_BLIT_WORKER='1',MNM_DRAW_WINE_BLIT_PACKET='1',MNM_DRAW_WINE_SOURCE_VERSION='1',MNM_DRAW_WINE_GPU_COMPLETION='1',MNM_DRAW_WINE_SOURCE_RESERVATIONS='1')
    try:
        with (run/'wineboot.log').open('x') as out:subprocess.run(['wineboot','-u'],env=env,stdout=out,stderr=subprocess.STDOUT,check=True,timeout=90)
        results=[]
        cases=[('0','0',None,1),('1','0',1,0),('2','0',1,0),('4','0',1,1),('5','0',1,0),('7','0',1,0),('8','0',None,1),('9','0',1,0),('a','0',None,1),('0','1',7,1),('0','2',9,0),('0','3',1,0),('0','4',11,0),('0','5',1,1),('0','6',None,1),('0','9',1,1),('0','a',12,0),('0','b',None,1),('off','0',None,1)]
        for mode,reservation,reason,forwards in cases:
            target=run/('case-'+mode+'-'+reservation);target.mkdir();trial=dict(env,MNM_TEST_VERSION='0' if mode=='off' else mode,MNM_TEST_RESERVATION=reservation,MNM_DRAW_SESSION_DIRECTORY='Z:'+str(target).replace('/',chr(92)))
            if mode=='off':trial.pop('MNM_DRAW_WINE_SOURCE_RESERVATIONS')
            with (target/'wine.log').open('x') as out:x=subprocess.run(['wine',str(binary)],env=trial,stdout=out,stderr=subprocess.STDOUT,timeout=30)
            raw_forwards=(target/'fixture-worker-forwards.bin').read_bytes()
            if raw_forwards!=struct.pack('<I',1)*forwards:raise ValueError('Original worker forward count: '+mode+' exit'+str(x.returncode))
            observed=None;raw=b''
            if mode=='off':
                if x.returncode or (target/'wine-source-reservations.bin').exists():raise ValueError('Default-off version guard changed forwarding')
            else:
                raw=(target/'wine-source-reservations.bin').read_bytes()
                if reason is None:
                    if x.returncode:raise ValueError('Owned positive failed: '+mode+' exit'+str(x.returncode))
                    observed=decode(raw)
                    if observed['reservations']!=1:raise ValueError('Owned worker closure')
                    if mode=='a':
                        snapshot=(target/'wine-source-map-000001.565').read_bytes()[0]
                        consumed=(target/'fixture-consumed-byte.bin').read_bytes()[0]
                        if snapshot!=3 or consumed!=2 or observed['physical_backing_owned']:raise ValueError('Unobserved direct-write limitation hidden')
                else:
                    last=struct.unpack_from('<32I',raw,len(raw)-128)
                    if x.returncode!=99 or last[1]!=8 or last[15]!=reason:raise ValueError('Owned refusal differs: '+mode+' exit'+str(x.returncode)+' reason'+str(last[15]))
                    try:decode(raw)
                    except ValueError:pass
                    else:raise ValueError('Incomplete refused version journal accepted')
            results.append(dict(mode=mode,reservation_case=reservation,exit_code=x.returncode,refusal_reason=reason,original_worker_forwards=forwards,journal_sha256=sha(raw) if raw else None,analysis=observed))
        controls=subprocess.run(['python3',str(ROOT/'tests/test-drawing-source-reservations.py')],capture_output=True,text=True,check=True);stable()
        report=dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),cases=results,parser_checks=controls.stderr,original_game_executed=False,worker_entry_return_fp_and_last_error_preserved=True,unobserved_direct_write_can_escape=True,physical_backing_owned=False,scope='Observed public-resource read reservation before READ map through exact worker/fence; observed writes, self alias, rebind, decref and early close fail before forwarding. Direct unhooked writes remain a retained limitation. No distinct-resource physical alias or native replacement proof.')
        a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=True,cases=len(results))))
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        if lock is not None:lock.close()


if __name__=='__main__':main()
