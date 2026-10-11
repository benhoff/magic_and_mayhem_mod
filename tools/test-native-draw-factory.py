#!/usr/bin/env python3
"""Isolated factory proof; this does not retire the game's surface backend."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    args=parser.parse_args();claims=json.loads(args.claims.read_text())
    if not os.environ.get('DISPLAY'):parser.error('Use isolated Xvfb')
    for name,digest in claims['sources'].items():
        if sha(ROOT/name)!=digest:raise ValueError('Changed prospective source: '+name)
    parent=ROOT/'working/tests/native-draw-factory';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(success=False,sources=claims['sources'],claims=claims['claims'],
      scope='128 isolated default native factory lifetimes with exact selected HRESULT/identity/per-interface reference/failure outputs versus real Wine DirectDraw. Native preserves incoming LastError as an explicit policy; Wine incidental error values are retained separately, not claimed equivalent. Original null-output creation fault is excluded from the common valid-caller corpus; native safe null-output refusal is a separate policy. Native enumeration is an explicit single-adapter policy. Native probe disables ddraw and verifies no module loads. Surface methods, real game integration, GPU presentation and DirectDraw retirement remain pending.',
      real_game_directdraw_retired=False)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEDEBUG='-all',LIBGL_ALWAYS_SOFTWARE='1',WINEPREFIX=str(out/'wineprefix'))
    reservation=None;before=False;inputs={}
    def run(name,command,extra=None,cwd=None):
        with (out/(name+'.log')).open('x') as log:
            subprocess.run([str(x) for x in command],cwd=cwd or out,env={**env,**(extra or {})},stdout=log,stderr=log,check=True,timeout=120)
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
        spec=importlib.util.spec_from_file_location('native_factory_reservation',ROOT/'tools/capture-original-surface-keys.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);reservation=module.reserve_wine()
        run('prefix',['cp','-a','--reflink=auto',(ROOT/'working/wineprefix-x86_64').resolve(),out/'wineprefix'])
        run('wineboot',['wineboot','-u'])
        driver=out/'wineprefix/drive_c/windows/syswow64/ddraw.dll'
        if driver.exists():inputs[str(driver)]=sha(driver)
        report['wine_version']=subprocess.check_output(['wine','--version'],text=True).strip()
        imports={'GetModuleHandleA':4,'VirtualQuery':12,'GetProcessHeap':0,'HeapAlloc':12,'HeapFree':12,'GetLastError':0,'SetLastError':4,'GetEnvironmentVariableA':12,'LoadLibraryA':4,'GetProcAddress':8,'CreateFileA':28,'WriteFile':20,'CloseHandle':4,'ExitProcess':4}
        definition=out/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{v}\n' for n,v in imports.items()))
        run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
        flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
        for label,source in [('factory','runtime/native-draw/factory.c'),('probe','tests/native-draw-factory-probe.c')]:
            run('compile-'+label,[*flags,'-MD','-MF',out/(label+'.d'),'-c',ROOT/source,'-o',out/(label+'.obj')])
        closure=set()
        for dep in out.glob('*.d'):
            for token in dep.read_text().replace('\\\n',' ').split():
                p=Path(token).resolve()
                if p.is_file() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):closure.add(str(p.relative_to(ROOT)))
        if closure-set(claims['sources']):raise ValueError('Undeclared compile inputs: '+repr(sorted(closure-set(claims['sources']))))
        report['compiled_sources']={n:sha(ROOT/n) for n in sorted(closure)}
        dll=out/'MnmDraw.dll'
        run('link-factory',['lld-link','/dll','/machine:x86','/entry:DllMain@12','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(dll),'/export:NativeDrawCreate=_NativeDrawCreate@12','/export:NativeDrawEnumerateA=_NativeDrawEnumerateA@8','/export:NativeDrawObjects=_NativeDrawObjects@0',out/'factory.obj',out/'kernel32.lib'])
        run('link-probe',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/'probe.exe'),out/'probe.obj',out/'kernel32.lib'])
        run('native-imports',['llvm-readobj','--coff-imports',dll])
        if 'ddraw.dll' in (out/'native-imports.log').read_text().lower():raise ValueError('Native factory imports DirectDraw')
        inputs[str(dll)]=sha(dll);inputs[str(out/'probe.exe')]=sha(out/'probe.exe')
        for name,extra in [('original',{'WINEDLLOVERRIDES':'ddraw=b'}),('native',{'WINEDLLOVERRIDES':'ddraw=','MNM_FACTORY_NATIVE':'1'})]:
            dest=out/name;dest.mkdir();run(name,['wine',out/'probe.exe'],extra,cwd=dest)
        original=(out/'original/factory-results.bin').read_bytes();native=(out/'native/factory-results.bin').read_bytes()
        if len(original)!=128*18*4 or len(native)!=len(original):raise ValueError('Incomplete factory cases')
        original_words=struct.unpack('<2304I',original);native_words=struct.unpack('<2304I',native)
        mismatch=[i for i,(a,b) in enumerate(zip(original_words,native_words)) if i%18!=2 and a!=b]
        if any(native_words[i*18+2]!=0x51 for i in range(128)):raise ValueError('Native LastError preservation policy failed')
        report['last_error_policy']=dict(native='preserve incoming0x51',original_values=sorted(set(original_words[i*18+2] for i in range(128))),independent_last_error_equivalence=False)
        report.update(cases=128,caller_words=2176,compared_columns=[i for i in range(18) if i!=2],original_results_sha256=sha(out/'original/factory-results.bin'),native_results_sha256=sha(out/'native/factory-results.bin'),mismatch_words=mismatch,
          native_ddraw_disabled=True,native_ddraw_module_absent=True,native_directdraw_imports=False,native_remaining_objects=max(struct.unpack_from('<I',native,(i*18+14)*4)[0] for i in range(128)))
        if mismatch:raise ValueError('Original/native caller mismatch: '+repr(mismatch[:30]))
        report['success']=True
    except Exception as error:report['error']=str(error)
    finally:
        if (out/'wineprefix').exists():
            subprocess.run(['wineserver','-k'],env=env,timeout=10,check=False)
            subprocess.run(['wineserver','-w'],env=env,timeout=10,check=False)
        if reservation:reservation.close()
        if before:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in report['sources'].items())
        report['inputs']=inputs;report['inputs_stable']=all(sha(Path(n))==h for n,h in inputs.items())
        report['success']=report['success'] and report['sources_stable'] and report['inputs_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
    return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
