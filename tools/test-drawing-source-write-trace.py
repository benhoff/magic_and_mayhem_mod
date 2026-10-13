#!/usr/bin/env python3
"""Run actual PE32 protected-page writes through distinct cached aliases."""
import argparse
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
from drawing_api_session import sha
from drawing_source_write_trace import snapshot, marker, fault, resolve_writer

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--prefix-template',type=Path,required=True)
    p.add_argument('--shared-fixture-prefix',type=Path)
    p.add_argument('--claims',type=Path,required=True)
    p.add_argument('--report',type=Path,required=True)
    a = p.parse_args()
    if a.report.exists(): p.error('Refusing evidence overwrite')
    claims = json.loads(a.claims.read_text()); sources = dict(claims['sources'])
    for name in ('runtime/scene/source_write_trace.h','runtime/scene/drawing_source_lease.h','runtime/scene/drawing_source_point.h','runtime/scene/drawing_source_bytes.h','runtime/scene/source_pending_policy.h','runtime/scene/producer_trace_guard.h','runtime/shadow/win32_min.h','tools/drawing_source_write_trace.py','tools/drawing_fixture_prefix.py','tests/drawing-source-write-trace-test.c','tests/test-drawing-source-write-trace.py','tools/test-drawing-source-write-trace.py'):
        sources[name] = sha((ROOT/name).read_bytes())
    def stable():
        if any(sha((ROOT/n).read_bytes()) != h for n,h in sources.items()):
            raise ValueError('Prospective sources changed')
    stable()
    parent = ROOT/'working/tests/drawing-source-write-trace'; parent.mkdir(parents=True,exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-',dir=parent)); print(run,flush=True)
    (run/'.retain').write_text('DRAW-02 writer tracing fixture and raw fault/alias evidence.\n')
    imports = dict(GetTickCount=0,ExitProcess=4,GetEnvironmentVariableA=12,GetLastError=0,SetLastError=4,InitializeCriticalSection=4,EnterCriticalSection=4,LeaveCriticalSection=4,GetCurrentThreadId=0,CreateThread=24,WaitForSingleObject=8,CloseHandle=4,VirtualQuery=12,VirtualAlloc=16,VirtualProtect=16,AddVectoredExceptionHandler=8,CreateFileA=28,WriteFile=20,CreateFileMappingA=24,MapViewOfFile=20)
    definition = run/'kernel32.def'; definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in imports.items()))
    subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(definition),'-l',str(run/'kernel32.lib')],check=True)
    flags = ['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
    subprocess.run(flags+['-c',str(ROOT/'tests/drawing-source-write-trace-test.c'),'-o',str(run/'fixture.obj')],check=True)
    binary = run/'fixture.exe'
    subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/map:'+str(run/'fixture.map'),'/out:'+str(binary),str(run/'fixture.obj'),str(run/'kernel32.lib')],check=True)
    from drawing_fixture_prefix import prepare
    prefix,lock = prepare(ROOT,run,a.prefix_template,a.shared_fixture_prefix)
    env = {k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEPREFIX=str(prefix),WINEDEBUG='-all',MNM_DRAW02_PRODUCER_BIND='1',MNM_DRAW_SOURCE_LEASE='1',MNM_DRAW_SOURCE_WRITE_TRACE='1')
    results = []
    try:
        with (run/'wineboot.log').open('x') as out:
            subprocess.run(['wineboot','-u'],env=env,stdout=out,stderr=subprocess.STDOUT,check=True,timeout=90)
        cases = [('normal','0',False),('direct','1',True),('exact-alias','2',True),('partial-alias','3',True),('upper-page-halo','4',True),('arming-window','5',True),('unrelated-same-reservation','6',False),('second-virtual-mapping-escape','7',False),('lower-page-halo','8',True),('default-off','0',False),('default-off-fault','1',True),('invalid-flag','9',False)]
        for label,mode,refused in cases:
            target = run/label; target.mkdir(); trial = dict(env,MNM_WRITE_TEST=mode)
            if label.startswith('default-off'): trial.pop('MNM_DRAW_SOURCE_WRITE_TRACE')
            if label == 'invalid-flag': trial['MNM_DRAW_SOURCE_WRITE_TRACE'] = 'x'
            with (target/'wine.log').open('x') as out:
                x = subprocess.run(['wine',str(binary)],cwd=target,env=trial,stdout=out,stderr=subprocess.STDOUT,timeout=30)
            if x.returncode != (99 if refused else 0):
                raise ValueError(f'{label}: exit {x.returncode}')
            observed = None; attribution = None
            if label.startswith('default-off') or label == 'invalid-flag':
                if list(target.glob('source-write-*.bin')): raise ValueError('Disabled trace produced files')
            else:
                observed = snapshot((target/'source-write-trace-0001.bin').read_bytes())
                if len(observed['canvases']) != 6: raise ValueError('Canvas cache snapshot incomplete')
                if label == 'arming-window':
                    if (target/'source-write-armed-0001.bin').exists(): raise ValueError('Arming fault silently admitted')
                else: marker((target/'source-write-armed-0001.bin').read_bytes(),observed,2)
                if refused:
                    record = fault((target/'source-write-fault-0001.bin').read_bytes(),observed)
                    if not record['foreign_thread'] or record['phase'] != (1 if label=='arming-window' else 2): raise ValueError('Writer/phase identity')
                    expected = [1,2,3] if label=='partial-alias' else [4] if label=='upper-page-halo' else [] if label=='lower-page-halo' else [1,2]
                    if record['recorded_pixel_aliases'] != expected: raise ValueError('Fault target alias classification')
                    if record['target_is_source_pixel'] != (label not in ('upper-page-halo','lower-page-halo')): raise ValueError('Page halo classified as source bytes')
                    if (target/'source-write-done-0001.bin').exists(): raise ValueError('Faulted lease closed')
                    attribution = resolve_writer(record,[binary])
                    if not attribution['resolved']: raise ValueError('Exact fixture instruction attribution failed')
                    observed['fault'] = record
                else:
                    marker((target/'source-write-done-0001.bin').read_bytes(),observed,3)
                    if label == 'second-virtual-mapping-escape':
                        pair = (target/'fixture-shared-read.bin').read_bytes()
                        if pair != struct.pack('<2H',0x1111,0xabcd): raise ValueError('Separate mapping escape not exercised')
                        observed['shared_mapping_bytes_changed'] = True
            if refused:
                legacy = (target/'source-lease-fault-0001.bin').read_bytes()
                if len(legacy)!=48 or legacy[:8]!=b'MNMSLFT1': raise ValueError('Legacy fail-stop evidence changed')
            results.append(dict(case=label,exit_code=x.returncode,observation=observed,writer_attribution=attribution))
        controls = subprocess.run(['python3',str(ROOT/'tests/test-drawing-source-write-trace.py')],capture_output=True,text=True,check=True)
        stable()
        report = dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary.read_bytes()),cases=results,parser_checks=controls.stderr,same_virtual_address_direct_writes_refused=True,arming_window_write_refused=True,exact_protection_restored=True,default_off_forwarding_preserved=True,separate_virtual_mapping_can_escape=True,original_game_executed=False,scope='Owned PE32 actual foreign stores, equal/partial pixel aliases and page-only overlaps; normalized writer instruction resolves to exact fixture bytes. Shared file-mapping views prove another virtual mapping can change protected source bytes. No original backing ownership, all-alias atomicity or GPU completion proof.')
        a.report.parent.mkdir(parents=True,exist_ok=True)
        with a.report.open('x') as out: json.dump(report,out,indent=2);out.write('\n')
        print(json.dumps(dict(success=True,cases=len(results))))
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        if lock is not None: lock.close()


if __name__ == '__main__': main()
