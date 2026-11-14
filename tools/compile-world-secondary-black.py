#!/usr/bin/env python3
"""Build a fully recorded two-caller runtime variant with one Clang VFS input."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import traceback
from types import SimpleNamespace
from coverage_evidence import Reader, write_report
from secondary_world_caller_policy import (BASE, ORIGINAL, configuration, derive_header,
                                         original_bindings, compiler_overlay, selected_builder)
from original_x86 import PE32
from rendering_pilot import require
ROOT = Path(__file__).resolve().parents[1]
P = 'research/runtime/broader-player-acceptance-results-20261110/'
POLICY = 'patches/world-secondary-black-callers-v1.json'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--report', required=True)
    args = parser.parse_args()
    out = args.output.absolute()
    require(args.report.startswith(P) and not (ROOT / args.report).exists(), 'Fresh owned report required')
    allowed = out.is_relative_to(ROOT / 'working') or out.is_relative_to('/dev/shm/mnm-whole-player-validation-20261110')
    require(allowed and not out.exists() and not out.is_symlink(), 'Fresh owned build directory required')
    require(shutil.disk_usage(out.parent).free > 2 * 1024**3, 'Build staging reserve')
    out.mkdir(parents=True)
    (out / '.retain').write_text('Exact derived runtime and compiler-input provenance; preserve until reviewed handoff.\n')
    q = dict(schema=1, kind='explicit_secondary_black_runtime_build_variant', success=False,
             original_game_runs=0, native_player_executions=0, original_raster_bodies_executed=False,
             shared_runtime_source_files_changed=False, base_runtime_artifacts_changed=False,
             commands=[], sources={}, scope=__doc__)
    inputs = {}
    old_dll = ROOT / 'working/build/scene-observer/MnmScene.dll'
    old_dll_hash = hashlib.sha256(old_dll.read_bytes()).hexdigest()
    base_header = (ROOT / BASE).read_bytes()
    prefix = args.report.removesuffix('.json')

    def artifact(name, body):
        path = prefix + '-' + name + '.bin'
        return dict(record=path, sha256=write_report(ROOT, path, body), bytes=len(body))

    def run(command, *pos, **kw):
        require(not pos and 'stdout' not in kw and 'stderr' not in kw, 'Unexpected build subprocess interface')
        check = kw.pop('check', False)
        done = subprocess.run(command, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                              timeout=120, **kw)
        q['commands'].append(dict(command=list(map(str, command)), returncode=done.returncode,
                                 log=artifact('command-' + str(len(q['commands'])), done.stdout)))
        if check: require(done.returncode == 0, done.stdout[-3000:].decode(errors='replace'))
        return done

    def manifest(label):
        done = run([str(ROOT / 'tools/original-manifest.sh'), 'verify'])
        require(done.returncode == 0, 'Original manifest refused: ' + label)

    try:
        manifest('before')
        original = (ROOT / 'original/Arcane_Nocd/Chaos.exe').read_bytes()
        require(hashlib.sha256(original).hexdigest() == ORIGINAL, 'Original build changed')
        policy = configuration(json.loads((ROOT / POLICY).read_bytes()))
        q['original_bindings'] = original_bindings(PE32(original), policy)
        derived, receipt = derive_header(base_header, policy)
        q['header_derivation'] = receipt
        derived_path = out / 'world-raster-callers-derived.h'
        derived_path.write_bytes(derived)
        q['derived_header'] = artifact('derived-header', derived)
        overlay = compiler_overlay(ROOT / BASE, derived_path)
        overlay_path = out / 'compiler-overlay.json'
        overlay_path.write_text(json.dumps(overlay, indent=2) + '\n')
        q['compiler_overlay'] = artifact('compiler-overlay', overlay_path.read_bytes())
        builder = ROOT / 'tools/build-scene-observer.py'
        selected = selected_builder(builder.read_text())
        q['selected_builder'] = artifact('selected-builder', selected.encode())
        # This source universe is the physical input to the explicit derivation;
        # the generated header is recorded separately as the compiled override.
        with Reader(ROOT, memoize=True) as reader:
            base_name = P + 'build-current-07.json'
            base = reader.json(base_name)
            require(base['success'] and base['scene']['sha256'] == old_dll_hash, 'Base runtime build changed')
            q['base_build'] = dict(record=base_name, sha256=reader.digest(base_name))
            names = set(base['runtime_sources']) | set(base['compiler_sources']) | {
                POLICY, 'tools/secondary_world_caller_policy.py', 'tools/compile-world-secondary-black.py',
                'tools/original_x86.py', 'tests/test-secondary-world-caller-policy.py',
                'tests/world-raster-callers-test.cpp', 'tests/world-secondary-black-continuation-original.cpp'}
            inputs = {n: (ROOT / n).read_bytes() for n in names}
            q['sources'] = {n: hashlib.sha256(body).hexdigest() for n, body in inputs.items()}
            require(all(q['sources'][n] == h for n, h in (base['runtime_sources'] | base['compiler_sources']).items()),
                    'Physical base build inputs changed')
        controls = run(['python3', '-B', 'tests/test-secondary-world-caller-policy.py'], check=True)
        require(b'Ran 15 tests' in controls.stdout and controls.stdout.rstrip().endswith(b'OK'), 'Policy controls incomplete')
        q['policy_tests'] = 15
        guards = []
        for mode in ('base', 'selected'):
            binary = out / ('caller-guard-' + mode)
            extra = [] if mode == 'base' else ['-DMNM_CALLER_HEADER="' + str(derived_path) + '"']
            run(['g++', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror', '-pedantic', *extra,
                 str(ROOT / 'tests/world-raster-callers-test.cpp'), '-o', str(binary)], check=True)
            done = run([str(binary), str(ROOT / 'original/Arcane_Nocd/Chaos.exe')], check=True)
            guards.append(json.loads(done.stdout))
            q[mode + '_guard_binary'] = artifact(mode + '-guard-executable', binary.read_bytes())
        require(all(v['success'] for v in guards) and guards[1]['callers'] == guards[0]['callers'] + 2
                and guards[1]['admitted_pairs'] == guards[0]['admitted_pairs'] + 2, 'Caller guard domain changed')
        q['native_caller_guards'] = dict(base=guards[0], selected=guards[1], all_old_pairs_preserved=True)
        ns = dict(__name__='secondary_black_selected_builder', __file__=str(builder),
                  _secondary_output=out, _secondary_overlay=overlay_path)
        exec(compile(selected, str(builder), 'exec'), ns)
        ns['subprocess'] = SimpleNamespace(run=run)
        dll = ns['build']()
        require(dll == out / 'MnmScene.dll' and dll.is_file(), 'Unexpected derived build output')
        # The ordinary builder hashes physical files. Retain that exact output,
        # then write an explicit manifest distinguishing physical inputs from
        # the bytes Clang actually compiled through the VFS mapping.
        physical_manifest = json.loads((out / 'manifest.json').read_bytes())
        q['original_physical_builder_manifest'] = artifact('physical-builder-manifest', (out / 'manifest.json').read_bytes())
        physical_sources = physical_manifest['sources']
        require(physical_sources[BASE] == receipt['base_sha256'], 'Physical header input changed')
        compiled_sources = dict(physical_sources, **{BASE: receipt['derived_sha256']})
        source = ROOT / 'runtime/scene/canvas_producers.c'
        preprocessor = run(['clang', '--target=i686-pc-windows-msvc', '-O2', '-ffreestanding',
            '-fno-builtin', '-fno-stack-protector', '-mno-sse', '-mno-mmx', '-ivfsoverlay', str(overlay_path),
            '-E', '-P', str(source)], check=True)
        for row in policy['additions']:
            require(('0x%x,' % row['address']).encode() in preprocessor.stdout, 'Compiler did not read the selected header')
        q['actual_preprocessed_canvas_producers'] = artifact('preprocessed-canvas-producers', preprocessor.stdout)
        manifest_value = dict(physical_manifest, schema=1, kind='explicit_derived_scene_compiler_inputs',
            sources=compiled_sources, physical_derivation_inputs=physical_sources,
            source_overrides={BASE: dict(base_sha256=receipt['base_sha256'], compiled_sha256=receipt['derived_sha256'],
                                       derived_header=q['derived_header'], policy=POLICY)},
            compiler_overlay=q['compiler_overlay'], selected_builder=q['selected_builder'],
            all_other_compiled_sources_unchanged=True)
        (out / 'manifest.json').write_text(json.dumps(manifest_value, indent=2) + '\n')
        q['compiled_manifest'] = artifact('compiled-manifest', (out / 'manifest.json').read_bytes())
        q['scene'] = dict(artifact('scene', dll.read_bytes()), physical_path=str(dll))
        require(q['scene']['sha256'] != old_dll_hash, 'Derived runtime did not change')
        q.update(success=True, compiled_source_override_count=1, new_exact_caller_pairs=2,
                 actual_compiled_header_sha256=receipt['derived_sha256'], old_scene_sha256=old_dll_hash,
                 Native_ELF_sha256=base['player']['sha256'], Native_ELF_unchanged=True,
                 all_old_runtime_source_bytes_retained=True, production_runtime_executed=False,
                 source_hash_waivers=[], historical_hashes_overwritten=False)
    except Exception as error:
        traceback.print_exc()
        q.update(success=False, error=str(error))
    finally:
        try:
            manifest('after')
            q['original_manifest_verified_before_after'] = True
        except Exception as error:
            q.update(success=False, original_manifest_error=str(error))
        q['sources_stable'] = bool(inputs) and all((ROOT / n).read_bytes() == body for n, body in inputs.items())
        q['base_header_unchanged'] = (ROOT / BASE).read_bytes() == base_header
        q['base_runtime_artifact_unchanged'] = hashlib.sha256(old_dll.read_bytes()).hexdigest() == old_dll_hash
        q['success'] &= q['sources_stable'] and q['base_header_unchanged'] and q['base_runtime_artifact_unchanged']
        q['scope'] = ('Explicit current scene variant: one Clang VFS header input adds only checked500a16/tag12 and500a32/tag17. '
            'Every physical base source, existing caller, compiler predicate and Native ELF is retained unchanged. '
            'The actual compiled header and manifest report their new hashes; physical derivation inputs keep their own identities. '
            'This is a new runtime binary/configuration, never a claim that it equals the base build. '
            'Original fragments and native positive/refusal controls remain separate; production execution and complete player acceptance are pending.')
        q['operator'] = artifact('operator', Path(__file__).read_bytes())
        digest = write_report(ROOT, args.report, (json.dumps(q, indent=2) + '\n').encode())
        print(json.dumps(dict(success=q['success'], sha256=digest, error=q.get('error'),
                             scene=q.get('scene'), guards=q.get('native_caller_guards'))), flush=True)
    return int(not q['success'])


if __name__ == '__main__': raise SystemExit(main())
