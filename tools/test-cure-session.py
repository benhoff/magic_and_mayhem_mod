#!/usr/bin/env python3
"""Source-bound native Cure composition and checkpoint continuation checks."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ['tests/cure-session-test.cpp', 'apps/cure-lab/session.cpp',
           'reconstruction/casting/admission.cpp', 'reconstruction/casting/cure_effect.cpp',
           'reconstruction/casting/cure_services.cpp', 'reconstruction/combat/health.cpp',
           'reconstruction/audio/voice_contract.cpp', 'reconstruction/audio/voice_lifetime.cpp']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--claims', type=Path)
    args = p.parse_args()
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=False)
    claims = json.loads(args.claims.read_text()) if args.claims else {'sources': {}, 'claims': []}
    assert all(sha(ROOT/path) == value for path, value in claims['sources'].items())
    report = dict(success=False, claims=claims['claims'], source_sha256=claims['sources'],
                  scope='Native headless positive Cure composition: compared admission/effect/ordinary-health/resource/status helpers with native fixed generation pool, explicit animation/audio replies and portable checkpoints. No original lifecycle execution comparison, installed profile, original creation/spatial/animation-binding/release equivalence, projectile delivery, native World/campaign embedding or live replacement.',
                  commands=[], original_artifacts_consumed=False, live_replacement=False)

    def run(name, command, env=None):
        command = [str(x) for x in command]
        report['commands'].append(command)
        r = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, text=True, timeout=120)
        (args.output/(name+'.stdout')).write_text(r.stdout)
        (args.output/(name+'.stderr')).write_text(r.stderr)
        if r.returncode or 'runtime error:' in r.stderr or 'ERROR: AddressSanitizer' in r.stderr:
            raise RuntimeError(f'{name}: {r.returncode}: {r.stdout[-2000:]}{r.stderr[-2000:]}')
        return r.stdout

    flags = ['g++','-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-Iaudio']
    try:
        dependency = run('dependencies', flags+['-MM',*SOURCES,'apps/cure-lab/main.cpp'])
        paths = {'tools/test-cure-session.py','apps/cure-lab/CMakeLists.txt'}
        for token in shlex.split(dependency.replace('\\\n',' ')):
            path = Path(token)
            if not path.is_absolute() and path.is_file():
                paths.add(path.resolve().relative_to(ROOT).as_posix())
        report['build_inputs_sha256'] = {path: sha(ROOT/path) for path in sorted(paths)}
        if args.claims and paths - claims['sources'].keys():
            raise ValueError('Unbound compiler/build inputs: '+str(sorted(paths-claims['sources'].keys())))
        results = []
        for name, options in [('native',['-O2']),('native32',['-m32','-O2']),
                              ('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'])]:
            run('build-'+name,flags+options+SOURCES+['-o',args.output/name])
            assert all(sha(ROOT/path) == value for path, value in report['build_inputs_sha256'].items())
            results.append(json.loads(run(name,[args.output/name],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1:print_stacktrace=1'})))
        assert results[0] == results[1] == results[2]
        run('cmake-configure',['cmake','-S','apps/cure-lab','-B',args.output/'cmake','-DCMAKE_BUILD_TYPE=Debug'])
        run('cmake-build',['cmake','--build',args.output/'cmake','-j2'])
        run('ctest',['ctest','--test-dir',args.output/'cmake','--output-on-failure'])
        demo = json.loads(run('demo',[args.output/'cmake/cure-lab','--checkpoint',args.output/'in-flight.bin']))
        resume = json.loads(run('resume',[args.output/'cmake/cure-lab','--resume',args.output/'in-flight.bin']))
        assert demo == resume == dict(success=True,headless=True,health_before=9,health_after=17,caster_resource_q8=17*256,status_after=0)
        report.update(**results[0], native_passed=True,native32_passed=True,sanitizer_passed=True,
                      native_cmake_passed=True, separate_process_resume_passed=True,
                      binaries={name:sha(args.output/name) for name in ['native','native32','sanitized']},
                      compiler=subprocess.check_output(['g++','--version'],text=True).splitlines()[0])
    finally:
        report['sources_unchanged'] = all(sha(ROOT/path) == value for path,value in claims['sources'].items())
        report['build_inputs_unchanged'] = all(sha(ROOT/path) == value for path,value in report.get('build_inputs_sha256',{}).items())
        (args.output/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    assert report['sources_unchanged'] and report['build_inputs_unchanged']
    print(json.dumps({k:report[k] for k in ['success','cases','checkpoint_continuation','separate_process_resume_passed']}))


if __name__ == '__main__':
    main()
