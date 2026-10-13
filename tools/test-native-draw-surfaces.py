#!/usr/bin/env python3
"""Isolated native owned RGB565 surface proof against Wine DirectDraw; no game retirement claim."""
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
    parser.add_argument('--prefix-template',type=Path,default=ROOT/'working/wineprefix-x86_64',help='Read-only stopped Wine prefix to copy into the private experiment')
    args=parser.parse_args();claims=json.loads(args.claims.read_text())
    if not os.environ.get('DISPLAY'):parser.error('Use isolated Xvfb')
    for name,digest in claims['sources'].items():
        if sha(ROOT/name)!=digest:raise ValueError('Changed prospective source: '+name)
    parent=ROOT/'working/tests/native-draw-surfaces';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(success=False,sources=claims['sources'],claims=claims['claims'],
      scope='Four native default-placement 510x180 RGB565 policies on108/124-byte descriptors and two malformed-cap/format refusals, separately from64 unchanged caller cases using DirectDraw1/4 and Surface1/2/4, odd-pitch system-memory RGB565, key/copy/fill/overlap and full native words against real Wine DirectDraw. Native run disables ddraw and checks no module loads; device modes are logical native policies. Primary/flip/DC/palette services, real game integration and GPU history read dependencies remain pending.',
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
        template=args.prefix_template.resolve()
        if not template.is_dir():raise ValueError('Missing prefix template')
        report['prefix_template']=str(template)
        for registry in ('system.reg','user.reg','userdef.reg'):
            path=template/registry
            if path.exists():inputs[str(path)]=sha(path)
        run('prefix',['cp','-a','--reflink=auto',template,out/'wineprefix'])
        run('wineboot',['wineboot','-u'])
        for directory in ('system32','syswow64'):
            driver=out/'wineprefix/drive_c/windows'/directory/'ddraw.dll'
            if driver.exists():inputs[str(driver)]=sha(driver)
        report['wine_version']=subprocess.check_output(['wine','--version'],text=True).strip()
        imports={'VirtualAlloc':16,'VirtualFree':12,'GetModuleHandleA':4,'VirtualQuery':12,'GetProcessHeap':0,'HeapAlloc':12,'HeapFree':12,'GetLastError':0,'SetLastError':4,'GetEnvironmentVariableA':12,'LoadLibraryA':4,'GetProcAddress':8,'CreateFileA':28,'WriteFile':20,'CloseHandle':4,'ExitProcess':4}
        definition=out/'kernel32.def';definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{n}@{v}\n' for n,v in imports.items()))
        run('imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
        user=out/'user32.def';user.write_text('LIBRARY USER32.dll\nEXPORTS\nGetDesktopWindow@0\n')
        run('user-imports',['llvm-dlltool','-m','i386','--kill-at','-d',user,'-l',out/'user32.lib'])
        flags=['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror']
        for label,source in [('probe','tests/native-draw-surface-probe.c')]:
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
        run('link-probe',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/'probe.exe'),out/'probe.obj',out/'kernel32.lib',out/'user32.lib'])
        run('native-imports',['llvm-readobj','--coff-imports',dll])
        if 'ddraw.dll' in (out/'native-imports.log').read_text().lower():raise ValueError('Native factory imports DirectDraw')
        inputs[str(dll)]=sha(dll);inputs[str(out/'probe.exe')]=sha(out/'probe.exe')
        for name,extra in [('original',{'WINEDLLOVERRIDES':'ddraw=b'}),('native',{'WINEDLLOVERRIDES':'ddraw=','MNM_FACTORY_NATIVE':'1'})]:
            dest=out/name;dest.mkdir();run(name,['wine',out/'probe.exe'],extra,cwd=dest)
        original=(out/'original/surface-results.bin').read_bytes();native=(out/'native/surface-results.bin').read_bytes()
        words=32+5*13*9
        if len(original)!=64*words*4 or len(native)!=len(original):raise ValueError('Incomplete surface cases')
        original_words=struct.unpack('<'+str(64*words)+'I',original);native_words=struct.unpack('<'+str(64*words)+'I',native)
        mismatch=[i for i,(a,b) in enumerate(zip(original_words,native_words)) if a!=b]
        report.update(cases=64,caller_words=64*32,pixel_words=64*5*13*9,original_results_sha256=sha(out/'original/surface-results.bin'),native_results_sha256=sha(out/'native/surface-results.bin'),mismatch_words=mismatch,
          native_ddraw_disabled=True,native_ddraw_module_absent=True,native_directdraw_imports=False,native_remaining_surfaces=max(native_words[i*words+29] for i in range(64)),native_remaining_devices=max(native_words[i*words+30] for i in range(64)))
        if mismatch:raise ValueError('Original/native caller or pixel mismatch: '+repr(mismatch[:30]))
        policy=struct.unpack('<7I',(out/'native/surface-policy.bin').read_bytes())
        if policy!=(10,117,512,0,4,367200,4):raise ValueError('Native refusal/capacity/tail-read policy incomplete')
        report['native_policy']=dict(refusals=policy[0],unchanged_words=policy[1],surface_capacity=policy[2],remaining_surfaces=policy[3],default_offscreen_cases=policy[4],default_offscreen_words=policy[5],final_pixel_dword_cases=policy[6],policy_sha256=sha(out/'native/surface-policy.bin'))
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
