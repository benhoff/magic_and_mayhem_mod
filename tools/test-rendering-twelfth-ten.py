#!/usr/bin/env python3
"""Execute ten bounded rendering/profile contracts; retain evidence in SQLite."""
import argparse,ast,hashlib,importlib.util,json,os,re,shlex,shutil,subprocess,sys,tempfile,time
from pathlib import Path
from coverage_evidence import evidence_digest,read_json,write_report
from drawing_fixture_prefix import prepare
from rendering_twelfth_controls import last_json,copy_cohorts,physical_frames,bind_case
ROOT=Path(__file__).resolve().parents[1]
BEHAVIORS=['RS.displacement-linear-stride','RS.shadow-linear-stride','NR.resident-stage-profiling','NR.native-draw-cost-profiling','NR.native-world-owner-costs','NR.scene-production-repetition','NR.scene-draw-budget','NR.callback-publication-isolation','NR.drawing-copy-phase-hash-comparison','NR.recorded-physical-frame-admission']
SCENARIO='NR.twelfth-ten.native-controls-20261022'
TESTS=['tests/test-rendering-twelfth-ten.py','tests/test-drawing-copy-phase-hashes.py','tests/test-drawing-copy-phase.py','tests/test-drawing-physical-generation.py','tests/test-drawing-recorded-hardware.py','tests/test-drawing-fixture-prefix.py']
TARGETS=['twelfth-linear','low-owner-profile','render-gpu-timing-test','palette-row-cache-test','low-resident-capture','twelfth_headless_egl','twelfth-resident-profile','mnm-drawing-present','mnm-drawing-scene-replay','canvas-sequence-test','other-writer-atomic-test']
def sha(p):return evidence_digest(Path(p),root=ROOT)
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',type=Path,required=True);parser.add_argument('--wine-prefix',type=Path,required=True);parser.add_argument('--prefix-owner-root',type=Path,required=True);parser.add_argument('--prepared',type=Path,help='Reuse a source/build/binary-bound native build from a retained earlier attempt')
    a=parser.parse_args();destination=a.report if a.report.is_absolute() else ROOT/a.report
    if destination.exists():parser.error('Refusing evidence overwrite')
    parent=ROOT/'working/tests/rendering-twelfth-ten';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(out/'.retain').write_text('Twelfth rendering raw native controls, binaries, logs, source-only inputs and failures; durable report in SQLite.\n');print(out,flush=True)
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());declaration=module('twelfth_claims','tools/draft-coverage-claims.py').declaration(ROOT,register,BEHAVIORS,[SCENARIO])
    todo=set(declaration['sources'])|set(TESTS)|{'tools/test-rendering-twelfth-ten.py','tools/rendering_twelfth_controls.py','tools/coverage_evidence.py','tools/coverage_claims.py','tools/draft-coverage-claims.py','tools/build-render-bridge.py','tools/build-scene-production.py','tools/build-menu-observer.py','tools/build-shadow-bridge.py','tools/drawing_fixture_prefix.py','tests/resident-bind-admission.c','tests/campaign-pacer-reference.c','tests/scene-production-reference.c','tests/scene-production-reference.S','tests/resident-profile-capture-test.py'}
    for directory in ('runtime','assets','renderer','compat/legacy','apps','protocols','reconstruction'):
        todo.update(p.relative_to(ROOT).as_posix() for p in (ROOT/directory).rglob('*') if p.is_file() and (p.suffix in {'.c','.h','.cpp','.hpp','.S','.inc'} or p.name=='CMakeLists.txt'))
    todo.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'protocols').rglob('*.json'))
    todo.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'tests').rglob('CMakeLists.txt'))
    todo.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'tools/native-render-profile').rglob('*') if p.is_file() and (p.suffix in {'.cpp','.hpp','.h','.json'} or p.name=='CMakeLists.txt'))
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
                names=([alias.name for alias in node.names] if isinstance(node,ast.Import) else [node.module]+[node.module+'.'+alias.name for alias in node.names] if isinstance(node,ast.ImportFrom) and node.module else [])
                for imported in names:
                    relative=Path(*imported.split('.'))
                    for base in (ROOT/'tools',ROOT/'protocols/python'):
                        for candidate in (base/relative.with_suffix('.py'),base/relative/'__init__.py'):
                            if candidate.is_file():todo.add(candidate.relative_to(ROOT).as_posix())
        if path.suffix in {'.c','.h','.cpp','.hpp','.S','.inc'}:
            for include in re.findall(r'^\s*#\s*include\s*"([^"]+)"',path.read_text(),re.M):
                child=(path.parent/include).resolve()
                if child.is_file() and child.is_relative_to(ROOT):todo.add(child.relative_to(ROOT).as_posix())
    report=dict(schema=1,success=False,sources=declaration['sources'],claims=declaration['claims'],commands=[],tests_run=0,original_game_executed=False,original_artifacts_consumed=False,original_comparison_claimed=False,live_replacement=False,complete_gameplay_or_performance_acceptance=False,reviewed_head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),artifacts=out.relative_to(ROOT).as_posix())
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE')) and k not in {'DISPLAY','WAYLAND_DISPLAY','QT_QPA_PLATFORM_PLUGIN_PATH','__EGL_VENDOR_LIBRARY_FILENAMES','__GLX_VENDOR_LIBRARY_NAME','MESA_LOADER_DRIVER_OVERRIDE','GALLIUM_DRIVER','QT_XCB_GL_INTEGRATION','LIBGL_ALWAYS_SOFTWARE'}}
    (out/'tmp').mkdir();(out/'runtime').mkdir(mode=0o700);env.update(TMPDIR=str(out/'tmp'),XDG_RUNTIME_DIR=str(out/'runtime'),PYTHONDONTWRITEBYTECODE='1',QT_QPA_PLATFORM='xcb',WINEPREFIX=str(a.wine_prefix.resolve()),WINEDEBUG='-all',XDG_CACHE_HOME=str(out/'cache'),MESA_SHADER_CACHE_DIR=str(out/'mesa-cache'),__GL_SHADER_DISK_CACHE_PATH=str(out/'nvidia-cache'))
    lock=None
    def private_display(command):
        temporary=Path(tempfile.mkdtemp(prefix='x11-host-',dir=out));(temporary/'.X11-unix').mkdir(mode=0o1777);(temporary/'.X11-unix').chmod(0o1777)
        shared=(ROOT/'working/coverage/evidence-pending.sqlite3').resolve().parent
        return ['bwrap','--die-with-parent','--new-session','--unshare-user','--unshare-pid','--unshare-net','--unshare-ipc','--unshare-uts','--ro-bind','/','/','--bind',str(ROOT),str(ROOT),'--bind',str(shared),str(shared),'--bind',str(temporary),'/tmp','--proc','/proc','--dev','/dev','--setenv','LIBGL_ALWAYS_SOFTWARE','1','--setenv','__EGL_VENDOR_LIBRARY_FILENAMES','/usr/share/glvnd/egl_vendor.d/50_mesa.json','--',*map(str,command)]
    def run(label,command,expected=0,extra=None,cwd=ROOT,timeout=180):
        command=list(map(str,command))
        if command[0]=='xvfb-run':command=private_display([command[0],'-d','-s','-screen 0 1280x1024x24','-e',str(out/(label+'-server.log')),*command[1:]])
        print(label,flush=True);path=out/(label+'.log')
        try:
            # Persistent Wine descendants may retain stdout; a regular file
            # records their output without waiting for pipe EOF after parent exit.
            with path.open('x') as output:r=subprocess.run(command,cwd=cwd,env={**env,**(extra or {})},stdout=output,stderr=subprocess.STDOUT,timeout=timeout)
        except subprocess.TimeoutExpired:
            log=path.read_text(errors='replace');report['commands'].append(dict(label=label,argv=command,expected_exit=expected,actual_exit='timeout',log=log,log_sha256=hashlib.sha256(log.encode()).hexdigest()));raise
        log=path.read_text(errors='replace')
        report['commands'].append(dict(label=label,argv=command,expected_exit=expected,actual_exit=r.returncode,log=log,log_sha256=hashlib.sha256(log.encode()).hexdigest()))
        if r.returncode not in (expected if isinstance(expected,tuple) else (expected,)):raise ValueError(label+': '+log[-6000:])
        return log
    try:
        if shutil.disk_usage('/var/local/magic-and-mayhem-archive').free<100*1024**3:raise ValueError('Second-drive reserve')
        owner=a.prefix_owner_root.resolve();prefix=a.wine_prefix.resolve()
        common=lambda p:(p/Path(subprocess.check_output(['git','-C',str(p),'rev-parse','--git-common-dir'],text=True).strip())).resolve()
        if common(owner)!=common(ROOT):raise ValueError('Fixture owner must share this registered Git checkout')
        prefix,lock=prepare(owner,out,prefix,prefix);report['fixture_prefix']=dict(path=str(prefix),owner=str(owner),exclusive_lock=True,idle_before_use=True)
        run('compiler-version',['c++','--version']);run('wine-version',['wine','--version']);run('wineboot',['wineboot','-u']);run('prefix-bootstrap-stop',['wineserver','-k'],expected=(0,1));run('prefix-bootstrap-retired',['wineserver','-w']);run('prefix-keepalive',['wineserver','-p'],expected=(0,1))
        build=out/'build'
        if a.prepared:
            old=read_json(a.prepared);build=(ROOT/old['artifacts']/'build').resolve()
            if not build.is_relative_to(ROOT/'working/tests/rendering-twelfth-ten') or not any(c['label']=='native-build' and c['actual_exit']==0 for c in old['commands']):raise ValueError('Prepared attempt lacks a completed owned native build')
            needed={p for p in old['sources'] if Path(p).name=='CMakeLists.txt' or p.startswith('tools/native-render-profile/')}
            for dep in build.rglob('*.o.d'):
                for token in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
                    path=Path(token).resolve()
                    if path.is_file() and path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):needed.add(path.relative_to(ROOT).as_posix())
            if not needed or any(old['sources'].get(p)!=report['sources'].get(p) or sha(ROOT/p)!=old['sources'][p] for p in needed):raise ValueError('Prepared compiled/build input changed')
            for name,item in old['binaries'].items():
                path=ROOT/item['path']
                if not path.is_relative_to(build) or sha(path)!=item['sha256']:raise ValueError('Prepared binary changed '+name)
            if sha(build/'platforms/libtwelfth_headless_egl.so')!=old['hardware_plugin_sha256']:raise ValueError('Prepared platform plugin changed')
            report['prepared_native_build']=dict(record=str(a.prepared),sha256=evidence_digest(a.prepared),build=build.relative_to(ROOT).as_posix(),verified_compile_and_build_inputs=len(needed),all_native_binaries_verified=True)
        else:run('native-configure',['cmake','-S',ROOT/'tests/rendering-twelfth-ten','-B',build,'-DCMAKE_BUILD_TYPE=Release','-DBUILD_TESTING=ON'])
        run('native-build',['cmake','--build',build,'--target',*TARGETS,'-j3'],timeout=900)
        binaries={name:next(p for p in build.rglob(name) if p.is_file() and os.access(p,os.X_OK)) for name in TARGETS if name!='twelfth_headless_egl'}
        report['binaries']={name:dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)) for name,p in binaries.items()}
        compiled=set()
        for dependency in build.rglob('*.o.d'):
            for token in shlex.split(dependency.read_text().replace('\\\n',' ').split(':',1)[1]):
                p=Path(token).resolve()
                if p.is_file() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):compiled.add(p.relative_to(ROOT).as_posix())
        if compiled-set(report['sources']):raise ValueError('Undeclared native compiler inputs: '+repr(sorted(compiled-set(report['sources']))))
        report['compiled_source_inputs']=len(compiled)
        report['native']={}
        for backend in ('software','hardware'):
            report['native'][backend]={}
            for name,label in [('twelfth-linear','linear'),('low-owner-profile','owner'),('render-gpu-timing-test','gpu'),('palette-row-cache-test','palette')]:
                extra={} if backend=='software' else dict(QT_QPA_PLATFORM='mnm-headless-egl',QT_QPA_PLATFORM_PLUGIN_PATH=str(build/'platforms'),MNM_PROFILE_EGL_VENDOR='NVIDIA',__EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/10_nvidia.json')
                command=['xvfb-run','-a',binaries[name]] if backend=='software' else [binaries[name]]
                result=last_json(run(backend+'-'+label,command,extra=extra))
                if not result['success'] or result.get('remaining_surfaces',0):raise ValueError('Native fixture closure failed')
                if backend=='hardware' and label in ('gpu','linear') and 'TITAN' not in result['renderer']:raise ValueError('Hardware provider is not TITAN')
                report['native'][backend][label]=result
        report['hardware_plugin_sha256']=sha(build/'platforms/libtwelfth_headless_egl.so')
        run('native-canvas-sequence',[binaries['canvas-sequence-test']]);report['atomic_writers']=last_json(run('native-atomic-writers',[binaries['other-writer-atomic-test']]))
        capture=out/'capture';report['capture_worker']=last_json(run('native-capture-worker',[binaries['low-resident-capture'],capture]));report['capture_roundtrip']=last_json(run('capture-python-roundtrip',[sys.executable,'-B',ROOT/'tests/resident-profile-capture-test.py',capture]))
        report['copy_cohorts'],frames=copy_cohorts(ROOT,out,binaries['mnm-drawing-scene-replay'],run)
        report['physical']=physical_frames(out,binaries['mnm-drawing-present'],frames,run)
        # Current production macros and selftest variants compile separately.
        run('production-render-bridge',[sys.executable,'-B',ROOT/'tools/build-render-bridge.py'])
        run('selftest-render-bridge',[sys.executable,'-B',ROOT/'tools/build-render-bridge.py','--selftest'])
        run('production-scene-observer',[sys.executable,'-B',ROOT/'tools/build-scene-production.py'])
        scene=out/'scene-pe32';scene.mkdir();code="import importlib.util,sys;from pathlib import Path;s=importlib.util.spec_from_file_location('fixture_builder',sys.argv[1]);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);m.build(Path(sys.argv[2]),selftest=True)"
        run('selftest-scene-observer',[sys.executable,'-B','-c',code,ROOT/'tools/build-scene-production.py',scene])
        flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
        for source in ('scene-production-reference.c','scene-production-reference.S'):
            run('compile-'+source,[*flags,'-MD','-MF',scene/(source+'.d'),'-c',ROOT/'tests'/source,'-o',scene/(source+'.obj')])
        exe=scene/'fixture.exe';run('link-scene-fixture',['lld-link','/machine:x86','/entry:start','/subsystem:console','/base:0x400000','/fixed','/dynamicbase:no','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(exe),scene/'scene-production-reference.c.obj',scene/'scene-production-reference.S.obj',scene/'MnmSceneProduction.lib',scene/'kernel32.lib'])
        report['scene_production_binary_sha256']=sha(exe)
        report['scene_production_cases']=[]
        for mode in (1,2,3,4,5):
            for case in [*range(16 if mode==5 else 11),16]:
                key=str(case) if case<10 else chr(ord('A')+case-10);key+=str(mode)
                run('scene-case-'+key,['wine',exe],extra=dict(MNM_PRODUCTION_TEST_CASE=key),cwd=scene,timeout=60);report['scene_production_cases'].append(dict(mode=mode,case=case,exit=0))
        report['scene_decoder']=last_json(run('scene-decoder',[sys.executable,'-B',ROOT/'tests/test-scene-production-decoder.py']))
        bind=out/'bind-pe32';bind.mkdir();dll=ROOT/'working/build/render-selftest/MnmRender.dll';shutil.copyfile(dll,bind/dll.name)
        exe=bind/'fixture.exe';obj=bind/'fixture.obj';run('compile-bind-fixture',[*flags,'-MD','-MF',bind/'fixture.d','-c',ROOT/'tests/resident-bind-admission.c','-o',obj]);run('link-bind-fixture',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(exe),obj,dll.with_suffix('.lib'),dll.parent/'kernel32.lib'])
        report['bind_binary_sha256']=sha(exe);report['bind_dll_sha256']=sha(dll)
        report['publication_cases']=[]
        for width,case,probe,deferred in ((640,'baseline',True,False),(800,'baseline',True,False),(800,'baseline',True,True),(800,'tracker',False,False),(800,'reentrant',False,False),(800,'timeout',False,False)):
            print('publication-'+str(width)+'-'+case+('-deferred' if deferred else ''),flush=True);report['publication_cases'].append(bind_case(ROOT,out,exe,dll,prefix,env,width,case,probe,deferred))
        pacer=out/'pacer.exe';run('compile-pacer-fixture',[*flags,'-MD','-MF',out/'pacer.d','-c',ROOT/'tests/campaign-pacer-reference.c','-o',out/'pacer.obj']);run('link-pacer-fixture',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(pacer),out/'pacer.obj',dll.parent/'kernel32.lib']);run('native-pacer-fixture',['wine',pacer]);report['pacer']=dict(binary_sha256=sha(pacer),success=True,mutated_anchor_bytes=33,original_clock_calls=8,paired_observations=2,last_error_preserved=True)
        for manifest,key in ((ROOT/'working/build/render/manifest.json','compiled_sources'),(ROOT/'working/build/render-selftest/manifest.json','compiled_sources'),(ROOT/'working/build/scene-production/manifest.json','sources'),(scene/'manifest.json','sources')):
            for name,digest in json.loads(manifest.read_text())[key].items():
                if report['sources'].get(name)!=digest:raise ValueError('Undeclared PE32 production compiler input '+name)
        for dependency in [*scene.glob('*.d'),*bind.glob('*.d'),out/'pacer.d']:
            for token in shlex.split(dependency.read_text().replace('\\\n',' ').split(':',1)[1]):
                p=Path(token).resolve()
                if p.is_file() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working') and p.relative_to(ROOT).as_posix() not in report['sources']:raise ValueError('Undeclared PE32 fixture input '+str(p))
        for index,path in enumerate(TESTS,1):
            log=run('python-'+str(index),[sys.executable,'-B',ROOT/path],extra={'PYTHONPATH':str(ROOT/'tools')});match=re.search(r'Ran (\d+) tests?',log)
            if not match:raise ValueError('No test count '+path)
            report['tests_run']+=int(match[1])
        report['sources_stable']=all(sha(ROOT/p)==digest for p,digest in report['sources'].items())
        current=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        report['contracts_stable']=module('twelfth_final_claims','tools/draft-coverage-claims.py').declaration(ROOT,current,BEHAVIORS,[SCENARIO])['claims']==declaration['claims']
        if not report['sources_stable'] or not report['contracts_stable']:raise ValueError('Source or contract drift')
        report['success']=True
    except Exception as error:
        import traceback
        report['error']=repr(error);report['traceback']=traceback.format_exc()
    finally:
        if lock is not None:
            try:run('owned-prefix-stop',['wineserver','-k'],expected=(0,1));run('owned-prefix-retired',['wineserver','-w'],timeout=20);report['private_wine_stopped']=True
            except Exception as error:report.update(success=False,retirement_error=repr(error))
            finally:lock.close()
        for row in report['commands']:
            log=(out/(row['label']+'.log')).read_text(errors='replace');row['log']=log;row['log_sha256']=hashlib.sha256(log.encode()).hexdigest()
        report['source_inputs']=len(report['sources']);digest=write_report(ROOT,destination,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],report=str(destination),sha256=digest,source_inputs=report['source_inputs'],tests=report['tests_run'],commands=len(report['commands']),error=report.get('error'))),flush=True)
    return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
