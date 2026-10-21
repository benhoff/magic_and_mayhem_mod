#!/usr/bin/env python3
"""Execute original/native/runtime creature drawing lifecycle composition."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
from coverage_claims import behavior_contract, required_sources

ROOT = Path(__file__).resolve().parents[1]
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
IDS = ['RS.creature-draw-preparation', 'RS.creature-draw-position-history',
       'RS.scene-queue-retirement', 'NP.native-creature-drawing-adapter']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    out = args.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working/tests'):
        parser.error('Require a new output under working/tests')
    (out/'tmp').mkdir(parents=True)
    env = dict(os.environ, TMPDIR=str(out/'tmp'))
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behaviors = {b['id']: b for b in register['behaviors']}
    builds = {b['id']: b for b in register['builds']}
    names = set().union(*(required_sources(behaviors[i]) for i in IDS)) | {'tools/coverage_claims.py'}
    sources = {p: sha(ROOT/p) for p in sorted(names)}
    report = dict(schema=1, success=False, sources=sources, original_sha256=BUILD,
                  claims=[dict(behavior=i, contract_sha256=behavior_contract(behaviors[i], builds)) for i in IDS],
                  scope=__doc__, live_replacement=False, original_pixels_used_as_native_inputs=False)
    (out/'prospective.json').write_text(json.dumps(report, indent=2)+'\n')

    def run(label, command):
        print(label, flush=True)
        with (out/(label+'.log')).open('x') as log:
            subprocess.run([str(c) for c in command], cwd=ROOT, env=env, stdout=log,
                           stderr=subprocess.STDOUT, check=True, timeout=180)

    before = False
    try:
        run('original-before', [ROOT/'tools/original-manifest.sh', 'verify'])
        before = True
        original = ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(original) != BUILD:
            raise ValueError('Wrong original build')
        run('adapter-build', ['clang', '-m32', '-O2', '-ffreestanding', '-fno-builtin',
            '-fno-stack-protector', '-mno-sse', '-mno-mmx', '-Wall', '-Wextra', '-Werror',
            '-DMNM_CREATURE_DRAW_TEST', '-D__declspec(x)=', '-c',
            ROOT/'runtime/creature-drawing/adapter.c', '-o', out/'adapter.o'])
        run('entry-build', ['clang', '-m32', '-c', ROOT/'runtime/creature-drawing/entry.S', '-o', out/'entry.o'])
        run('composition-build', ['g++', '-m32', '-std=c++17', '-O2', '-fno-pie', '-no-pie',
            '-Wall', '-Wextra', '-Werror', ROOT/'tests/creature-drawing-composition.cpp',
            ROOT/'tests/creature-drawing-call.S', out/'adapter.o', out/'entry.o', '-o', out/'composition'])
        run('composition', [out/'composition', original])
        result = json.loads((out/'composition.log').read_text())
        if not result['success'] or result['cases'] != 2240 or not result['original_tails_poisoned']:
            raise ValueError('Incomplete lifecycle comparison')
        run('pe32-build', ['python3', ROOT/'tools/build-creature-drawing.py', '--output', out/'pe32'])
        # Exercise the same import helper used by scene staging, without a full
        # installed-game copy. Original file bytes remain read-only.
        spec = importlib.util.spec_from_file_location('creature_stage', ROOT/'tools/prepare-shadow-experiment.py')
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        staged = module.add_import(original.read_bytes(), 'MnmCreatureDrawing.dll',
                                   'CreatureDrawingAnchor', b'.mncreat')
        raw = original.read_bytes()
        pe = struct.unpack_from('<I', raw, 60)[0]
        sections = pe+24+struct.unpack_from('<H', raw, pe+20)[0]
        for i in range(struct.unpack_from('<H', raw, pe+6)[0]):
            size, offset = struct.unpack_from('<II', raw, sections+i*40+16)
            if staged[offset:offset+size] != raw[offset:offset+size]:
                raise ValueError('Import staging modified an original section')
        (out/'staged.exe').write_bytes(staged)
        if b'MnmCreatureDrawing.dll\0' not in staged:
            raise ValueError('Missing staged native import')
        run('prepare-cli', ['python3', '-B', ROOT/'tools/prepare-scene-observer.py', '--help'])
        run('capture-cli', ['python3', '-B', ROOT/'tools/capture-scene-game.py', '--help'])
        if any('--native-creature-drawing' not in (out/(n+'.log')).read_text()
               for n in ('prepare-cli', 'capture-cli')):
            raise ValueError('Missing exposed creature drawing option')
        report.update(success=True, result=result, pe32_dll_sha256=sha(out/'pe32/MnmCreatureDrawing.dll'),
                      import_staging_pass=True, staged_executable_sha256=sha(out/'staged.exe'),
                      original_helpers_unchanged_during_oracle=True,
                      runtime_entry_bypass_executed_in_owned_fixture=True)
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        report['error'] = str(error)
    finally:
        if before:
            try:
                run('original-after', [ROOT/'tools/original-manifest.sh', 'verify'])
                report['original_manifest_verified_before_after'] = True
            except subprocess.SubprocessError as error:
                report.update(success=False, preservation_error=str(error))
        report['sources_stable'] = all(sha(ROOT/n) == value for n, value in sources.items())
        report['success'] &= report['sources_stable']
        (out/'report.json').write_text(json.dumps(report, indent=2)+'\n')
        print(out/'report.json', flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
