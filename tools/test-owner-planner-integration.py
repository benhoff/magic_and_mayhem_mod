#!/usr/bin/env python3
"""Fresh native planner continuation, rejection and installed-data regression run."""
import argparse
import hashlib
from importlib.machinery import SourceFileLoader
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SUITES = ['native-owner-planner-test', 'native-owner-group-planner-test', 'native-owner-visibility-test',
          'native-owner-knowledge-test', 'native-regional-danger-test']
CORE = ['native-command-session-test', 'native-command-behavior-session-test',
        'native-command-action-session-test', 'native-command-action-bodies-session-test',
        'native-creature-providers-test']
INPUTS = ['CFG/Encrypted/creature.cfg', 'CFG/Encrypted/HTH.cfg',
          'Creatures/crocodile.ani', 'Realms/Celtic/Plains/CPsec01.map',
          'Realms/Celtic/Plains/Terrain.ttd', 'AI/Brain.dat']


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for key in ['output', 'build-root', 'install']:
        parser.add_argument('--'+key, type=Path, required=True)
    parser.add_argument('--claims', type=Path)
    a = parser.parse_args()
    out = a.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    (out/'.retain').write_text('NS79 native grouped planner raw continuation evidence.\n')
    declaration = json.loads(a.claims.read_text()) if a.claims else dict(sources={}, claims=[])
    if any(sha(ROOT/k) != v for k, v in declaration['sources'].items()):
        raise ValueError('Declared sources changed before execution')
    r = dict(success=False, claims=declaration['claims'], source_sha256=declaration['sources'],
             scope='Native lab policy: ordinary two-actor installed32x32x23 encounter; taskless selector, ordered anonymous-memory groups, ordinary regional probes, 72 produced features and original DAT inference; explicit1/0/1 weights and100 percent, danger excess0, independent strategic RNG, fresh-sweep caches and origin-change probe restart, active-task retention. No full World cadence or task replacement equivalence. CMD1v13, two seeds x512 frames, every8-frame restore, cold-process continuation and legacy regressions',
             wine_required=False, live_replacement=False, commands=[])

    def run(name, command, env=None):
        command = [str(v) for v in command]
        r['commands'].append(command)
        p = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=900, env=env)
        (out/(name+'.stdout')).write_text(p.stdout)
        (out/(name+'.stderr')).write_text(p.stderr)
        if p.returncode or 'ERROR: AddressSanitizer' in p.stderr or 'runtime error:' in p.stderr:
            raise RuntimeError(name+' failed: '+p.stdout[-2000:]+p.stderr[-3000:])
        return p.stdout

    env = {**os.environ, 'ASAN_OPTIONS': 'detect_leaks=1:halt_on_error=1',
           'UBSAN_OPTIONS': 'halt_on_error=1:print_stacktrace=1'}
    try:
        run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        r['installed_inputs'] = {k: sha(a.install/k) for k in INPUTS}
        dependency = SourceFileLoader('planner_dependencies', str(ROOT/'tools/test-installed-encounter.py')).load_module()
        results, builds = {}, {}
        for tag, flags, link in [
            ('native64', '-O1 -g', ''),
            ('sanitized', '-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie', '-fsanitize=address,undefined -no-pie')]:
            build = a.build_root.resolve()/tag
            builds[tag] = build
            run(tag+'-configure', ['cmake', '-S', ROOT/'apps/native-encounter', '-B', build,
                                   '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON', '-DCMAKE_BUILD_TYPE=',
                                   '-DCMAKE_CXX_FLAGS='+flags, '-DCMAKE_EXE_LINKER_FLAGS='+link])
            closure = dependency.dependencies(build, run, tag, extra={s+'.cpp' for s in SUITES})
            closure.add('tools/test-owner-planner-integration.py')
            hashes = {k: sha(ROOT/k) for k in sorted(closure)}
            if a.claims and closure - declaration['sources'].keys():
                raise ValueError('Unbound compiler/build inputs: '+str(sorted(closure-declaration['sources'].keys())))
            r.setdefault('build_inputs_sha256', {}).update(hashes)
            run(tag+'-build', ['cmake', '--build', build, '--target', 'native-encounter', *SUITES, *CORE, '-j2'])
            if any(sha(ROOT/k) != v for k, v in hashes.items()):
                raise ValueError('Mixed-version build')
            results[tag] = {}
            for suite in SUITES:
                args = [build/suite, a.install]
                if suite == 'native-owner-planner-test':
                    args.append(out/(tag+'-journey'))
                results[tag][suite] = json.loads(run(tag+'-'+suite, args, env))
            run(tag+'-core', ['ctest', '--test-dir', build/'command-session', '-R',
                             '^(native-command-((behavior|action|action-bodies)-)?session|native-creature-providers)$', '--output-on-failure'], env)
        if results['native64'] != results['sanitized']:
            raise ValueError('Native/sanitizer results differ')
        for name in ['initial', '1', '3', '64', '257', '512']:
            if (out/'native64-journey'/(name+'.snapshot')).read_bytes() != (out/'sanitized-journey'/(name+'.snapshot')).read_bytes():
                raise ValueError('Native/sanitizer portable state differs')
        exe = builds['native64']/'native-encounter'
        for tick in [0, 1, 3, 64, 257]:
            source = out/'native64-journey'/('initial.snapshot' if not tick else str(tick)+'.snapshot')
            dest = out/('cold-'+str(tick)+'.snapshot')
            run('cold-'+str(tick), [exe, 'resume', source, 512-tick, dest], {**env, 'PATH': '/nonexistent'})
            if dest.read_bytes() != (out/'native64-journey/512.snapshot').read_bytes():
                raise ValueError('Cold continuation differs')
        # Exercise the public activation path independently of the test fixture.
        whole, split, resumed = [out/(n+'.snapshot') for n in ['cli-whole', 'cli-split', 'cli-resumed']]
        run('cli-whole', [exe, 'run-planner', a.install, 0, 0, 32, 32, 17, 512, whole])
        run('cli-split', [exe, 'run-planner', a.install, 0, 0, 32, 32, 17, 3, split])
        run('cli-resumed', [exe, 'resume', split, 509, resumed], {**env, 'PATH': '/nonexistent'})
        if whole.read_bytes() != resumed.read_bytes() or whole.read_bytes() != (out/'native64-journey/512.snapshot').read_bytes():
            raise ValueError('CLI activation/resume differs')
        linkage = run('linkage', ['ldd', exe])
        if any(k in linkage.lower() for k in ['wine', 'libqt']):
            raise ValueError('Unexpected runtime dependency')
        r.update(success=True, native_passed=True, sanitizer_passed=True, core_suites_passed=True,
                 suites=results['native64'], cold_process_restores=6, cli_resume_identical=True,
                 runtime_linkage=linkage, binaries={k: sha(v/'native-encounter') for k, v in builds.items()},
                 installed_inputs_unchanged=all(sha(a.install/k) == v for k, v in r['installed_inputs'].items()))
    finally:
        run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
        r['original_manifest_verified_before_after'] = True
        r['sources_unchanged'] = all(sha(ROOT/k) == v for k, v in declaration['sources'].items())
        r['build_inputs_unchanged'] = all(sha(ROOT/k) == v for k, v in r.get('build_inputs_sha256', {}).items())
        (out/'report.json').write_text(json.dumps(r, indent=2)+'\n')
    if not all(r[k] for k in ['sources_unchanged', 'build_inputs_unchanged', 'installed_inputs_unchanged']):
        raise ValueError('Sources or inputs changed during run')
    print(json.dumps(r['suites']['native-owner-planner-test']))


if __name__ == '__main__':
    main()
