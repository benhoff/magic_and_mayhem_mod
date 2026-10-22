#!/usr/bin/env python3
"""Execute ten bounded offline rendering contracts with immutable SQLite evidence."""
import argparse
import ast
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import shlex
import struct
from drawing_fixture_prefix import prepare
from coverage_evidence import write_report, evidence_digest

ROOT=Path(__file__).resolve().parents[1]
BEHAVIORS=['TOOL.native-campaign-artifact-check']+['NR.drawing-'+n for n in ['physical-sample-timing','quit-dispatch-observation','whole120-acquisition','copy-order-observation','copy-request-trace','worker-producer-prefix','region-mouse-observation','cursor-mutex-observation','source-pending-copy-policy']]
SCENARIO='NR.ninth-ten.observer-completion-20261022'
TESTS=['tests/test-native-campaign-artifacts.py']+['tests/test-drawing-'+n+'.py' for n in ['delivery-timing','quit-completion','process-exit','source-completion','copy-order','worker-producer-points','cursor-mutex','region-input','source-pending-admission','source-pending-refusal','source-held-pending','source-held-refusal','source-cursor-gate']]+['tests/test-canvas-producers-v4.py','tests/test-rendering-ninth-ten.py']
NATIVE=['drawing-region-input','drawing-cursor-mutex','drawing-worker-producer-points','drawing-source-pending-policy','drawing-copy-order','drawing-sample-admission','drawing-process-exit-child']



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
    parent=ROOT/'working/tests/rendering-ninth-ten';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    (out/'.retain').write_text('Ninth rendering batch raw controls and failed attempts. Generated evidence is in SQLite.\n')
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    declaration=module('ninth_claims','tools/draft-coverage-claims.py').declaration(ROOT,register,BEHAVIORS,[SCENARIO])
    todo=set(declaration['sources'])|set(TESTS)|{'tools/test-rendering-ninth-ten.py','tools/coverage_evidence.py','tools/coverage_claims.py','tools/draft-coverage-claims.py','tools/drawing_observed_inputs.py'}
    todo.update({'tools/build-menu-observer.py','tools/build-drawing-session.py','tools/build-drawing-process-exit.py','tools/build-shadow-bridge.py','tools/drawing_fixture_prefix.py','tools/drawing_process_exit.py','tests/canvas-producers-v4-test.cpp','runtime/drawing/process_exit_observer.c'})
    todo.update('tests/'+name+'-test.c' for name in NATIVE if not name.endswith('-child'))
    todo.add('tests/drawing-process-exit-child.c')
    for directory in ('runtime/menu','runtime/drawing','assets','renderer','compat/legacy'):
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
    def run(label,command,expected=0,extra=None,cwd=ROOT,timeout=120):
        command=[str(x) for x in command];print(label,flush=True)
        process=subprocess.run(command,cwd=cwd,env={**env,**(extra or {})},capture_output=True,text=True,timeout=timeout)
        log=process.stdout+process.stderr;(out/(label+'.log')).write_text(log)
        report['commands'].append(dict(label=label,argv=command,expected_exit=expected,actual_exit=process.returncode,log=log,log_sha256=hashlib.sha256(log.encode()).hexdigest()))
        if process.returncode not in (expected if isinstance(expected,tuple) else (expected,)):raise ValueError(label+': '+log)
        return log
    try:
        owner=args.prefix_owner_root.resolve();prefix=args.wine_prefix.resolve()
        if owner!=ROOT and (owner.parent!=ROOT.parent or not (owner/'.git').is_file()):raise ValueError('Prefix owner must be this worktree or a registered sibling')
        prefix,lock=prepare(owner,out,prefix,prefix)
        report['fixture_prefix']=dict(owner=str(owner),path=str(prefix),exclusive_lock=True,idle_before_use=True)
        report['wine_version']=run('wine-version',['wine','--version']).strip()
        report['clang_version']=run('clang-version',['clang','--version']).strip()
        report['qt_version']=run('qt-version',['pkg-config','--modversion','Qt6Core']).strip()
        run('wineboot',['wineboot','-u'])
        run('menu-production',[sys.executable,'-B',ROOT/'tools/build-menu-observer.py'])
        manifest=json.loads((ROOT/'working/build/menu-observer/manifest.json').read_text())
        for name,digest in manifest['compiled_sources'].items():
            if report['sources'].get(name)!=digest:raise ValueError('Undeclared production compiler input: '+name)
        run('drawing-production',[sys.executable,'-B',ROOT/'tools/build-drawing-session.py'])
        manifest=json.loads((ROOT/'working/build/drawing-session/manifest.json').read_text())
        for name,digest in manifest['sources'].items():
            if report['sources'].get(name)!=digest:raise ValueError('Undeclared drawing production source: '+name)
        cmake=out/'native-build'
        run('native-v4-configure',['cmake','-S',ROOT/'compat/legacy/canvas-producers','-B',cmake,'-DCMAKE_BUILD_TYPE=Release','-DBUILD_TESTING=ON'])
        run('native-v4-build',['cmake','--build',cmake,'--target','canvas-producers-v4-test','-j2'],timeout=300)
        for dep in cmake.rglob('*.o.d'):
            for value in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
                path=Path(value).resolve()
                if path.is_file() and path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):
                    key=path.relative_to(ROOT).as_posix()
                    if report['sources'].get(key)!=sha(path):raise ValueError('Undeclared C++ compiler input: '+key)
        run('native-v4-replay',[cmake/'canvas-producers-v4-test'])
        report['native_v4']=dict(queues=[1,16,120],binary_sha256=sha(cmake/'canvas-producers-v4-test'),owned_inputs_only=True)
        imports=dict(module('ninth_menu','tools/build-menu-observer.py').IMPORTS)
        imports.update(module('ninth_shadow','tools/build-shadow-bridge.py').IMPORTS)
        imports.update(module('ninth_process','tools/build-drawing-process-exit.py').IMPORTS)
        imports.update(dict(CreateThread=24,WaitForSingleObject=8,GetCurrentProcessId=0,InitializeCriticalSection=4,EnterCriticalSection=4,LeaveCriticalSection=4,GetModuleHandleExA=12,FreeLibrary=4,AddVectoredExceptionHandler=8,MoveFileExA=12,VirtualFree=12,Sleep=4,ReadFile=20,GetFileSize=8,LoadLibraryA=4,CreateFileMappingA=24,MapViewOfFile=20,QueryPerformanceCounter=4,QueryPerformanceFrequency=4))
        definition=out/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{name}@{size}\n' for name,size in imports.items()))
        run('kernel-imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
        flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-ffunction-sections','-Wall','-Wextra','-Werror','-Wno-unused-function']
        sources={name:'tests/'+name+('-test.c' if not name.endswith('-child') else '.c') for name in NATIVE}
        sources['process-exit']='runtime/drawing/process_exit_observer.c'
        for name,source in sources.items():
            obj=out/(name+'.obj');dep=out/(name+'.d');binary=out/(name+'.exe')
            run(name+'-compile',[*flags,'-MD','-MF',dep,'-c',ROOT/source,'-o',obj])
            for value in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
                path=Path(value).resolve()
                if path.is_file() and path.is_relative_to(ROOT):
                    key=path.relative_to(ROOT).as_posix()
                    if report['sources'].get(key)!=sha(path):raise ValueError('Undeclared PE32 compiler input: '+key)
            run(name+'-link',['lld-link','/machine:x86','/base:0x18000000','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),obj,out/'kernel32.lib'])
        def win(path):return 'Z:'+str(path).replace('/','\\')
        def fixture(label,binary,extra,expected=0):
            target=out/label;target.mkdir()
            run(label,['wine',out/(binary+'.exe')],expected=expected,extra={**extra,'MNM_FIXTURE_DIRECTORY':win(target)},cwd=target,timeout=60)
            return target
        from drawing_region_input import decode as region_decode
        report['region']=[]
        for version in (2,3):
            target=out/('region-v'+str(version));target.mkdir();extra={'MNM_DRAW_REGION_INPUT_FILE':win(target/'region.bin')}
            if version==3:extra['MNM_DRAW_REGION_TRACK_SLOT']='0'
            run('native-region-v'+str(version),['wine',out/'drawing-region-input.exe'],extra=extra,cwd=target)
            observed=region_decode((target/'region.bin').read_bytes())
            if observed['wire_version']!=version or len(observed['calls'])!=32:raise ValueError('Region native matrix changed')
            report['region'].append(observed)
        target=out/'native-mutex';target.mkdir()
        run('native-mutex',['wine',out/'drawing-cursor-mutex.exe'],extra={'MNM_DRAW_CURSOR_MUTEX_FILE':win(target/'mutex.bin')},cwd=target)
        from drawing_cursor_mutex import decode as mutex_decode
        report['mutex']=mutex_decode((target/'mutex.bin').read_bytes())
        if len(report['mutex']['calls'])!=20 or report['mutex']['created_identities']!=2 or report['mutex']['remaining_live_ids']:raise ValueError('Mutex native matrix changed')
        from drawing_worker_producer_points import decode as worker_decode
        report['workers']=[]
        common=dict(MNM_DRAW_SESSION_MODE='physical',MNM_DRAW02_PRODUCER_BIND='1',MNM_DRAW_WINE_SUBMISSION='1',MNM_DRAW_WINE_BLIT_WORKER='1')
        for label,flag,expected in [('enabled','1',0),('disabled',None,0),('unavailable','1',0),('refused','1',0),('mid-close','1',0),('budget','1',99),('invalid','x',0)]:
            name='worker-'+label;target=out/name;target.mkdir()
            extra=dict(common,MNM_WORKER_POINT_CASE=label,MNM_DRAW_SESSION_DIRECTORY=win(target))
            if flag is not None:extra['MNM_DRAW_WINE_WORKER_PRODUCER']=flag
            run(name,['wine',out/'drawing-worker-producer-points.exe'],expected=expected,extra=extra,cwd=target,timeout=60)
            path=target/'wine-worker-producer-points.bin'
            if label in ('disabled','invalid'):
                if path.exists():raise ValueError('Disabled worker wrote a sidecar')
                observation=dict(no_journal=True)
            elif label in ('mid-close','budget'):
                try:worker_decode(path.read_bytes(),(target/'wine-blit-worker.bin').read_bytes())
                except ValueError:observation=dict(incomplete_refused=True)
                else:raise ValueError('Incomplete worker journal admitted')
            else:
                observation=worker_decode(path.read_bytes(),(target/'wine-blit-worker.bin').read_bytes())
                if len(observation['calls'])!=3 or observation['unavailable_pairs']!=(3 if label in ('unavailable','refused') else 0):raise ValueError('Worker source prefix matrix differs')
            report['workers'].append(dict(case=label,observation=observation))
        from drawing_copy_order import decode as copy_decode
        report['copies']=[]
        for label,flag,expected in [('reverse','1',0),('disabled',None,0),('invalid','x',0),('partial','1',0),('budget','1',99)]:
            extra=dict(MNM_DRAW02_PRODUCER_BIND='1',MNM_COPY_ORDER_CASE=label)
            if flag is not None:extra['MNM_DRAW_COPY_ORDER']=flag
            target=fixture('copy-'+label,'drawing-copy-order',extra,expected);path=target/'copy-order-0000.bin'
            if label=='reverse':
                observation=copy_decode(path.read_bytes())
                if [r['request'] for r in observation['returns']]!=[2,1] or len({r['owner'] for r in observation['entries']})!=2:raise ValueError('COPY native two-thread reversal missing')
            elif label in ('partial','budget'):
                try:copy_decode(path.read_bytes())
                except ValueError:observation=dict(incomplete_refused=True)
                else:raise ValueError('Incomplete COPY admitted')
            else:
                if path.exists():raise ValueError('Disabled COPY wrote journal')
                observation=dict(no_journal=True)
            report['copies'].append(dict(case=label,observation=observation))
        from drawing_source_pending_admission import decode as pending_decode
        report['pending']=[]
        for label,extra,expected in [('disabled',{},0),('enabled',dict(MNM_DRAW_SOURCE_PENDING_COPY='1'),0),('alias-write',dict(MNM_DRAW_SOURCE_PENDING_COPY='1',MNM_PENDING_POLICY_TEST='fault'),99)]:
            target=fixture('pending-'+label,'drawing-source-pending-policy',dict(MNM_DRAW02_PRODUCER_BIND='1',MNM_DRAW_SOURCE_LEASE='1',**extra),expected)
            admissions=[pending_decode(p.read_bytes()) for p in sorted(target.glob('source-pending-admission-*.bin'))]
            if len(admissions)!={'disabled':0,'enabled':2,'alias-write':1}[label]:raise ValueError('Pending native admission matrix changed')
            if label=='alias-write':
                words=struct.unpack('<12I',next(target.glob('source-lease-fault-*.bin')).read_bytes())
                if words[3]!=13 or not words[6] or words[6]==words[7]:raise ValueError('Actual foreign protected alias fault missing')
            report['pending'].append(dict(case=label,admissions=admissions))
        fixture('sample-admission','drawing-sample-admission',dict(MNM_DRAW_UPDATE_BINDING='1'))
        from drawing_process_exit import Monitor
        import time
        report['process_exits']=[]
        for code in (0,37,259):
            target=out/('process-'+str(code));target.mkdir();ready=target/'ready.bin';release=target/'release.flag'
            extra=dict(MNM_CHILD_READY_FILE=win(ready),MNM_CHILD_RELEASE_FILE=win(release),MNM_CHILD_EXIT_CODE=str(code))
            monitor=None
            with (target/'child.log').open('x') as log:
                child=subprocess.Popen(['wine',str(out/'drawing-process-exit-child.exe')],cwd=target,env={**env,**extra},stdout=log,stderr=subprocess.STDOUT)
                try:
                    deadline=time.monotonic()+15
                    while not ready.exists() or ready.stat().st_size!=8:
                        if child.poll() is not None or time.monotonic()>deadline:raise ValueError('Owned child did not publish live identity')
                        time.sleep(.02)
                    thread,pid=struct.unpack('<2I',ready.read_bytes());monitor=Monitor(out/'process-exit.exe',env,thread,target,lambda:None)
                    if monitor.attached['process']!=pid:raise ValueError('Observer attached to wrong owned process')
                    release.write_bytes(b'exit\n');observed=monitor.finish();launcher=child.wait(timeout=10)
                    if observed['exit_code']!=code:raise ValueError('Actual observed target exit differs')
                    report['process_exits'].append(dict(expected=code,observation=observed,launcher_exit=launcher))
                finally:
                    if monitor:monitor.close()
                    if child.poll() is None:child.terminate();child.wait(timeout=5)
        report['raw_journals']={p.relative_to(out).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in out.rglob('*.bin')}
        for path in TESTS:
            print(path,flush=True)
            command=[sys.executable,'-B',str(ROOT/path)]
            execution=subprocess.run(command,cwd=ROOT,env=env,capture_output=True,text=True,timeout=120)
            log=execution.stdout+execution.stderr
            (out/(Path(path).stem+'.log')).write_text(log)
            report['commands'].append(dict(argv=command,actual_exit=execution.returncode,expected_exit=0,log=log,log_sha256=hashlib.sha256(log.encode()).hexdigest()))
            if execution.returncode:raise ValueError(path+': '+log)
            match=re.search(r'Ran (\d+) tests?',log)
            if not match:raise ValueError('Missing executed-test count: '+path)
            report['tests_run']+=int(match[1])
        report['sources_stable']=all(sha(ROOT/name)==digest for name,digest in report['sources'].items())
        if not report['sources_stable']:raise ValueError('Source drift during execution')
        current=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        fresh=module('ninth_final_claims','tools/draft-coverage-claims.py').declaration(ROOT,current,BEHAVIORS,[SCENARIO])
        if fresh['claims']!=declaration['claims']:raise ValueError('Contract drift during execution')
        report['success']=True
    except Exception as error:report['error']=repr(error)
    finally:
        if lock is not None:
            try:
                run('owned-prefix-stop',['wineserver','-k'],expected=(0,1))
                run('owned-prefix-retired',['wineserver','-w'],timeout=20)
            except Exception as error:report.update(success=False,retirement_error=repr(error))
            finally:lock.close()
        report['source_inputs']=len(report['sources'])
        digest=write_report(ROOT,report_path,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],report=str(report_path),sha256=digest,tests_run=report['tests_run'],source_inputs=report['source_inputs'],error=report.get('error'))),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
