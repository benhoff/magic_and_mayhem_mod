#!/usr/bin/env python3
"""Compare complete original World queue ordering or owner-storage lifecycle."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import resource
import signal
import subprocess

from coverage_claims import behavior_contract, required_sources

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--suite', choices=('sort', 'owner', 'occlusion', 'startup', 'sweep', 'ownerlife', 'shutdown'), required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    out = args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):
        parser.error('Require a new retained output under working/tests')
    out.mkdir(parents=True)
    (out/'tmp').mkdir()
    env = dict(os.environ, TMPDIR=str(out/'tmp'))
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    bid = {'sort':'RS.world-queue-sort','owner':'RS.world-queue-owner-storage',
           'occlusion':'RS.world-occlusion-mask','startup':'RS.canvas-startup-initializer',
           'sweep':'RS.world-occlusion-actor-writers','ownerlife':'RS.particle-owner-slot-lifecycle',
           'shutdown':'RS.drawing-session-release'}[args.suite]
    behavior = next(b for b in register['behaviors'] if b['id'] == bid)
    builds = {b['id']: b for b in register['builds']}
    names = required_sources(behavior) | {'tools/coverage_claims.py'}
    sources = {p: sha(ROOT/p) for p in sorted(names)}
    report = dict(schema=1, success=False, sources=sources, original_sha256=BUILD,
                  scope=behavior['scope'], claims=[dict(behavior=bid,
                  contract_sha256=behavior_contract(behavior, builds))],
                  live_replacement=False, original_pixels_used_as_native_inputs=False,
                  physical_driver_equivalence=False)
    (out/'prospective.json').write_text(json.dumps(report, indent=2)+'\n')

    def run(label, command, compressed=False):
        print(label, flush=True)
        with (out/(label+'.log')).open('x') as log:
            if not compressed:
                subprocess.run(list(map(str, command)), cwd=ROOT, env=env, stdout=log,
                               stderr=subprocess.STDOUT, check=True, timeout=180)
                return
            with (out/(label+'.bin.zst')).open('xb') as target:
                encoder = subprocess.Popen(['zstd', '-q', '-3', '-c'], stdin=subprocess.PIPE,
                                           stdout=target, stderr=log)
                process = subprocess.Popen(list(map(str, command)), cwd=ROOT, env=env,
                                           stdout=encoder.stdin, stderr=log)
                encoder.stdin.close()
                try:
                    if process.wait(timeout=180) or encoder.wait(timeout=30):
                        raise ValueError(label+' failed; inspect retained log')
                finally:
                    for child in (process, encoder):
                        if child.poll() is None:
                            child.kill()
                        child.wait()

    before = False
    try:
        run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        before = True
        executable = ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(executable) != BUILD:
            raise ValueError('Unsupported original build')
        fixture = ROOT/('tests/canvas-initializer-reference.cpp' if args.suite=='startup' else
                       'tests/drawing-session-release-reference.cpp' if args.suite=='shutdown' else
                       'tests/particle-owner-lifecycle-reference.cpp' if args.suite=='ownerlife' else
                       'tests/world-occlusion-sweep-reference.cpp' if args.suite=='sweep' else
                       'tests/world-'+('queue-' if args.suite!='occlusion' else '')+args.suite+'-reference.cpp')
        for name, flags in [('native', []), ('original', ['-m32', '-fno-pie', '-no-pie', '-DMNM_ORIGINAL_REFERENCE'])]:
            run(name+'-build', ['g++', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
                               *flags, '-MD', '-MF', out/(name+'.d'), fixture, '-o', out/name])
            for item in (out/(name+'.d')).read_text().replace('\\\n', ' ').split()[1:]:
                p = Path(item)
                if p.is_file() and p.resolve().is_relative_to(ROOT):
                    if p.resolve().relative_to(ROOT).as_posix() not in sources:
                        raise ValueError('Undeclared compiler input: '+item)
            command = ['bwrap', '--die-with-parent', '--unshare-net', '--ro-bind', '/', '/',
                       '--bind', out, out, '--dev', '/dev', '--proc', '/proc', '--', out/name]
            if name == 'original':
                command.append(executable)
            run(name+'-execute', command, True)
        procs = []
        size = 0
        digest = hashlib.sha256()
        try:
            procs = [subprocess.Popen(['zstd', '-qdc', out/(name+'-execute.bin.zst')],
                                     stdout=subprocess.PIPE) for name in ('native', 'original')]
            while True:
                left, right = [p.stdout.read(1024*1024) for p in procs]
                if left != right:
                    at = next((i for i, (a, b) in enumerate(zip(left, right)) if a != b), min(len(left), len(right)))
                    raise ValueError('Original/native byte mismatch at '+str(size+at))
                if not left:
                    break
                digest.update(left)
                size += len(left)
            if any(p.wait(timeout=30) for p in procs):
                raise ValueError('Decompression failed')
        finally:
            for child in procs:
                if child.poll() is None:
                    child.kill()
                child.wait()
        count = {'sort':46570,'owner':360,'occlusion':49664,'startup':46656,'sweep':27648,'ownerlife':1296,'shutdown':4608}[args.suite]
        for name in ('native', 'original'):
            if (out/(name+'-execute.log')).read_text().strip() != 'cases='+str(count):
                raise ValueError('Case denominator differs: '+name)
        if args.suite == 'owner':
            with (out/'original-null-mask.log').open('x') as log:
                result = subprocess.run([out/'original', executable, 'null-mask'], cwd=out,
                                        env=env, stdout=log, stderr=subprocess.STDOUT, timeout=30,
                                        preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)))
            if result.returncode != -signal.SIGSEGV:
                raise ValueError('Null mask did not reach the expected original access fault')
            report['original_null_mask_signal'] = 'SIGSEGV'
        if args.suite == 'ownerlife':
            report['original_null_list_signals'] = {}
            for ordinal in (2, 3):
                with (out/('original-null-list-'+str(ordinal)+'.log')).open('x') as log:
                    result = subprocess.run([out/'original', executable, str(ordinal)], cwd=out,
                                            env=env, stdout=log, stderr=subprocess.STDOUT, timeout=30,
                                            preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)))
                if result.returncode != -signal.SIGSEGV:
                    raise ValueError('Null owner list allocation did not reach the expected original fault')
                report['original_null_list_signals'][str(ordinal)] = 'SIGSEGV'
        if args.suite == 'shutdown':
            with (out/'original-no-progress.log').open('x') as log:
                try:
                    subprocess.run([out/'original', executable, 'no-progress'], cwd=out,
                                   env=env, stdout=log, stderr=subprocess.STDOUT, timeout=1,
                                   check=False)
                except subprocess.TimeoutExpired:
                    report['original_nonprogressing_registry_timed_out'] = True
                else:
                    raise ValueError('Original null-Surface2 registry unexpectedly made progress')
        report.update(success=True, cases=count, bytes_compared=size,
                      comparison_sha256=digest.hexdigest(), selected_original_bodies_unchanged=True,
                      owned_allocator_boundary=args.suite in {'owner', 'ownerlife'},
                      binaries_sha256={name: sha(out/name) for name in ('native', 'original')},
                      outputs_sha256={p.name: sha(p) for p in out.glob('*.bin.zst')})
        if sha(executable) != BUILD:
            raise ValueError('Original input changed')
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        report.update(success=False, error=str(error))
    finally:
        if before:
            try:
                run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
                report['original_manifest_verified_before_after'] = True
            except subprocess.SubprocessError as error:
                report.update(success=False, preservation_error=str(error))
        report['sources_stable'] = all(sha(ROOT/p) == h for p, h in sources.items())
        report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
