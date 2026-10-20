#!/usr/bin/env python3
"""Exercise the production World caller predicate against pinned PE32 bytes."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import struct
from coverage_claims import behavior_contract, required_sources

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
BEHAVIOR = 'NP.world-inherited-kind8-caller'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def declaration():
    r = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    b = next(b for b in r['behaviors'] if b['id'] == BEHAVIOR)
    claims = [dict(behavior=BEHAVIOR,
                   contract_sha256=behavior_contract(b, {v['id']: v for v in r['builds']}),
                   scenarios={})]
    paths = required_sources(b)
    for directory in ('runtime/scene', 'runtime/shadow', 'protocols/include'):
        paths.update(str(p.relative_to(ROOT)) for p in (ROOT/directory).rglob('*')
                     if p.is_file() and p.suffix in ('.c', '.h', '.S'))
    return claims, {p: sha(ROOT/p) for p in sorted(paths)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--executable', type=Path, default=ROOT/'original/Arcane_Nocd/Chaos.exe')
    args = parser.parse_args()
    out = args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working'):
        parser.error('Require new output under working/')
    out.mkdir(parents=True)
    claims, sources = declaration()
    report = dict(schema=1, success=False, claims=claims, sources=sources,
                  original_sha256=BUILD, original_instructions_executed=False,
                  scope='Production caller/tag/signature predicate: retain all existing pinned pairs, '
                        'admit inherited54aaa0 continuation54aab2 for additive tag26, and reject changed '
                        'bytes, missing bytes, other tags and unknown PCs. Original54aaa0 low16 return '
                        'over8316 RGB565 clipped/empty-intersection cases, native predicate execution '
                        'and production hook compilation; no pixel, natural-live, ownership or replacement proof.')
    (out/'prospective.json').write_text(json.dumps(report, indent=2)+'\n')

    def run(label, command):
        with (out/(label+'.log')).open('x') as log:
            subprocess.run([str(v) for v in command], cwd=ROOT, check=True,
                           stdout=log, stderr=subprocess.STDOUT, timeout=300)

    run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
    try:
        if sha(args.executable) != BUILD:
            raise ValueError('Unsupported original executable')
        run('build', ['g++', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror', '-pedantic',
                      ROOT/'tests/world-raster-callers-test.cpp', '-o', out/'test'])
        run('native', [out/'test', args.executable])
        result = json.loads((out/'native.log').read_text())
        if not result['success'] or not result['inherited_kind8_admitted']:
            raise ValueError('Incomplete caller controls')
        # An authored empty queue has no observed raster caller. The checked
        # inherited extension must still survive regeneration of the policy.
        header = b'MNMPRO01'+struct.pack('<14I',1,64,0x40209ca7,32768,1,*([0]*9))
        rows = []
        for sequence, operation in ((1,11),(2,12)):
            row = [0]*24
            row[0],row[1],row[2],row[14] = 96,sequence,operation,1
            rows.append(struct.pack('<24I',*row))
        (out/'authored-empty-queue.bin').write_bytes(header+b''.join(rows))
        run('generator', ['python3',ROOT/'tools/inspect-world-raster-callers.py',
                          '--stream',out/'authored-empty-queue.bin','--header',out/'generated.h',
                          '--report',out/'generated.json'])
        generated = json.loads((out/'generated.json').read_text())
        if generated['caller_count'] != 12 or sum(c['origin']=='checked_inherited_wrapper' for c in generated['callers']) != 1 or sum(c['origin']=='checked_effect_return' for c in generated['callers']) != 11:
            raise ValueError('Generator dropped or mislabeled the static inherited extension')
        (out/'generated-test.cpp').write_text(
            'using u32=unsigned;using u8=unsigned char;\n#include "generated.h"\n'
            'int main(){u8 b[]={0xc2,8,0,0x90,0x90,0x90,0x90,0x90,0x90,0x90};'
            'if(!mnm_world_raster_caller(0x54aab2,26,b)||mnm_world_raster_caller(0x54aab2,52,b))return 1;'
            'b[0]^=1;return mnm_world_raster_caller(0x54aab2,26,b);}\n')
        run('generated-build', ['g++','-std=c++17','-O2','-Wall','-Wextra','-Werror',
                                out/'generated-test.cpp','-o',out/'generated-test'])
        run('generated-native', [out/'generated-test'])
        run('return-reference-build', ['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror',
                                       '-fno-pie','-no-pie',ROOT/'tests/inherited-kind8-return-reference.cpp',
                                       '-o',out/'return-reference'])
        run('return-reference', [out/'return-reference',args.executable])
        reference = json.loads((out/'return-reference.log').read_text())
        if reference != dict(cases=8316,low16_values=[0]):
            raise ValueError('Inherited wrapper return differs from native zero policy')
        run('production-observer-build', ['python3', ROOT/'tools/build-scene-observer.py'])
        run('selftest-observer-build', ['python3', ROOT/'tools/build-scene-observer.py', '--selftest'])
        if declaration() != (claims, sources) or sha(args.executable) != BUILD:
            raise ValueError('Inputs or source contract changed during execution')
        report.update(result, sources_stable=True, production_and_selftest_hooks_compile=True,
                      generator_retains_unobserved_static_extension=True,
                      generator_input_is_authored_empty_queue=True,
                      original_instructions_executed=True,
                      original_return_cases=8316, original_low16_return_matches_native_zero=True)
    finally:
        run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
        report['original_manifest_verified_before_after'] = True
        report['artifacts'] = {p.name: sha(p) for p in out.iterdir() if p.is_file()}
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)


if __name__ == '__main__':
    main()
