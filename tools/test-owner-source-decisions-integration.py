#!/usr/bin/env python3
"""Fresh native planner continuation, rejection and installed-data regression run."""
import argparse
import hashlib
from importlib.machinery import SourceFileLoader
import json
import os
import shutil
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SUITES = ['native-owner-source-decisions-test', 'native-owner-sources-test', 'native-owner-planner-test', 'native-owner-group-planner-test', 'native-owner-replanning-test', 'native-owner-task-inputs-test']
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
    (out/'.retain').write_text('NS85 automatic friendly source decision raw continuation evidence.\n')
    declaration = json.loads(a.claims.read_text()) if a.claims else dict(sources={}, claims=[])
    if any(sha(ROOT/k) != v for k, v in declaration['sources'].items()):
        raise ValueError('Declared sources changed before execution')
    r = dict(success=False, claims=declaration['claims'], source_sha256=declaration['sources'],
             scope='NS85 bounded native two-friendly/one-enemy automatic source search using recovered423590 two-member decision stages, per-member431ce0 probes/features, actual installed model inference, selected topology commits, existing ordinary task lifetime/event/scheduler services, shared RNG/serials and CMD1v18 continuation. Explicit native host visits one root/frame, allocates4 planning units independently of execution, restarts changed origin/member/task epochs, derives availability from active health and folds ordinary queues. Compare native/sanitizer grouped and initially split and allocation1 sliced768-frame journeys, pending selectors/probes, automatic topology/task lifecycle, malformed/lossy checks, old v13-v17 source/planner/core/event regressions, public CLI and cold-process continuation. Original task6 probe assembly and automatic search are separately isolated comparisons; this native integration does not establish composed whole-original423590 equivalence, World cadence, difficulty weighted-mode producer, specialized queues, dual brains, training, factories or live replacement.',
             wine_required=False, live_replacement=False, commands=[])

    def run(name, command, env=None):
        command = [str(v) for v in command]
        r['commands'].append(command)
        p = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=1800 if name.endswith("-build") else 900, env=env)
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
                                   '-DCMAKE_CXX_FLAGS='+flags, '-DCMAKE_EXE_LINKER_FLAGS='+link, '-DMNM_ENCOUNTER_ORIGINAL_REFERENCE=OFF'])
            closure = dependency.dependencies(build, run, tag, extra={s+'.cpp' for s in SUITES})
            closure.add('tools/test-owner-source-decisions-integration.py')
            hashes = {k: sha(ROOT/k) for k in sorted(closure)}
            if a.claims and closure - declaration['sources'].keys():
                raise ValueError('Unbound compiler/build inputs: '+str(sorted(closure-declaration['sources'].keys())))
            r.setdefault('build_inputs_sha256', {}).update(hashes)
            run(tag+'-build', ['cmake', '--build', build, '--clean-first', '--target', 'native-encounter', *SUITES, *CORE, '-j2'])
            if any(sha(ROOT/k) != v for k, v in hashes.items()):
                raise ValueError('Mixed-version build')
            archived=out/(tag+'-binaries')
            archived.mkdir()
            for target in ['native-encounter',*SUITES,*CORE]:
                shutil.copy2(build/target if target not in CORE else build/'command-session'/target,archived/target)
            builds[tag]=archived
            r.setdefault('test_binaries',{})[tag]={name:sha(archived/name) for name in ['native-encounter',*SUITES,*CORE]}
            results[tag] = {}
            for suite in SUITES:
                args = [archived/suite, a.install]
                if suite=='native-owner-source-decisions-test':args.append(out/(tag+'-automatic'))
                if suite in ['native-owner-planner-test','native-owner-sources-test']:
                    args.append(out/(tag+('-legacy-journey' if suite=='native-owner-planner-test' else '-journey')))
                if suite=='native-owner-task-inputs-test':args.append('--scheduler')
                results[tag][suite] = json.loads(run(tag+'-'+suite, args, env))
            run(tag+'-core', ['ctest', '--test-dir', build/'command-session', '-R',
                             '^(native-command-((behavior|action|action-bodies)-)?session|native-creature-providers)$', '--output-on-failure'], env)
        if results['native64'] != results['sanitized']:
            raise ValueError('Native/sanitizer results differ')
        for name in ['initial', '1', '3', '64', '128', '256','lifecycle-33','lifecycle-96','lifecycle-97','lifecycle-256']:
            if (out/'native64-journey'/(name+'.snapshot')).read_bytes() != (out/'sanitized-journey'/(name+'.snapshot')).read_bytes():
                raise ValueError('Native/sanitizer portable state differs')
        exe = builds['native64']/'native-encounter'
        for tick in [0, 1, 3, 64, 128]:
            source = out/'native64-journey'/('initial.snapshot' if not tick else str(tick)+'.snapshot')
            dest = out/('cold-'+str(tick)+'.snapshot')
            run('cold-'+str(tick), [exe, 'resume', source, 256-tick, dest], {**env, 'PATH': '/nonexistent'})
            if dest.read_bytes() != (out/'native64-journey/256.snapshot').read_bytes():
                raise ValueError('Cold continuation differs')
        for start,end in [(33,96),(97,256)]:
            source=out/'native64-journey'/('lifecycle-'+str(start)+'.snapshot')
            dest=out/('cold-lifecycle-'+str(start)+'.snapshot')
            run('cold-lifecycle-'+str(start),[exe,'resume',source,end-start,dest],{**env,'PATH':'/nonexistent'})
            if dest.read_bytes()!=(out/'native64-journey'/('lifecycle-'+str(end)+'.snapshot')).read_bytes():
                raise ValueError('Cold topology continuation differs')
        # Exercise the public activation path independently of the test fixture.
        whole, split, resumed = [out/(n+'.snapshot') for n in ['cli-whole', 'cli-split', 'cli-resumed']]
        run('cli-whole', [exe, 'run-sources', a.install, 0, 0, 32, 32, 17, 256, whole])
        run('cli-split', [exe, 'run-sources', a.install, 0, 0, 32, 32, 17, 3, split])
        run('cli-resumed', [exe, 'resume', split, 253, resumed], {**env, 'PATH': '/nonexistent'})
        if whole.read_bytes() != resumed.read_bytes() or whole.read_bytes() != (out/'native64-journey/256.snapshot').read_bytes():
            raise ValueError('CLI activation/resume differs')
        for prefix in ['group','split','sliced']:
            for tick in [0,1,3,16,64,128]:
                source=out/'native64-automatic'/(prefix+'-'+str(tick)+'.snapshot')
                reference=out/'native64-automatic'/(prefix+'-256.snapshot')
                dest=out/('cold-auto-'+prefix+'-'+str(tick)+'.snapshot')
                run('cold-auto-'+prefix+'-'+str(tick),[exe,'resume',source,256-tick,dest],{**env,'PATH':'/nonexistent'})
                if dest.read_bytes()!=reference.read_bytes():raise ValueError('Cold automatic search differs')
            for tick in [0,1,3,16,64,128,256]:
                name=prefix+'-'+str(tick)+'.snapshot'
                if (out/'native64-automatic'/name).read_bytes()!=(out/'sanitized-automatic'/name).read_bytes():raise ValueError('Automatic sanitizer state differs')
        autoWhole,autoSplit,autoResumed=[out/(v+'.snapshot') for v in ['auto-cli-whole','auto-cli-split','auto-cli-resumed']]
        run('auto-cli-whole',[exe,'run-autogroups',a.install,0,0,32,32,17,256,autoWhole])
        run('auto-cli-split',[exe,'run-autogroups',a.install,0,0,32,32,17,3,autoSplit])
        run('auto-cli-resumed',[exe,'resume',autoSplit,253,autoResumed],{**env,'PATH':'/nonexistent'})
        if autoWhole.read_bytes()!=autoResumed.read_bytes() or autoWhole.read_bytes()!=(out/'native64-automatic/group-256.snapshot').read_bytes():raise ValueError('Automatic CLI differs')
        linkage = run('linkage', ['ldd', exe])
        if any(k in linkage.lower() for k in ['wine', 'libqt']):
            raise ValueError('Unexpected runtime dependency')
        r.update(success=True, native_passed=True, sanitizer_passed=True, core_suites_passed=True, clean_rebuilds=True,
                 suites=results['native64'], cold_process_restores=27, cli_resume_identical=True,
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
    print(json.dumps(r['suites']['native-owner-sources-test']))


if __name__ == '__main__':
    main()
