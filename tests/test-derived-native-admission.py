#!/usr/bin/env python3
import ast,copy,hashlib,importlib.util,inspect,json,os,subprocess,sys,tempfile,unittest
from pathlib import Path
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
import broader_derived_admission as derived
import native_only_contract as original
sp=importlib.util.spec_from_file_location('derived_baseline_fixture',ROOT/'tests/test-native-only-contract.py');baseline=importlib.util.module_from_spec(sp);sp.loader.exec_module(baseline)
class Admission(unittest.TestCase):
 def setUp(self):
  self.base=baseline.Contract('test_startup_selects_all_required_capabilities');self.base.setUp();self.addCleanup(self.base.doCleanups)
  self.repo=self.base.repo;self.stage=self.repo/'working/experiments/menu-observer/control';self.stage.parent.mkdir(parents=True);self.base.root.rename(self.stage);self.base.root=self.stage
  self.source=self.repo/derived.BASE;self.source.parent.mkdir(parents=True);self.source.write_bytes(b'physical-base');self.base_sha=hashlib.sha256(b'physical-base').hexdigest();self.derived_raw=b'exact-compiled-derived';self.derived_sha=hashlib.sha256(self.derived_raw).hexdigest();self.mirror=derived.mirror_path(self.repo,self.derived_sha);self.mirror.parent.mkdir(parents=True);self.mirror.write_bytes(self.derived_raw)
  self.scene=json.loads((self.stage/'scene-bridge-build.json').read_bytes());self.scene['sources'][derived.BASE]=self.derived_sha;self.scene['source_overrides']={derived.BASE:{'base_sha256':self.base_sha,'compiled_sha256':self.derived_sha}};self.scene['physical_derivation_inputs']=dict(self.scene['sources'],**{derived.BASE:self.base_sha});self.write_scene()
  self.retained_manifest=(self.stage/'scene-bridge-build.json').read_bytes();self.phase={'scene_runtime_variant':{'record':'build','sha256':'build'}};self.proof=dict(scene_sha256=self.scene['sha256'],actual_compiled_header_sha256=self.derived_sha,physical_header_sha256=self.base_sha);self.built=dict(compiled_manifest={'record':'manifest'},derived_header={'record':'header'})
  owner=self
  class Reader:
   def read_bytes(self,n):return owner.retained_manifest if n=='manifest'else owner.derived_raw
   def digest(self,n):return hashlib.sha256((owner.repo/n).read_bytes()).hexdigest()
  self.reader=Reader();self.check=patch.object(derived,'checked_configuration',return_value=(self.phase,{},self.proof,self.built,self.mirror));self.check.start();self.addCleanup(self.check.stop)
 def write_scene(self):(self.stage/'scene-bridge-build.json').write_text(json.dumps(self.scene))
 def run_admit(self):return derived.admit_derived(self.reader,self.repo,{'record':'phase','sha256':'phase'},'report',self.stage,self.repo,self.base.metadata,self.base.env)
 def refuses(self):
  with self.assertRaises(ValueError):self.run_admit()
 def test_actual_compiled_and_physical_sources_are_explicitly_separate(self):
  q=self.run_admit();self.assertEqual(q['sources'][derived.BASE],self.base_sha);self.assertEqual(q['actual_compiled_sources_by_binary']['MnmScene.dll'][derived.BASE],self.derived_sha);self.assertEqual(q['source_identity_policy'],derived.POLICY);self.assertEqual(q['source_hash_waivers'],[]);self.assertFalse(q['complete_replacement'])
 def test_original_default_guard_still_refuses_derived_header(self):
  with self.assertRaisesRegex(ValueError,'Native-only build source changed'):original.admit(self.stage,self.repo,self.base.metadata,self.base.env)
 def test_physical_base_change_refuses(self):self.source.write_bytes(b'changed');self.refuses()
 def test_materialized_derived_change_refuses(self):self.mirror.write_bytes(b'changed');self.refuses()
 def test_staged_compiler_manifest_change_refuses(self):self.scene['source_overrides']['other']={};self.write_scene();self.refuses()
 def test_scene_binary_change_refuses(self):(self.stage/'game/MnmScene.dll').write_bytes(b'changed');self.refuses()
 def test_other_binary_change_still_refuses(self):(self.stage/'game/MnmRender.dll').write_bytes(b'changed');self.refuses()
 def test_other_source_change_still_refuses(self):self.base.source.write_bytes(b'changed');self.refuses()
 def test_missing_capability_still_refuses(self):self.base.metadata['native_draw']=False;self.refuses()
 def test_missing_required_environment_still_refuses(self):self.base.env.pop('MNM_NATIVE_ONLY');self.refuses()
 def test_wrong_glyph_mode_still_refuses(self):self.base.metadata['glyph_backend_mode']='shadow';self.refuses()
 def test_original_terminal_capability_guard_still_refuses(self):
  p=self.stage/'native-draw-build.json';q=json.loads(p.read_bytes());q['directdraw_imports']=True;p.write_text(json.dumps(q));self.refuses()
 def test_extra_override_in_other_binary_refuses(self):
  p=self.stage/'render-bridge-build.json';q=json.loads(p.read_bytes());q['source_overrides']={'unreviewed':{}};p.write_text(json.dumps(q));self.refuses()
 def test_wrong_repository_refuses(self):
  with self.assertRaises(ValueError):derived.admit_derived(self.reader,self.repo,{},'report',self.stage,self.repo/'foreign',self.base.metadata,self.base.env)
 def test_materialized_input_parent_cannot_escape_repository(self):
  external=tempfile.TemporaryDirectory();self.addCleanup(external.cleanup);p=Path(external.name);(p/self.mirror.name).write_bytes(self.derived_raw);parent=self.mirror.parent;parent.rename(parent.with_name('original-inputs'));parent.symlink_to(p,target_is_directory=True);self.refuses()
 def test_input_objects_are_not_mutated(self):
  metadata=copy.deepcopy(self.base.metadata);env=copy.deepcopy(self.base.env);self.run_admit();self.assertEqual(metadata,self.base.metadata);self.assertEqual(env,self.base.env)
class Source(unittest.TestCase):
 def test_inverse_AST_changes_only_the_source_resolution_assignment(self):
  source=inspect.getsource(derived.ORIGINAL_ADMIT);selected=ast.parse(derived.selected_source(source));count=0
  for node in ast.walk(selected):
   if isinstance(node,ast.Assign)and len(node.targets)==1 and isinstance(node.targets[0],ast.Name)and node.targets[0].id=='source':
    self.assertIsInstance(node.value,ast.Call);self.assertEqual(node.value.func.id,'resolve_compiled_input');node.value=ast.parse('(repository/path).resolve()',mode='eval').body;count+=1
  self.assertEqual(count,1);self.assertEqual(ast.dump(ast.parse(source),include_attributes=False),ast.dump(selected,include_attributes=False))
 def test_changed_base_expression_refuses(self):
  with self.assertRaises(ValueError):derived.selected_source(inspect.getsource(derived.ORIGINAL_ADMIT).replace('(repository/path).resolve()','repository/path'))
class Startup(unittest.TestCase):
 def invoke(self,args,raw):
  env=dict(os.environ,PYTHONPATH=str(ROOT/Path(derived.STARTUP).parent)+os.pathsep+str(ROOT/'tools'));env[derived.FLAG]=raw
  return subprocess.run([sys.executable,*args],env=env,capture_output=True)
 def test_unrelated_child_is_unchanged(self):
  q=self.invoke(['-c','print("ordinary")'],json.dumps({'root':str(ROOT)}));self.assertEqual(q.returncode,0);self.assertEqual(q.stdout,b'ordinary\n')
 def test_malformed_selection_fails_closed(self):
  q=self.invoke(['-c','print("bad")'],'{');self.assertEqual(q.returncode,78);self.assertNotIn(b'bad',q.stdout)
 def test_prior_startup_receives_actual_preparer_argument_identity(self):
  with tempfile.TemporaryDirectory()as tmp:
   root=Path(tmp);prior=root/'patches/secondary-scene-python-startup/sitecustomize.py';prior.parent.mkdir(parents=True);prior.write_text('import sys;print("startup-argv="+sys.argv[0])');target=root/'tools/prepare-menu-observer.py';target.parent.mkdir();target.write_text('print("preparer-main")')
   q=self.invoke([str(target)],json.dumps({'root':str(root)}));self.assertEqual(q.returncode,0);self.assertEqual(q.stdout,('startup-argv='+str(target)+'\npreparer-main\n').encode())
 def test_real_owned_runner_is_intercepted_before_its_main(self):
  q=self.invoke([str(ROOT/'tools/owned-menu-game-runner.py')],json.dumps({'root':str(ROOT)}));self.assertEqual(q.returncode,78);self.assertIn(b'derived admission startup refused',q.stderr)
if __name__=='__main__':unittest.main()
