#!/usr/bin/env python3
"""Source-bound native wizard spell delivery, rollback and portable continuation."""
import argparse
import hashlib
from importlib.machinery import SourceFileLoader
import json
import os
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SUITES = ['native-wizard-encounter-test', 'native-owner-skirmish-test', 'native-encounter-test']
CORE = ['native-command-session-test', 'native-command-behavior-session-test',
        'native-command-action-session-test', 'native-command-action-bodies-session-test',
        'native-creature-providers-test']
INPUTS = ['CFG/Encrypted/creature.cfg', 'CFG/Encrypted/HTH.cfg',
          'CFG/Encrypted/spells.cfg', 'CFG/Encrypted/effectani.cfg',
          'Creatures/crocodile.ani', 'Creatures/Greek Male.ani', 'Wizards/Greek41.wzd',
          'Realms/Celtic/Plains/CPsec01.map', 'Realms/Celtic/Plains/Terrain.ttd', 'AI/Brain.dat']
INPUTS += ['Sprites/EFFECTS'+str(i)+'.ani' for i in range(13)]
SCOPE = ('Opt-in native eight-slot installed Gaea/type17 encounter on32x32x23 Celtic Plains, '
         'authored mid-game knowledge17/41/73; independent recovered mana wait, target/projectile '
         'task execution, cast admission, Cure and type3 projectile contracts composed with native '
         'candidate utilities, count cadence, current-position trajectory/terrain/creature shot '
         'prediction, positioning, ordinary combat, first-use summon slots and effect release policies. '
         'Two2048-frame mixed journeys and authored512-frame care/resource and wizard-defeat fixtures. '
         'CMD1v20 owns partial selectors, tasks, mana, effects/animation/trajectory/RNG/serials; '
         'lossy/malformed/profile-changing continuation refuses; frame verifier failures roll back. '
         'Clean native/sanitized builds, complete artifact equality, sliced and cold effect/search '
         'resumes, public CLI and ordinary skirmish/encounter/core regressions. '
         'No complete original per-spell candidate generation, World/wizard controller equivalence, '
         'other repertoire, original summoning factory/producer/inventory ownership, visual/audio '
         'impact ownership, campaign defeat/progression or live replacement.')


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def installed(root, key):
    path = root/key
    if path.exists():
        return path
    matches = [p for p in path.parent.iterdir() if p.name.lower() == path.name.lower()]
    if len(matches) != 1:
        raise ValueError('Missing/ambiguous installed input: '+key)
    return matches[0]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for key in ['output', 'build-root', 'install']:
        parser.add_argument('--'+key, type=Path, required=True)
    parser.add_argument('--claims', type=Path)
    parser.add_argument('--incremental', action='store_true', help='Reuse a previously configured build; reported explicitly')
    a = parser.parse_args()
    out = a.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    (out/'.retain').write_text('NS87 wizard source-bound native/sanitizer and continuation evidence.\n')
    declaration = json.loads(a.claims.read_text()) if a.claims else dict(sources={}, claims=[])
    (out/'declaration.json').write_text(json.dumps(declaration, indent=2)+'\n')
    if any(sha(ROOT/k) != v for k, v in declaration['sources'].items()):
        raise ValueError('Declared sources changed before execution')
    r = dict(success=False, claims=declaration['claims'], source_sha256=declaration['sources'],
             scope=SCOPE, wine_required=False, live_replacement=False, commands=[])

    def run(name, command, env=None):
        command = [str(v) for v in command]
        r['commands'].append(command)
        p = subprocess.run(command, cwd=ROOT, capture_output=True, text=True,
                           timeout=1800 if name.endswith('-build') else 1200, env=env)
        (out/(name+'.stdout')).write_text(p.stdout)
        (out/(name+'.stderr')).write_text(p.stderr)
        if p.returncode or 'ERROR: AddressSanitizer' in p.stderr or 'runtime error:' in p.stderr:
            raise RuntimeError(name+' failed: '+p.stdout[-2000:]+p.stderr[-3000:])
        return p.stdout

    env = {**os.environ, 'ASAN_OPTIONS': 'detect_leaks=1:halt_on_error=1',
           'UBSAN_OPTIONS': 'halt_on_error=1:print_stacktrace=1'}
    try:
        run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        inputs = {k: installed(a.install, k) for k in INPUTS}
        r['installed_inputs'] = {k: sha(p) for k, p in inputs.items()}
        dependency = SourceFileLoader('wizard_dependencies', str(ROOT/'tools/test-installed-encounter.py')).load_module()
        results, builds = {}, {}
        for tag, flags, link in [
            ('native64', '-O1 -g', ''),
            ('sanitized', '-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie',
             '-fsanitize=address,undefined -no-pie')]:
            build = a.build_root.resolve()/tag
            run(tag+'-configure', ['cmake', '-S', ROOT/'apps/native-encounter', '-B', build,
                                  '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON', '-DCMAKE_BUILD_TYPE=',
                                  '-DCMAKE_CXX_FLAGS='+flags, '-DCMAKE_EXE_LINKER_FLAGS='+link,
                                  '-DMNM_ENCOUNTER_ORIGINAL_REFERENCE=OFF'])
            closure = dependency.dependencies(build, run, tag, extra={s+'.cpp' for s in SUITES})
            closure.add('tools/test-wizard-ai-integration.py')
            hashes = {k: sha(ROOT/k) for k in sorted(closure)}
            if a.claims and closure-declaration['sources'].keys():
                raise ValueError('Unbound compiler/build inputs: '+str(sorted(closure-declaration['sources'].keys())))
            r.setdefault('build_inputs_sha256', {}).update(hashes)
            run(tag+'-build', ['cmake', '--build', build, *([] if a.incremental else ['--clean-first']), '--target',
                              'native-encounter', *SUITES, *CORE, '-j4'])
            if any(sha(ROOT/k) != v for k, v in hashes.items()):
                raise ValueError('Mixed-version build')
            archived = out/(tag+'-binaries')
            archived.mkdir()
            for target in ['native-encounter', *SUITES, *CORE]:
                shutil.copy2(build/target if target not in CORE else build/'command-session'/target,
                             archived/target)
            builds[tag] = archived
            r.setdefault('test_binaries', {})[tag] = {t: sha(archived/t) for t in ['native-encounter', *SUITES, *CORE]}
            results[tag] = {}
            for suite in SUITES:
                args = [archived/suite, a.install]
                if suite == 'native-wizard-encounter-test':
                    args.append(out/(tag+'-wizards'))
                if suite == 'native-owner-skirmish-test':
                    args.append(out/(tag+'-ordinary'))
                results[tag][suite] = json.loads(run(tag+'-'+suite, args, env))
            run(tag+'-core', ['ctest', '--test-dir', build/'command-session', '-R',
                             '^(native-command-((behavior|action|action-bodies)-)?session|native-creature-providers)$',
                             '--output-on-failure'], env)
        if results['native64'] != results['sanitized']:
            raise ValueError('Native/sanitizer results differ')
        files = {p.name: p for p in (out/'native64-wizards').iterdir() if p.is_file()}
        other = {p.name: p for p in (out/'sanitized-wizards').iterdir() if p.is_file()}
        if files.keys() != other.keys() or any(sha(p) != sha(other[k]) for k, p in files.items()):
            raise ValueError('Native/sanitizer wizard portable artifacts differ')
        exe = builds['native64']/'native-encounter'
        cold = 0
        for line in (out/'native64-wizards/snapshots.jsonl').read_text().splitlines():
            row = json.loads(line)
            if row['tick'] > 256:
                continue
            source = out/'native64-wizards'/row['file']
            destination = out/('cold-'+row['file'])
            run('cold-'+row['file'], [exe, 'resume', source, 512-row['tick'], destination],
                {**env, 'PATH': '/nonexistent'})
            if sha(destination) != sha(out/'native64-wizards'/('mixed-'+str(row['seed'])+'-512.snapshot')):
                raise ValueError('Cold wizard continuation differs')
            cold += 1
        whole, split, resumed = [out/(n+'.snapshot') for n in ['cli-whole', 'cli-split', 'cli-resumed']]
        run('cli-whole', [exe, 'run-wizards', a.install, 0, 0, 32, 32, 17, 512, whole], env)
        run('cli-split', [exe, 'run-wizards', a.install, 0, 0, 32, 32, 17, 3, split], env)
        run('cli-resumed', [exe, 'resume', split, 509, resumed], {**env, 'PATH': '/nonexistent'})
        if sha(whole) != sha(resumed) or sha(whole) != sha(out/'native64-wizards/mixed-17-512.snapshot'):
            raise ValueError('Public wizard CLI differs')
        linkage = run('linkage', ['ldd', exe])
        if any(k in linkage.lower() for k in ['wine', 'libqt']):
            raise ValueError('Unexpected runtime dependency')
        r.update(success=True, native_passed=True, sanitizer_passed=True, core_suites_passed=True,
                 clean_rebuilds=not a.incremental, incremental_rebuilds=a.incremental, suites=results['native64'], cold_process_restores=cold,
                 cli_resume_identical=True, runtime_linkage=linkage,
                 wizard_artifacts_sha256={k: sha(p) for k, p in sorted(files.items())},
                 installed_inputs_unchanged=all(sha(inputs[k]) == v for k, v in r['installed_inputs'].items()))
    finally:
        run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
        r['original_manifest_verified_before_after'] = True
        r['sources_unchanged'] = all(sha(ROOT/k) == v for k, v in declaration['sources'].items())
        r['build_inputs_unchanged'] = all(sha(ROOT/k) == v for k, v in r.get('build_inputs_sha256', {}).items())
        (out/'report.json').write_text(json.dumps(r, indent=2)+'\n')
    if not all(r[k] for k in ['sources_unchanged', 'build_inputs_unchanged', 'installed_inputs_unchanged']):
        raise ValueError('Sources or inputs changed during run')
    print(json.dumps(r['suites']['native-wizard-encounter-test']))


if __name__ == '__main__':
    main()
