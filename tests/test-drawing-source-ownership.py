#!/usr/bin/env python3
import sys,struct,unittest,copy,tempfile
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_source_ownership import configure,decode,analyze,REQUIRED_OPTIONS,validate_manifest


def journal():
    h=[0]*16;h[2:7]=[1,64,0x40209ca7,1023,2048]
    a=[1,1,7,1023]+[0]*12;b=[2,2,8,1023,1,1]+[0]*10
    raw=bytearray(struct.pack('<48I',*(h+a+b)));raw[:8]=b'MNMSOW01';return raw


class ProfileTests(unittest.TestCase):
    def test_positive_and_metadata_boundary(self):
        got=decode(journal());self.assertEqual(got['copies'],1);self.assertFalse(got['global_host_driver_alias_ownership'])
    def test_each_missing_capability(self):
        for row in [1,2]:
            for bit in range(10):
                raw=journal();struct.pack_into('<I',raw,row*64+12,1023^(1<<bit))
                with self.assertRaises(ValueError):decode(raw)
    def test_envelope_and_unfinished_or_extra_session(self):
        for raw in [journal()[:128],journal()+bytes(64),journal()[1:],b'']:
            with self.assertRaises(ValueError):decode(raw)
    def test_reserved_and_identity_corruption(self):
        for field in [2,3,4,5,6,7,16,17,18,20,32,33,34,36,37,38,47]:
            raw=journal();struct.pack_into('<I',raw,field*4,0 if field in [2,3,4,5,6,16,17,18,32,33,34,36,37] else 17)
            with self.assertRaises(ValueError):decode(raw)
    def test_refused_phase(self):
        raw=journal();struct.pack_into('<I',raw,132,3)
        with self.assertRaises(ValueError):decode(raw)
    def test_opt_in(self):
        a=SimpleNamespace(wine_source_ownership=False);configure(a);self.assertEqual(vars(a),dict(wine_source_ownership=False))
    def test_expands_all_prerequisites(self):
        a=SimpleNamespace(wine_source_ownership=True,observer_mode='physical',control=False,wine_client_memory=False)
        configure(a);self.assertTrue(all(getattr(a,k) for k in REQUIRED_OPTIONS))
    def test_incompatible_modes(self):
        a=SimpleNamespace(wine_source_ownership=True,observer_mode='physical',control=False,wine_client_memory=False)
        for key,value in [('observer_mode','metadata'),('control',True),('wine_client_memory',True)]:
            b=copy.copy(a);setattr(b,key,value)
            with self.assertRaises(ValueError):configure(b)
    def test_complete_manifest(self):
        m=dict.fromkeys(REQUIRED_OPTIONS,True);m.update(observer_mode='physical',api_observer_enabled=True)
        validate_manifest(m)
    def test_each_missing_or_malformed_manifest_dependency(self):
        m=dict.fromkeys(REQUIRED_OPTIONS,True);m.update(observer_mode='physical',api_observer_enabled=True)
        for key in [*REQUIRED_OPTIONS,'api_observer_enabled']:
            for bad in [None,False,'true',1]:
                with self.assertRaises(ValueError):validate_manifest(dict(m,**{key:bad}))
        for key,bad in [('observer_mode','metadata'),('wine_client_memory',True)]:
            with self.assertRaises(ValueError):validate_manifest(dict(m,**{key:bad}))
    def test_marker_needs_actual_handoff_proof(self):
        with tempfile.TemporaryDirectory() as tmp:
            directory=Path(tmp);(directory/'wine-source-ownership.bin').write_bytes(journal())
            with patch('drawing_source_handoff.analyze',return_value=dict(handoffs=2)):
                with self.assertRaises(ValueError):analyze(directory,{})
            with patch('drawing_source_handoff.analyze',side_effect=ValueError('Missing fence proof')):
                with self.assertRaises(ValueError):analyze(directory,{})



if __name__=='__main__':unittest.main()
