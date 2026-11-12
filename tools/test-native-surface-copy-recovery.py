#!/usr/bin/env python3
"""SQLite-backed native copy recovery and original retry scheduling controls."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys

from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import read_bytes, write_report

ROOT = Path(__file__).resolve().parents[1]
BEHAVIOR = 'NP.surface-copy.owned-recovery'
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def digest(path):
    path = Path(path).resolve()
    data = read_bytes(path, root=ROOT) if path.is_relative_to(ROOT) else path.read_bytes()
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--main-root', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--build-root', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    out, builds, report_path = args.output.resolve(), args.build_root.resolve(), args.report.resolve()
    if (out.exists() or report_path.exists() or not out.is_relative_to(ROOT/'working/tests')
            or not builds.is_relative_to(ROOT/'working/build')
            or not report_path.is_relative_to(ROOT/'research/runtime')):
        parser.error('Require new owned execution/report paths and an owned build directory')
    out.mkdir(parents=True)
    (out/'.retain').write_text('Retain copy recovery execution inputs, binaries and logs.\n')
    (out/'tmp').mkdir()
    env = dict(os.environ, TMPDIR=str(out/'tmp'), LIBGL_ALWAYS_SOFTWARE='1', LP_NUM_THREADS='2',
               ASAN_OPTIONS='detect_leaks=0:halt_on_error=1',
               UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
    behavior = next(b for b in register['behaviors'] if b['id'] == BEHAVIOR)
    paths = required_sources(behavior) | set(behavior['documents'])
    # The full renderer library is linked. Explicitly pin its local build inputs,
    # then check actual compiler dependencies after each build.
    paths |= {p.relative_to(ROOT).as_posix() for p in (ROOT/'renderer').rglob('*')
              if p.suffix in {'.cpp', '.hpp', '.h', '.inc'}}
    sources = {p: digest(ROOT/p) for p in sorted(paths)}
    report = dict(schema=1, success=False, sources=sources,
                  claims=[dict(behavior=BEHAVIOR, contract_sha256=behavior_contract(
                      behavior, {b['id']: b for b in register['builds']}),
                      scenarios={s['id']:scenario_contract(s) for s in register['scenarios']
                                 if BEHAVIOR in s['behaviors']})],
                  commands=[], native={}, original_sha256=BUILD, live_replacement=False,
                  scope=behavior['scope'], artifacts=out.relative_to(ROOT).as_posix(),
                  source_revision=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                  sanitizer_scope='ASan/UBSan; external Qt/driver leak scanning disabled; owned resources checked explicitly.')

    def run(label, argv, timeout=180):
        print(label, flush=True)
        result = subprocess.run([str(v) for v in argv], cwd=ROOT, env=env,
                                capture_output=True, timeout=timeout)
        log = result.stdout + b'\n' + result.stderr
        (out/(label+'.log')).write_bytes(log)
        report['commands'].append(dict(label=label, exit_code=result.returncode,
                                       output_sha256=hashlib.sha256(log).hexdigest(),
                                       output_tail=log[-3000:].decode(errors='replace')))
        result.check_returncode()
        return result.stdout

    original = args.main_root.resolve()/'original/Arcane_Nocd/Chaos.exe'
    manifest = args.main_root.resolve()/'tools/original-manifest.sh'
    try:
        run('manifest-before', [manifest, 'verify'])
        if digest(original) != BUILD:
            raise ValueError('Unsupported original executable hash')
        for mode, flags in [('release', ['-DCMAKE_BUILD_TYPE=Release']), ('sanitized', [
                '-DCMAKE_BUILD_TYPE=Debug',
                '-DCMAKE_CXX_FLAGS=-O1 -g1 -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -fno-pie',
                '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
            build = builds/mode
            run(mode+'-configure', ['cmake', '-S', ROOT/'renderer', '-B', build, *flags])
            run(mode+'-build', ['cmake', '--build', build, '--target', 'owned-surface-wrappers-test', '-j2'], 900)
            native = json.loads(run(mode+'-native', [sys.executable, ROOT/'tools/run-surface-wrapper-control.py',
                '--log-root', out/(mode+'-displays'), '--', build/'owned-surface-wrappers-test']))
            if not native['success'] or native['copy_recovery_cases'] != 64:
                raise ValueError('Incomplete native recovery controls')
            report['native'][mode] = native
            report['native'][mode]['binary_sha256'] = digest(build/'owned-surface-wrappers-test')
            dependencies = set()
            for dep in build.rglob('*.o.d'):
                for token in dep.read_text().replace('\\\n', ' ').split()[1:]:
                    target = Path(token).resolve()
                    if target.is_file() and target.is_relative_to(ROOT) and not target.is_relative_to(ROOT/'working'):
                        dependencies.add(target.relative_to(ROOT).as_posix())
            if dependencies - set(sources):
                raise ValueError('Unpinned compiler inputs: '+str(sorted(dependencies-set(sources))))
        spec = importlib.util.spec_from_file_location('copy_recovery_retry', ROOT/'tools/check-original-surface-retry.py')
        retry = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(retry)
        rows, _ = retry.load()
        inputs = out/'retry-inputs.txt'
        inputs.write_text(''.join(' '.join(str(v) for v in [*(c[k] for k in [
            'id', 'entry', 'fast', 'no_wait', 'key_enabled', 'reload_bits', 'source_restore',
            'destination_restore', 'key_result', 'color']), len(c['draw_results']), *c['draw_results']])+'\n'
            for c in (row['input'] for row in rows)))
        reference, model = out/'original-retry', out/'native-retry'
        for label, source, binary, flags in [
                ('original-retry-build', 'tests/surface-retry-reference.cpp', reference, ['-m32', '-fno-pie', '-no-pie']),
                ('native-retry-build', 'tests/surface-retry-model.cpp', model, [])]:
            run(label, ['g++', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror', *flags, ROOT/source, '-o', binary])
        actual_original = json.loads(run('original-retry', [reference, original, inputs]))
        native = json.loads(run('native-retry', [model, inputs]))
        bounded = json.loads(run('native-budget', [model, inputs, '2']))
        for row, old, new, limited in zip(rows, actual_original, native, bounded, strict=True):
            if old != row['original'] or {k:new[k] for k in old} != old or not new['returned'] or new['budget_exhausted']:
                raise ValueError('Original scheduling mismatch')
            if (limited['events'] != old['events'][:len(limited['events'])]
                    or limited['draws_consumed'] > 2
                    or limited['budget_exhausted'] and limited['returned']):
                raise ValueError('Budget did not preserve original prefix')
        report['original'] = dict(matched=len(native), budget_exhaustions=sum(v['budget_exhausted'] for v in bounded),
                                  real_driver_loss=False, input_sha256=digest(inputs),
                                  reference_binary_sha256=digest(reference), model_binary_sha256=digest(model))
        report['success'] = True
    except Exception as error:
        report['error'] = repr(error)
    finally:
        try:
            run('manifest-after', [manifest, 'verify'])
            report['original_manifest_verified_before_after'] = True
        except Exception as error:
            report['original_manifest_verified_before_after'] = False
            report['manifest_error'] = repr(error)
        report['sources_stable'] = all(digest(ROOT/p) == h for p, h in sources.items())
        report['success'] &= report['sources_stable'] and report['original_manifest_verified_before_after']
        report['owned_source_variants'] = {p:(ROOT/p).read_text() for p in sources if p in {
            'reconstruction/rendering/owned_surface_wrappers.hpp', 'reconstruction/rendering/owned_surface_wrappers.cpp',
            'tests/owned-surface-wrappers-test.cpp', 'tools/test-native-surface-copy-recovery.py'}}
        write_report(ROOT, report_path, (json.dumps(report, indent=2)+'\n').encode())
        print(json.dumps({k:report.get(k) for k in ['success', 'error', 'native', 'original']}), flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
