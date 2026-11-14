"""Require actual staged scene provenance before a derived-runtime case can admit."""
import hashlib
import json
from pathlib import Path
from rendering_pilot import require
from broader_secondary_scene_variant import verify_variant, manifest_binding, POLICY
from secondary_world_caller_policy import BASE, derive_header, configuration


def staged_values(acquired, stage, compiled, variant, phase, expected):
    require(acquired['success'] and acquired['sources_stable'] and acquired['actual_provider_verified']
            and acquired['broader_phase'] == phase, 'Not a complete current acquisition in this phase')
    receipt = acquired['provider_adaptation']['scene_runtime_variant']
    require(receipt['build'] == expected and receipt['base_runtime_sources_edited'] is False
            and receipt['actual_compiled_header_is_derived'] is True
            and receipt['additional_exact_caller_pairs'] == 2
            and receipt['other_builders_actions_deadlines_and_capture_policy_changed'] is False,
            'Unreviewed runtime selection or execution changes')
    require(receipt['selections'] and all(v['proof']['verified'] and v['proof']['build'] == expected
            and v['proof']['scene_sha256'] == variant['scene_sha256']
            and v['proof']['actual_compiled_header_sha256'] == variant['actual_compiled_header_sha256']
            and v['proof']['source_hash_waivers'] == [] and v['proof']['materialized_checked']
            for v in receipt['selections']), 'Staging lacks exact materialized variant selections')
    require(stage['resident_world'] is True and stage['native_only_policy'] == 'native-only-rendering-v1'
            and stage['glyph_backend_mode'] == 'takeover'
            and stage['resident_scene_sha256'] == stage['glyph_dll_sha256'] == variant['scene_sha256'],
            'Actual staged scene/glyph DLL differs from declared variant')
    require(compiled['sha256'] == variant['scene_sha256']
            and compiled['sources'][BASE] == variant['actual_compiled_header_sha256']
            and compiled['physical_derivation_inputs'][BASE] == variant['physical_header_sha256'],
            'Actual staged compiler manifest hides a source difference')
    return dict(verified=True, phase=phase, build=expected, scene_sha256=variant['scene_sha256'],
                actual_compiled_header_sha256=variant['actual_compiled_header_sha256'],
                physical_header_sha256=variant['physical_header_sha256'],
                exact_scene_and_glyph_staging=True, source_hash_waivers=[],
                physical_base_sources_changed=False, compiled_header_difference_explicit=True)


def checked_artifact(reader, acquired, expected_path):
    expected_path = Path(expected_path)
    candidates = [v for v in acquired['artifacts'] if (reader.root / v['path']) == expected_path]
    require(len(candidates) == 1, 'Missing unique exact staged artifact: ' + str(expected_path))
    value = candidates[0]
    raw = reader.read_bytes(value['record'])
    require(hashlib.sha256(raw).hexdigest() == value['sha256'], 'Staged artifact payload changed')
    return raw, dict(record=value['record'], sha256=value['sha256'])


def verify_staged(reader, root, phase_ref, acquisition_ref, validation_ref=None):
    root = Path(root)
    for ref in (phase_ref, acquisition_ref):
        require(reader.digest(ref['record']) == ref['sha256'], 'Phase/acquisition input changed')
    phase, acquired = reader.json(phase_ref['record']), reader.json(acquisition_ref['record'])
    require('scene_runtime_variant' in phase, 'No prospectively declared scene variant')
    variant_ref = phase['scene_runtime_variant']
    variant = verify_variant(reader, root, variant_ref)
    build = reader.json(variant_ref['record'])
    manifest_ref = acquired['launch_manifest']
    require(reader.digest(manifest_ref['record']) == manifest_ref['sha256'], 'Launch manifest changed')
    stage = reader.json(manifest_ref['record'])
    raw, compiled_ref = checked_artifact(reader, acquired, Path(acquired['session']) / 'scene-bridge-build.json')
    require(raw == reader.read_bytes(build['compiled_manifest']['record']), 'Staged compiled manifest is not the selected build')
    compiled = json.loads(raw)
    header, derivation = derive_header((root / BASE).read_bytes(), configuration(reader.json(POLICY)))
    manifest_binding(build, compiled, derivation)
    proof = staged_values(acquired, stage, compiled, variant, phase_ref, variant_ref)
    if validation_ref is not None:
        require(reader.digest(validation_ref['record']) == validation_ref['sha256'], 'Validation changed')
        validation = reader.json(validation_ref['record'])
        require(validation['success'] and validation['sources_stable']
                and validation['acquisition'] == acquisition_ref, 'Incomplete or foreign base validation')
    return proof | dict(acquisition=acquisition_ref, validation=validation_ref,
                        launch_manifest=manifest_ref, staged_compiler_manifest=compiled_ref,
                        original_game_runs=0, new_native_executions=0,
                        scope='Current full-source and base semantic/pixel validation remain mandatory. This additional join binds the actual staged scene/glyph DLL and explicit derived compiled header to the prospectively selected configuration. No prior source hash is rewritten and no old binary is represented as the new variant.')
