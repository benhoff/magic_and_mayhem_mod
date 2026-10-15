#!/usr/bin/env python3
"""Isolated native clipper/DC and logical primary/flip proof; no game retirement claim."""
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
    parent=ROOT/'working/tests/native-draw-services';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(success=False,sources=claims['sources'],claims=claims['claims'],
      scope='64 unchanged clipper/DC caller cases and complete RGB565/COLORREF outputs against Wine DirectDraw, including DC-during-lock, repeated DC/refusals, GDI writes and teardown. Native ddraw disabled/module absent. Native logical primary/attached flip lifecycle is a separate policy; no driver presentation, clipped blit, palette or real game replacement claim.',
      real_game_directdraw_retired=False)
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(WINEDEBUG='-all',LIBGL_ALWAYS_SOFTWARE='1',WINEPREFIX=str(out/'wineprefix'))
    reservation=None;before=False;inputs={}
    def run(name,command,extra=None,cwd=None,check=True):
        with (out/(name+'.log')).open('x') as log:
            return subprocess.run([str(x) for x in command],cwd=cwd or out,env={**env,**(extra or {})},stdout=log,stderr=log,check=check,timeout=120)
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
        spec=importlib.util.spec_from_file_location('native_factory_reservation',ROOT/'tools/capture-original-surface-keys.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);reservation=module.reserve_wine()
        run('prefix',['cp','-a','--reflink=auto',(ROOT/'working/wineprefix-x86_64').resolve(),out/'wineprefix'])
        run('wineboot',['wineboot','-u'])
        driver=out/'wineprefix/drive_c/windows/syswow64/ddraw.dll'
        if driver.exists():inputs[str(driver)]=sha(driver)
        report['wine_version']=subprocess.check_output(['wine','--version'],text=True).strip()
        imports={'VirtualAlloc':16,'VirtualFree':12,'GetModuleHandleA':4,'VirtualQuery':12,'GetProcessHeap':0,'HeapAlloc':12,'HeapFree':12,'GetLastError':0,'SetLastError':4,'GetEnvironmentVariableA':12,'LoadLibraryA':4,'GetProcAddress':8,'CreateFileA':28,'WriteFile':20,'CloseHandle':4,'ExitProcess':4}
        definition=out/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{v}\n' for n,v in imports.items()))
        run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
        user=out/'user32.def';user.write_text('LIBRARY USER32.dll\nEXPORTS\nGetDesktopWindow@0\n')
        run('user-imports',['llvm-dlltool','-m','i386','--kill-at','-d',user,'-l',out/'user32.lib'])
        gdi=out/'gdi32.def';gdi.write_text('LIBRARY GDI32.dll\nEXPORTS\nGetPixel@12\nSetPixel@16\nGdiFlush@0\n')
        run('gdi-imports',['llvm-dlltool','-m','i386','--kill-at','-d',gdi,'-l',out/'gdi32.lib'])
        flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
        for label,source in [('probe','tests/native-draw-services-probe.c')]:
            run('compile-'+label,[*flags,'-MD','-MF',out/(label+'.d'),'-c',ROOT/source,'-o',out/(label+'.obj')])
        closure=set()
        for dep in out.glob('*.d'):
            for token in dep.read_text().replace('\\\n',' ').split():
                p=Path(token).resolve()
                if p.is_file() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):closure.add(str(p.relative_to(ROOT)))
        if closure-set(claims['sources']):raise ValueError('Undeclared compile inputs: '+repr(sorted(closure-set(claims['sources']))))
        report['compiled_sources']={n:sha(ROOT/n) for n in sorted(closure)}
        spec=importlib.util.spec_from_file_location('native_draw_build',ROOT/'tools/build-native-draw.py')
        builder=importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)
        dll=builder.build(out/'native-build',claims['sources'])
        import shutil
        shutil.copyfile(dll,out/'MnmDraw.dll');dll=out/'MnmDraw.dll'
        report['native_build']=json.loads((out/'native-build/manifest.json').read_text())
        report['compiled_sources'].update(report['native_build']['sources'])
        run('link-probe',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/'probe.exe'),out/'probe.obj',out/'kernel32.lib',out/'user32.lib',out/'gdi32.lib'])
        run('native-imports',['llvm-readobj','--coff-imports',dll])
        if 'ddraw.dll' in (out/'native-imports.log').read_text().lower():raise ValueError('Native factory imports DirectDraw')
        inputs[str(dll)]=sha(dll);inputs[str(out/'probe.exe')]=sha(out/'probe.exe')
        for name,extra in [('original',{'WINEDLLOVERRIDES':'ddraw=b'}),('native',{'WINEDLLOVERRIDES':'ddraw=','MNM_FACTORY_NATIVE':'1','MNM_NATIVE_DRAW_STATUS':'Z:'+str(out/'status.bin').replace('/','\\')})]:
            dest=out/name;dest.mkdir();run(name,['wine',out/'probe.exe'],extra,cwd=dest)
        original=(out/'original/services-results.bin').read_bytes();native=(out/'native/services-results.bin').read_bytes()
        words=32+3*13*9
        if len(original)!=64*words*4 or len(native)!=len(original):raise ValueError('Incomplete surface cases')
        original_words=struct.unpack('<'+str(64*words)+'I',original);native_words=struct.unpack('<'+str(64*words)+'I',native)
        mismatch=[i for i,(a,b) in enumerate(zip(original_words,native_words)) if a!=b]
        report.update(cases=64,caller_words=64*32,pixel_words=64*3*13*9,original_results_sha256=sha(out/'original/services-results.bin'),native_results_sha256=sha(out/'native/services-results.bin'),mismatch_words=mismatch,
          native_ddraw_disabled=True,native_ddraw_module_absent=True,native_directdraw_imports=False,native_remaining_surfaces=max(native_words[i*words+26] for i in range(64)),native_remaining_devices=max(native_words[i*words+27] for i in range(64)))
        if mismatch:raise ValueError('Original/native caller or pixel mismatch: '+repr(mismatch[:30]))
        report['native_remaining_dcs']=max(native_words[i*words+24] for i in range(64))
        report['native_remaining_clippers']=max(native_words[i*words+25] for i in range(64))
        policy=struct.unpack('<4I',(out/'native/services-policy.bin').read_bytes())
        if policy!=(2,351,1,0):raise ValueError('Native refusal/capacity policy incomplete')
        report['native_policy']=dict(flip_surfaces=policy[0],compared_words=policy[1],duplicate_primary_refusals=policy[2],remaining_surfaces=policy[3],policy_sha256=sha(out/'native/services-policy.bin'))
        startup_a=(out/'original/startup-results.bin').read_bytes();startup_b=(out/'native/startup-results.bin').read_bytes()
        if len(startup_a)!=16*16*4 or startup_a!=startup_b:raise ValueError('Startup null-window/primary/Surface3/4 caller mismatch')
        stats=struct.unpack('<24I',(out/'status.bin').read_bytes())
        if stats[:5]!=(0x444d4e4d,0x31305752,2,96,0) or any(stats[5:9]) or not stats[20] or stats[22] or stats[23]:raise ValueError('Invalid module/resource diagnostic teardown')
        report['startup_policy']=dict(original_compared_cases=16,original_compared_words=256,caller_sha256=sha(out/'native/startup-results.bin'),logical_mode_callbacks=3,host_storage_budget_bytes=256*1024*1024,allocation_accounting_bytes=32*9,original_mode_memory_equivalence=False,status_sha256=sha(out/'status.bin'),status_words=list(stats))
        refusals=[]
        for letter,family,kind,slot in [('D',1,1,3),('S',2,1,8),('C',3,0,5)]:
            dest=out/('refusal-'+letter);dest.mkdir()
            result=run('refusal-'+letter,['wine',out/'probe.exe'],{'WINEDLLOVERRIDES':'ddraw=','MNM_FACTORY_NATIVE':'1','MNM_REFUSE':letter},cwd=dest,check=False)
            words=struct.unpack('<8I',(dest/'native-draw-refusal.bin').read_bytes())
            if result.returncode!=96+family or words[:7]!=(0x444d4e4d,0x31464552,1,32,family,kind,slot) or not words[7] or (dest/'services-results.bin').exists():raise ValueError('Unknown method failed to refuse terminally')
            refusals.append(dict(family=family,kind=kind,slot=slot,exit_code=result.returncode,record_sha256=sha(dest/'native-draw-refusal.bin')))
        report['terminal_unknown_refusals']=refusals
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
