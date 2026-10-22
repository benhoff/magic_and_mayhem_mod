#!/usr/bin/env python3
"""Execute ten bounded display/submission/source observers with SQLite evidence."""
import argparse,ast,hashlib,importlib.util,json,os,re,shlex,struct,subprocess,sys,tempfile,time
from pathlib import Path
from drawing_fixture_prefix import prepare
from coverage_evidence import write_report,evidence_digest
ROOT=Path(__file__).resolve().parents[1]
BEHAVIORS=['NR.drawing-'+name for name in ['x11-delivery-observation','glx-delivery-observation','egl-delivery-observation','egl-physical-witness','wine-submission-observation','wine-submission-generations','api-source-byte-witness','source-readonly-lease','source-held-snapshot','aux-save-readback']]
SCENARIO='NR.tenth-ten.observer-completion-20261022'
TESTS=['tests/test-drawing-'+name+'.py' for name in ['x11-delivery','egl-physical','egl-physical-binding','wine-imports','wine-submission','wine-lifetime','source-byte-witness','source-lease','source-held-pending','source-held-refusal','aux-save-readback']]+['tests/test-rendering-tenth-ten.py']
NATIVE=['drawing-wine-submission','drawing-source-byte-export','drawing-source-byte-recorder','drawing-source-lease','drawing-source-lease-recorder','drawing-source-held-snapshot','drawing-aux-save-readback']
def sha(path):return evidence_digest(Path(path),root=ROOT)


def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path)
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report',type=Path,required=True)
    parser.add_argument('--wine-prefix',type=Path,required=True)
    parser.add_argument('--prefix-owner-root',type=Path,required=True)
    args=parser.parse_args()
    report_path=args.report if args.report.is_absolute() else ROOT/args.report
    if report_path.exists():parser.error('Refusing evidence overwrite')
    parent=ROOT/'working/tests/rendering-tenth-ten';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    (out/'.retain').write_text('Tenth rendering batch raw controls and failed attempts. Generated evidence is in SQLite.\n')
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    declaration=module('tenth_claims','tools/draft-coverage-claims.py').declaration(ROOT,register,BEHAVIORS,[SCENARIO])
    todo=set(declaration['sources'])|set(TESTS)|{'tools/test-rendering-tenth-ten.py','tools/coverage_evidence.py','tools/coverage_claims.py','tools/draft-coverage-claims.py','tools/drawing_observed_inputs.py'}
    todo.update({'tools/rendering_egl_fixture_peer.py','tests/drawing-egl-root-peer.c','tests/drawing-egl-delivery-test.c','tests/drawing-glx-delivery-test.c','tests/drawing-x11-delivery-test.c','tests/drawing-glx-alias-provider.c','tests/drawing-glx-alias-test.c','tools/build-drawing-x11-delivery.py','tools/build-menu-observer.py','tools/build-drawing-session.py','tools/build-drawing-process-exit.py','tools/build-shadow-bridge.py','tools/drawing_fixture_prefix.py','tools/drawing_process_exit.py','tests/canvas-producers-v4-test.cpp','runtime/drawing/process_exit_observer.c'})
    todo.update('tests/'+name+'-test.c' for name in NATIVE if not name.endswith('-child'))
    for directory in ('runtime',):
        todo.update(p.relative_to(ROOT).as_posix() for p in (ROOT/directory).rglob('*') if p.is_file() and (p.suffix in {'.c','.h','.S','.cpp','.hpp','.inc'} or p.name=='CMakeLists.txt'))
    visited=set()
    while todo:
        name=todo.pop()
        if name in visited:continue
        visited.add(name);path=ROOT/name;declaration['sources'][name]=sha(path)
        if path.suffix=='.py':
            for node in ast.walk(ast.parse(path.read_text())):
                if isinstance(node,ast.Constant) and isinstance(node.value,str) and node.value.endswith('.py'):
                    for candidate in (ROOT/node.value,path.parent/node.value,ROOT/'tools'/node.value,ROOT/'tests'/node.value):
                        if candidate.is_file() and candidate.resolve().is_relative_to(ROOT):todo.add(candidate.resolve().relative_to(ROOT).as_posix())
                names=([alias.name for alias in node.names] if isinstance(node,ast.Import) else
                  [node.module]+[node.module+'.'+alias.name for alias in node.names] if isinstance(node,ast.ImportFrom) and node.module else [])
                for name in names:
                    relative=Path(*name.split('.'))
                    for base in (ROOT/'tools',ROOT/'protocols/python'):
                        for candidate in (base/relative.with_suffix('.py'),base/relative/'__init__.py'):
                            if candidate.is_file():todo.add(candidate.relative_to(ROOT).as_posix())
        if path.suffix in {'.c','.cpp','.h','.hpp','.S','.inc'}:
            for include in re.findall(r'^\s*#\s*include\s*"([^"]+)"',path.read_text(),re.M):
                child=(path.parent/include).resolve()
                if child.is_file() and child.is_relative_to(ROOT):todo.add(child.relative_to(ROOT).as_posix())
    report=dict(schema=1,success=False,sources=declaration['sources'],claims=declaration['claims'],commands=[],tests_run=0,
        scope=__doc__,original_game_executed=False,original_artifacts_consumed=False,original_comparison_claimed=False,live_replacement=False,
        reviewed_head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),artifacts=out.relative_to(ROOT).as_posix())
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
    (out/'tmp').mkdir();env.update(TMPDIR=str(out/'tmp'),PYTHONDONTWRITEBYTECODE='1',WINEPREFIX=str(args.wine_prefix.resolve()),WINEDEBUG='-all')
    lock=None
    def private_display(command):
        # Xvfb's xkbcomp uses hardcoded /tmp, independent of TMPDIR.
        temporary=Path(tempfile.mkdtemp(prefix='x11-host-',dir=out))
        (temporary/'.X11-unix').mkdir(mode=0o1777);(temporary/'.X11-unix').chmod(0o1777)
        shared=(ROOT/'working/coverage').resolve()
        return ['bwrap','--die-with-parent','--new-session','--unshare-user','--unshare-pid','--unshare-net','--unshare-ipc','--unshare-uts','--ro-bind','/','/','--bind',str(ROOT),str(ROOT),'--bind',str(shared),str(shared),'--bind',str(temporary),'/tmp','--proc','/proc','--dev','/dev','--setenv','LIBGL_ALWAYS_SOFTWARE','1','--setenv','__EGL_VENDOR_LIBRARY_FILENAMES','/usr/share/glvnd/egl_vendor.d/50_mesa.json','--',*map(str,command)]
    def run(label,command,expected=0,extra=None,cwd=ROOT,timeout=120):
        command=[str(x) for x in command]
        if command[0]=='xvfb-run':command=private_display([command[0],'-d','-e',str(out/(label+'-server.log')),*command[1:]])
        print(label,flush=True)
        process=subprocess.run(command,cwd=cwd,env={**env,**(extra or {})},capture_output=True,text=True,timeout=timeout)
        log=process.stdout+process.stderr;(out/(label+'.log')).write_text(log)
        report['commands'].append(dict(label=label,argv=command,expected_exit=expected,actual_exit=process.returncode,log=log,log_sha256=hashlib.sha256(log.encode()).hexdigest()))
        if process.returncode not in (expected if isinstance(expected,tuple) else (expected,)):raise ValueError(label+': '+log)
        return log
    try:
        owner=args.prefix_owner_root.resolve();prefix=args.wine_prefix.resolve()
        if owner!=ROOT and (owner.parent!=ROOT.parent or not (owner/'.git').is_file()):raise ValueError('Prefix owner must be a registered sibling')
        prefix,lock=prepare(owner,out,prefix,prefix)
        report['fixture_prefix']=dict(owner=str(owner),path=str(prefix),exclusive_lock=True,idle_before_use=True)
        run('wine-version',['wine','--version']);run('compiler-version',['clang','--version']);run('wineboot',['wineboot','-u'])
        run('drawing-production',[sys.executable,'-B',ROOT/'tools/build-drawing-session.py'])
        manifest=json.loads((ROOT/'working/build/drawing-session/manifest.json').read_text())
        for name,digest in manifest['sources'].items():
            if report['sources'].get(name)!=digest:raise ValueError('Undeclared production input: '+name)
        builder=module('tenth_display_builder','tools/build-drawing-x11-delivery.py')
        libraries={}
        # Capture builder subprocesses through our command recorder as well.
        for name,source,version in [('x11','x11_delivery.c',1),('glx','glx_delivery.c',2),('egl','egl_delivery.c',3)]:
            library=out/('lib-'+name+'.so');dep=out/(name+'.d')
            run(name+'-library',['gcc','-std=c11','-O2','-fPIC','-shared','-Wall','-Wextra','-Werror','-pthread','-MD','-MF',dep,ROOT/'runtime/display'/source,*([ROOT/'runtime/display/glx_symbol.S'] if version>1 else []),'-ldl','-lX11',*(['-lxcb','-lxcb-shm'] if version==3 else []),'-o',library])
            libraries[name]=library
        from drawing_x11_delivery import decode_delivery
        display_env=dict(LIBGL_ALWAYS_SOFTWARE='1')
        report['display']={}
        for name,source,libs,method,count in [('x11','drawing-x11-delivery-test.c',['-lX11','-lXext'],None,9),('glx','drawing-glx-delivery-test.c',['-lGL','-lX11','-ldl'],'glXSwapBuffers',5),('egl','drawing-egl-delivery-test.c',['-lEGL','-lGLESv2','-lX11','-ldl'],'eglSwapBuffers',4)]:
            binary=out/(name+'-fixture')
            run(name+'-fixture-build',['gcc','-std=c11','-O2','-Wall','-Wextra','-Werror','-MD','-MF',out/(name+'-fixture.d'),ROOT/'tests'/source,*libs,'-o',binary])
            control=run(name+'-control',['xvfb-run','-a',binary],extra=display_env)
            target=out/(name+'-observed');target.mkdir()
            observed=run(name+'-observed',['xvfb-run','-a','env','LD_PRELOAD='+str(libraries[name]),'MNM_DRAW_X11_DIRECTORY='+str(target),binary],extra=display_env)
            if control!=observed:raise ValueError(name+' control output/results/errors changed')
            decoded=decode_delivery((target/'x11-delivery.bin').read_bytes())
            if method:
                if decoded['methods'].get(method)!=count:raise ValueError(name+' missing dispatch routes')
            elif len(decoded['methods'])!=count:raise ValueError('Missing X11 methods')
            report['display'][name]=dict(observation=decoded,control_sha256=hashlib.sha256(control.encode()).hexdigest(),identical_control_output=True,library_sha256=sha(libraries[name]))
        # Independent provider identity and exhaustion, not a graphics driver claim.
        provider=out/'provider.so'
        run('alias-provider',['gcc','-O2','-fPIC','-shared','-Wl,-Bsymbolic-functions','-Wall','-Wextra','-Werror',ROOT/'tests/drawing-glx-alias-provider.c','-o',provider])
        import shutil
        for i in range(17):shutil.copyfile(provider,out/f'provider-{i:02d}.so')
        binary=out/'alias-fixture'
        run('alias-fixture-build',['gcc','-std=c11','-O2','-Wall','-Wextra','-Werror',ROOT/'tests/drawing-glx-alias-test.c','-ldl','-o',binary])
        report['aliases']=[]
        for mode,expected in [('swap',0),('proc',0),('exhaust',97),('proc-exhaust',97)]:
            target=out/('alias-'+mode);target.mkdir()
            log=run('alias-'+mode,[binary,out,mode],expected=expected,extra=dict(LD_PRELOAD=str(libraries['glx']),MNM_DRAW_X11_DIRECTORY=str(target)))
            if expected and 'aliases exhausted' not in log:raise ValueError('Wrong provider exhaustion refusal')
            decoded=decode_delivery((target/'x11-delivery.bin').read_bytes())
            if decoded['methods']!={'glXSwapBuffers':32} or len({c['arguments'][1] for c in decoded['calls']})!=16:raise ValueError('Exact provider aliases differ')
            report['aliases'].append(dict(mode=mode,swaps=32,aliases=16,exit=expected))
        # Exact and malformed peer ACKs after real EGL calls.
        from drawing_egl_physical import decode_request
        report['fences']=[]
        control=(out/'egl-control.log').read_text()
        for mode in ('exact','wrong-id','oversize','missing','bad-mode'):
            target=out/('fence-'+mode);target.mkdir()
            extra=dict(LD_PRELOAD=str(libraries['egl']),MNM_DRAW_X11_DIRECTORY=str(target),MNM_DRAW_EGL_PHYSICAL='0' if mode=='bad-mode' else '1')
            command=['xvfb-run','-d','-e',str(target/'server.log'),'env',*[k+'='+v for k,v in extra.items()],str(out/'egl-fixture')]
            with (target/'stderr.log').open('x') as log:
                process=subprocess.Popen(private_display(command),cwd=target,env={**env,**display_env},stdout=subprocess.PIPE,stderr=log)
                ordinal=1;deadline=time.monotonic()+30;requests=[]
                try:
                    while process.poll() is None:
                        if time.monotonic()>deadline:raise ValueError('EGL ACK fixture timed out')
                        path=target/f'egl-physical-request-{ordinal:06d}.bin'
                        if path.exists() and path.stat().st_size==128:
                            r=decode_request(path.read_bytes());requests.append(r[7]);ack=b'MNMEACK1'+struct.pack('<2I',r[4],r[7]+(mode=='wrong-id'))
                            if mode=='oversize':ack+=b'0'
                            if mode!='missing':
                                tmp=target/f'ack-{ordinal}.tmp';tmp.write_bytes(ack);tmp.rename(target/f'egl-physical-ack-{ordinal:06d}.bin')
                            ordinal+=1
                        time.sleep(.001)
                    code=process.wait();output=process.stdout.read().decode();(target/'stdout.log').write_text(output)
                finally:
                    if process.poll() is None:process.kill();process.wait()
            if code!=(0 if mode=='exact' else 97) or mode=='exact' and (output!=control or len(requests)!=4):raise ValueError('EGL fence forwarding/refusal changed: '+mode)
            report['fences'].append(dict(mode=mode,exit=code,requests=len(requests)))
        # Actual private 800x600 root capture and complete request/ACK/oracle join.
        binary=out/'egl-viewport-fixture';peer=out/'egl-root-peer'
        run('egl-viewport-build',['gcc','-std=c11','-O2','-Wall','-Wextra','-Werror','-DMNM_EGL_FIXTURE_WIDTH=800','-DMNM_EGL_FIXTURE_HEIGHT=600','-DMNM_EGL_FIXTURE_CLOSE_SWAP=1',ROOT/'tests/drawing-egl-delivery-test.c','-lEGL','-lGLESv2','-lX11','-ldl','-o',binary])
        run('egl-root-peer-build',['gcc','-std=c11','-O2','-Wall','-Wextra','-Werror',ROOT/'tests/drawing-egl-root-peer.c','-lX11','-o',peer])
        peer_report=report_path.with_name(report_path.stem+'-egl-peer.json')
        log=run('egl-viewport',['xvfb-run','-a','-s','-screen 0 800x600x24',sys.executable,'-B',ROOT/'tools/rendering_egl_fixture_peer.py',ROOT,libraries['egl'],binary,peer,out/'viewport',peer_report],extra=display_env)
        report['viewport']=json.loads(log)
        report['viewport']['record']=peer_report.relative_to(ROOT).as_posix();report['viewport']['sha256']=sha(peer_report)
        imports=dict(module('tenth_menu','tools/build-menu-observer.py').IMPORTS)
        imports.update(module('tenth_shadow','tools/build-shadow-bridge.py').IMPORTS)
        imports.update(dict(CreateThread=24,WaitForSingleObject=8,GetCurrentProcessId=0,InitializeCriticalSection=4,EnterCriticalSection=4,LeaveCriticalSection=4,GetModuleHandleExA=12,FreeLibrary=4,AddVectoredExceptionHandler=8,MoveFileExA=12,VirtualFree=12,Sleep=4,ReadFile=20,GetFileSize=8,LoadLibraryA=4,CreateFileMappingA=24,MapViewOfFile=20,QueryPerformanceCounter=4,QueryPerformanceFrequency=4))
        definition=out/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in imports.items()))
        run('kernel-imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
        flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-ffunction-sections','-Wall','-Wextra','-Werror','-Wno-unused-function']
        for name in NATIVE:
            obj=out/(name+'.obj');binary=out/(name+'.exe')
            run(name+'-compile',[*flags,'-MD','-MF',out/(name+'.d'),'-c',ROOT/'tests'/(name+'-test.c'),'-o',obj])
            packet=[ROOT/'working/build/drawing-session/packet.obj'] if name in ('drawing-source-byte-recorder','drawing-source-lease-recorder','drawing-source-held-snapshot') else []
            run(name+'-link',['lld-link','/machine:x86','/base:0x18000000','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),obj,*packet,out/'kernel32.lib'])
        # Verify compiler source closure, preserving command-generated .d receipts.
        for dep in out.glob('*.d'):
            for value in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
                path=Path(value).resolve()
                if path.is_file() and path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):
                    key=path.relative_to(ROOT).as_posix()
                    if report['sources'].get(key)!=sha(path):raise ValueError('Undeclared compiler input: '+key)
        def win(path):return 'Z:'+str(path).replace('/','\\')
        def fixture(label,name,extra,expected=0):
            target=out/label;target.mkdir()
            run(label,['wine',out/(name+'.exe')],expected=expected,extra={**extra,'MNM_DRAW_SESSION_DIRECTORY':win(target)},cwd=target,timeout=60)
            return target
        common=dict(MNM_DRAW_SESSION_MODE='physical',MNM_DRAW02_PRODUCER_BIND='1')
        from drawing_wine_submission import decode as decode_submissions
        target=fixture('native-submission','drawing-wine-submission',{**common,'MNM_DRAW_WINE_SUBMISSION':'1'})
        report['submissions']=decode_submissions((target/'wine-submission.bin').read_bytes(),lifetime=(target/'wine-lifetime.bin').read_bytes())
        if len(report['submissions']['calls'])!=9 or report['submissions']['calls'][0]['first_generation']!=4:raise ValueError('Native cdecl/reuse-generation matrix changed')
        fixture('native-byte-export','drawing-source-byte-export',common)
        from drawing_source_byte_witness import analyze as byte_analysis
        target=fixture('native-byte-recorder','drawing-source-byte-recorder',{**common,'MNM_DRAW_SOURCE_BYTES':'1'})
        report['source_bytes']=byte_analysis(target)
        if report['source_bytes']['admitted_planes']!=2 or report['source_bytes']['refused_points']!=2:raise ValueError('Source-byte recorder counts changed')
        expected=b''.join(struct.pack('<H',i&65535) for i in range(480000))
        if any((target/f['path']).read_bytes()!=expected for f in report['source_bytes']['frames'] if f['oracle']):raise ValueError('Source byte pixels changed')
        lease_env={**common,'MNM_DRAW_SOURCE_LEASE':'1'}
        for label,extra,code in [('restore',{},0),('write-fault',{'MNM_LEASE_TEST':'fault'},99)]:
            target=fixture('native-lease-'+label,'drawing-source-lease',{**lease_env,**extra},code)
            if code:
                fault=(target/'source-lease-fault-.bin').read_bytes()
                if fault[:8]!=b'MNMSLFT1' or len(fault)!=48:raise ValueError('Missing source protection write fault')
        from drawing_source_lease import decode as decode_lease
        target=fixture('native-lease-recorder','drawing-source-lease-recorder',{**lease_env,'MNM_DRAW_SOURCE_BYTES':'1'})
        report['leases']=decode_lease((target/'api-source-lease.bin').read_bytes())
        if report['leases']['accepted_leases']!=1:raise ValueError('Lease recorder count changed')
        from drawing_source_held_snapshot import analyze as held_analysis
        from drawing_source_held_refusal import decode as held_refusal
        held_env={**lease_env,'MNM_DRAW_SOURCE_BYTES':'1','MNM_DRAW_WINE_SUBMISSION':'1','MNM_DRAW_WINE_TEXTURE_ENTRY':'1','MNM_DRAW_WINE_BLIT_WORKER':'1','MNM_DRAW_WINE_BLIT_PACKET':'1','MNM_DRAW_SOURCE_WORKER_LEASE':'1','MNM_DRAW_SOURCE_HELD_SNAPSHOT':'1'}
        report['held']=[]
        for mode,code in [('g',0),('c',99),('m',99),('r',99),('b',99),('s',99),('p',0),('e',99)]:
            extra={**held_env,'MNM_FIXTURE_MODE':mode}
            if mode in ('p','e'):extra['MNM_DRAW_SOURCE_PENDING_COPY']='1'
            target=fixture('native-held-'+mode,'drawing-source-held-snapshot',extra,code)
            if not code:
                observation=held_analysis(target)
                if observation['stable_snapshot_pairs']!=1 or observation['admitted_planes']!=2 or any((target/f['path']).read_bytes()!=expected for f in observation['frames']):raise ValueError('Held source closure/pixels changed')
            else:
                observation=held_refusal((target/'source-held-refusal.bin').read_bytes())
                if observation['reason_code']!={'c':5,'m':4,'r':1,'b':3,'s':2,'e':11}[mode]:raise ValueError('Held source wrong refusal')
            report['held'].append(dict(mode=mode,exit=code,observation=observation))
        from drawing_aux_save_readback import analyze as auxiliary
        for mode in ('complete','forced'):
            target=out/('native-aux-'+mode);target.mkdir();extra=dict(MNM_DRAW_AUX_SAVE_READBACK='1',MNM_AUX_TEST_DIR=win(target))
            if mode=='forced':extra['MNM_AUX_TEST_FORCED']='1'
            run('native-aux-'+mode,['wine',out/'drawing-aux-save-readback.exe'],extra=extra,cwd=target)
            if mode=='forced':
                try:auxiliary(target)
                except ValueError:report['forced_aux_refused']=True
                else:raise ValueError('Forced-close auxiliary admitted')
            else:
                report['auxiliary']=auxiliary(target)
                if len(report['auxiliary']['samples'])!=256 or len(report['auxiliary']['images'])!=247 or report['auxiliary']['refused']!=9 or report['auxiliary']['budget_skips']!=1 or report['auxiliary']['original_driver_state_restored']:raise ValueError('Auxiliary fault/budget counts changed')
                words=b''.join(struct.pack('<H',(17+i*37)&65535) for i in range(1600))
                if any((target/i['path']).read_bytes()!=words for i in report['auxiliary']['images']):raise ValueError('Auxiliary mapped row/padding bytes changed')
        for number,test in enumerate(TESTS,1):
            log=run('python-'+str(number),[sys.executable,'-B',ROOT/test],extra={'PYTHONPATH':str(ROOT/'tools')})
            match=re.search(r'Ran (\d+) tests?',log)
            if not match:raise ValueError('No test count: '+test)
            report['tests_run']+=int(match[1])
        report['sources_stable']=all(sha(ROOT/name)==digest for name,digest in report['sources'].items())
        if not report['sources_stable']:raise ValueError('Source drift during execution')
        current=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        if module('tenth_final_claims','tools/draft-coverage-claims.py').declaration(ROOT,current,BEHAVIORS,[SCENARIO])['claims']!=declaration['claims']:raise ValueError('Contract drift')
        report['success']=True
    except Exception as error:
        import traceback
        report['error']=repr(error);report['traceback']=traceback.format_exc()
    finally:
        if lock is not None:
            try:
                run('owned-prefix-stop',['wineserver','-k'],expected=(0,1));run('owned-prefix-retired',['wineserver','-w'],timeout=20)
            except Exception as error:report.update(success=False,retirement_error=repr(error))
            finally:lock.close()
        report['source_inputs']=len(report['sources'])
        digest=write_report(ROOT,report_path,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],report=str(report_path),sha256=digest,tests_run=report['tests_run'],source_inputs=report['source_inputs'],error=report.get('error'))),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
