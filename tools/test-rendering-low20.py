#!/usr/bin/env python3
"""Execute bounded component controls for the first20 Low rendering contracts.

This is fixture and diagnostic validation, not a complete live-game acceptance
run. Every task retains its wider original, driver and journey boundaries.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
FAMILIES = {
    'publication': ('NR.drawing-actions',),
    'physical': ('NR.drawing-fenced-acquisition',),
    'interactive': ('NR.drawing-interactive-acquisition',),
    'shutdown': ('NR.drawing-observer-shutdown',),
    'api': ('NR.drawing-session-api',),
    'producer': ('NR.drawing-session-producer-capture',),
    'cadence': ('NR.engine-cadence-observation',),
    'canvas': ('NR.font-glyph-live-shadow',),
    'minimap': ('NR.minimap-input-fixture', 'NR.minimap-owned-observation'),
    'gpu': ('NR.native-draw-cost-profiling',),
    'owner': ('NR.native-world-owner-costs',),
    'locks': ('NR.observer-lock-avoidance',),
    'command': ('NR.real-game-command-shadow',),
    'capture': ('NR.resident-stage-profiling',),
    'production': ('NR.scene-draw-budget', 'NR.scene-production-repetition'),
    'batch': ('NR.world-batch-profiling',),
    'refusal': ('NR.world-kind-diagnostic',),
    'rolling': ('NR.world-rolling-hardware',),
}
PYTHON = {
    'publication': ['test-drawing-publication.py'],
    'physical': ['test-drawing-physical.py'],
    'interactive': ['test-drawing-diagnostic-schedule.py', 'test-drawing-quit-completion.py', 'test-drawing-fixture-prefix.py'],
    'api': ['test-drawing-api-session.py', 'test-drawing-api-physical.py', 'test-drawing-call-intervals.py', 'test-drawing-surface-lifetimes.py'],
    'producer': ['test-drawing-session-producers.py'],
    'batch': ['native-world-batch-test.py'],
    'refusal': ['world-refusal-diagnostics-test.py'],
}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--hardware', action='store_true', help='Also execute authored GPU controls through the independent NVIDIA EGL plugin')
    parser.add_argument('--prepared', type=Path, help='Reuse immutable native binaries only after exact compiled-source/build-input and binary hash checks')
    args = parser.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    declaration = json.loads(args.claims.read_text())
    selected = sorted(b for family in FAMILIES.values() for b in family)
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    native = {b['id'] for b in register['behaviors'] if b['subsystem'] == 'native_rendering'}
    importance = json.loads((ROOT/'apps/coverage-ui/importance.json').read_text())
    expected = sorted(e['behavior'] for e in importance['entries'] if e['weight'] == 2 and e['behavior'] in native)[:20]
    if selected != expected or sorted(c['behavior'] for c in declaration['claims']) != expected:
        parser.error('Require exactly the first20 Low native-rendering contracts in UI order')
    sources = declaration['sources']
    if any(sha(ROOT/name) != digest for name,digest in sources.items()):
        raise ValueError('Prospective source changed')
    # Do not inherit a caller's Zink/NVIDIA selection into the software cohort.
    env = {k:v for k,v in os.environ.items() if not k.startswith('MNM_') and k not in {
        'QT_QPA_PLATFORM_PLUGIN_PATH', '__EGL_VENDOR_LIBRARY_FILENAMES', '__GLX_VENDOR_LIBRARY_NAME',
        'MESA_LOADER_DRIVER_OVERRIDE', 'GALLIUM_DRIVER', 'QT_XCB_GL_INTEGRATION'}}
    env.update(QT_QPA_PLATFORM='xcb', LIBGL_ALWAYS_SOFTWARE='1', GALLIUM_DRIVER='llvmpipe', LP_NUM_THREADS='8')
    results = []
    report = dict(schema=1, success=False, sources=sources, claims=declaration['claims'],
                  scope=__doc__, original_game_executed=False, live_replacement=False,
                  complete_live_acceptance=False, cases=results, tasks=[])

    def run(family, label, command, *, timeout=600, environment=env):
        command = [str(x) for x in command]
        log = out/(label+'.log')
        print(label, flush=True)
        with log.open('x') as stream:
            trial = subprocess.run(command, cwd=ROOT, env=environment, stdout=stream,
                                   stderr=subprocess.STDOUT, timeout=timeout)
        text = log.read_text(errors='replace')
        row = dict(family=family, label=label, command=command, exit_code=trial.returncode,
                   log_sha256=sha(log), log=str(log.relative_to(ROOT)))
        counts = re.findall(r'Ran (\d+) tests?', text)
        if counts: row['python_tests'] = sum(map(int,counts))
        results.append(row)
        if trial.returncode: raise RuntimeError(label+' failed; inspect '+str(log))

    try:
        for family,names in PYTHON.items():
            for name in names:
                run(family, Path(name).stem, ['python3','-B',ROOT/'tests'/name])
        prior = json.loads(args.prepared.read_text()) if args.prepared else None
        build = args.prepared.resolve().parent/'build' if prior else out/'build'
        targets = ['low-owner-profile','low-resident-capture','render-gpu-timing-test',
                   'canvas-sequence-test','minimap-producer-test','world-rolling-channel-test',
                   'canvas-world-test','live-render-channel-test']
        if not prior:
            run('build','configure',['cmake','-S',ROOT/'tests/rendering-low20','-B',build,'-DCMAKE_BUILD_TYPE=Debug'])
            run('build','build',['cmake','--build',build,'--target',*targets,'-j4'],timeout=1200)
        binaries = {p.name:p for p in build.rglob('*') if p.is_file() and p.name in targets}
        if set(binaries) != set(targets): raise ValueError('Native fixture target missing')
        report['binaries'] = {str(p.relative_to(ROOT)):sha(p) for p in binaries.values()}
        dependencies = set()
        import shlex
        for path in build.rglob('*.o.d'):
            for name in shlex.split(path.read_text().replace('\\\n',' ')):
                p = Path(name).resolve()
                if p.is_file() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                    dependencies.add(str(p.relative_to(ROOT)))
        if dependencies-set(sources): raise ValueError('Undeclared compiled dependencies: '+repr(sorted(dependencies-set(sources))))
        if prior:
            build_inputs = {n for n in sources if Path(n).name=='CMakeLists.txt' or Path(n).suffix=='.cmake'}
            if (not prior.get('sources_stable') or
                    not any(c['label']=='build' and c['exit_code']==0 for c in prior['cases']) or
                    any(prior['sources'].get(n)!=sources[n] for n in dependencies|build_inputs) or
                    prior.get('binaries') != report['binaries']):
                raise ValueError('Prepared source/build closure or native binary changed')
            report['prepared_binary_receipt'] = dict(path=str(args.prepared),sha256=sha(args.prepared),
                                                    compiled_sources_and_build_inputs_equal=True)
        report['compiled_dependencies'] = sorted(dependencies)
        for family,target in [('owner','low-owner-profile'),('gpu','render-gpu-timing-test'),
                              ('canvas','canvas-sequence-test'),('minimap','minimap-producer-test'),
                              ('batch','canvas-world-test'),('command','live-render-channel-test'),
                              ('rolling','world-rolling-channel-test')]:
            run(family,'native-'+target,['xvfb-run','-a',binaries[target]])
        run('capture','native-capture',[binaries['low-resident-capture'],out/'capture'])
        run('capture','capture-python',['python3',ROOT/'tests/resident-profile-capture-test.py',out/'capture'])
        # Serial fixtures own this prefix, exclusive marker/lock and stop only
        # their own wineserver. No original installation is copied or executed.
        suite = Path(tempfile.mkdtemp(prefix='low20-',dir=ROOT/'working/tests/drawing-fixture-prefix-suite'))
        prefix = suite/'wineprefix';prefix.mkdir(mode=0o700)
        (suite/'fixture-prefix-owner.json').write_text(json.dumps(dict(schema=1,root=str(ROOT),scope='serial-owned-fixtures-only'))+'\n')
        wine = dict(env,WINEPREFIX=str(prefix),WINEDEBUG='-all')
        run('build','wineboot',['xvfb-run','-a','wineboot','-u'],environment=wine,timeout=180)
        run('build','wineboot-stop',['wineserver','-k'],environment=wine)
        run('build','wineboot-wait',['wineserver','-w'],environment=wine)
        run('cadence','engine-cadence',['python3',ROOT/'tools/test-engine-cadence.py','--shared-fixture-prefix',prefix,'--claims',args.claims.resolve()])
        run('production','scene-production',['python3',ROOT/'tools/test-scene-production.py','--claims',args.claims.resolve()],timeout=1200)
        run('shutdown','shutdown',['xvfb-run','-a','python3',ROOT/'tools/test-drawing-gpu-completion.py','--prefix-template',prefix,'--shared-fixture-prefix',prefix,'--claims',args.claims.resolve(),'--report',out/'shutdown.json'])
        for physical in (False,True):
            run('api','api-'+str(physical),['xvfb-run','-a','python3',ROOT/'tools/test-drawing-api-session.py','--shared-fixture-prefix',prefix,'--claims',args.claims.resolve(),'--report',out/('api-'+str(physical)+'.json'),*(['--physical'] if physical else [])])
        run('locks','lock-lifecycle',['xvfb-run','-a','python3',ROOT/'tools/test-render-lock-lifecycle.py','--shared-fixture-prefix',prefix])
        if args.hardware:
            egl = out/'egl'
            run('build','egl-configure',['cmake','-S',ROOT/'tools/native-render-profile','-B',egl,'-DCMAKE_BUILD_TYPE=Debug'])
            run('build','egl-build',['cmake','--build',egl,'--target','mnm_headless_egl','-j4'])
            report['egl_plugin_sha256'] = sha(egl/'platforms/libmnm_headless_egl.so')
            hardware = dict(env,QT_QPA_PLATFORM='mnm-headless-egl',QT_QPA_PLATFORM_PLUGIN_PATH=str(egl/'platforms'),MNM_PROFILE_EGL_VENDOR='NVIDIA',__EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/10_nvidia.json')
            for name in ('LIBGL_ALWAYS_SOFTWARE','GALLIUM_DRIVER'): hardware.pop(name,None)
            for family,target in [('owner','low-owner-profile'),('gpu','render-gpu-timing-test')]:
                run(family,'hardware-'+target,[binaries[target]],environment=hardware)
            report['hardware_authored_controls'] = True
        report['sources_stable'] = all(sha(ROOT/n)==h for n,h in sources.items())
        report['tasks'] = [dict(behavior=b,component_controls='PASS',complete_contract_acceptance='pending',
                               families=[f for f,bs in FAMILIES.items() if b in bs]) for b in selected]
        if not report['sources_stable']: raise ValueError('Source changed during execution')
        report['success'] = True
    except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:
        report['error'] = str(error)
    finally:
        report['sources_stable'] = all((ROOT/n).is_file() and sha(ROOT/n)==h for n,h in sources.items())
        report['success'] = report['success'] and report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
