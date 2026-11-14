#!/usr/bin/env python3
import copy,hashlib,inspect,sys,unittest
from pathlib import Path
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
import broader_derived_admission_current as current
class Reader:
 def __init__(self,values):self.values=values;self.changed=set()
 def digest(self,name):return 'changed'if name in self.changed else'physical'if name==current.BASE else'h'
 def json(self,name):return copy.deepcopy(self.values[name])
class Join(unittest.TestCase):
 def setUp(self):
  self.phase={'record':'phase','sha256':'h'};self.acquisition={'record':'acq','sha256':'h'};self.build={'record':'build','sha256':'h'};self.manifest={'record':'manifest','sha256':'h'};self.header={'record':'header','sha256':'compiled'};self.mirror=ROOT/'working/broader-player/derived-compiler-inputs/compiled.h';self.compiled={'source_overrides':{current.BASE:{'physical':'physical','compiled':'compiled'}}}
  self.binding=dict(logical_path=current.BASE,physical_sha256='physical',compiled_sha256='compiled',materialized_path=self.mirror.relative_to(ROOT).as_posix(),exact_compiled_bytes=self.header,build=self.build,phase=self.phase,acquisition_path='acq',source_overrides=self.compiled['source_overrides'],compiled_manifest=self.manifest)
  native=dict(source_identity_policy=current.POLICY,original_admission_predicates_preserved=True,source_hash_waivers=[],selected_source_resolution_sha256=hashlib.sha256(current.selected_source(inspect.getsource(current.ORIGINAL_ADMIT)).encode()).hexdigest(),derived_compiler_input=self.binding,sources={current.BASE:'physical','other':'h'},actual_compiled_sources_by_binary={v[0]:{'other':'h'}for v in current.BUILDS.values()},binaries={v[0]:'h'for v in current.BUILDS.values()});native['binaries']['MnmScene.dll']='scene';native['actual_compiled_sources_by_binary']['MnmScene.dll'][current.BASE]='compiled'
  self.name=current.P+'case-actual-derived-admission-01.json';self.a=dict(success=True,sources_stable=True,actual_provider_verified=True,original_game_runs=1,broader_phase=self.phase,session='session',native_admission=dict(session='session',runner_identity={'pid':123},native_only=copy.deepcopy(native)),provider_adaptation=dict(native_only_derived_admission={'record':self.name,'sha256':'h'}))
  self.q=dict(kind='actual_owned_runner_derived_compiler_input_admission',success=True,phase=self.phase,acquisition_path='acq',session='session',Original_started_by_admission=False,runner_pid=123,sources={'helper':'h'},native_only=copy.deepcopy(native))
  self.reader=Reader({'acq':self.a,self.name:self.q,'manifest':self.compiled});self.plan=dict(broader_cases=[dict(id='case',report='acq')],scene_runtime_variant=self.build);self.proof=dict(scene_sha256='scene',physical_header_sha256='physical',actual_compiled_header_sha256='compiled');self.built=dict(compiled_manifest=self.manifest,derived_header=self.header)
  self.check=patch.object(current,'checked_configuration',return_value=(self.plan,{},self.proof,self.built,self.mirror));self.check.start();self.addCleanup(self.check.stop)
 def run_join(self):return current.verify_admission(self.reader,ROOT,self.phase,self.acquisition)
 def refuses(self):
  with self.assertRaises(ValueError):self.run_join()
 def mutate_both(self,fn):
  fn(self.a['native_admission']['native_only']);fn(self.q['native_only'])
 def test_complete_actual_binding(self):self.assertTrue(self.run_join()['verified'])
 def test_failed_acquisition_refuses(self):self.a['success']=False;self.refuses()
 def test_zero_original_setup_has_no_positive_credit(self):self.a['original_game_runs']=0;self.refuses()
 def test_unverified_provider_refuses(self):self.a['actual_provider_verified']=False;self.refuses()
 def test_foreign_acquisition_hash_refuses(self):self.reader.changed.add('acq');self.refuses()
 def test_wrong_actual_phase_refuses(self):self.q['phase']={};self.refuses()
 def test_wrong_stage_refuses(self):self.q['session']='other';self.refuses()
 def test_wrong_runner_refuses(self):self.q['runner_pid']=124;self.refuses()
 def test_stale_helper_refuses(self):self.reader.changed.add('helper');self.refuses()
 def test_mismatched_executed_admission_refuses(self):self.q['native_only']['source_identity_policy']='other';self.refuses()
 def test_hidden_compiled_header_in_physical_map_refuses(self):self.mutate_both(lambda n:n['sources'].__setitem__(current.BASE,'compiled'));self.refuses()
 def test_other_compiled_input_drift_refuses(self):self.reader.changed.add('other');self.refuses()
 def test_wrong_scene_binary_refuses(self):self.mutate_both(lambda n:n['binaries'].__setitem__('MnmScene.dll','other'));self.refuses()
 def test_omitted_other_binary_refuses(self):self.mutate_both(lambda n:n['binaries'].pop('MnmMenu.dll'));self.refuses()
 def test_omitted_actual_compiler_closure_refuses(self):self.mutate_both(lambda n:n['actual_compiled_sources_by_binary'].pop('MnmMenu.dll'));self.refuses()
 def test_other_binary_header_override_refuses(self):self.mutate_both(lambda n:n['actual_compiled_sources_by_binary']['MnmMenu.dll'].__setitem__(current.BASE,'compiled'));self.refuses()
 def test_different_compiled_scene_header_refuses(self):self.mutate_both(lambda n:n['actual_compiled_sources_by_binary']['MnmScene.dll'].__setitem__(current.BASE,'other'));self.refuses()
 def test_source_waiver_refuses(self):self.mutate_both(lambda n:n['source_hash_waivers'].append('header'));self.refuses()
 def test_unpreserved_other_guard_refuses(self):self.mutate_both(lambda n:n.__setitem__('original_admission_predicates_preserved',False));self.refuses()
 def test_changed_resolver_refuses(self):self.mutate_both(lambda n:n.__setitem__('selected_source_resolution_sha256','other'));self.refuses()
 def test_absent_acquisition_producer_receipt_refuses(self):self.a['provider_adaptation']['native_only_derived_admission']={};self.refuses()
 def test_wrong_materialized_recipe_refuses(self):self.mutate_both(lambda n:n['derived_compiler_input'].__setitem__('physical_sha256','compiled'));self.refuses()
if __name__=='__main__':unittest.main()
