#!/usr/bin/env python3
"""Execute ten bounded native admission/lifetime contracts with fresh claims."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import struct
import subprocess
import sys
from drawing_fixture_prefix import prepare

ROOT = Path(__file__).resolve().parents[1]
BEHAVIORS = ['NP.owned-plane-lookup', 'NP.native-source-copy-refusal',
    'NR.directdraw-import-retirement', 'NP.dc-copy-hook-retirement',
    'NR.world-kind-diagnostic', 'NP.owned-dc-copy', 'NP.owned-dib-paint',
    'NP.colour-rectangle-admission', 'NP.held-lease-flip',
    'NP.native-primary-initial-history']
SCENARIO = 'NR.third-ten.admission-lifetimes-20261021'
RUNNERS = ['tools/test-native-draw-import.py', 'tools/test-dc-copy-retirement.py',
    'tools/test-world-refusal-diagnostics.py', 'tools/test-native-dc-copy.py',
    'tools/test-native-dib-pixels.py', 'tools/test-colour-rect-splice.py']


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT / path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--wine-prefix', type=Path, required=True)
    parser.add_argument('--build-dir', type=Path, required=True,
                        help='Owned incremental CMake fixture tree; always reconfigure/rebuild')
    args = parser.parse_args()
    out = args.output.resolve()
    build = args.build_dir.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests') or not build.is_relative_to(ROOT/'working/tests'):
        parser.error('Use a new working/tests output and an owned working/tests build')
    out.mkdir(parents=True)
    (out/'.retain').write_text('Fresh ten admission/lifetime controls and failed attempts.\n')
    (out/'tmp').mkdir()
    (out/'runtime').mkdir(mode=0o700)
    env = {k:v for k,v in os.environ.items() if not k.startswith(('MNM_', 'WINE'))}
    env.update(TMPDIR=str(out/'tmp'), XDG_RUNTIME_DIR=str(out/'runtime'),
        WINEPREFIX=str(args.wine_prefix.resolve()), WINEDEBUG='-all',
        QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', GALLIUM_DRIVER='llvmpipe', LP_NUM_THREADS='4',
        __GLX_VENDOR_LIBRARY_NAME='mesa', __EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json',
        ASAN_OPTIONS='detect_leaks=1:halt_on_error=1', UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    for key in ('MESA_LOADER_DRIVER_OVERRIDE', 'QT_XCB_GL_INTEGRATION'):
        env.pop(key, None)
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    declaration = load('third_claims', 'tools/draft-coverage-claims.py').declaration(ROOT, register, BEHAVIORS, [SCENARIO])
    # Python report reads resolve SQLite payloads; bind the actual reader too.
    declaration['sources']['tools/coverage_evidence.py'] = sha(ROOT / 'tools/coverage_evidence.py')
    candidates = dict(declaration['sources'])
    for directory in ('apps','assets','audio','compat','reconstruction','renderer','runtime','protocols','tests','tools'):
        for p in (ROOT/directory).rglob('*'):
            if p.is_file() and (p.suffix in {'.c','.cpp','.h','.hpp','.inc','.S','.cmake','.py','.json'} or p.name=='CMakeLists.txt'):
                candidates.setdefault(p.relative_to(ROOT).as_posix(), sha(p))
    # Pin quoted compiler includes and dynamically loaded build dependencies
    # prospectively. Actual compiler dependency files are checked again below.
    extra = [*RUNNERS, 'tools/test-rendering-third-ten.py', 'tools/drawing_fixture_prefix.py',
        'tools/build-native-draw.py', 'tools/build-dc-copy.py', 'tools/build-colour-rect.py',
        'tools/build-shadow-bridge.py', 'tools/prepare-shadow-experiment.py', 'tools/prepare-native-draw.py',
        'tools/original-manifest.sh', 'research/original-files.tsv',
        'tools/world_channel.py', 'tools/inspect-startup-queues.py', 'tools/world_refusal.py',
        'tools/prepare-scene-observer.py', 'tools/prepare-menu-observer.py',
        'tools/draft-coverage-claims.py', 'tools/coverage_claims.py',
        'tests/native-held-flip-probe.c', 'tests/native-owned-initial-producer-test.c',
        'tests/native-world-rolling/CMakeLists.txt']
    todo = list(declaration['sources']) + extra
    visited = set()
    while todo:
        name = todo.pop()
        if name in visited:
            continue
        visited.add(name)
        p = ROOT/name
        if not p.is_file():
            raise ValueError('Missing prospective input: '+name)
        declaration['sources'][name] = sha(p)
        if p.suffix in {'.c','.cpp','.h','.hpp','.S','.inc'}:
            for include in re.findall(r'^\s*#\s*include\s*"([^"]+)"', p.read_text(), re.M):
                child = (p.parent/include).resolve()
                if child.is_relative_to(ROOT) and child.is_file():
                    todo.append(child.relative_to(ROOT).as_posix())
    candidates.update(declaration['sources'])
    claims = out/'claims.json'
    claims.write_text(json.dumps(declaration,indent=2)+'\n')
    report = dict(schema=1, success=False, claims=declaration['claims'], sources=dict(declaration['sources']),
        controls={}, commands=[], inputs={}, scope=__doc__, original_game_executed=False,
        original_code_modified=False, original_pixels_used_as_native_inputs=False,
        live_replacement=False, performance_acceptance=False,
        reviewed_head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip())

    def run(label, command, *, timeout=240, cwd=ROOT, extra=None):
        command = [str(x) for x in command]
        print(label, flush=True)
        with (out/(label+'.log')).open('x') as log:
            result = subprocess.run(command,cwd=cwd,env={**env,**(extra or {})},stdout=log,stderr=log,timeout=timeout)
        report['commands'].append(dict(label=label,argv=command,exit_code=result.returncode,
            log_sha256=sha(out/(label+'.log'))))
        if result.returncode:
            raise RuntimeError(label+': exit '+str(result.returncode))
        return (out/(label+'.log')).read_text()

    def pin(path):
        name = path.relative_to(ROOT).as_posix()
        if name not in candidates or sha(path)!=candidates[name]:
            raise ValueError('Changed/unpinned compiler input: '+name)
        report['sources'][name] = candidates[name]

    def dependencies(path):
        for token in shlex.split(path.read_text().replace('\\\n',' ').split(':',1)[1]):
            p = Path(token).resolve()
            if p.is_file() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                pin(p)

    prefix_lock = None
    verified = False
    try:
        prefix,prefix_lock = prepare(ROOT,out,args.wine_prefix,args.wine_prefix)
        report['prefix'] = dict(path=str(prefix),exclusively_locked=True,idle_before_use=True)
        run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
        verified = True
        pe = ROOT/'original/Arcane_Nocd/Chaos.exe'
        report['original_pe_sha256'] = sha(pe)
        if report['original_pe_sha256']!='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168':
            raise ValueError('Original executable changed')
        for runner in RUNNERS:
            label = Path(runner).stem
            command = [sys.executable,'-B',ROOT/runner,'--claims',claims]
            if label in {'test-dc-copy-retirement','test-native-dc-copy','test-native-dib-pixels'}:
                command += ['--wine-prefix',prefix]
            text = run(label,command,timeout=600)
            results = [Path(line) for line in text.splitlines() if line.endswith('/report.json')]
            if not results:
                raise ValueError('Missing child report: '+label)
            child = results[-1]
            data = json.loads(child.read_text())
            if not data['success']:
                raise ValueError('Failed child result: '+label)
            (child.parent/'.retain').write_text('Referenced by '+str(out.relative_to(ROOT))+'; preserve exact fixture.\n')
            for name,digest in data['sources'].items():
                if sha(ROOT/name)!=digest:
                    raise ValueError('Child source drift: '+name)
                report['sources'][name] = digest
            report['controls'][label] = dict(path=str(child.relative_to(ROOT)),sha256=sha(child),result=data)
            for dep in child.parent.rglob('*.d'):
                dependencies(dep)
            for manifest in child.parent.rglob('manifest.json'):
                data = json.loads(manifest.read_text())
                for name in data.get('sources',{}):
                    pin(ROOT/name)
        for source in ['native-primary-initial-test','native-owned-initial-producer-test']:
            for mode,flags in [('optimized',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
                exe = out/(source+'-'+mode)
                run(exe.name+'-build',['clang','-std=c11','-Wall','-Wextra','-Werror','-UNDEBUG',*flags,
                    '-Wno-int-to-void-pointer-cast','-MD','-MF',exe.with_suffix('.d'),ROOT/('tests/'+source+'.c'),'-o',exe])
                dependencies(exe.with_suffix('.d'))
                report['controls'][exe.name] = dict(success=True,output=run(exe.name,[exe]),binary_sha256=sha(exe))
        run('configure',['cmake','-S',ROOT/'tests/native-world-rolling','-B',build,'-DCMAKE_BUILD_TYPE=Release','-DBUILD_TESTING=ON'])
        run('build',['cmake','--build',build,'--parallel','4','--target','resident-canvas-transition-test'],timeout=1200)
        for dep in build.rglob('*.o.d'):
            dependencies(dep)
        for token in re.findall(r'"([^"\n]+)"',(build/'CMakeFiles/Makefile.cmake').read_text()):
            p = Path(token).resolve()
            if p.is_file() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                pin(p)
        exe = next(p for p in build.rglob('resident-canvas-transition-test') if p.is_file())
        text = run('resident-canvas-transition',['xvfb-run','-a',exe])
        report['controls']['resident-canvas-transition'] = dict(success=True,binary_sha256=sha(exe),result=json.loads(text.splitlines()[-1]))
        # Compile the same caller for independent real Wine and native facade
        # processes, without the completion runner's unrelated prebuilt binaries.
        builder = load('third_draw_build','tools/build-native-draw.py')
        dll = builder.build(out/'flip-build',declaration['sources'])
        shutil.copyfile(dll,out/'MnmDraw.dll')
        for name in json.loads((out/'flip-build/manifest.json').read_text())['sources']:
            pin(ROOT/name)
        libraries = []
        imports = {'kernel32':{'LoadLibraryA':4,'GetProcAddress':8,'CreateFileA':28,'WriteFile':20,'CloseHandle':4,'ExitProcess':4,'GetEnvironmentVariableA':12,'Sleep':4},'user32':{'GetDesktopWindow':0}}
        for name,symbols in imports.items():
            definition = out/(name+'.def')
            definition.write_text('LIBRARY '+name+'.dll\nEXPORTS\n'+''.join(f'{n}@{s}\n' for n,s in symbols.items()))
            library = out/(name+'.lib')
            run(name+'-imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',library])
            libraries.append(library)
        run('flip-compile',[*builder.FLAGS,'-MD','-MF',out/'flip.d','-c',ROOT/'tests/native-held-flip-probe.c','-o',out/'flip.obj'])
        dependencies(out/'flip.d')
        run('flip-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/'flip.exe'),out/'flip.obj',*libraries])
        for label,extra in [('driver',{'WINEDLLOVERRIDES':'ddraw=b'}),('native',{'MNM_FACTORY_NATIVE':'1','WINEDLLOVERRIDES':'ddraw='})]:
            dest = out/label
            dest.mkdir()
            shutil.copyfile(dll,dest/'MnmDraw.dll')
            run('flip-'+label,['xvfb-run','-a','wine',out/'flip.exe'],cwd=dest,extra=extra)
            run('flip-'+label+'-stopped',['wineserver','-w'])
        a = (out/'driver/held-flips.bin').read_bytes()
        b = (out/'native/held-flips.bin').read_bytes()
        if len(a)!=4*12*4 or a!=b:
            raise ValueError('Held-flip comparison mismatch')
        driver = prefix/'drive_c/windows/syswow64/ddraw.dll'
        report['inputs'][str(driver)] = sha(driver)
        report['controls']['held-flip'] = dict(success=True,cases=4,driver_sha256=sha(driver),
            driver_output_sha256=sha(out/'driver/held-flips.bin'),native_output_sha256=sha(out/'native/held-flips.bin'),
            rows=[list(struct.unpack_from('<12I',a,i*48)) for i in range(4)],wine_version=subprocess.check_output(['wine','--version'],text=True).strip())
        report['sources_stable'] = all(sha(ROOT/p)==h for p,h in report['sources'].items())
        if not report['sources_stable']:
            raise ValueError('Sources changed during execution')
        report['success'] = True
    except Exception as error:
        report['error'] = repr(error)
    finally:
        if prefix_lock:
            subprocess.run(['wineserver','-k'],env=env,timeout=15,check=False)
            subprocess.run(['wineserver','-w'],env=env,timeout=15,check=False)
            prefix_lock.close()
        if verified:
            try:
                run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
                report['original_manifest_verified_before_after'] = True
            except Exception as error:
                report['success'] = False
                report['manifest_error'] = repr(error)
        report['sources_stable'] = all(sha(ROOT/p)==h for p,h in report['sources'].items())
        report['success'] = report['success'] and report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
