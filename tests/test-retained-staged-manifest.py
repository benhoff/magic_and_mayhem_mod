#!/usr/bin/env python3
"""Separate retained installation TSV from the staged JSON without relaxing guards."""
import ast,hashlib,inspect,importlib.util,json,sys,unittest
from pathlib import Path
from unittest.mock import Mock,patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import broader_retained_staged_manifest as selected
spec=importlib.util.spec_from_file_location('retained_original_stage_controls',Path(__file__).with_name('test-secondary-staged-scene.py'));old=importlib.util.module_from_spec(spec);spec.loader.exec_module(old)
class Stage(old.Stage):pass
class Manifest(unittest.TestCase):
    def setUp(self):
        self.raw=b'sha256\tsize\tpath\n';self.reader=Mock();self.reader.read_bytes.return_value=self.raw
        self.acq={'launch_manifest':{'record':'tsv','sha256':hashlib.sha256(self.raw).hexdigest()},'session':'/owned/stage'}
        self.stage={'native_render':True,'resident_world':True};self.ref={'record':'retained-stage','sha256':'exact'}
    def run_binding(self):
        with patch.object(selected,'checked_artifact',return_value=(json.dumps(self.stage).encode(),self.ref))as f:
            value=selected.retained_manifest(self.reader,self.acq);f.assert_called_once_with(self.reader,self.acq,Path('/owned/stage/manifest.json'));return value
    def test_exact_retained_manifest(self):self.assertEqual(self.run_binding(),self.ref)
    def test_input_tsv_hash_still_required(self):
        self.acq['launch_manifest']['sha256']='wrong'
        with self.assertRaises(ValueError):self.run_binding()
    def test_wrong_tsv_encoding_refuses(self):
        self.reader.read_bytes.return_value=b'{}';self.acq['launch_manifest']['sha256']=hashlib.sha256(b'{}').hexdigest()
        with self.assertRaises(ValueError):self.run_binding()
    def test_native_render_required(self):
        self.stage['native_render']=False
        with self.assertRaises(ValueError):self.run_binding()
    def test_resident_world_required(self):
        self.stage['resident_world']=False
        with self.assertRaises(ValueError):self.run_binding()
    def test_missing_artifact_refuses(self):
        with patch.object(selected,'checked_artifact',side_effect=ValueError('Missing unique')):
            with self.assertRaises(ValueError):selected.retained_manifest(self.reader,self.acq)
    def test_exact_other_AST_preserved(self):
        original=inspect.getsource(selected.original);tree=ast.parse(selected.selected_source(original))
        class Restore(ast.NodeTransformer):
            def visit_Call(self,node):
                if isinstance(node.func,ast.Name)and node.func.id=='retained_manifest':return ast.parse(selected.OLD).body[0].value
                return self.generic_visit(node)
        self.assertEqual(ast.dump(Restore().visit(tree),include_attributes=False),ast.dump(ast.parse(original),include_attributes=False))
    def test_changed_assignment_refuses(self):
        with self.assertRaises(ValueError):selected.selected_source(inspect.getsource(selected.original).replace("manifest_ref = acquired['launch_manifest']","manifest_ref = acquired['other']"))
if __name__=='__main__':unittest.main()
