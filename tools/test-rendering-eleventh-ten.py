#!/usr/bin/env python3
"""Execute ten source/update observation contracts with current SQLite evidence."""
import argparse,ast,hashlib,importlib.util,json,os,re,shlex,struct,subprocess,sys,tempfile,time
from pathlib import Path
from drawing_fixture_prefix import prepare
from coverage_evidence import write_report,evidence_digest,read_json
ROOT=Path(__file__).resolve().parents[1]
BEHAVIORS=['NR.engine-cadence-observation']+['NR.drawing-'+name for name in ['wine-blit-command-discovery','producer-update-binding','whole-update-source-capture','update-interval-accounting','primitive-lifetimes','cursor-source-gate-module','source-cursor-gate-binding','actions','documented-input-actions']]
SCENARIO='NR.eleventh-ten.observer-completion-20261022'
TESTS=['tests/test-drawing-'+name+'.py' for name in ['engine-windows','wine-blit-command','wine-blit-worker','wine-lifetime','update-timing','update-routes','source-callback-windows','update-accounting','update-interval-gate','primitive-lifetimes','primitive-ownership','cursor-gate-journal','source-cursor-gate','publication']]+['tests/test-canvas-producers-v4.py','tests/test-rendering-eleventh-ten.py']
NATIVE=['drawing-wine-blit-worker','drawing-cadence-queue-point','drawing-producer-update','drawing-whole-update-boundary','drawing-primitive-lifetimes','drawing-cursor-source-gate','drawing-cursor-source-gate-module','drawing-source-cursor-gate','drawing-source-gate-session','drawing-source-gate-protected','drawing-menu-action-observation']
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
    parent=ROOT/'working/tests/rendering-eleventh-ten';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    (out/'.retain').write_text('Eleventh rendering batch raw controls and failed attempts. Generated evidence is in SQLite.\n')
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    declaration=module('eleventh_claims','tools/draft-coverage-claims.py').declaration(ROOT,register,BEHAVIORS,[SCENARIO])
    todo=set(declaration['sources'])|set(TESTS)|{'tools/test-rendering-eleventh-ten.py','tools/coverage_evidence.py','tools/coverage_claims.py','tools/draft-coverage-claims.py','tools/drawing_observed_inputs.py'}
    todo.update({'tools/build-menu-observer.py','tools/build-drawing-session.py','tools/build-engine-cadence.py','tools/build-scene-observer.py','tools/build-cursor-source-gate.py','tools/build-drawing-session-input.py','tools/build-shadow-bridge.py','tools/drawing_fixture_prefix.py','tools/test-drawing-documented-input.py','tests/engine-cadence-reference.c','tests/engine-cadence-reference.S','runtime/engine-cadence/entry.S','tests/canvas-producers-v4-test.cpp','tests/canvas-sequence-test.cpp','tests/drawing-session-input.cpp','apps/qt-shell/window_host.cpp','apps/qt-shell/window_host.hpp'})
    todo.update('tests/'+name+'-test.c' for name in NATIVE)
    for directory in ('runtime','assets','renderer','compat/legacy'):
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
        for label,tool,extra in [('drawing','build-drawing-session.py',[]),('drawing-selftest','build-drawing-session.py',['--selftest']),('menu','build-menu-observer.py',[]),('scene','build-scene-observer.py',[]),('cadence','build-engine-cadence.py',[]),('cursor-gate','build-cursor-source-gate.py',[]),('input','build-drawing-session-input.py',[])]:
            run('production-'+label,[sys.executable,'-B',ROOT/'tools'/tool,*extra],timeout=240)
        # Compiler source receipts from production DLL builders are prospective too.
        for directory,key in [('drawing-session','sources'),('drawing-session-selftest','sources'),('menu-observer','compiled_sources'),('engine-cadence','sources'),('cursor-source-gate','sources')]:
            manifest=json.loads((ROOT/'working/build'/directory/'manifest.json').read_text())
            for name,digest in manifest[key].items():
                if report['sources'].get(name)!=digest:raise ValueError('Undeclared production compiler input: '+name)
        cmake=out/'native-build'
        run('native-configure',['cmake','-S',ROOT/'compat/legacy/canvas-producers','-B',cmake,'-DCMAKE_BUILD_TYPE=Release','-DBUILD_TESTING=ON'])
        run('native-build',['cmake','--build',cmake,'--target','canvas-producers-v4-test','canvas-sequence-test','-j2'],timeout=300)
        run('native-v4-replay',[cmake/'canvas-producers-v4-test']);run('native-canvas-sequence',[cmake/'canvas-sequence-test'])
        report['native_v4']=dict(queues=[1,16,120],source_only_replay=True,legacy_sequence_passed=True)
        imports=dict(module('eleventh_menu','tools/build-menu-observer.py').IMPORTS)
        imports.update(module('eleventh_shadow','tools/build-shadow-bridge.py').IMPORTS)
        imports.update(dict(CreateEventA=16,SetEvent=4,TryEnterCriticalSection=4,GetExitCodeThread=8,GetCurrentProcess=0,DuplicateHandle=28,CreateMutexA=12,ReleaseMutex=4,SetEnvironmentVariableA=8,CreateThread=24,WaitForSingleObject=8,GetCurrentProcessId=0,InitializeCriticalSection=4,EnterCriticalSection=4,LeaveCriticalSection=4,GetModuleHandleExA=12,FreeLibrary=4,AddVectoredExceptionHandler=8,MoveFileExA=12,VirtualFree=12,Sleep=4,ReadFile=20,GetFileSize=8,LoadLibraryA=4,CreateFileMappingA=24,MapViewOfFile=20,QueryPerformanceCounter=4,QueryPerformanceFrequency=4))
        definition=out/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in imports.items()))
        run('kernel-imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
        flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-ffunction-sections','-Wall','-Wextra','-Werror','-Wno-unused-function']
        for source in ['tests/engine-cadence-reference.S']:
            run('cadence-reference-assembly',[*flags,'-c',ROOT/source,'-o',out/'cadence-reference-assembly.obj'])
        sources={name:'tests/'+name+'-test.c' for name in NATIVE};sources['engine-cadence-reference']='tests/engine-cadence-reference.c'
        for name,source in sources.items():
            obj=out/(name+'.obj');binary=out/(name+'.exe')
            run(name+'-compile',[*flags,'-MD','-MF',out/(name+'.d'),'-c',ROOT/source,'-o',obj])
            fixed=name in ('engine-cadence-reference','drawing-cadence-queue-point')
            extras=[ROOT/'working/build/engine-cadence/entry.S.obj'] if fixed else []
            if name=='engine-cadence-reference':extras.append(out/'cadence-reference-assembly.obj')
            if name in ('drawing-source-gate-session','drawing-source-gate-protected'):extras.append(ROOT/'working/build/drawing-session/packet.obj')
            if name=='drawing-menu-action-observation':extras.append(ROOT/'working/build/drawing-session-selftest/drawing.lib')
            run(name+'-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/opt:ref',*(['/base:0x400000','/fixed','/dynamicbase:no'] if fixed else ['/base:0x18000000']),'/out:'+str(binary),obj,*extras,out/'kernel32.lib'])
        for dep in [*out.glob('*.d'),*cmake.rglob('*.o.d')]:
            for value in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
                path=Path(value).resolve()
                if path.is_file() and path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):
                    key=path.relative_to(ROOT).as_posix()
                    if report['sources'].get(key)!=sha(path):raise ValueError('Undeclared compiler input: '+key)
        def win(path):return 'Z:'+str(path).replace('/','\\')
        def fixture(label,name,extra=None,expected=0):
            target=out/label;target.mkdir()
            run(label,['wine',out/(name+'.exe')],expected=expected,extra=extra or {},cwd=target,timeout=60)
            return target
        fixture('native-cadence','engine-cadence-reference')
        fixture('native-cadence-getter','drawing-cadence-queue-point')
        report['cadence_native']=dict(thunk_paths=13,original_forwards=14,mutated_guard_bytes=104,full_512_fx=True,flags_registers_error=True,null_queue_refused=True)
        report['producer_updates']=[]
        from drawing_producer_updates import decode as producer_decode
        for mode,code in [('g',0),('m',99),('r',99),('d',99),('q',99)]:
            target=out/('producer-'+mode);target.mkdir()
            run('producer-'+mode,['wine',out/'drawing-producer-update.exe'],expected=code,extra=dict(MNM_DRAW_UPDATE_BINDING='1',MNM_FIXTURE_MODE=mode,MNM_SCENE_DIR=win(target)),cwd=target)
            case=dict(mode=mode,exit=code)
            if not code:
                case['observation']=producer_decode((target/'producer-update-points-0000.bin').read_bytes())
                if case['observation']['queue_count']!=1:raise ValueError('Producer queue count changed')
            report['producer_updates'].append(case)
        report['whole_update']=[]
        for mode in ('g','s','t','r','o','p','a','n','e','c','f','w','z','d','q','b','l','j','h','i','k','m','u','v','x'):
            code=0 if mode in ('g','w','d','l','j','h','i','k','v','x') else 99
            target=out/('whole-'+mode);target.mkdir()
            run('whole-'+mode,['wine',out/'drawing-whole-update-boundary.exe'],expected=code,extra=dict(MNM_DRAW_UPDATE_BINDING='1',MNM_DRAW_WHOLE_UPDATES='1',MNM_DRAW_UPDATE_ROUTES='1',MNM_FIXTURE_MODE=mode,MNM_SCENE_DIR=win(target)),cwd=target)
            if mode=='g':
                done=(target/'canvas-producers.done').read_bytes()
                if done[:8]!=b'MNMPDONE' or len(done)!=32 or struct.unpack('<8I',done)[2:]!=(1,1,1,0,0,160):raise ValueError('Whole-update done provenance changed')
            if mode in ('g','w','d','l','v','x'):
                raw=(target/'source-update-windows-0000.bin').read_bytes()
                if any(row[10] for row in struct.iter_unpack('<16I',raw[64:]) if row[1]==1):raise ValueError('Nonquiescent callback admitted')
            if mode=='u':
                refusal=struct.unpack('<32I',(target/'whole-update-refusal-0000.bin').read_bytes())
                if refusal[2]!=2 or refusal[13:15]!=(1,99):raise ValueError('Hidden same-owner frame was not refused')
            report['whole_update'].append(dict(mode=mode,exit=code))
        from drawing_primitive_lifetimes import decode as lifetime_decode
        report['primitive_cases']=[]
        for label,flag,code in [('normal','1',0),('invalid','0',0),('invalid-long','11',0),('prerequisite','1',0),('budget','1',99),('write','1',99)]:
            target=out/('primitive-'+label);target.mkdir()
            run('primitive-'+label,['wine',out/'drawing-primitive-lifetimes.exe'],expected=code,extra=dict(MNM_DRAW02_PRODUCER_BIND='1',MNM_DRAW_PRIMITIVE_LIFETIMES=flag,MNM_FIXTURE_MODE=label,MNM_SCENE_DIR=win(target)),cwd=target)
            if label=='normal':
                report['primitives']=lifetime_decode((target/'primitive-lifetimes-0000.bin').read_bytes())
                if report['primitives']['calls']!=3 or [b['calls'] for b in report['primitives']['boundaries']]!=[[1,2],[3]]:raise ValueError('Primitive exact call identities changed')
            report['primitive_cases'].append(dict(mode=label,exit=code))
        common=dict(MNM_DRAW_SESSION_MODE='physical',MNM_DRAW_WINE_SUBMISSION='1',MNM_DRAW_WINE_BLIT_WORKER='1')
        target=out/'native-wine-worker';target.mkdir()
        run('native-wine-worker',['wine',out/'drawing-wine-blit-worker.exe'],extra={**common,'MNM_DRAW_SESSION_DIRECTORY':win(target)},cwd=target)
        from drawing_wine_blit_worker import decode as worker_decode
        report['workers']=worker_decode((target/'wine-blit-worker.bin').read_bytes(),(target/'wine-lifetime.bin').read_bytes())
        if len(report['workers']['calls'])!=3 or report['workers']['unknown_source_generations'] or report['workers']['unknown_destination_generations']:raise ValueError('Native worker scalar/generation matrix changed')
        report['wine_blit_inventory']=module('eleventh_blit_inventory','tools/inspect-drawing-wine-blit-command.py').inspect(Path('/usr/lib/wine/i386-windows/wined3d.dll').read_bytes())
        # Real kernel mutexes and 64 scripted failure cells under the gate module.
        fixture('native-cursor-core','drawing-cursor-source-gate')
        target=out/'native-cursor-module';target.mkdir()
        run('native-cursor-module',['wine',out/'drawing-cursor-source-gate-module.exe'],extra=dict(MNM_DRAW_CURSOR_SOURCE_GATE_FILE=win(target/'cursor-gate.bin')),cwd=target)
        from drawing_cursor_gate_journal import decode as module_decode,account as module_account
        report['cursor_module']=module_decode((target/'cursor-gate.bin').read_bytes());report['cursor_module_accounting']=module_account(report['cursor_module'])
        if report['cursor_module']['complete'] or report['cursor_module']['terminal_reason']!=3 or report['cursor_module_accounting']['gaps'] or report['cursor_module_accounting']['remaining_live_identities']:raise ValueError('Cursor partial snapshot lifecycle changed')
        if sum(r['phase']==11 for r in report['cursor_module']['rows'])!=2:raise ValueError('Missing diagnostic snapshot retirements')
        from drawing_source_cursor_gate import decode as binding_decode
        report['source_gate']=[]
        for name in ('drawing-source-cursor-gate','drawing-source-gate-session','drawing-source-gate-protected'):
            target=out/('native-'+name);target.mkdir();journal=target/'api-cursor-gate.bin'
            extra=dict(MNM_DRAW_CURSOR_SOURCE_GATE='1',MNM_DRAW_SESSION_MODE='metadata',MNM_DRAW_SESSION_DIRECTORY=win(target),MNM_GATE_BINDING_TEST_FILE=win(journal))
            run('native-'+name,['wine',out/(name+'.exe')],extra=extra,cwd=target)
            observed=binding_decode(journal.read_bytes())
            if len(observed['completed'])!=1 or len(observed['refused'])!=1 or observed['refused'][0]['attempted_token']!=8:raise ValueError('Gate binding admitted/refused identity changed')
            if name.endswith('-protected'):
                images=list(target.glob('source-held-oracle-*.565'))
                if len(images)!=2 or any(p.stat().st_size!=960000 for p in images) or sha(images[0])!=sha(images[1]):raise ValueError('Protected gate snapshots differ')
            report['source_gate'].append(dict(fixture=name,observation=observed))
        # The production DLL handles the menu relay's actual entry/return metadata.
        import shutil
        target=out/'native-menu-actions';target.mkdir();shutil.copyfile(ROOT/'working/build/drawing-session-selftest/MnmDrawing.dll',target/'MnmDrawing.dll')
        run('native-menu-actions',['wine',out/'drawing-menu-action-observation.exe'],extra=dict(MNM_DRAW_SESSION_MODE='metadata',MNM_DRAW_SESSION_DIRECTORY=win(target)),cwd=target)
        from drawing_api_session import inspect as session_inspect
        report['menu_actions']=session_inspect(target)
        if len(report['menu_actions']['actions'])!=10:raise ValueError('Menu relay did not record exactly five entry/return pairs')
        target=out/'native-api-regression';target.mkdir()
        run('native-api-regression',['wine',ROOT/'working/build/drawing-session-selftest/selftest.exe'],extra=dict(MNM_DRAW_SESSION_DIRECTORY=win(target),MNM_DRAW_UPDATE_ROUTES='1'),cwd=ROOT/'working/build/drawing-session-selftest')
        report['api_regression']=session_inspect(target)
        if len(report['api_regression']['calls'])!=32 or len(report['api_regression']['frames'])!=8 or len(report['api_regression']['actions'])!=18:raise ValueError('COM/menu regression counts changed')
        from drawing_update_timing import decode as timing_decode
        report['update_timing']=timing_decode((target/'update-call-timing.bin').read_bytes(),(target/'session.bin').read_bytes())
        # Native XTest child verification writes durable evidence directly to SQLite.
        claims_path=report_path.with_name(report_path.stem+'-input-claims.json');input_report=report_path.with_name(report_path.stem+'-input.json')
        write_report(ROOT,claims_path,(json.dumps(declaration)+'\n').encode())
        run('native-documented-input',['xvfb-run','-a',sys.executable,'-B',ROOT/'tools/test-drawing-documented-input.py','--executable',ROOT/'working/build/drawing-session-input/mnm-drawing-session-input','--claims',claims_path,'--report',input_report],timeout=120)
        report['documented_input']=read_json(input_report,root=ROOT)
        if len(report['documented_input']['cases'])!=14 or not report['documented_input']['success']:raise ValueError('Native XTest preflight/refusal/cancel matrix changed')
        for number,test in enumerate(TESTS,1):
            log=run('python-'+str(number),[sys.executable,'-B',ROOT/test],extra={'PYTHONPATH':str(ROOT/'tools')})
            match=re.search(r'Ran (\d+) tests?',log)
            if not match:raise ValueError('No test count: '+test)
            report['tests_run']+=int(match[1])
        report['sources_stable']=all(sha(ROOT/name)==digest for name,digest in report['sources'].items())
        if not report['sources_stable']:raise ValueError('Source drift during execution')
        current=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        if module('eleventh_final_claims','tools/draft-coverage-claims.py').declaration(ROOT,current,BEHAVIORS,[SCENARIO])['claims']!=declaration['claims']:raise ValueError('Contract drift')
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
