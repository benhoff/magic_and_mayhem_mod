#!/usr/bin/env python3
"""Compare production panel splices with isolated pinned original wrappers."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--executable', type=Path, default=ROOT/'original/Arcane_Nocd/Chaos.exe')
    args = parser.parse_args()
    declaration = json.loads(args.claims.read_text())
    sources = declaration['sources']
    if any(sha(ROOT/p) != h for p, h in sources.items()):
        raise ValueError('Prospective sources changed')
    parent = ROOT/'working/tests/panel-splice'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    report = dict(schema=1, success=False, sources=sources, claims=declaration['claims'],
                  original_sha256=HASH, scope='Isolated pinned original panel wrappers and actual production installer/resolver/entry/kernel. Supplied external Lock/Unlock and Linux-hosted Win32 memory/API adapters; complete physical pixels, caller GPR/defined flags/DF/FX, Unlock arguments/count/volatile GPR and LastError. No real DirectDraw, Wine/game integration, live replacement or full renderer claim.',
                  experiment=str(out.relative_to(ROOT)), live_replacement=False, original_work_bypassed=False)
    commands = []

    def run(label, argv):
        env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0')
        result = subprocess.run([str(a) for a in argv], cwd=ROOT, env=env, capture_output=True,
                                text=True, timeout=240)
        log = out/(label+'.log')
        log.write_text(result.stdout+result.stderr)
        commands.append(dict(label=label, argv=[str(a) for a in argv], returncode=result.returncode,
                             log=str(log.relative_to(ROOT)), sha256=sha(log)))
        result.check_returncode()
        return result.stdout

    try:
        run('manifest-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        if sha(args.executable) != HASH:
            raise ValueError('Unsupported original executable')
        run('kernel-build', ['gcc', '-O1', '-g', '-std=c11', '-Wall', '-Wextra', '-Werror', '-pedantic',
                            '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-MMD', '-MF', out/'unit.d',
                            ROOT/'tests/panel-kernel-test.c', ROOT/'reconstruction/rendering/panel_kernel.c', '-o', out/'unit'])
        report['kernel'] = json.loads(run('kernel-sanitized', [out/'unit']))
        names = ['reconstruction/rendering/panel_kernel.c', 'runtime/canvas-writers/panel_route.c',
                 'runtime/canvas-writers/panel_entry.S', 'runtime/canvas-writers/panel_bridge.c',
                 'tests/panel-splice-state-probe.S']
        objects = []
        for name in names:
            obj = out/(Path(name).name+'.o')
            run(Path(name).name, ['clang', '-m32', '-O2', '-fno-pie', '-fdeclspec', '-Wno-ignored-attributes',
                                 '-DMNM_PANEL_WRITER_SELFTEST', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror',
                                 '-MMD', '-MF', obj.with_suffix('.d'), '-c', ROOT/name, '-o', obj])
            objects.append(obj)
        run('reference-build', ['clang++', '-m32', '-O2', '-std=c++17', '-fno-pie', '-no-pie', '-fdeclspec',
                                '-Wno-ignored-attributes', '-Wall', '-Wextra', '-Werror', '-MMD', '-MF', out/'reference.d',
                                ROOT/'tests/panel-splice-reference.cpp', *objects, '-o', out/'reference'])
        closure = {'tools/test-panel-splice.py', 'tools/original-manifest.sh'}
        for path in out.glob('*.d'):
            for name in shlex.split(path.read_text().replace('\\\n', ' ').split(':', 1)[1]):
                p = Path(name).resolve()
                if p.is_absolute() and p.is_relative_to(ROOT) and not p.is_relative_to(ROOT/'working'):
                    closure.add(str(p.relative_to(ROOT)))
        if closure - sources.keys():
            raise ValueError('Undeclared compiler dependencies: '+repr(sorted(closure-sources.keys())))
        report['compiled_project_sources'] = sorted(closure)
        run('production-pe32-build', ['python3', ROOT/'tools/build-panel-writers.py'])
        run('launch-admission', ['python3', '-B', ROOT/'tests/test-panel-backend-launch.py'])
        # Exercise the actual source guard, import patcher, builder/copy and
        # activation path on an empty disposable fixture, without launching it.
        from panel_backend_launch import stage, authorize
        staged = out/'staging'
        (staged/'game').mkdir(parents=True)
        raw = args.executable.read_bytes()
        changed, metadata = stage(staged, raw, raw, True)
        if changed == raw or metadata.get('native_panel_takeover') is not True:
            raise ValueError('Native panel staging did not append its import')
        environment = {'MNM_PANEL_TAKEOVER':'inherited'}
        authorize(staged, {**metadata, 'native_render':True, 'native_draw':True,
                           'origin':'menu_action_bridge'}, environment)
        if environment.get('MNM_PANEL_TAKEOVER') != '1':
            raise ValueError('Staged panel adapter did not activate')
        report['source_staging_passed'] = True
        report['staged_executable_sha256'] = hashlib.sha256(changed).hexdigest()
        run('original-native-splices', [out/'reference', args.executable, out/'comparison.json'])
        report['comparison'] = json.loads((out/'comparison.json').read_text())
        report['launch_admission_tests'] = 8
        report['success'] = report['comparison']['success'] and report['kernel']['success']
    except Exception as error:
        report['error'] = repr(error)
    finally:
        try:
            run('manifest-after', [ROOT/'tools/original-manifest.sh', 'verify'])
        except Exception as error:
            report.update(success=False, manifest_error=repr(error))
        report['sources_stable'] = all(sha(ROOT/p) == h for p, h in sources.items())
        report['inputs_stable'] = sha(args.executable) == HASH
        report['success'] = report['success'] and report['sources_stable'] and report['inputs_stable']
        report['commands'] = commands
        report['artifacts'] = {str(p.relative_to(ROOT)): dict(sha256=sha(p), bytes=p.stat().st_size)
                               for p in sorted(out.iterdir()) if p.is_file()}
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
