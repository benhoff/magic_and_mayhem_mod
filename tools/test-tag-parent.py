#!/usr/bin/env python3
"""Prospective source-bound evidence for the installed TAG application workflow."""
import argparse
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys

from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import read_bytes, write_report

ROOT = Path(__file__).resolve().parents[1]
SCENARIO = 'AS.tag.installed-preview'


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--installation', type=Path, required=True)
    parser.add_argument('--original-checkout', type=Path, default=ROOT)
    parser.add_argument('--build-root', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    output, buildroot, installation = args.output.resolve(), args.build_root.resolve(), args.installation.resolve()
    if output.exists() or args.report.exists() or not output.is_relative_to(ROOT/'working/tests'):
        parser.error('Require new owned immutable output and report paths')
    if not args.report.absolute().is_relative_to(ROOT/'research/runtime'):
        parser.error('Require a durable research SQLite reference')
    if buildroot.exists() or not buildroot.is_relative_to(Path('/tmp')):
        parser.error('Require a new task-specific temporary build root')
    if subprocess.check_output(['findmnt', '-n', '-o', 'FSTYPE', '-T', str(buildroot.parent)], text=True).strip() != 'tmpfs':
        parser.error('Build root must use verified tmpfs')
    output.mkdir(parents=True)
    (output/'.retain').write_text('Durable TAG parent logs, test binaries, source variants and input identities.\n')
    buildroot.mkdir(); (buildroot/'tmp').mkdir()
    env = dict(os.environ, TMPDIR=str(buildroot/'tmp'), ASAN_OPTIONS='detect_leaks=0:halt_on_error=1',
               UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1', LIBGL_ALWAYS_SOFTWARE='1', LP_NUM_THREADS='2')
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior = next(b for b in register['behaviors'] if b['id'] == 'AS.tag')
    scenario = next(s for s in register['scenarios'] if s['id'] == SCENARIO)
    paths = required_sources(behavior) | set(behavior['documents']) | set(scenario['tests'])
    for base in ['assets', 'renderer', 'apps/sprite-session', 'apps/sprite-timeline', 'apps/sprite-player', 'protocols/include']:
        paths.update(p.relative_to(ROOT).as_posix() for p in (ROOT/base).rglob('*')
                     if p.is_file() and (p.suffix in {'.hpp', '.h', '.inc', '.cpp', '.c', '.S'} or p.name == 'CMakeLists.txt'))
    sources = {p: sha(read_bytes(ROOT/p, root=ROOT)) for p in sorted(paths)}
    assert sha((args.original_checkout/'tools/original-manifest.sh').read_bytes()) == sources['tools/original-manifest.sh']
    report = dict(schema=1, success=False, sources=sources, inputs={}, commands=[], native={},
                  claims=[dict(behavior='AS.tag', contract_sha256=behavior_contract(behavior, {b['id']: b for b in register['builds']}),
                               scenarios={SCENARIO: scenario_contract(scenario)})],
                  scope=behavior['scope'], original_game_executed=False, live_replacement=False,
                  sanitizer_scope='ASan/UBSan; external Qt/driver leak scanning disabled, GPU surface balance asserted',
                  revision=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip())

    def run(label, argv, timeout=1800):
        result = subprocess.run([str(x) for x in argv], cwd=ROOT, env=env, capture_output=True, timeout=timeout)
        data = result.stdout + b'\n' + result.stderr
        (output/(label+'.log')).write_bytes(data)
        report['commands'].append(dict(label=label, argv=[str(x) for x in argv], exit_code=result.returncode,
                                       output_sha256=sha(data), tail=data[-3000:].decode(errors='replace')))
        result.check_returncode()
        return result.stdout

    try:
        run('original-before', [args.original_checkout.resolve()/'tools/original-manifest.sh', 'verify'])
        # Independent bounded container fields: no native loader output used to
        # select companions, establish expected records, or count occurrences.
        expected = {}
        for path in sorted(installation.rglob('*')):
            if not path.is_file() or path.suffix.lower() != '.tag':
                continue
            data = path.read_bytes(); relative = path.relative_to(installation).as_posix()
            report['inputs'][str(path)] = sha(data)
            entries = [dict(name=n.hex(), occurrence=i) for n, i in struct.iter_unpack('<8sI', data)]
            companion = path.with_suffix('.spr')
            if not companion.is_file():
                companion = path.parent/'Terrain.spr'
            sprite = companion.read_bytes(); report['inputs'][str(companion)] = sha(sprite)
            magic, size, version, count, palettes, flags = struct.unpack_from('<6I', sprite)
            assert magic == 0x00525053 and size == len(sprite) and version == 4 and palettes <= 4
            table = 24+palettes*768; base = table+count*4; names = []
            for i in range(count):
                start = base+struct.unpack_from('<I', sprite, table+i*4)[0]
                assert start+40 <= len(sprite)
                names.append(sprite[start+20:start+28])
            assert [e['name'] for e in entries] == [n.hex() for n in names]
            seen = Counter()
            for entry in entries:
                assert entry['occurrence'] == seen[entry['name']]
                seen[entry['name']] += 1
            expected[relative] = dict(path=relative, sprite=companion.relative_to(installation).as_posix(), entries=entries)
        assert len(expected) == 85 and sum(len(v['entries']) for v in expected.values()) == 130891
        for mode, flags in [('release', ['-DCMAKE_BUILD_TYPE=Release']),
                            ('sanitized', ['-DCMAKE_BUILD_TYPE=Debug', '-DCMAKE_CXX_FLAGS=-O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -fno-pie',
                                           '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
            build = buildroot/mode
            print('Building and validating '+mode, flush=True)
            run(mode+'-configure', ['cmake', '-S', ROOT/'apps/sprite-player', '-B', build, *flags])
            run(mode+'-build', ['cmake', '--build', build, '--target', 'sprite-player-test', 'mnm-sprite-player-app',
                               'tag-loader-test', 'mnm-tag-inspect', 'sprite-session-test', 'sprite-timeline-test', '-j2'])
            native = json.loads(run(mode+'-installed', [sys.executable, ROOT/'tools/run-surface-wrapper-control.py',
                                                      '--log-root', output/(mode+'-display'), '--', build/'sprite-player-test', installation]))
            assert native['success'] and {r['path']: r for r in native['installed']} == expected
            report['native'][mode] = {k: v for k, v in native.items() if k != 'installed'}
            report['native'][mode].update(independent_corpus_match=True, installed_files=len(expected),
                                         installed_entries=sum(len(v['entries']) for v in expected.values()),
                                         corpus_sha256=sha(json.dumps(native['installed'], sort_keys=True).encode()))
            run(mode+'-raw', [build/'timeline/session/sprites/assets/tag-loader-test'])
            # Outer before/after manifest verification belongs to the checkout
            # actually containing originals; invoke the existing independent
            # comparison body without its worktree-relative CLI wrapper.
            run(mode+'-raw-independent', [sys.executable, '-c',
                "import argparse,runpy,sys; from pathlib import Path; m=runpy.run_path(sys.argv[1]); m['run'](argparse.Namespace(inspector=sys.argv[2], installation=Path(sys.argv[3]), report=None))",
                ROOT/'tests/test-tag-loader.py', build/'timeline/session/sprites/assets/mnm-tag-inspect', installation])
            for name, binary in [('session', build/'timeline/session/sprite-session-test'), ('timeline', build/'timeline/sprite-timeline-test')]:
                run(mode+'-'+name, [sys.executable, ROOT/'tools/run-surface-wrapper-control.py', '--log-root', output/(mode+'-'+name+'-display'), '--', binary])
            run(mode+'-ctest', ['ctest', '--test-dir', build, '--output-on-failure', '-R', '^sprite-table-application$'])
            report['native'][mode]['supplier_regressions'] = True
            dependencies = set()
            for p in build.rglob('*.o.d'):
                for token in p.read_text().replace('\\\n', ' ').split()[1:]:
                    dep = Path(token).resolve()
                    if dep.is_file() and dep.is_relative_to(ROOT) and not dep.is_relative_to(ROOT/'working'):
                        dependencies.add(dep.relative_to(ROOT).as_posix())
            assert not dependencies-set(sources), sorted(dependencies-set(sources))
            report['native'][mode]['compiler_sources'] = sorted(dependencies)
            for name in ['sprite-player-test', 'mnm-sprite-player']:
                binary = (build/name).read_bytes(); (output/(mode+'-'+name)).write_bytes(binary)
                report['native'][mode][name+'_sha256'] = sha(binary)
        report['success'] = True
    except Exception as exc:
        report['error'] = repr(exc)[:1500]
        report['owned_source_variants'] = {p: (ROOT/p).read_text() for p in paths if p.startswith('apps/sprite-player/') or p in {'tools/test-tag-parent.py', 'tests/sprite-player-test.cpp'}}
    finally:
        try:
            run('original-after', [args.original_checkout.resolve()/'tools/original-manifest.sh', 'verify'])
        except Exception as exc:
            report['original_after_error'] = repr(exc); report['success'] = False
        report['sources_stable'] = all(sha(read_bytes(ROOT/p, root=ROOT)) == h for p, h in sources.items())
        report['inputs_stable'] = all(sha(Path(p).read_bytes()) == h for p, h in report['inputs'].items())
        report['success'] = bool(report['success'] and report['sources_stable'] and report['inputs_stable'])
        write_report(ROOT, args.report.absolute(), (json.dumps(report, indent=2)+'\n').encode())
        print(json.dumps({k: report.get(k) for k in ['success', 'error', 'sources_stable', 'inputs_stable']}), flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
