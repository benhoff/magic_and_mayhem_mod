#!/usr/bin/env python3
"""Witnesses must match original journal bytes, full planes and exact ACKs."""
import copy
import importlib.util
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_api_physical import analyze
from drawing_api_session import sha

spec = importlib.util.spec_from_file_location('fixture', Path(__file__).with_name('test-drawing-api-session.py'))
fixture = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixture)


class WitnessTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        raw = bytearray(fixture.fixture())
        struct.pack_into('<I', raw, 32, 3)
        struct.pack_into('<II', raw, 64+72, 0xffffffff, 0xffffffff)
        struct.pack_into('<I', raw, 192+100, 192)
        self.request = bytes(raw[64:192])
        (self.root / 'session.bin').write_bytes(raw)
        (self.root / 'session.done').write_bytes(raw[-128:])
        (self.root / 'api-physical-request-000001.bin').write_bytes(self.request)
        (self.root / 'api-physical-ack-000001.bin').write_bytes(b'MNMAPI01'+struct.pack('<II', 1, 1))
        (self.root / 'api-physical-oracle-000001.rgba').write_bytes(bytes(192))
        self.peer = dict(success=True, original_draw_thread_fenced=True, native_rendering=False,
                         all_global_presentation_routes=False, source_inputs_bound=False,
                         boundary='observed_primary_api_physical', frames=[dict(
                             ordinal=1, record=1, entry=1, method=5, object=1, caller=0, thread=5,
                             width=8, height=6, path='api-physical-oracle-000001.rgba',
                             request_sha256=sha(self.request), sha256=sha(bytes(192)), stable_double_capture=True)])
        self.save()

    def save(self):
        (self.root / 'peer.json').write_text(json.dumps(self.peer))

    def run_admission(self):
        return analyze(self.root, self.root / 'peer.json')

    def test_scoped_full_plane_admission(self):
        result = self.run_admission()
        self.assertEqual(result['pixels'], 48)
        for k in ('driver_readbacks_enabled', 'all_global_presentation_routes_observed',
                  'source_inputs_bound', 'complete_presented_journey', 'semantic_input_outcomes_complete'):
            self.assertIs(result[k], False)

    def test_changed_request_ack_pixels_and_extra_artifacts_refuse(self):
        cases = [('api-physical-request-000001.bin', bytes(128)),
                 ('api-physical-ack-000001.bin', b'MNMAPI01'+struct.pack('<II', 2, 1)),
                 ('api-physical-oracle-000001.rgba', bytes(188))]
        for name, bad in cases:
            path = self.root / name
            old = path.read_bytes()
            path.write_bytes(bad)
            with self.assertRaises(ValueError): self.run_admission()
            path.write_bytes(old)
        (self.root / 'api-physical-ack-000002.bin').write_bytes(bytes(16))
        with self.assertRaises(ValueError): self.run_admission()

    def test_identity_type_hash_and_stability_refuse(self):
        old = copy.deepcopy(self.peer)
        for name, value in (('ordinal', True), ('method', 7), ('thread', 6),
                            ('request_sha256', '0'*64), ('sha256', '0'*64),
                            ('stable_double_capture', False)):
            self.peer = copy.deepcopy(old)
            self.peer['frames'][0][name] = value
            self.save()
            with self.assertRaises(ValueError): self.run_admission()

    def test_settling_samples_are_retained_and_bound_to_the_frame(self):
        plane=bytes([7])*192
        name='api-physical-transient-000001-01.rgba'
        (self.root/name).write_bytes(plane)
        sample=dict(ordinal=1,sample=1,width=8,height=6,path=name,sha256=sha(plane))
        self.peer['transient_samples']=[sample];self.peer['transient_bytes']=192
        self.peer['all_physical_changes_captured']=False
        self.peer['frames'][0]['settling_samples']=[sample]
        self.save()
        result=self.run_admission();self.assertEqual(result['transient_bytes'],192)
        self.assertFalse(result['all_physical_changes_captured'])
        (self.root/name).write_bytes(bytes(192))
        with self.assertRaises(ValueError):self.run_admission()
        (self.root/name).write_bytes(plane)
        self.peer['frames'][0]['settling_samples']=[];self.save()
        with self.assertRaises(ValueError):self.run_admission()
    def test_scope_promotion_and_incomplete_peer_refuse(self):
        old = copy.deepcopy(self.peer)
        for name, value in (('source_inputs_bound', True), ('all_global_presentation_routes', True),
                            ('native_rendering', True), ('success', False), ('frames', [])):
            self.peer = copy.deepcopy(old)
            self.peer[name] = value
            self.save()
            with self.assertRaises(ValueError): self.run_admission()


if __name__ == '__main__':
    unittest.main()
