#!/usr/bin/env python3
"""Execute ten bounded native rendering ownership and refusal contracts."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import sys
import tempfile
from coverage_evidence import write_report
from drawing_fixture_prefix import prepare

ROOT=Path(__file__).resolve().parents[1]
BEHAVIORS=['NR.drawing-cursor-source-gate','NR.drawing-cursor-mutex-lifetime',
 'NR.drawing-cursor-gate-forwarding','NR.drawing-cursor-gate-leases',
 'NR.drawing-cursor-kernel-lease-state','NR.producer-recorder-thread-policy',
 'NR.drawing-source-held-refusal','NR.drawing-source-pending-frame-refusal',
 'NR.drawing-wine-object-lifetimes','NR.drawing-wine-buffer-origins']
SCENARIO='NR.fourth-ten.ownership-refusals-20261021'


def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path)
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--wine-prefix',type=Path,required=True)
    parser.add_argument('--report',type=Path,required=True)
    args=parser.parse_args()
    report_path=args.report if args.report.is_absolute() else ROOT/args.report
    if report_path.exists():parser.error('Refusing evidence overwrite')
    parent=ROOT/'working/tests/rendering-fourth-ten';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    (out/'.retain').write_text('Fourth rendering batch raw execution, binaries and failed attempts.\n')
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    declaration=module('fourth_claims','tools/draft-coverage-claims.py').declaration(ROOT,register,BEHAVIORS,[SCENARIO])
    # Pin both production compilers' full source closure before execution.
    todo=set(declaration['sources'])|{'tools/test-rendering-fourth-ten.py','tools/coverage_evidence.py',
      'tools/coverage_claims.py','tools/draft-coverage-claims.py','tools/build-drawing-session.py',
      'tools/build-scene-observer.py','tools/build-shadow-bridge.py','tools/drawing_api_session.py',
      'tools/drawing_fixture_prefix.py','tests/test-drawing-source-held-pending.py','tests/test-drawing-source-pending-admission.py','tests/rendering-recorder-state-test.c','tests/test-rendering-fourth-ten.py'}
    for directory in ('runtime/drawing','runtime/scene'):
        todo.update(p.relative_to(ROOT).as_posix() for p in (ROOT/directory).iterdir() if p.suffix in {'.c','.h','.S'})
    todo.update('reconstruction/rendering/'+name for name in ('glyph_backend.c','glyph_backend_state.c','glyph_backend_x87.S'))
    visited=set()
    while todo:
        name=todo.pop()
        if name in visited:continue
        visited.add(name);path=ROOT/name;declaration['sources'][name]=sha(path)
        if path.suffix in {'.c','.cpp','.h','.hpp','.S','.inc'}:
            for include in re.findall(r'^\s*#\s*include\s*"([^"]+)"',path.read_text(),re.M):
                child=(path.parent/include).resolve()
                if child.is_file() and child.is_relative_to(ROOT):todo.add(child.relative_to(ROOT).as_posix())
    report=dict(schema=1,success=False,sources=declaration['sources'],claims=declaration['claims'],
      commands=[],controls={},scope=__doc__,original_game_executed=False,original_artifacts_consumed=False,
      live_capture_integrated=False,original_comparison_claimed=False,live_replacement=False,
      reviewed_head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),artifacts=out.relative_to(ROOT).as_posix())
    (out/'tmp').mkdir()
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
    env['TMPDIR']=str(out/'tmp')
    env.update(WINEPREFIX=str(args.wine_prefix.resolve()),WINEDEBUG='-all',LIBGL_ALWAYS_SOFTWARE='1',
      __GLX_VENDOR_LIBRARY_NAME='mesa',__EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json')
    def run(label,command,expected=0,extra=None,cwd=ROOT,timeout=180):
        command=[str(x) for x in command];print(label,flush=True)
        log=out/(label+'.log')
        with log.open('x') as stream:
            result=subprocess.run(command,cwd=cwd,env={**env,**(extra or {})},stdout=stream,stderr=subprocess.STDOUT,timeout=timeout)
        report['commands'].append(dict(label=label,argv=command,expected_exit=expected,actual_exit=result.returncode,log_sha256=sha(log)))
        if result.returncode not in (expected if isinstance(expected,tuple) else (expected,)):raise ValueError(f'{label}: expected {expected}, got {result.returncode}; {log}')
        return log.read_text()
    def dependencies(path):
        for token in shlex.split(path.read_text().replace('\\\n',' ').split(':',1)[1]):
            p=Path(token).resolve()
            if p.is_file() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                name=p.relative_to(ROOT).as_posix()
                if name not in report['sources'] or sha(p)!=report['sources'][name]:raise ValueError('Undeclared compiler input: '+name)
    lock=None
    try:
        # Initialization is allowed only in an explicitly marked fixture-suite
        # directory. prepare() checks its owner and exclusive lock before Wine.
        prefix=args.wine_prefix.resolve()
        if not prefix.is_relative_to(ROOT/'working/tests/drawing-fixture-prefix-suite') or prefix.name!='wineprefix':raise ValueError('Use an owned fixture-suite prefix')
        prefix.mkdir(exist_ok=True)
        prefix,lock=prepare(ROOT,out,prefix,prefix)
        report['prefix']=dict(path=str(prefix),exclusive_lock=True,idle_before_use=True)
        report['wine_version']=run('wine-version',['wine','--version']).strip()
        run('wineboot',['wineboot','-u'],timeout=120)
        run('drawing-production',[sys.executable,'-B',ROOT/'tools/build-drawing-session.py'])
        run('drawing-selftest-build',[sys.executable,'-B',ROOT/'tools/build-drawing-session.py','--selftest'])
        run('scene-production',[sys.executable,'-B',ROOT/'tools/build-scene-observer.py'])
        for directory in ('drawing-session','drawing-session-selftest','scene-observer'):
            build=ROOT/'working/build'/directory
            data=json.loads((build/'manifest.json').read_text())
            for name,digest in data['sources'].items():
                if report['sources'].get(name)!=digest:raise ValueError('Unpinned production build input: '+name)
        drawing=ROOT/'working/build/drawing-session-selftest'
        definitions=(drawing/'kernel32.def').read_text()+'GetExitCodeThread@8\nCreateMutexA@12\nReleaseMutex@4\nCreateEventA@16\nSetEvent@4\nDuplicateHandle@28\n'
        definition=out/'kernel32.def';definition.write_text(definitions)
        run('kernel-imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
        flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-Wno-unused-function']
        probes=['drawing-cursor-source-gate','drawing-cursor-gate-forward','drawing-cursor-kernel-adapter',
          'drawing-producer-trace','drawing-source-pending-refusal','drawing-source-held-snapshot',
          'drawing-wine-lifetime','drawing-wine-buffer']
        for name in probes:
            obj=out/(name+'.obj');binary=out/(name+'.exe');dep=out/(name+'.d')
            run(name+'-compile',[*flags,'-MD','-MF',dep,'-c',ROOT/('tests/'+name+'-test.c'),'-o',obj])
            dependencies(dep)
            run(name+'-link',['lld-link','/machine:x86','/base:0x18000000','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),obj,*([drawing/'packet.obj'] if name in {'drawing-source-held-snapshot','drawing-wine-lifetime','drawing-wine-buffer'} else []),out/'kernel32.lib'])
            report['controls'][name]=dict(binary_sha256=sha(binary))
        def pe(name,label=None,extra=None,expected=0):
            target=out/(label or name);target.mkdir()
            values=dict(MNM_DRAW02_PRODUCER_BIND='1',MNM_DRAW_WINE_SUBMISSION='1',MNM_DRAW_SESSION_MODE='physical',
              MNM_DRAW_SESSION_DIRECTORY='Z:'+str(target).replace('/','\\'),MNM_FIXTURE_DIRECTORY='Z:'+str(target).replace('/','\\'))
            run(label or name,['wine',out/(name+'.exe')],expected,{**values,**(extra or {})},target,timeout=45)
            return target
        for name in probes[:3]:pe(name)
        trace=[('ownership',{},0),('exhaustion',{'MNM_TRACE_TEST':'exhaustion'},98),('zero-owner',{'MNM_TRACE_TEST':'zero'},98)]
        trace += [('invalid-'+str(i),dict(MNM_TRACE_TEST='invalid',MNM_DRAW02_PRODUCER_BIND=value),0) for i,value in enumerate(('0','2','11','','invalid'))]
        for label,extra,expected in trace:pe('drawing-producer-trace','recorder-'+label,extra,expected)
        pending=pe('drawing-source-pending-refusal',extra={'MNM_DRAW_SOURCE_LEASE':'1'})
        from drawing_source_pending_refusal import decode as pending_decode
        refused=[pending_decode(p.read_bytes()) for p in sorted(pending.glob('source-pending-refusal-*.bin'))]
        if len(refused)!=2 or [r['reason'] for r in refused]!=[1,2] or any(r['source_copy_admitted'] for r in refused):raise ValueError('Pending refusal closure')
        report['controls']['drawing-source-pending-refusal']['cases']=refused
        from drawing_wine_lifetime import decode as lifetime_decode
        target=pe('drawing-wine-lifetime');observed=lifetime_decode((target/'wine-lifetime.bin').read_bytes())
        if observed['observed_generations']!=4 or observed['remaining_observed_objects'] or len(observed['events'])!=14:raise ValueError('Lifetime closure')
        report['controls']['drawing-wine-lifetime']['result']=observed
        from drawing_wine_buffer import decode as buffer_decode
        target=pe('drawing-wine-buffer');observed=buffer_decode((target/'wine-buffer.bin').read_bytes(),(target/'wine-lifetime.bin').read_bytes())
        if len(observed['calls'])!=8 or observed['unknown_texture_returns']!=2:raise ValueError('Buffer origins')
        report['controls']['drawing-wine-buffer']['result']=observed
        from drawing_source_held_refusal import decode as held_decode
        from drawing_source_held_snapshot import analyze
        held=[]
        for mode,reason in [('g',0),('p',0),('r',1),('s',2),('b',3),('m',4),('c',5),('w',6),('o',7),('l',8),('q',8),('a',9),('z',10),('e',11)]:
            extra=dict(MNM_DRAW_SOURCE_BYTES='1',MNM_DRAW_SOURCE_LEASE='1',MNM_DRAW_WINE_TEXTURE_ENTRY='1',
              MNM_DRAW_WINE_BLIT_WORKER='1',MNM_DRAW_WINE_BLIT_PACKET='1',MNM_DRAW_SOURCE_WORKER_LEASE='1',
              MNM_DRAW_SOURCE_HELD_SNAPSHOT='1',MNM_FIXTURE_MODE=mode)
            if mode in ('p','e'):extra['MNM_DRAW_SOURCE_PENDING_COPY']='1'
            target=pe('drawing-source-held-snapshot','held-'+mode,extra,99 if reason else 0)
            if reason:
                value=held_decode((target/'source-held-refusal.bin').read_bytes())
                if value['reason_code']!=reason:raise ValueError('Wrong held refusal: '+mode)
            else:
                value=analyze(target)
                if value['stable_snapshot_pairs']!=1 or value['admitted_planes']!=2:raise ValueError('Held acceptance')
                import struct
                expected=b''.join(struct.pack('<H',i&65535) for i in range(480000))
                if any((target/frame['path']).read_bytes()!=expected for frame in value['frames']):raise ValueError('Held oracle differs from independent pattern')
            held.append(dict(mode=mode,reason=reason,result=value))
        report['controls']['drawing-source-held-snapshot']['cases']=held
        for name,source in [('lifetime','tests/test-drawing-cursor-mutex-lifetime.cpp'),('tokens','tests/test-drawing-cursor-gate-tokens.cpp'),('leases','tests/test-drawing-cursor-gate-lease.cpp'),('recorder','tests/rendering-recorder-state-test.c')]:
            for mode,opts in [('optimized',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
                label=name+'-'+mode;binary=out/label;dep=out/(label+'.d')
                compiler=['clang','-std=c11'] if source.endswith('.c') else ['clang++','-std=c++17','-pthread']
                run(label+'-build',[*compiler,*opts,'-UNDEBUG','-Wall','-Wextra','-Werror','-MD','-MF',dep,ROOT/source,'-o',binary])
                dependencies(dep);run(label,[binary],extra={'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'})
                report['controls'][label]=dict(success=True,binary_sha256=sha(binary))
        for name in ['source-held-refusal','source-pending-refusal','wine-lifetime','wine-buffer','source-held-pending','source-pending-admission']:
            run('decoder-'+name,[sys.executable,'-B',ROOT/('tests/test-drawing-'+name+'.py')])
        run('fourth-ten-controls',[sys.executable,'-B',ROOT/'tests/test-rendering-fourth-ten.py'])
        report['sources_stable']=all(sha(ROOT/name)==digest for name,digest in report['sources'].items())
        if not report['sources_stable']:raise ValueError('Source drift during execution')
        current=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        fresh=module('fourth_final_claims','tools/draft-coverage-claims.py').declaration(ROOT,current,BEHAVIORS,[SCENARIO])
        if fresh['claims']!=declaration['claims']:raise ValueError('Contract drift during execution')
        report['success']=True
    except Exception as error:report['error']=repr(error)
    finally:
        if lock is not None:
            try:
                run('owned-prefix-stop',['wineserver','-k'],expected=(0,1))
                run('owned-prefix-retired',['wineserver','-w'],timeout=20)
            except Exception as error:
                report.update(success=False,retirement_error=repr(error))
            finally:lock.close()
        report['source_inputs']=len(report['sources'])
        report['sha256']=write_report(ROOT,report_path,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],report=str(report_path),sha256=report['sha256'],error=report.get('error'))),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
