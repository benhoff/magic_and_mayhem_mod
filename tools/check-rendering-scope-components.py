#!/usr/bin/env python3
"""Bounded current-source Qt fullscreen and owned facade format controls.

No original executable/media, game journey, strict-player or physical-reset
acceptance is asserted. RAM build/prefix artifacts are retained through SQLite.
"""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import resource
import shlex
import shutil
import struct
import subprocess

from coverage_claims import behavior_contract, required_sources
from coverage_evidence import read_json, write_report

ROOT = Path(__file__).resolve().parents[1]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--report',required=True)
    parser.add_argument('--build-cache',type=Path,help='Reusable RAM CMake build; configure/build checks still run')
    args=parser.parse_args();out=args.output.resolve()
    temporary=Path(os.environ.get('TMPDIR','/tmp')).resolve()
    if out.exists() or not out.is_relative_to(temporary) or (ROOT/args.report).exists():
        parser.error('Require fresh RAM output and research report')
    target=ROOT/args.report
    if Path(args.report).is_absolute() or not target.resolve().is_relative_to(ROOT/'research/runtime'):
        parser.error('Report must be a new research/runtime reference')
    out.mkdir(parents=True)
    (out/'runtime').mkdir(mode=0o700)
    build=args.build_cache.resolve() if args.build_cache else out/'build'
    if not build.is_relative_to(temporary):parser.error('Build cache must stay in RAM staging')
    register=read_json(ROOT/'research/runtime/coverage/register.json',root=ROOT)
    bids=['HOST.presentation','NP.scope-format-descriptor-control']
    behaviors={b['id']:b for b in register['behaviors']};builds={b['id']:b for b in register['builds']}
    paths=set().union(*(required_sources(behaviors[b]) for b in bids))
    for folder in ['renderer','assets','audio','compat/legacy','reconstruction/rendering','protocols/include','apps/qt-shell','runtime/native-draw','runtime/shadow']:
        for p in (ROOT/folder).rglob('*'):
            if p.is_file() and (p.suffix in {'.c','.h','.cpp','.hpp','.S','.inc','.inl','.cmake'} or p.name=='CMakeLists.txt'):
                paths.add(p.relative_to(ROOT).as_posix())
    paths.update(['tools/check-rendering-scope-components.py','tools/build-native-draw.py','tests/native-scope-format-probe.c'])
    sources={p:digest(ROOT/p) for p in sorted(paths)}
    report=dict(schema=1,success=False,sources=sources,
        claims=[dict(behavior=b,contract_sha256=behavior_contract(behaviors[b],builds)) for b in bids],
        scope=__doc__,commands=[],original_media_consumed=False,original_game_executed=False,
        strict_player_executed=False,physical_device_reset=False,complete_replacement=False,
        artifact_root=str(out),temporary_artifacts_retained_in_sqlite=False)
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE')) and k not in
         {'DISPLAY','WAYLAND_DISPLAY','MESA_LOADER_DRIVER_OVERRIDE','GALLIUM_DRIVER','__GLX_VENDOR_LIBRARY_NAME',
          '__EGL_VENDOR_LIBRARY_FILENAMES','QT_QPA_PLATFORM_PLUGIN_PATH','QT_XCB_GL_INTEGRATION'}}
    env.update(TMPDIR=str(temporary),QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',GALLIUM_DRIVER='llvmpipe',
               LP_NUM_THREADS='4',WINEPREFIX=str(out/'prefix'),WINEDEBUG='-all',
               XDG_RUNTIME_DIR=str(out/'runtime'),XDG_CACHE_HOME=str(out/'cache'))
    def run(label,argv,trial=None,timeout=180):
        print(label,flush=True)
        argv=list(map(str,argv));display_log=None
        if argv[0]=='xvfb-run':
            display_log=out/(label+'-xvfb.log');argv[1:1]=['-e',str(display_log)]
        result=subprocess.run(list(map(str,argv)),cwd=out,env=trial or env,capture_output=True,timeout=timeout)
        raw=result.stdout+b'\n'+result.stderr+(b'\n'+display_log.read_bytes() if display_log and display_log.exists() else b'')
        sha=write_report(ROOT,str(Path(args.report).with_suffix(''))+'-'+label+'.log',raw)
        report['commands'].append(dict(label=label,argv=list(map(str,argv)),returncode=result.returncode,sha256=sha))
        if result.returncode:raise ValueError(label+' failed: '+raw.decode(errors='replace')[-3000:])
        return result.stdout
    try:
        run('configure',['cmake','-S',ROOT/'apps/qt-shell','-B',build,'-DCMAKE_BUILD_TYPE=Debug'])
        run('build',['cmake','--build',build,'--target','viewport-presentation-test','-j2'],timeout=900)
        binary=out/'viewport-presentation-test';shutil.copy2(build/'viewport-presentation-test',binary)
        report['presentation_binary_sha256']=digest(binary)
        report['presentations']=[]
        for scale in ['1','1.5']:
            raw=run('qt-'+scale,['xvfb-run','-a','-s','-screen 0 1920x1440x24',binary],dict(env,QT_SCALE_FACTOR=scale))
            value=json.loads(raw);assert value['success'] and value['fullscreen_restore'] and value['f9_consumed'] and value['logical_input_preserved']
            report['presentations'].append(dict(scale=scale,**value))
        spec=importlib.util.spec_from_file_location('scope_facade',ROOT/'tools/build-native-draw.py')
        builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)
        dll=builder.build(out/'facade',sources)
        report['facade_build']=json.loads((out/'facade/manifest.json').read_bytes())
        imports={'LoadLibraryA':4,'GetProcAddress':8,'GetEnvironmentVariableA':12,'ExitProcess':4,
                 'CreateFileA':28,'WriteFile':20,'CloseHandle':4}
        definition=out/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(n+'@'+str(s)+'\n' for n,s in imports.items()))
        run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
        run('format-compile',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector',
              '-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-MD','-MF',out/'format.d','-c',ROOT/'tests/native-scope-format-probe.c','-o',out/'format.obj'])
        run('format-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no',
                          '/timestamp:0','/out:'+str(out/'format.exe'),out/'format.obj',out/'kernel32.lib'])
        run('wine-version',['wine','--version'])
        run('wineboot',['xvfb-run','-a','wineboot','-i'],dict(env,WINEDLLOVERRIDES='mscoree,mshtml=;winemenubuilder.exe=d'),timeout=150)
        run('formats',['xvfb-run','-a','wine',out/'format.exe'],dict(env,MNM_SCOPE_LIBRARY='Z:'+str(dll).replace('/',chr(92))))
        rows=list(struct.iter_unpack('<12I',(out/'formats.bin').read_bytes()))
        assert len(rows)==3 and [r[2:5] for r in rows]==[(0xf800,0x7e0,31),(0xf800,0x7e0,31),(0x7c00,0x3e0,31)]
        assert all(r[1]==16 and r[5:7]==(13,9) and r[8]==0x9234 and r[9:]==(0,0,0) for r in rows)
        report['format_rows']=[dict(kind=r[0],bits=r[1],masks=list(r[2:5]),width=r[5],height=r[6],pitch=r[7],
                                   preserved_word=r[8],remaining_surfaces=r[9],remaining_devices=r[10]) for r in rows]
        compiled=set()
        for dep in list(build.rglob('*.o.d'))+[out/'format.d']:
            for token in shlex.split(dep.read_text().replace('\\\n',' ')):
                p=Path(token)
                if p.is_file() and p.resolve().is_relative_to(ROOT):compiled.add(p.resolve().relative_to(ROOT).as_posix())
        assert not compiled-set(sources),'Undeclared compiler inputs: '+repr(sorted(compiled-set(sources)))
        report['compiled_sources']={p:sources[p] for p in sorted(compiled)}
        report['success']=True
    except Exception as error:
        report['error']=str(error)
    finally:
        if (out/'prefix').exists():
            for command in [['wineserver','-k'],['wineserver','-w']]:
                result=subprocess.run(command,env=env,capture_output=True,timeout=30)
                report.setdefault('prefix_cleanup',[]).append(dict(command=command,returncode=result.returncode))
        artifacts=[]
        # Retain unique controls/binaries/declarations; system Wine files and
        # reproducible build objects are not copied as a second whole prefix.
        selected=[out/'formats.bin',out/'format.exe',out/'format.obj',out/'format.d',out/'facade/MnmDraw.dll',out/'facade/manifest.json',
                  out/'facade/factory.obj',out/'facade/pixel_kernel.obj',out/'kernel32.def',out/'kernel32.lib',
                  out/'viewport-presentation-test']
        for p in selected:
            if p.is_file():
                name=str(Path(args.report).with_suffix(''))+'-raw-'+p.relative_to(out).as_posix().replace('/','-')+'.bin'
                sha=write_report(ROOT,name,p.read_bytes());artifacts.append(dict(name=p.relative_to(out).as_posix(),record=name,sha256=sha))
        report['retained_artifacts']=artifacts
        report['temporary_artifacts_retained_in_sqlite']=True
        report['sources_stable']=all(digest(ROOT/p)==h for p,h in sources.items())
        report['success']=report['success'] and report['sources_stable']
        sha=write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],sha256=sha,error=report.get('error'),report=args.report)),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
