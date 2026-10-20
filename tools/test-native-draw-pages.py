#!/usr/bin/env python3
"""Compare unchanged Surface2/3/4 page-hint callers with Wine DirectDraw.

Run in an isolated X display under working-retention.py. This checks owned
system-memory storage, not physical driver residency or whole-game retirement.
"""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess

from native_draw_retirement import require_retired

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--prefix-template', type=Path,
                        help='Clone a completed Wine prefix into this isolated run')
    args = parser.parse_args()
    if not os.environ.get('DISPLAY'):
        parser.error('An isolated X display is required')
    out = args.output.resolve()
    if not out.is_relative_to(ROOT/'working'):
        parser.error('Choose a fresh output below working/')
    out.mkdir(parents=True, exist_ok=False)
    declaration = json.loads(args.claims.read_text())
    sources = declaration['sources']
    sha = lambda p: hashlib.sha256(Path(p).read_bytes()).hexdigest()
    stable = lambda: all(sha(ROOT/p) == h for p,h in sources.items())
    report = dict(schema=1, success=False, scope=__doc__, sources=sources,
                  claims=declaration['claims'], original_game_executed=False,
                  complete_retirement=False)
    env = {k:v for k,v in os.environ.items() if not k.startswith(('MNM_', 'WINE'))}
    env.update(WINEPREFIX=str(out/'prefix'), WINEDEBUG='-all', LIBGL_ALWAYS_SOFTWARE='1')
    def run(label, command, extra=None, cwd=out):
        with (out/(label+'.log')).open('x') as log:
            result = subprocess.run([str(x) for x in command], cwd=cwd,
                                    env=env|dict(extra or {}), stdout=log,
                                    stderr=subprocess.STDOUT, timeout=120)
        if result.returncode:
            raise RuntimeError(label+': exit '+str(result.returncode))
    verified = False
    try:
        if not stable():raise ValueError('Prospective page-hint sources changed')
        run('original-before', [ROOT/'tools/original-manifest.sh','verify']);verified=True
        spec = importlib.util.spec_from_file_location('native_pages_builder',ROOT/'tools/build-native-draw.py')
        builder = importlib.util.module_from_spec(spec);spec.loader.exec_module(builder)
        imports = {k:dict(v) for k,v in builder.IMPORTS.items()}
        imports['kernel32'].update(LoadLibraryA=4,GetProcAddress=8)
        libraries=[]
        for name,entries in imports.items():
            definition=out/(name+'.def')
            definition.write_text('LIBRARY '+name+'.dll\nEXPORTS\n'+''.join(k+'@'+str(v)+'\n' for k,v in entries.items()))
            library=out/(name+'.lib')
            run('imports-'+name,['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',library]);libraries.append(library)
        run('compile',[*builder.FLAGS,'-MD','-MF',out/'probe.d','-c',ROOT/'tests/native-draw-page-probe.c','-o',out/'probe.obj'])
        run('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no',
                    '/timestamp:0','/out:'+str(out/'probe.exe'),out/'probe.obj',*libraries])
        dll=builder.build(out/'native-build',sources)
        if args.prefix_template:
            template=args.prefix_template.resolve()
            if (not (template/'system.reg').is_file()
                    or not (template/'drive_c').is_dir() or template == out/'prefix'):
                raise ValueError('Require a separate completed Wine prefix template')
            run('prefix',['cp','-a','--reflink=auto',template,out/'prefix'])
        else:
            run('boot',['wineboot','-i'],
                dict(WINEDLLOVERRIDES='mscoree,mshtml=;winemenubuilder.exe=d'))
        driver=out/'prefix/drive_c/windows/syswow64/ddraw.dll'
        inputs={str(dll):sha(dll),str(out/'probe.exe'):sha(out/'probe.exe')}
        if driver.exists():inputs[str(driver)]=sha(driver)
        report['inputs']=inputs
        # LoadLibrary searches the probe's directory, so bind its native name to
        # the exact freshly built DLL, with the binary fingerprint retained.
        import shutil
        shutil.copyfile(dll,out/'MnmDraw.dll')
        inputs[str(out/'MnmDraw.dll')]=sha(out/'MnmDraw.dll')
        for name,extra in [('original',dict(WINEDLLOVERRIDES='ddraw=b')),
                           ('native',dict(WINEDLLOVERRIDES='ddraw=',MNM_FACTORY_NATIVE='1',
                              MNM_NATIVE_DRAW_RETIREMENT='Z:'+str(out/'retirement.bin').replace('/','\\')))]:
            case=out/name;case.mkdir();run(name,['wine',out/'probe.exe'],extra,case)
        original=(out/'original/page-results.bin').read_bytes()
        native=(out/'native/page-results.bin').read_bytes()
        if len(original)!=30*149*4 or len(native)!=len(original):raise ValueError('Incomplete page-hint corpus')
        a=struct.unpack('<4470I',original);b=struct.unpack('<4470I',native)
        differences=[i for i,(x,y) in enumerate(zip(a,b)) if x!=y]
        report.update(cases=30,interfaces=['Surface2','Surface3','Surface4'],page_calls=180,
                      compared_caller_fields=480,pixel_words=3510,mismatch_words=differences,
                      original_sha256=sha(out/'original/page-results.bin'),native_sha256=sha(out/'native/page-results.bin'))
        if differences:raise ValueError('Page-hint caller/pixel mismatch: '+str(differences[:20]))
        report['retirement']=require_retired(out/'retirement.bin')
        report['build']=json.loads((out/'native-build/manifest.json').read_text())
        closure={}
        for token in (out/'probe.d').read_text().replace('\\\n',' ').split():
            p=Path(token).resolve()
            if p.is_file() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                closure[str(p.relative_to(ROOT))]=sha(p)
        if any(sources.get(p)!=h for p,h in closure.items()):raise ValueError('Undeclared probe compiler input')
        report['compiled_sources']=closure
        report['native_ddraw_disabled']=report['native_ddraw_module_absent']=True
        report['success']=all(sha(p)==h for p,h in inputs.items())
    except Exception as error:report['error']=str(error)
    finally:
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10)
        if verified:
            try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_verified_after']=True
            except Exception as error:report.update(success=False,original_verified_after=False,error=str(error))
        report['sources_stable']=stable();report['success'] &= report['sources_stable']
        with (out/'report.json').open('x') as file:json.dump(report,file,indent=2);file.write('\n')
    print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
