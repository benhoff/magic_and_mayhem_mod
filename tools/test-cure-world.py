#!/usr/bin/env python3
"""Validate native World Cure ownership, ANI continuation and installed loading."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--assets', type=Path, default=ROOT/'working/game-nocd')
    parser.add_argument('--claims', type=Path)
    args = parser.parse_args()
    args.output = args.output.resolve()
    args.assets = args.assets.resolve()
    args.output.mkdir(parents=True, exist_ok=False)
    claims = json.loads(args.claims.read_text()) if args.claims else {'sources': {}, 'claims': []}
    assert all(sha(ROOT/p) == value for p,value in claims['sources'].items())
    report = dict(success=False, claims=claims['claims'], source_sha256=claims['sources'],
                  scope='Native World direct positive Cure adapter: real owned ANI cursor drives completion, generation-bound actors/effects, native effect-phase release and checkpoint continuation with embedded selected records.512 authored cases plus installed recipe74 load/playback and separate-process asset-free resume. Native scheduling/follow/cancellation policies; no full original lifecycle, original cadence, live campaign replacement, sprite rendering, projectile or AI-systems composition equivalence.',
                  commands=[], original_lifecycle_compared=False, live_replacement=False)

    def run(name, command, env=None):
        command = list(map(str,command));report['commands'].append(command)
        result = subprocess.run(command,cwd=ROOT,capture_output=True,text=True,env=env,timeout=300)
        (args.output/(name+'.stdout')).write_text(result.stdout)
        (args.output/(name+'.stderr')).write_text(result.stderr)
        if result.returncode or 'runtime error:' in result.stderr or 'ERROR: AddressSanitizer' in result.stderr:
            raise RuntimeError(f'{name}: {result.returncode}: {result.stdout[-2000:]}{result.stderr[-2000:]}')
        return result.stdout

    run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
    try:
        cmake = ROOT/'apps/cure-world/CMakeLists.txt'
        definitions = dict(re.findall(r'add_library\((mnm-cure-world(?:-assets)?) STATIC (.*?)\)',cmake.read_text(),re.S))
        sources = {key:[(cmake.parent/p).resolve().relative_to(ROOT).as_posix() for p in value.split()]
                   for key,value in definitions.items()}
        core = ['tests/cure-world-test.cpp',*sources['mnm-cure-world']]
        all_sources = [*core,*sources['mnm-cure-world-assets'],'apps/cure-world/main.cpp']
        flags = ['g++','-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-Igame','-Iaudio','-Iassets','-Ireconstruction/animation']
        qt_flags = shlex.split(subprocess.check_output(['pkg-config','--cflags','Qt6Core'],text=True))
        dependencies = run('dependencies',flags+qt_flags+['-MM',*all_sources])
        paths = {'tools/test-cure-world.py','apps/cure-world/CMakeLists.txt',*all_sources}
        for token in shlex.split(dependencies.replace('\\\n',' ')):
            p=Path(token)
            if not p.is_absolute() and p.is_file():paths.add(p.resolve().relative_to(ROOT).as_posix())
        report['compiler_warning_boundary']='GCC16 maybe-uninitialized is nonfatal only for unchanged game/simulation/movement.cpp; all new sources retain strict -Werror.'
        report['build_inputs_sha256']={p:sha(ROOT/p) for p in sorted(paths)}
        if args.claims and paths-claims['sources'].keys():raise ValueError('Unbound compiler inputs: '+str(sorted(paths-claims['sources'].keys())))
        inputs = [args.assets/'CFG/Encrypted/effectani.cfg']
        inputs += sorted(p for p in (args.assets/'Sprites').iterdir() if re.fullmatch(r'effects\d+\.ani',p.name,re.I))
        if len(inputs)!=14:raise ValueError('Expected complete installed13-file effect catalog')
        report['installed_inputs_sha256']={p.relative_to(args.assets).as_posix():sha(p) for p in inputs}
        results=[]
        for name,options in [('native',['-O2']),('native32',['-O2','-m32']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'])]:
            objects=[]
            for i,source in enumerate(core):
                obj=args.output/f'{name}-{i}.o'
                diagnostic=['-Wno-error=maybe-uninitialized'] if source=='game/simulation/movement.cpp' else []
                run(f'build-{name}-{i}',flags+options+diagnostic+['-c',source,'-o',obj])
                objects.append(obj)
            run('link-'+name,flags+options+objects+['-o',args.output/name])
            assert all(sha(ROOT/p)==value for p,value in report['build_inputs_sha256'].items())
            results.append(json.loads(run(name,[args.output/name],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1:print_stacktrace=1'})))
        assert results[0]==results[1]==results[2]
        run('cmake-configure',['cmake','-S','apps/cure-world','-B',args.output/'cmake','-DCMAKE_BUILD_TYPE=Debug'])
        run('cmake-build',['cmake','--build',args.output/'cmake','-j2'])
        run('ctest',['ctest','--test-dir',args.output/'cmake','--output-on-failure'])
        demo=json.loads(run('installed-demo',[args.output/'cmake/cure-world','--assets',args.assets,'--checkpoint',args.output/'in-flight-world.bin']))
        resume=json.loads(run('asset-free-resume',[args.output/'cmake/cure-world','--resume',args.output/'in-flight-world.bin']))
        assert demo==resume and demo['recipe']==74 and demo['health_before']==9 and demo['health_after']==17 and demo['caster_resource_q8']==17*256 and demo['status_after']==0 and demo['effect_released']
        report.update(**results[0],native_passed=True,native32_passed=True,sanitizer_passed=True,native_cmake_passed=True,installed_catalog_passed=True,separate_process_asset_free_resume_passed=True,demo=demo,
                      compiler=subprocess.check_output(['g++','--version'],text=True).splitlines()[0],qt_version=subprocess.check_output(['pkg-config','--modversion','Qt6Core'],text=True).strip())
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
        report['original_manifest_verified_before_after']=True
        report['sources_unchanged']=all(sha(ROOT/p)==value for p,value in claims['sources'].items())
        report['build_inputs_unchanged']=all(sha(ROOT/p)==value for p,value in report.get('build_inputs_sha256',{}).items())
        report['installed_inputs_unchanged']=all(sha(args.assets/p)==value for p,value in report.get('installed_inputs_sha256',{}).items())
        (args.output/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    assert report['sources_unchanged'] and report['build_inputs_unchanged'] and report['installed_inputs_unchanged']
    print(json.dumps({k:report[k] for k in ['success','cases','installed_catalog_passed','separate_process_asset_free_resume_passed','demo']}))


if __name__=='__main__':
    main()
