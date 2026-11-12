#!/usr/bin/env python3
"""Original palette tables to owned chains/profile/resource/software-GL pixels."""
import argparse
import hashlib
import io
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys
import tarfile

from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import read_bytes, write_report

ROOT = Path(__file__).resolve().parents[1]
PIN = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
IDS = ['RE.palette-effect-builders', 'RE.palette-effect-modes',
       'RE.palette-effect-chains', 'NR.scene-palette-effects']
SCENARIO = 'PALETTE.platform-delivery-20261026'


def sha(path):
    return hashlib.sha256(read_bytes(path, root=ROOT)).hexdigest()


def atlas(rgb):
    """Authored fully opaque atlas: each source palette has all 256 indices."""
    count = len(rgb) // 768
    frame_size = 40 + 16 * 8 + 32 + 256
    data = bytearray(struct.pack('<6I', 0x00525053, 0, 4, count, count, 1))
    data.extend(rgb)
    data.extend(struct.pack('<'+'I'*count, *(p*frame_size for p in range(count))))
    for p in range(count):
        frame = bytearray(struct.pack('<10I', frame_size, 16, 16, 0, 0, 0, 0, p, 0, 0))
        for y in range(16):
            frame.extend(struct.pack('<2I', 168+y*2, 200+y*16))
        frame.extend(bytes([0, 16])*16)
        frame.extend(bytes(range(256)))
        data.extend(frame)
    struct.pack_into('<I', data, 4, len(data))
    return data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--installation', type=Path, required=True)
    parser.add_argument('--prefix-template', type=Path, required=True)
    parser.add_argument('--reuse-build-root', type=Path)
    args = parser.parse_args()
    out, destination = args.output.resolve(), args.report.absolute()
    if out.exists() or destination.exists() or not out.is_relative_to(ROOT/'working/tests'):
        parser.error('Require new owned output and report identities')
    out.mkdir(parents=True)
    (out/'.retain').write_text('Palette closure: raw inputs, platform records, original/native binaries and failed/successful executions.\n')
    (out/'tmp').mkdir()
    env = dict(os.environ, TMPDIR=str(out/'tmp'), LP_NUM_THREADS='2',
               ASAN_OPTIONS='detect_leaks=0:halt_on_error=1',
               UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1',
               QT_QPA_PLATFORM='mnm-headless-egl', LIBGL_ALWAYS_SOFTWARE='1',
               MNM_PROFILE_EGL_VENDOR='Mesa', QT_FORCE_STDERR_LOGGING='1')
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
    bs = {b['id']: b for b in register['behaviors']}
    builds = {b['id']: b for b in register['builds']}
    scenario = next(s for s in register['scenarios'] if s['id'] == SCENARIO)
    paths = set().union(*(required_sources(bs[i]) for i in IDS))
    # Pin the transitive source domain before any compilation. Verify actual
    # project compiler dependencies against it below rather than inferring them.
    for base in ['assets', 'renderer', 'reconstruction', 'apps/scene-session', 'compat', 'game', 'protocols']:
        paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT/base).rglob('*')
                     if p.is_file() and (p.suffix in {'.h', '.hpp', '.c', '.cpp', '.inc', '.S'} or p.name == 'CMakeLists.txt'))
    paths.update(['tests/resource-fixtures.hpp', 'tests/palette-platform-reference.cpp',
                  'tests/palette-platform-scene.cpp', 'tests/palette-profile-platform.c',
                  'tools/test-palette-effect-closure.py', 'tools/coverage_claims.py',
                  'tools/coverage_evidence.py', 'tools/experiment_environment.py',
                  'tools/original-manifest.sh'])
    # The included PE fixture has a deliberately separate original main.
    paths.update(['tests/palette-effects-reference.cpp', 'tests/sprite-binary-reference.cpp'])
    sources = {p: sha(ROOT/p) for p in sorted(paths)}
    report = dict(schema=1, success=False, sources=sources, inputs={}, commands=[],
                  claims=[dict(behavior=i, contract_sha256=behavior_contract(bs[i], builds),
                               scenarios={SCENARIO: scenario_contract(scenario)}) for i in IDS],
                  original_sha256=PIN, original_output_used_as_native_input=False,
                  platform='Actual Wine PE32 GetCurrentDirectoryA/GetPrivateProfileStringA results; original callers replay the exact external-service transcript.',
                  scope='All ten builders and sequential source mutations, RGB565/RGB555 tables, real profile file/default/truncation paths and owned RGB565 resource/GL delivery on installed-derived palettes. Controlled original allocator failures retain their separate provider matrix. No Windows SEH injection, physical driver/hardware, original unsafe allocation emulation or whole-game/live replacement claim.',
                  cohorts=[], live_replacement=False, physical_driver_equivalence=False)
    prospective = dict(schema=1, sources=sources, claims=report['claims'],
                       source_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
                       changed_sources={p:(ROOT/p).read_text() for p in paths if p in {
                           'tools/test-palette-effect-closure.py','tests/palette-platform-reference.cpp',
                           'tests/palette-platform-scene.cpp','tests/palette-profile-platform.c'}})
    write_report(ROOT, destination.with_name(destination.stem+'-inputs.json'),
                 (json.dumps(prospective, indent=2)+'\n').encode())
    prefix = out/'wine-prefix'
    verified = False

    def run(label, command, *, cwd=ROOT, extra=None, timeout=900, accepted_codes=(0,)):
        command = [str(v) for v in command]
        print(label, flush=True)
        result = subprocess.run(command, cwd=cwd, env=dict(env, **(extra or {})),
                                capture_output=True, timeout=timeout)
        log = result.stdout+b'\n'+result.stderr
        (out/(label+'.log')).write_bytes(log)
        logpath = destination.parent/(destination.stem+'-'+label+'.log')
        write_report(ROOT, logpath, log)
        report['commands'].append(dict(label=label, argv=command, exit_code=result.returncode,
                                       log=logpath.relative_to(ROOT).as_posix(), sha256=hashlib.sha256(log).hexdigest()))
        if result.returncode not in accepted_codes:
            result.check_returncode()
        return result.stdout.decode()

    def retain(label, path):
        target=destination.parent/(destination.stem+'-'+label+'.bin')
        payload=path.read_bytes()
        write_report(ROOT,target,payload)
        report.setdefault('retained_artifacts',[]).append(dict(path=target.relative_to(ROOT).as_posix(),sha256=hashlib.sha256(payload).hexdigest(),bytes=len(payload)))

    def checkpoint(label):
        # A reserve stop/SIGKILL cannot run finally. Retain each completed
        # original/GL cohort and its exact binary/input identity immediately.
        target=destination.parent/(destination.stem+'-checkpoint-'+label+'.json')
        write_report(ROOT,target,(json.dumps(report,indent=2)+'\n').encode())

    try:
        run('manifest-before', [ROOT/'tools/original-manifest.sh','verify']); verified = True
        executable = ROOT/'original/Arcane_Nocd/Chaos.exe'
        if hashlib.sha256(executable.read_bytes()).hexdigest() != PIN:
            raise ValueError('Original build mismatch')
        installation = args.installation.resolve()
        rgb = bytearray()
        for name in ['Creatures/Wizard1.spr','Creatures/Crocodile.spr','Creatures/Brownie.spr']:
            data = (installation/name).read_bytes()
            if struct.unpack_from('<I',data,8)[0] != 4 or struct.unpack_from('<I',data,16)[0] < 1:
                raise ValueError('Require installed v4 indexed SPR '+name)
            report['inputs'][str(installation/name)] = hashlib.sha256(data).hexdigest()
            rgb.extend(data[24:24+768])
        (out/'rgb.bin').write_bytes(rgb)
        report['installed_palette_sources'] = 3
        imports = {'GetCurrentDirectoryA':8, 'GetPrivateProfileStringA':24,
                   'CreateFileA':28, 'WriteFile':20, 'CloseHandle':4, 'ExitProcess':4}
        definition = out/'kernel32.def'
        definition.write_text('LIBRARY KERNEL32.dll\nEXPORTS\n'+''.join(f'{k}@{v}\n' for k,v in imports.items()))
        run('platform-imports',['llvm-dlltool','-m','i386','--kill-at','-d',definition,'-l',out/'kernel32.lib'])
        run('platform-compile',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-Wall','-Wextra','-Werror','-c',ROOT/'tests/palette-profile-platform.c','-o',out/'platform.obj'])
        run('platform-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0',f'/out:{out/"platform.exe"}',out/'platform.obj',out/'kernel32.lib'])
        from experiment_environment import _busy
        if _busy(args.prefix_template.resolve()):
            raise ValueError('Require a stopped, idle Wine-prefix template')
        run('prefix-copy',['cp','-a','--reflink=auto',args.prefix_template.resolve(),prefix])
        wine = dict(WINEPREFIX=str(prefix),WINEDEBUG='-all',WINEDLLOVERRIDES='mscoree,mshtml=')
        report['wine_version'] = run('wine-version',['wine','--version'],extra=wine).strip()
        run('oracle-build',['g++','-m32','-std=c++17','-O2','-g','-DORIGINAL_REFERENCE','-fno-pie','-no-pie','-I'+str(ROOT/'reconstruction/rendering'),ROOT/'tests/palette-platform-reference.cpp',ROOT/'reconstruction/rendering/palette_effects.cpp',ROOT/'reconstruction/rendering/palette_shading.cpp',ROOT/'reconstruction/rendering/terrain_selection.cpp','-o',out/'original-oracle'])
        retain('original-oracle',out/'original-oracle');retain('platform-executable',out/'platform.exe');retain('rgb-input',out/'rgb.bin')
        cfg = (installation/'CFG/tables.cfg').read_bytes()
        report['inputs'][str(installation/'CFG/tables.cfg')] = hashlib.sha256(cfg).hexdigest()
        cases = {
            'installed': cfg,
            'missing-file': None,
            'empty-file': b'',
            'partial-quoted': b'[STONE_EFFECT]\r\nReducer=" +.25tail "\r\nSpecalValue=1\r\nDarkest=9\r\n[METAL_EFFECT]\r\nContrast=200\r\nBlueTint=-3\r\n[REDTINT_EFFECT]\r\nReducer=+.5suffix\r\n',
            'empty-nonnumeric': b'[STONE_EFFECT]\nReducer=\nSpecalValue=1\n[METAL_EFFECT]\nContrast=nonnumeric\n[REDTINT_EFFECT]\nReducer=0\n',
            'truncated-value': b'[REDTINT_EFFECT]\nReducer=0.25'+b'x'*300+b'\n',
        }
        for name, data in cases.items():
            case = out/name;case.mkdir();(case/'cfg').mkdir()
            if data is not None:(case/'cfg/tables.cfg').write_bytes(data)
            (case/'atlas.spr').write_bytes(atlas(rgb))
            run(name+'-platform',['wine',out/'platform.exe'],cwd=case,extra=wine,timeout=120)
            transcript=(case/'profile.bin').read_bytes()
            if len(transcript)!=2600:raise ValueError('Incomplete platform result')
            oracle=json.loads(run(name+'-original',[out/'original-oracle',executable,case/'profile.bin',out/'rgb.bin',case/'oracle.bin']))
            report['cohorts'].append(dict(name=name, original=oracle, platform_sha256=hashlib.sha256(transcript).hexdigest(),native={}))
            retain(name+'-profile',case/'profile.bin');retain(name+'-oracle',case/'oracle.bin');retain(name+'-atlas',case/'atlas.spr')
            if data is not None:retain(name+'-configuration',case/'cfg/tables.cfg')
            checkpoint('original-'+name)
        for mode, flags in [('release',['-DCMAKE_BUILD_TYPE=Release']),
                            ('sanitized',['-DCMAKE_BUILD_TYPE=Debug','-DCMAKE_CXX_FLAGS=-O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -fno-pie','-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
            build=(args.reuse_build_root.resolve() if args.reuse_build_root else out)/(mode+'-build')
            run(mode+'-configure',['cmake','-S',ROOT/'apps/scene-session','-B',build,*flags,'-DCMAKE_EXPORT_COMPILE_COMMANDS=ON'])
            run(mode+'-build',['cmake','--build',build,'--target','palette-platform-scene','scene-palette-test','-j2'])
            retain(mode+'-native-executable',build/'palette-platform-scene');retain(mode+'-platform-plugin',build/'platforms/libmnm_scene_palette_headless.so')
            gl=dict(QT_QPA_PLATFORM_PLUGIN_PATH=str(build/'platforms'))
            for cohort in report['cohorts']:
                case=out/cohort['name']
                result=json.loads(run(mode+'-'+cohort['name'],[build/'palette-platform-scene',case,case/'oracle.bin'],extra=gl))
                if not result['success'] or result['modes']!=10 or result['remaining_surfaces']:
                    raise ValueError('Incomplete native palette delivery')
                cohort['native'][mode]=result
                checkpoint(mode+'-'+cohort['name'])
            run(mode+'-existing-scene',['ctest','--test-dir',build,'--output-on-failure','-R','^native-scene-palette-effects$'],extra=gl)
            linked=out/'linked'/mode;linked.mkdir(parents=True)
            for p in [build/'palette-platform-scene',build/'scene-palette-test',build/'platforms/libmnm_scene_palette_headless.so']:
                shutil.copy2(p,linked/p.name)
            # Verify the actual compiler dependency closure, not just the target
            # list. Generated build files and system headers are not source.
            deps=set()
            for p in build.rglob('*.o.d'):
                for token in p.read_text().replace('\\\n',' ').split()[1:]:
                    q=Path(token).resolve()
                    if q.is_file() and q.is_relative_to(ROOT) and not q.is_relative_to(ROOT/'working'):
                        deps.add(q.relative_to(ROOT).as_posix())
            if deps-set(sources):raise ValueError('Undeclared compiler inputs '+str(sorted(deps-set(sources))))
            report[mode+'_compiler_inputs']=sorted(deps)
        report['success']=True
    except Exception as error:
        report['error']=repr(error)
    finally:
        if prefix.exists():
            try:
                # -k also returns1 when its server has already retired. Accept
                # that result only with a successful wait and an independent
                # host-visible prefix/process check; no surviving owner passes.
                run('wine-retirement',['wineserver','-k'],extra=dict(WINEPREFIX=str(prefix)),timeout=30,accepted_codes=(0,1))
                run('wine-wait',['wineserver','-w'],extra=dict(WINEPREFIX=str(prefix)),timeout=30)
                from experiment_environment import _busy
                if _busy(prefix):raise ValueError('Private Wine prefix still has a live consumer')
                report['private_wine_retired']=True
            except Exception as error:report['retirement_error']=repr(error);report['success']=False
        if verified:
            try:run('manifest-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
            except Exception as error:report['manifest_error']=repr(error);report['success']=False
        report['sources_stable']=all(sha(ROOT/p)==h for p,h in sources.items())
        report['inputs_stable']=all(hashlib.sha256(Path(p).read_bytes()).hexdigest()==h for p,h in report['inputs'].items())
        report['success']=bool(report['success'] and report['sources_stable'] and report['inputs_stable'])
        # Persist unique inputs, failed outputs, binaries and source declaration
        # before any disposable execution storage can be removed. Build object
        # files are rebuildable; linked executables/plugins are retained.
        capsule=io.BytesIO()
        with tarfile.open(fileobj=capsule,mode='w:gz') as archive:
            for p in sorted(out.rglob('*')):
                if not p.is_file() or p.is_symlink() or 'CMakeFiles' in p.parts or p.suffix in {'.o','.a'}:continue
                if any(part.endswith('-build') for part in p.parts) and not (p.suffix=='.so' or p.name in {'palette-platform-scene','scene-palette-test'}):continue
                archive.add(p,arcname=p.relative_to(out).as_posix(),recursive=False)
        raw=destination.with_name(destination.stem+'-raw.tar.gz');report['raw_capsule_sha256']=write_report(ROOT,raw,capsule.getvalue())
        report['raw_capsule']=raw.relative_to(ROOT).as_posix()
        write_report(ROOT,destination,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps({k:report.get(k) for k in ['success','error','sources_stable','inputs_stable']}),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
