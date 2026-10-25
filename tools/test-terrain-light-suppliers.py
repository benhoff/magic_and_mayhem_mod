#!/usr/bin/env python3
"""Compare bounded terrain light suppliers against a pinned private PE32 image."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
from coverage_claims import behavior_contract, required_sources
from coverage_evidence import write_report, read_bytes
ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
IDS = ['TL.relation-production', 'TL.resource-lifetime', 'TL.refresh-request']


def sha(path):
    return hashlib.sha256(read_bytes(path, root=ROOT) if path.is_relative_to(ROOT) else path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--main-root', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--executable', type=Path, help='Default: immutable original/Arcane_Nocd/Chaos.exe in main checkout')
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    main_root, out = args.main_root.resolve(), args.output.resolve()
    report_path = args.report.absolute()
    if out.exists() or report_path.exists() or not report_path.is_relative_to(ROOT/'research/runtime'):
        parser.error('Require new output and immutable checkout research report paths')
    out.mkdir(parents=True)
    (out/'.retain').write_text('Terrain light supplier execution outputs; retain until durable evidence is verified.\n')
    (out/'tmp').mkdir()
    env = dict(os.environ, TMPDIR=str(out/'tmp'), ASAN_OPTIONS='detect_leaks=1:halt_on_error=1', UBSAN_OPTIONS='halt_on_error=1')
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
    behaviors = [b for b in register['behaviors'] if b['id'] in IDS]
    if len(behaviors) != len(IDS):
        raise ValueError('Missing prospective supplier behavior contracts')
    builds = {b['id']: b for b in register['builds']}
    paths = set().union(*(required_sources(b) | set(b['documents']) for b in behaviors))
    paths.update(['tools/coverage_claims.py', 'tools/coverage_evidence.py',
                  'tests/terrain-creature-lighting-reference.cpp', 'tests/sprite-binary-reference.cpp',
                  'reconstruction/rendering/terrain_lighting.cpp', 'reconstruction/rendering/terrain_lighting.hpp',
                  'reconstruction/rendering/terrain_creature_lighting.cpp', 'reconstruction/rendering/terrain_creature_lighting.hpp'])
    sources = {p: sha(ROOT/p) for p in sorted(paths)}
    pe = args.executable.resolve() if args.executable else main_root/'original/Arcane_Nocd/Chaos.exe'
    report = dict(schema=1, success=False, sources=sources,
                  claims=[dict(behavior=b['id'], contract_sha256=behavior_contract(b, builds)) for b in behaviors],
                  original_sha256=BUILD, commands=[], results={}, live_replacement=False,
                  scope='Bounded relation defaults/pair writes, supplied-kernel resource init/reuse/clear and explicit refresh request. Allocator/free and kernel endpoint recorders; no original heap failure, full kernel execution, script consequences, installed producer or live acceptance.')

    def run(label, command, timeout=180):
        result = subprocess.run([str(v) for v in command], cwd=ROOT, env=env,
                                capture_output=True, text=True, timeout=timeout)
        (out/(label+'.log')).write_text(result.stdout+result.stderr)
        report['commands'].append(dict(label=label, exit_code=result.returncode,
                                       log_sha256=sha(out/(label+'.log'))))
        result.check_returncode()
        return result.stdout

    original_before = False
    try:
        run('original-before', [main_root/'tools/original-manifest.sh', 'verify'])
        original_before = True
        if sha(pe) != BUILD:
            raise ValueError('Original executable hash mismatch')
        common = ['g++', '-std=c++17', '-I'+str(ROOT/'reconstruction/rendering')]
        models = [ROOT/'reconstruction/rendering'/name for name in ['terrain_lighting.cpp', 'terrain_creature_lighting.cpp']]
        for fixture in ['terrain-light-suppliers', 'terrain-creature-lighting']:
            variants = []
            for name, flags in [('original', ['-O2', '-m32', '-fno-pie', '-no-pie', '-DORIGINAL_REFERENCE']),
                                ('native', ['-O2']),
                                ('sanitized', ['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie'])]:
                label = fixture+'-'+name
                print(label, flush=True)
                binary, output = out/label, out/(label+'.bin')
                run(label+'-build', [*common, *flags, ROOT/('tests/'+fixture+'-reference.cpp'), *models, '-o', binary])
                stats = json.loads(run(label, [binary, *([pe] if name == 'original' else []), output]))
                variants.append(dict(variant=name, stats=stats, output_sha256=sha(output), binary_sha256=sha(binary)))
            if any((v['stats'], v['output_sha256']) != (variants[0]['stats'], variants[0]['output_sha256']) for v in variants):
                raise ValueError(fixture+' original/native/sanitizer mismatch')
            report['results'][fixture] = dict(all_match=True, variants=variants)
        # Retain exact read-only disassembly supporting caller/producer boundaries.
        report['disassembly'] = {}
        for name, begin, end in [('owner-defaults', 0x465440, 0x46548a), ('pair-setters', 0x465500, 0x465643),
                                 ('resource-clear', 0x4ecbe0, 0x4ecc7e), ('resource-init', 0x4ecdd0, 0x4ecf1c),
                                 ('refresh-request', 0x4f2dd0, 0x4f2ddb)]:
            report['disassembly'][name] = run('disassemble-'+name, ['objdump', '-d', '-Mintel', '--start-address='+hex(begin), '--stop-address='+hex(end), pe])
        report['success'] = True
    except Exception as exc:
        report['error'] = str(exc)
    finally:
        try:
            run('original-after', [main_root/'tools/original-manifest.sh', 'verify'])
            report['original_manifest_verified_before_after'] = original_before
        except Exception as exc:
            report['success'] = False
            report['manifest_error'] = str(exc)
        report['sources_stable'] = all(sha(ROOT/p) == h for p, h in sources.items())
        report['inputs_stable'] = pe.is_file() and sha(pe) == BUILD
        report['success'] &= report['sources_stable'] and report['inputs_stable']
        digest = write_report(ROOT, report_path, (json.dumps(report, indent=2)+'\n').encode())
        # Verify payload through the SQLite-aware reader before closeout.
        if hashlib.sha256(read_bytes(report_path, root=ROOT)).hexdigest() != digest:
            raise ValueError('SQLite report verification failed')
        print(json.dumps(dict(success=report['success'], report=str(report_path), sha256=digest,
                              results={k: v['variants'][0]['stats'] for k, v in report['results'].items()}, error=report.get('error'))))
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
