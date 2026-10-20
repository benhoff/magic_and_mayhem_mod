#!/usr/bin/env python3
"""Execute guarded startup alias retirement and atomic refusal with owned PE32 stubs."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess

from native_draw_retirement import read_retirement, read_startup_retirement, require_retired

ROOT=Path(__file__).resolve().parents[1]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();out=args.output.resolve()
    if not os.environ.get('DISPLAY') or not out.is_relative_to(ROOT/'working'):
        parser.error('Require isolated DISPLAY and new working/ output')
    out.mkdir(parents=True,exist_ok=False)
    declaration=json.loads(args.claims.read_text());sources=declaration['sources']
    sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
    stable=lambda:all(sha(ROOT/p)==h for p,h in sources.items())
    report=dict(schema=1,success=False,scope=__doc__,sources=sources,claims=declaration['claims'],
                original_game_executed=False,complete_retirement=False,cases=[])
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
    env.update(WINEPREFIX=str(out/'prefix'),WINEDEBUG='-all',LIBGL_ALWAYS_SOFTWARE='1')
    def run(label,command,extra=None,cwd=out,expected=0):
        with (out/(label+'.log')).open('x') as log:
            result=subprocess.run([str(x) for x in command],cwd=cwd,env=env|dict(extra or {}),
                                  stdout=log,stderr=subprocess.STDOUT,timeout=120)
        if result.returncode!=expected:raise RuntimeError(label+': exit '+str(result.returncode)+', expected '+str(expected))
    try:
        if not stable():raise ValueError('Prospective startup fixture sources changed')
        spec=importlib.util.spec_from_file_location('startup_retirement_builder',ROOT/'tools/build-native-draw.py')
        builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)
        dll=builder.build(out/'native-build',sources)
        imports={k:dict(v) for k,v in builder.IMPORTS.items()};imports['kernel32'].update(LoadLibraryA=4,GetProcAddress=8)
        libraries=[]
        for name,entries in imports.items():
            definition=out/(name+'.def');definition.write_text('LIBRARY '+name+'.dll\nEXPORTS\n'+''.join(k+'@'+str(v)+'\n' for k,v in entries.items()))
            library=out/(name+'.lib');run('imports-'+name,['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',library]);libraries.append(library)
        run('compile',[*builder.FLAGS,'-MD','-MF',out/'probe.d','-c',ROOT/'tests/native-draw-startup-retirement-probe.c','-o',out/'probe.obj'])
        run('assemble',['clang','--target=i686-pc-windows-msvc','-c',ROOT/'tests/native-draw-startup-retirement-pad.S','-o',out/'pad.obj'])
        run('link',['lld-link','/machine:x86','/base:0x400000','/entry:start','/subsystem:console','/nodefaultlib',
                    '/safeseh:no','/timestamp:0','/out:'+str(out/'probe.exe'),out/'pad.obj',out/'probe.obj',*libraries])
        import shutil
        shutil.copyfile(dll,out/'MnmDraw.dll')
        inputs={str(p):sha(p) for p in [dll,out/'MnmDraw.dll',out/'probe.exe']}
        run('boot',['wineboot','-i'],dict(WINEDLLOVERRIDES='mscoree,mshtml=;winemenubuilder.exe=d'))
        for mode,meaning in [('1','first surface interface query failure'),('2','second surface interface query failure'),('3','complete capability probe'),
                              ('W','unrelated caller'),('C','unrelated creator'),('R','extra reference'),('B','active Lock'),('E','additional child')]:
            case=out/('case-'+mode);case.mkdir();expected=0 if mode in '123' else 127
            run('case-'+mode,['wine',out/'probe.exe'],dict(WINEDLLOVERRIDES='ddraw=',MNM_PROBE_CASE=mode,
                MNM_NATIVE_DRAW_RETIREMENT='Z:'+str(case/'retirement.bin').replace('/','\\')),case,expected)
            observed=dict(mode=mode,meaning=meaning,exit_code=expected,retirement=read_retirement(case/'retirement.bin'))
            if not expected:
                require_retired(case/'retirement.bin')
                receipt=read_startup_retirement(case/'native-draw-startup-retirement.bin')
                if receipt['surface_references_released']!=(1 if mode=='1' else 2):raise ValueError('Wrong temporary alias count')
                if struct.unpack('<4I',(case/'startup-policy.bin').read_bytes())!=(1,ord(mode),0x51,0):raise ValueError('Caller result, LastError or zero-owner assertions missing')
                observed['last_error_preserved']=True
                observed['startup_probe']=receipt
            else:
                if ((case/'native-draw-startup-retirement.bin').exists() or (case/'startup-policy.bin').exists()
                        or observed['retirement']['counts']['devices']!=1
                        or observed['retirement']['counts']['surfaces']!=(2 if mode=='E' else 1)):
                    raise ValueError('Invalid graph changed owners before refusal')
                rows=observed['retirement']['surviving_owners'];surface=next(row for row in rows if row[0]==2)
                if mode=='R' and surface[7]!=2:raise ValueError('Extra reference changed before refusal')
                if mode=='B' and surface[13]!=1:raise ValueError('Active Lock changed before refusal')
                observed['atomic_refusal']=True
            report['cases'].append(observed)
        closure={}
        for token in (out/'probe.d').read_text().replace('\\\n',' ').split():
            p=Path(token).resolve()
            if p.is_file() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):closure[str(p.relative_to(ROOT))]=sha(p)
        if any(sources.get(p)!=h for p,h in closure.items()):raise ValueError('Undeclared fixture compiler input')
        report.update(compiled_sources=closure,build=json.loads((out/'native-build/manifest.json').read_text()),
                      inputs=inputs,inputs_stable=all(sha(p)==h for p,h in inputs.items()),success=True)
    except Exception as error:report['error']=str(error)
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        report['sources_stable']=stable();report['success'] &= report['sources_stable'] and report.get('inputs_stable',False)
        with (out/'report.json').open('x') as file:json.dump(report,file,indent=2);file.write('\n')
    print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
