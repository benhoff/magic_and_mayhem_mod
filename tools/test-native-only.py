#!/usr/bin/env python3
"""Prospective native-only launch, Qt state and actual PE32 terminal/refusal controls."""
import argparse
from coverage_evidence import write_report
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',type=Path,required=True)
    parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--report',type=Path,required=True)
    args=parser.parse_args()
    if args.output.exists() or args.report.exists():parser.error('Require new output/report paths')
    args.output.mkdir(parents=True);root=args.output.resolve()
    declaration=json.loads(args.claims.read_text());sources=declaration['sources']
    def stable():return all(hashlib.sha256((ROOT/path).read_bytes()).hexdigest()==value for path,value in sources.items())
    if not stable():raise RuntimeError('Declared native-only inputs changed')
    manifest=json.loads((args.library.parent/'manifest.json').read_text())
    if manifest['sha256']!=hashlib.sha256(args.library.read_bytes()).hexdigest() or any(sources.get(p)!=h for p,h in manifest['sources'].items()):
        raise RuntimeError('Facade compiler closure does not match declaration')
    report=dict(schema=1,success=False,claims=declaration['claims'],sources=sources,
                scope=__doc__,original_game_executed=False,complete_replacement=False,cases=[])
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
    env.update(TMPDIR=str(root),PYTHONDONTWRITEBYTECODE='1',WINEPREFIX=str(root/'prefix'),WINEDEBUG='-all',DISPLAY='')
    def run(name,argv,trial=env,timeout=60):
        with (root/(name+'.log')).open('x') as out:
            return subprocess.run(argv,env=trial,stdout=out,stderr=subprocess.STDOUT,timeout=timeout)
    try:
        for path in ('tests/test-native-only-contract.py','tests/test-native-output-owner.py'):
            result=run(Path(path).stem,['python3','-B',str(ROOT/path)])
            if result.returncode:raise RuntimeError('Python launch contract regression: '+path)
        flags=subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Core'],text=True).split()
        subprocess.run(['c++','-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-fPIC','-I'+str(ROOT/'apps/qt-shell'),str(ROOT/'tests/native-only-session-test.cpp'),*flags,'-o',str(root/'qt-session')],env=env,check=True)
        if run('qt-session',[str(root/'qt-session')]).returncode:raise RuntimeError('Qt sticky session controls failed')
        definitions=root/'kernel32.def';definitions.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(n+'@'+str(s)+'\n' for n,s in [('LoadLibraryA',4),('GetProcAddress',8),('GetEnvironmentVariableA',12),('ExitProcess',4),('VirtualQuery',12),('VirtualProtect',16),('GetCurrentThreadId',0),('GetLastError',0),('SetLastError',4),('CreateFileA',28),('WriteFile',20),('CloseHandle',4)]))
        subprocess.run(['llvm-dlltool','-m','i386','--kill-at','-d',str(definitions),'-l',str(root/'kernel32.lib')],env=env,check=True)
        subprocess.run(['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',str(ROOT/'tests/native-only-facade-probe.c'),'-o',str(root/'probe.obj')],env=env,check=True)
        binary=root/'probe.exe';subprocess.run(['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(binary),str(root/'probe.obj'),str(root/'kernel32.lib')],env=env,check=True)
        if run('wineboot',['wineboot','-i'],dict(env,WINEDLLOVERRIDES='mscoree,mshtml=;winemenubuilder.exe=d'),120).returncode:raise RuntimeError('Owned fixture Wine initialization failed')
        for strict in (False,True):
            for case in ('unsupported_flags','key_flags','busy','rectangle','glyph'):
                directory=root/(str(int(strict))+'-'+case);directory.mkdir()
                trial=dict(env,MNM_TEST_CASE=case,MNM_TEST_LIBRARY='Z:'+str(args.library.resolve()).replace('/',chr(92)))
                if strict:trial.update(MNM_NATIVE_ONLY='1',MNM_NATIVE_OWNERSHIP_REQUIRED='1',MNM_NATIVE_CPU_GUARD='3',MNM_PUBLIC_RESIDENT_WORLD='1',MNM_GLYPH_BACKEND='takeover',MNM_GLYPH_CAPTURE='0')
                with (directory/'wine.log').open('x') as out:
                    result=subprocess.run(['wine',str(binary)],cwd=directory,env=trial,stdout=out,stderr=subprocess.STDOUT,timeout=30)
                terminal=strict and case in ('unsupported_flags','key_flags','glyph');expected=124 if terminal else 0
                if result.returncode!=expected:raise RuntimeError('Actual facade '+str(strict)+'/'+case+' exited '+str(result.returncode))
                record=directory/('native-glyph-forward-refusal.bin' if case=='glyph' else 'native-render-capability-refusal.bin')
                if terminal:
                    import struct
                    raw=record.read_bytes();words=struct.unpack('<8I',raw)
                    if case=='glyph':
                        if words[:5]!=(0x474d4e4d,0x3146524c,1,32,17) or not words[5] or any(words[6:]):raise RuntimeError('Terminal glyph refusal diagnostic missing/invalid')
                    elif words[:5]!=(0x434d4e4d,0x31305041,1,32,0x80004001) or not all(words[5:]):raise RuntimeError('Terminal capability diagnostic missing/invalid')
                elif record.exists():raise RuntimeError('Expected application error became terminal')
                report['cases'].append(dict(strict=strict,case=case,exit_code=result.returncode,terminal=terminal))
        report['success']=True
    except Exception as error:report['error']=str(error)
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        report['sources_stable']=stable();report['success']=report['success'] and report['sources_stable']
        write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps({k:report.get(k) for k in ('success','sources_stable','error')}))
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
