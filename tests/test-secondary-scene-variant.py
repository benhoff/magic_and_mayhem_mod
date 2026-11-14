#!/usr/bin/env python3
import copy
import importlib.util
import os
import tempfile
from types import SimpleNamespace
from unittest.mock import patch
from pathlib import Path
import sys
import unittest
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from broader_secondary_scene_variant import BASE, POLICY, BUILDER, manifest_binding, choose_build


def fixture():
    derivation = dict(base_sha256='base-header', derived_sha256='derived-header', insertion_offset=1,
                      inserted_bytes='00', existing_ordinary_callers=54, added_exact_pairs=2, all_base_bytes_retained=True)
    build = dict(success=True, sources_stable=True, base_header_unchanged=True,
        base_runtime_artifact_unchanged=True, original_manifest_verified_before_after=True,
        kind='explicit_secondary_black_runtime_build_variant', policy_tests=15,
        compiled_source_override_count=1, new_exact_caller_pairs=2, source_hash_waivers=[],
        historical_hashes_overwritten=False, shared_runtime_source_files_changed=False,
        base_runtime_artifacts_changed=False, production_runtime_executed=False,
        native_caller_guards=dict(all_old_pairs_preserved=True,
            base=dict(success=True, callers=54, admitted_pairs=64, refusal_controls=4258),
            selected=dict(success=True, callers=56, admitted_pairs=66, refusal_controls=4412)),
        header_derivation=copy.deepcopy(derivation), actual_compiled_header_sha256='derived-header',
        derived_header=dict(record='header', sha256='derived-header'), scene=dict(sha256='new-dll'),
        old_scene_sha256='old-dll', compiler_overlay=dict(record='overlay'), selected_builder=dict(record='builder'))
    manifest = dict(kind='explicit_derived_scene_compiler_inputs', all_other_compiled_sources_unchanged=True,
        source_overrides={BASE: dict(base_sha256='base-header', compiled_sha256='derived-header',
            derived_header=build['derived_header'], policy=POLICY)},
        physical_derivation_inputs={BASE: 'base-header', 'other.c': 'other'},
        sources={BASE: 'derived-header', 'other.c': 'other'}, sha256='new-dll',
        compiler_overlay=build['compiler_overlay'], selected_builder=build['selected_builder'])
    return build, manifest, derivation


class Manifest(unittest.TestCase):
    def setUp(self): self.build, self.manifest, self.derivation = fixture()
    def test_exact_physical_and_compiled_inputs_remain_distinct(self):
        proof = manifest_binding(self.build, self.manifest, self.derivation)
        self.assertTrue(proof['verified']); self.assertEqual(proof['new_exact_caller_pairs'], 2)
    def refuses(self):
        with self.assertRaises(ValueError): manifest_binding(self.build, self.manifest, self.derivation)
    def test_source_drift_refuses(self): self.build['sources_stable'] = False; self.refuses()
    def test_base_mutation_refuses(self): self.build['base_header_unchanged'] = False; self.refuses()
    def test_base_binary_mutation_refuses(self): self.build['base_runtime_artifact_unchanged'] = False; self.refuses()
    def test_guard_omission_refuses(self): self.build['native_caller_guards']['selected']['refusal_controls'] -= 1; self.refuses()
    def test_lost_base_pair_refuses(self): self.build['native_caller_guards']['base']['admitted_pairs'] -= 1; self.refuses()
    def test_source_waiver_refuses(self): self.build['source_hash_waivers'].append('runtime'); self.refuses()
    def test_forged_physical_header_refuses(self): self.manifest['physical_derivation_inputs'][BASE] = 'derived-header'; self.refuses()
    def test_hidden_compiled_header_difference_refuses(self): self.manifest['sources'][BASE] = 'base-header'; self.refuses()
    def test_another_compiled_source_change_refuses(self): self.manifest['sources']['other.c'] = 'changed'; self.refuses()
    def test_additional_override_refuses(self): self.manifest['source_overrides']['other.c'] = {}; self.refuses()
    def test_wrong_binary_refuses(self): self.manifest['sha256'] = 'other-dll'; self.refuses()
    def test_wrong_recipe_refuses(self): self.manifest['source_overrides'][BASE]['policy'] = 'other'; self.refuses()
    def test_refreshed_historical_hash_refuses(self): self.build['historical_hashes_overwritten'] = True; self.refuses()


class Dispatch(unittest.TestCase):
    def test_only_scene_selects_variant(self):
        self.assertEqual(choose_build(BUILDER, lambda *a, **kw: self.fail('base scene builder called'),
                                      lambda: 'selected', object()), 'selected')
    def test_other_builders_keep_exact_arguments(self):
        calls=[]; token=object(); root=Path('/tmp/owned')
        def original(*a, **kw): calls.append((a,kw)); return 'base'
        self.assertEqual(choose_build('tools/build-menu-observer.py', original,
            lambda: self.fail('other builder selected'), token, options={'same': True}, root=root), 'base')
        self.assertEqual(calls, [(('tools/build-menu-observer.py', token),dict(options={'same': True},root=root))])
    def test_scene_options_refuse_before_execution(self):
        with self.assertRaises(ValueError): choose_build(BUILDER, lambda *a, **kw: self.fail(), lambda: self.fail(), object(), options={'selftest': True})



class Staging(unittest.TestCase):
    def setUp(self):
        from broader_secondary_scene_variant import variant_binding
        class Base:
            def __init__(self, plan): self.root = ROOT; self.plan = plan
            def patch(self, module): return module
            def receipt(self): return {'existing': 'retained'}
        self.binding = variant_binding(Base)({'scene_runtime_variant': {'record': 'variant', 'sha256': 'identity'}})
    def module(self):
        return SimpleNamespace(__file__=str(ROOT / 'tools/prepare-menu-observer.py'),
            build_adapter=lambda *args, **kw: ('base', args, kw))
    def test_scene_staging_records_verified_selection(self):
        module=self.module(); self.binding.patch(module)
        proof=dict(verified=True, physical_scene='/tmp/scene/MnmScene.dll')
        with patch('broader_secondary_scene_variant.Reader'), patch('broader_secondary_scene_variant.verify_variant', return_value=proof) as verify:
            selected=module.build_adapter(BUILDER, object())
        self.assertEqual(selected, Path(proof['physical_scene']))
        self.assertTrue(verify.call_args.kwargs['materialized'])
        receipt=self.binding.receipt()
        self.assertEqual(receipt['existing'], 'retained')
        self.assertEqual(len(receipt['scene_runtime_variant']['selections']), 1)
    def test_patch_is_not_applied_twice(self):
        module=self.module(); self.binding.patch(module); once=module.build_adapter
        self.binding.patch(module); self.assertIs(module.build_adapter, once)
    def test_foreign_modules_are_not_wrapped(self):
        module=self.module(); module.__file__='/tmp/foreign.py'; original=module.build_adapter
        self.binding.patch(module); self.assertIs(module.build_adapter, original)
    def test_no_scene_selection_refuses_receipt(self):
        with self.assertRaises(ValueError): self.binding.receipt()
    def test_other_builder_does_not_record_scene_selection(self):
        module=self.module(); self.binding.patch(module)
        value=module.build_adapter('tools/build-menu-observer.py', object())
        self.assertEqual(value[0], 'base')
        with self.assertRaises(ValueError): self.binding.receipt()


class Bus(unittest.TestCase):
    def test_private_bus_relaunch_keeps_the_variant_launcher(self):
        path=ROOT/'tools/run-broader-secondary-world.py'
        spec=importlib.util.spec_from_file_location('variant_bus_control',path)
        module=importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
        with tempfile.TemporaryDirectory(prefix='secondary-variant-bus-') as tmp:
            module.ROOT=Path(tmp)
            class Relaunched(Exception): pass
            calls=[]
            def execute(file, argv, env): calls.append((file,argv,env)); raise Relaunched()
            with patch.dict(os.environ, {'AT_SPI_BUS_ADDRESS':'old'}, clear=True), patch.object(module.sys,'argv',[str(path),'--plan','phase','--report','research/runtime/case.json']), patch.object(module.os,'execvpe',execute):
                with self.assertRaises(Relaunched): module.main()
            self.assertEqual(calls[0][1][3],str(path))
            self.assertEqual(calls[0][2]['MNM_BROADER_WORLD_BUS_OWNED'],'1')
            self.assertNotIn('AT_SPI_BUS_ADDRESS',calls[0][2])
            self.assertTrue((Path(tmp)/'working/broader-player/world-runtime/case').is_dir())

if __name__ == '__main__': unittest.main()
