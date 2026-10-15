#!/usr/bin/env python3
"""Original terrain-derived navigation graphs, paired journeys and portable restores."""
import argparse
import hashlib
from importlib.machinery import SourceFileLoader
import json
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
PIN = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--install', type=Path, required=True)
    parser.add_argument('--build-root', type=Path, required=True)
    parser.add_argument('--claims', type=Path)
    parser.add_argument('--ticks', type=int, default=1024)
    parser.add_argument('--seeds', type=int, nargs='+', default=[17, 91])
    parser.add_argument('--scenarios', type=int, nargs='+', default=[0, 1, 2, 3, 4])
    a = parser.parse_args()
    out = a.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    declaration = json.loads(a.claims.read_text()) if a.claims else {'sources': {}, 'claims': []}
    if any(sha(ROOT / p) != v for p, v in declaration['sources'].items()):
        raise ValueError('Declared source changed')
    r = dict(success=False, source_executable_sha256=PIN,
             source_sha256=declaration['sources'], claims=declaration['claims'], commands=[],
             scope='Pinned53ab20 node placement,53b600 adjustment,53d710 ordering,53bb40/53c320/53cd30/53ce50 connection/clearance generation and542220 direct probes, independently constructed original/native graphs over bounded installed32x32 Celtic Plains terrain and installed28-creature shape inputs. Eleven helper fixtures include obstacles, water boundaries and capacity stops; five ordinary-type17 paired journeys include alias-derived truncated-path displacement. CMD1/v7 restores and native CLI regional construction; dynamic graph repair/World danger/campaign/general-species execution/live replacement excluded.',
             wine_required=False, live_replacement=False)
    sources = {}
    builds = {}

    def run(name, command, expected=0, env=None):
        command = [str(x) for x in command]
        r['commands'].append(command)
        p = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=900, env=env)
        (out / (name + '.stdout')).write_text(p.stdout)
        (out / (name + '.stderr')).write_text(p.stderr)
        if p.returncode != expected or 'ERROR: AddressSanitizer' in p.stderr or 'runtime error:' in p.stderr:
            raise RuntimeError(name + ' failed: ' + str(p.returncode) + p.stdout[-1500:] + p.stderr[-3000:])
        return p.stdout

    try:
        run('original-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
        pe = ROOT / 'original/Arcane_Nocd/Chaos.exe'
        if sha(pe) != PIN:
            raise ValueError('Original executable hash differs')
        inputs = ['CFG/Encrypted/creature.cfg', 'CFG/Encrypted/HTH.cfg', 'Creatures/crocodile.ani',
                  'Realms/Celtic/Plains/CPsec01.map', 'Realms/Celtic/Plains/Terrain.ttd']
        r['installed_inputs'] = {p: sha(a.install / p) for p in inputs}
        deps = SourceFileLoader('regional_dependencies', str(ROOT / 'tools/test-installed-encounter.py')).load_module()
        for tag, options in [
            ('original32', ['-DCMAKE_CXX_FLAGS=-m32 -O1 -g', '-DCMAKE_EXE_LINKER_FLAGS=-m32 -no-pie', '-DMNM_ENCOUNTER_ORIGINAL_REFERENCE=ON']),
            ('native64', ['-DCMAKE_BUILD_TYPE=', '-DCMAKE_CXX_FLAGS=-O1 -g']),
            ('sanitized', ['-DCMAKE_CXX_FLAGS=-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie', '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
            build = a.build_root.resolve() / tag
            builds[tag] = build
            run(tag + '-configure', ['cmake', '-S', ROOT / 'apps/native-encounter', '-B', build, '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON', *options])
            closure = deps.dependencies(build, run, tag, extra={'regional-navigation-test.cpp', 'regional-encounter-test.cpp', 'regional-map-admission-test.cpp', 'regional-graph-test.cpp'})
            closure.update({'tools/test-regional-graph.py', 'tools/export-regional-graph.py', 'tools/test-regional-navigation.py', 'tools/export-regional-navigation.py', 'tools/ghidra/ExportPersistenceProgression.java'})
            sources.update({p: sha(ROOT / p) for p in closure})
            if a.claims and closure - declaration['sources'].keys():
                raise ValueError('Undeclared build inputs: ' + str(sorted(closure - declaration['sources'].keys())))
            run(tag + '-build', ['cmake', '--build', build, '--target', 'regional-navigation-test', 'regional-encounter-test', 'regional-map-admission-test', 'regional-graph-test', 'regional-graph-encounter-test', 'native-encounter',
                                'native-command-session-test', 'native-command-behavior-session-test', 'native-command-action-session-test', 'native-command-action-bodies-session-test', 'native-creature-providers-test', 'native-encounter-test', '-j2'])
        env = {**os.environ, 'ASAN_OPTIONS': 'detect_leaks=1:halt_on_error=1', 'UBSAN_OPTIONS': 'halt_on_error=1:print_stacktrace=1'}
        helpers = []
        for fixture in range(11):
            compared=[]
            for tag, build in builds.items():
                compared.append(json.loads(run(tag + '-graph-helpers-' + str(fixture), [build / 'regional-graph-test', *([pe] if tag == 'original32' else []), a.install, fixture], env=env)))
            if not all(v == compared[0] for v in compared):
                raise ValueError('Graph helper architecture/sanitizer mismatch: '+str(fixture))
            helpers.append(dict(fixture=fixture,**compared[0]))
        r['helpers'] = helpers
        r['helper_checks'] = sum(v['checks'] for v in helpers)
        regression=[]
        for tag,build in builds.items():
            regression.append(json.loads(run(tag+'-regional-regression',[build/'regional-navigation-test',*([pe] if tag=='original32' else []),a.install],env=env)))
            run(tag+'-map-admission',[build/'regional-map-admission-test'],env=env)
        if not all(v==regression[0] for v in regression):
            raise ValueError('Existing regional helper regression differs')
        r['regional_regression']=regression[0]
        run('static', [sys.executable, ROOT / 'tools/export-regional-graph.py', '--output', out / 'static'])
        rows = []
        resumes = 0
        for scenario in a.scenarios:
            for seed in a.seeds:
                name = f'scenario-{scenario}-seed-{seed}'
                directory = out / name
                row = json.loads(run(name, [builds['original32'] / 'regional-graph-encounter-test', pe, a.install, scenario, seed, a.ticks, directory], env=env))
                row.update(scenario=scenario, seed=seed)
                rows.append(row)
                for tag in ['native64', 'sanitized']:
                    snapshot, trace = out / (tag + '-' + name + '.snapshot'), out / (tag + '-' + name + '.trace')
                    result = json.loads(run(tag + '-' + name, [builds[tag] / 'regional-graph-encounter-test', directory / 'initial.snapshot', scenario, a.ticks, snapshot, trace, 16], env=env))
                    if snapshot.read_bytes() != (directory / 'final.snapshot').read_bytes() or trace.read_bytes() != (directory / 'trace.txt').read_bytes():
                        raise ValueError('Regional native64/sanitized journey differs: ' + name)
                    resumes += result['restores']
            # Fresh native processes restore graph, actor metadata and search
            # state directly; no installed files are reopened by the runtime.
            directory = out / f'scenario-{scenario}-seed-{a.seeds[0]}'
            previous, last = directory / 'initial.snapshot', 0
            for end in sorted({1, 2, 4, 24, 128, 512, a.ticks}):
                if end > a.ticks:
                    continue
                name = f'resume-{scenario}-{end}'
                snapshot, trace = out / (name + '.snapshot'), out / (name + '.trace')
                run(name, [builds['native64'] / 'regional-graph-encounter-test', previous, scenario, end - last, snapshot, trace, 0], env={**env, 'PATH': '/nonexistent'})
                previous, last = snapshot, end
                resumes += 1
            if previous.read_bytes() != (directory / 'final.snapshot').read_bytes():
                raise ValueError('Fresh process regional continuation differs')
        r['journeys'] = rows
        if sum(v['regional_ticks'] for v in rows) == 0 or sum(v['region_changes'] for v in rows) == 0:
            raise ValueError('No actual regional continuation exercised')
        for scenario in [0, 1]:
            if scenario in a.scenarios and not any(v['arrivals'] for v in rows if v['scenario'] == scenario):
                raise ValueError('Regional journey never arrived: ' + str(scenario))
        if any(v['arrivals'] for v in rows if v['scenario'] == 2):
            raise ValueError('Unreachable regional fixture unexpectedly arrived')
        if 3 in a.scenarios and not any(v['displacement_orders'] for v in rows if v['scenario'] == 3):
            raise ValueError('Regional friendly displacement absent')
        if 4 in a.scenarios and not any(v['displacement_orders'] for v in rows if v['scenario']==4):
            raise ValueError('Truncated-path friendly displacement absent')
        for tag in ['native64', 'sanitized']:
            run(tag + '-core', ['ctest', '--test-dir', builds[tag] / 'command-session', '-R', '^(native-command-((behavior|action|action-bodies)-)?session|native-creature-providers)$', '--output-on-failure'], env=env)
            run(tag + '-installed', [builds[tag] / 'native-encounter-test', a.install], env=env)
        old_regression=[]
        for scenario in range(4):
            name='declared-regression-'+str(scenario)
            old_regression.append(json.loads(run(name,[builds['original32']/'regional-encounter-test',pe,a.install,scenario,17,a.ticks,out/name],env=env)))
        r['declared_graph_regression']=old_regression
        whole=out/'cli-regional-whole.snapshot'
        r['cli_regional']=json.loads(run('cli-regional-whole',[builds['native64']/'native-encounter','run-regional',a.install,0,0,32,32,17,a.ticks,whole],env=env))
        initial=out/'cli-regional-initial.snapshot'
        run('cli-regional-initial',[builds['native64']/'native-encounter','run-regional',a.install,0,0,32,32,17,0,initial],env=env)
        resumed=out/'cli-regional-resumed.snapshot'
        run('cli-regional-resume',[builds['native64']/'native-encounter','resume',initial,a.ticks,resumed],env={**env,'PATH':'/nonexistent'})
        if resumed.read_bytes()!=whole.read_bytes():raise ValueError('CLI regional fresh process differs')
        r['cli_process_resume_identical']=True
        r.update(success=True, native_passed=True, sanitizer_passed=True, original_target_text_unchanged=True,
                 process_resume_identical=True, process_resumes=resumes, compared_ticks=sum(v['ticks'] for v in rows))
    finally:
        run('original-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
        r['original_manifest_verified_before_after'] = True
        r['sources_unchanged'] = all(sha(ROOT / p) == v for p, v in declaration['sources'].items())
        r['build_inputs_sha256'] = sources
        r['build_inputs_unchanged'] = all(sha(ROOT / p) == v for p, v in sources.items())
        r['installed_inputs_unchanged'] = all(sha(a.install / p) == v for p, v in r.get('installed_inputs', {}).items())
        r['artifacts_sha256'] = {str(p.relative_to(out)): sha(p) for p in out.rglob('*') if p.is_file()}
        (out / 'report.json').write_text(json.dumps(r, indent=2) + '\n')
    if not all(r[k] for k in ['sources_unchanged', 'build_inputs_unchanged', 'installed_inputs_unchanged']):
        raise ValueError('Inputs changed during regional comparison')
    print(json.dumps({k: r[k] for k in ['success', 'compared_ticks', 'process_resumes', 'sanitizer_passed']}))


if __name__ == '__main__':
    main()
