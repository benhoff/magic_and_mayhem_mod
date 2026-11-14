"""Verify and stage an explicit derived scene without changing the base runtime."""
import hashlib
import json
from pathlib import Path
from coverage_evidence import Reader
from rendering_pilot import require
from secondary_world_caller_policy import BASE, configuration, derive_header
POLICY = 'patches/world-secondary-black-callers-v1.json'
BUILDER = 'tools/build-scene-observer.py'


def manifest_binding(build, manifest, derivation):
    require(build['success'] and build['sources_stable'] and build['base_header_unchanged']
            and build['base_runtime_artifact_unchanged'] and build['original_manifest_verified_before_after'],
            'Incomplete current scene variant build')
    require(build['kind'] == 'explicit_secondary_black_runtime_build_variant'
            and build['policy_tests'] == 15 and build['compiled_source_override_count'] == 1
            and build['new_exact_caller_pairs'] == 2 and build['source_hash_waivers'] == []
            and build['historical_hashes_overwritten'] is False
            and build['shared_runtime_source_files_changed'] is False
            and build['base_runtime_artifacts_changed'] is False
            and build['production_runtime_executed'] is False, 'Different derived runtime policy')
    guards = build['native_caller_guards']
    require(guards['all_old_pairs_preserved'] and guards['base']['success'] and guards['selected']['success']
            and (guards['base']['callers'], guards['base']['admitted_pairs'], guards['base']['refusal_controls']) == (54, 64, 4258)
            and (guards['selected']['callers'], guards['selected']['admitted_pairs'], guards['selected']['refusal_controls']) == (56, 66, 4412),
            'Incomplete derived caller positive/refusal controls')
    require(build['header_derivation'] == derivation
            and build['actual_compiled_header_sha256'] == derivation['derived_sha256']
            and build['derived_header']['sha256'] == derivation['derived_sha256'], 'Derived header identity changed')
    require(manifest['kind'] == 'explicit_derived_scene_compiler_inputs'
            and manifest['all_other_compiled_sources_unchanged'] is True
            and set(manifest['source_overrides']) == {BASE}, 'Different compiled override domain')
    physical = manifest['physical_derivation_inputs']
    expected = dict(physical, **{BASE: derivation['derived_sha256']})
    require(physical[BASE] == derivation['base_sha256'] and manifest['sources'] == expected,
            'Physical inputs and actual compiled source were conflated')
    override = manifest['source_overrides'][BASE]
    require(override == dict(base_sha256=derivation['base_sha256'], compiled_sha256=derivation['derived_sha256'],
                             derived_header=build['derived_header'], policy=POLICY), 'Compiled source recipe changed')
    require(manifest['sha256'] == build['scene']['sha256'] != build['old_scene_sha256']
            and manifest['compiler_overlay'] == build['compiler_overlay']
            and manifest['selected_builder'] == build['selected_builder'], 'Derived binary/recipe identity changed')
    return dict(verified=True, physical_header_sha256=physical[BASE],
                actual_compiled_header_sha256=derivation['derived_sha256'],
                compiled_overrides=1, scene_sha256=manifest['sha256'], base_scene_sha256=build['old_scene_sha256'],
                all_old_caller_pairs_preserved=True, new_exact_caller_pairs=2, source_hash_waivers=[])


def verify_variant(reader, root, reference, *, materialized=False):
    root = Path(root)
    require(reader.digest(reference['record']) == reference['sha256'], 'Scene variant reference changed')
    build = reader.json(reference['record'])
    require(all(reader.digest(n) == h for n, h in build['sources'].items()), 'Scene variant source input changed')
    policy = configuration(reader.json(POLICY))
    header, derivation = derive_header((root / BASE).read_bytes(), policy)
    require(reader.read_bytes(build['derived_header']['record']) == header, 'Compiled derived header differs')
    for key in ('compiled_manifest', 'compiler_overlay', 'selected_builder', 'scene', 'actual_preprocessed_canvas_producers'):
        require(reader.digest(build[key]['record']) == build[key]['sha256'], 'Derived build payload changed: ' + key)
    manifest = reader.json(build['compiled_manifest']['record'])
    proof = manifest_binding(build, manifest, derivation)
    require(all(reader.digest(n) == h for n, h in manifest['physical_derivation_inputs'].items()),
            'Physical compiler derivation input changed')
    base = build['base_build']
    require(reader.digest(base['record']) == base['sha256']
            and reader.json(base['record'])['scene']['sha256'] == build['old_scene_sha256'], 'Base build was replaced')
    overlay = reader.json(build['compiler_overlay']['record'])
    require(overlay['version'] == 0 and len(overlay['roots']) == 1
            and overlay['roots'][0]['type'] == 'file'
            and overlay['roots'][0]['name'] == str(root / BASE), 'Compiler mapped another source')
    preprocessed = reader.read_bytes(build['actual_preprocessed_canvas_producers']['record'])
    require(all(preprocessed.count(('0x%x,' % v['address']).encode()) == 1 for v in policy['additions']),
            'Actual preprocessed caller additions missing or duplicated')
    binary = Path(build['scene']['physical_path'])
    if materialized:
        require(binary.is_file() and hashlib.sha256(binary.read_bytes()).hexdigest() == build['scene']['sha256']
                and (binary.parent / 'manifest.json').read_bytes() == reader.read_bytes(build['compiled_manifest']['record']),
                'Materialized runtime binary/manifest differs from retained variant')
    return proof | dict(build=reference, compiled_manifest=build['compiled_manifest'],
                        policy=dict(record=POLICY, sha256=reader.digest(POLICY)),
                        materialized_checked=materialized, physical_scene=str(binary))


def choose_build(builder, original, selected, build, *, options=None, root=None):
    if builder != BUILDER:
        kwargs = {} if root is None else {'root': root}
        return original(builder, build, options=options, **kwargs)
    require(options is None, 'Derived scene does not authorize another builder option')
    return selected()


def variant_binding(base):
    class VariantBinding(base):
        def __init__(self, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self._variant_modules = set()
            self._variant_selections = []
            require('scene_runtime_variant' in self.plan, 'Explicit prospective scene variant required')

        def patch(self, module):
            module = super().patch(module)
            filename = getattr(module, '__file__', None)
            original = vars(module).get('build_adapter')
            if (not filename or not Path(filename).resolve().is_relative_to(self.root / 'tools')
                    or not callable(original) or id(module) in self._variant_modules):
                return module
            self._variant_modules.add(id(module))

            def selected():
                with Reader(self.root, memoize=True) as reader:
                    proof = verify_variant(reader, self.root, self.plan['scene_runtime_variant'], materialized=True)
                self._variant_selections.append(dict(module=Path(filename).relative_to(self.root).as_posix(), proof=proof))
                return Path(proof['physical_scene'])

            def adapter(builder, build, *, options=None, root=None):
                return choose_build(builder, original, selected, build, options=options, root=root)
            module.build_adapter = adapter
            return module

        def receipt(self):
            result = super().receipt()
            require(self._variant_selections and all(v['proof']['verified'] for v in self._variant_selections),
                    'No verified scene variant was selected for staging')
            result['scene_runtime_variant'] = dict(build=self.plan['scene_runtime_variant'],
                selections=list(self._variant_selections), base_runtime_sources_edited=False,
                actual_compiled_header_is_derived=True, additional_exact_caller_pairs=2,
                other_builders_actions_deadlines_and_capture_policy_changed=False,
                scope='Only explicit scene builder requests select this independently verified variant. The copied compiled manifest names the actual derived header and DLL. Physical base files and all other builder requests remain exact.')
            return result
    return VariantBinding
