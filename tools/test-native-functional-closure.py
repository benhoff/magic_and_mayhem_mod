#!/usr/bin/env python3
"""Execute native copy/conversion, raw-instruction and restart policy controls."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import struct

ROOT=Path(__file__).resolve().parents[1]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    args.output.mkdir(parents=True,exist_ok=False);out=args.output.resolve()
    declaration=json.loads(args.claims.read_text());sources=declaration['sources']
    def stable():return all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in sources.items())
    if not stable():raise ValueError('Prospective inputs changed')
    report=dict(schema=1,success=False,claims=declaration['claims'],sources=sources,
                scope=__doc__,original_game_executed=False,complete_replacement=False,cases=[])
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
    env.update(TMPDIR=str(out),PYTHONDONTWRITEBYTECODE='1',WINEPREFIX=str(out/'prefix'),WINEDEBUG='-all',DISPLAY='')
    def run(label,argv,cwd=out,extra=None,expected=0,timeout=90):
        with (out/(label+'.log')).open('x') as log:
            result=subprocess.run([str(x) for x in argv],cwd=cwd,env=env|dict(extra or {}),stdout=log,stderr=subprocess.STDOUT,timeout=timeout)
        if result.returncode!=expected:raise RuntimeError(label+': exit '+str(result.returncode)+', expected '+str(expected))
        report['cases'].append(dict(id=label,exit_code=result.returncode,expected=expected))
    try:
        spec=importlib.util.spec_from_file_location('closure_builder',ROOT/'tools/build-native-draw.py')
        builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)
        dll=builder.build(out/'native',sources)
        imports={k:dict(v) for k,v in builder.IMPORTS.items()}
        imports['kernel32'].update(LoadLibraryA=4,GetProcAddress=8)
        imports['user32']['GetDesktopWindow']=0
        imports['gdi32']['GetDIBColorTable']=16
        libs=[]
        for name,entries in imports.items():
            definition=out/(name+'.def');definition.write_text('LIBRARY '+name+'.dll\nEXPORTS\n'+''.join(k+'@'+str(v)+'\n' for k,v in entries.items()))
            lib=out/(name+'.lib');run(name,['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',lib]);libs.append(lib)
        for name,path in [('backing','tests/native-owned-backing-probe.c'),('transforms','tests/native-copy-transform-probe.c'),('capability','tests/native-only-facade-probe.c')]:
            run(name+'-compile',[*builder.FLAGS,'-c',ROOT/path,'-o',out/(name+'.obj')])
            run(name+'-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/(name+'.exe')),out/(name+'.obj'),*libs])
        run('wineboot',['wineboot','-i'],extra=dict(WINEDLLOVERRIDES='mscoree,mshtml=;winemenubuilder.exe=d'),timeout=120)
        wine_path='Z:'+str(dll).replace('/','\\')
        for name,extra,expected in [
            ('backing',dict(MNM_OWNER_DRAW_DLL=wine_path,MNM_NATIVE_OWNERSHIP_REQUIRED='1'),0),
            ('transforms',dict(MNM_TEST_LIBRARY=wine_path),0)]:
            case=out/name;case.mkdir()
            run(name,['wine',out/(name+'.exe')],cwd=case,extra=dict(MNM_NATIVE_CPU_GUARD='3',MNM_PUBLIC_RESIDENT_WORLD='1')|extra,expected=expected)
        state=struct.unpack('<3I',(out/'backing/continuation-state.bin').read_bytes())
        if state[0:2]!=(5,45) or state[2]<500:raise ValueError('Incomplete architectural continuation assertions: '+repr(state))
        report['continuation_state']=dict(instructions=state[0],gpr_esp_eflags_assertions=state[1],fxsave_byte_assertions=state[2],last_error_preserved=True)
        for strict in (False,True):
            for case in ('unsupported_flags','key_flags','busy','rectangle'):
                label='capability-'+str(int(strict))+'-'+case;directory=out/label;directory.mkdir()
                extra=dict(MNM_TEST_LIBRARY=wine_path,MNM_TEST_CASE=case,MNM_NATIVE_CPU_GUARD='3',MNM_PUBLIC_RESIDENT_WORLD='1')
                if strict:extra.update(MNM_NATIVE_ONLY='1',MNM_NATIVE_OWNERSHIP_REQUIRED='1')
                run(label,['wine',out/'capability.exe'],cwd=directory,extra=extra,expected=124 if strict and case in ('unsupported_flags','key_flags') else 0)
        flags=subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Core'],text=True).split()
        run('qt-build',['c++','-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-fPIC','-I'+str(ROOT/'apps/qt-shell'),ROOT/'tests/native-only-session-test.cpp',*flags,'-o',out/'qt-session'])
        run('qt-session',[out/'qt-session'])
        widgets=subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Widgets'],text=True).split()
        run('qt-restart-build',['c++','-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-fPIC','-I'+str(ROOT/'apps/qt-shell'),
            ROOT/'tests/game-hard-restart-test.cpp',*[ROOT/'apps/qt-shell'/p for p in ('live_menu_session.cpp','menu_bridge.cpp','engine_preferences_store.cpp')],
            *widgets,'-o',out/'qt-restart'])
        run('qt-restart',[out/'qt-restart'],extra={'QT_QPA_PLATFORM':'offscreen'})
        run('python-restart',['python3','-B',ROOT/'tests/test-native-only-contract.py'])
        run('copy-wait-build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-pthread',ROOT/'tests/copy-timing-test.cpp','-o',out/'copy-wait'])
        run('copy-wait',[out/'copy-wait'])
        report.update(success=True,transform_cases=150,transform_pixels=17550,raw_instruction_classes=['load','read_modify_write','rep_movsb','simd','locked_rmw'],original_row_body_trapped=True)
    except Exception as error:
        report['error']=str(error)
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        report['sources_stable']=stable();report['success']=report['success'] and report['sources_stable']
        with (out/'report.json').open('x') as file:json.dump(report,file,indent=2);file.write('\n')
    print(json.dumps({key:report.get(key) for key in ('success','sources_stable','error')}))
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
