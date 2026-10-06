#!/usr/bin/env python3
"""Independent v1 layout checks; no original artifacts or game launch required."""
import json
from pathlib import Path
import struct
import subprocess
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'python'))
from mnm_protocols import frame_v1 as frame, input_v1 as inputs, media_v1 as media, render_control_v1 as control


class Contracts(unittest.TestCase):
    def test_render_control(self):
        expected=b'MNMRCV01'+struct.pack('<II',1,576)+bytes(48)
        self.assertEqual(control.initial_header(),expected)
        self.assertTrue(control.valid_header(expected,576))
        self.assertFalse(control.valid_header(expected,575))
        self.assertEqual([control.LAUNCH_ID_OFFSET,control.REQUEST_SEQUENCE_OFFSET,
                          control.TARGET_SESSION_OFFSET,control.RESPONSE_SEQUENCE_OFFSET,
                          control.CANCEL_OFFSET,control.PATH_LENGTH_OFFSET,control.ONLINE_OFFSET,
                          control.PATH_OFFSET,control.PATH_SIZE],[16,20,28,32,40,44,48,64,512])

    def test_generated_files_current(self):
        subprocess.run([sys.executable, '-B', str(ROOT/'generate.py'), '--check'], check=True)

    def test_literal_identity_and_rejection(self):
        # Literal independently specified legacy headers, not generated expectations.
        fixtures = [(frame, bytes.fromhex('4d4e4d474c3030310100000040000000'), 16777280),
                    (inputs, bytes.fromhex('4d4e4d494e4b30310100000040040000'), 1088),
                    (media, bytes.fromhex('4d4e4d4d454430310100000000080000'), 2048)]
        for module, prefix, size in fixtures:
            with self.subTest(module=module.__name__):
                expected = prefix + bytes(48)
                self.assertEqual(module.initial_header(), expected)
                self.assertTrue(module.valid_header(expected, size))
                self.assertFalse(module.valid_header(expected, size-1))
                self.assertFalse(module.valid_header(expected[:15], size))
                for offset in (0, 8, 12):
                    wrong = bytearray(expected)
                    wrong[offset] ^= 0xff
                    self.assertFalse(module.valid_header(wrong, size))

    def test_independent_field_layouts(self):
        # Each tuple is (name, byte offset, byte length), frozen from baseline docs.
        common = [('magic', 0, 8), ('version', 8, 4), ('declared_size', 12, 4)]
        layouts = [
            (frame, common + list(zip(
                ['sequence', 'width', 'height', 'stride', 'pixel_format', 'status',
                 'frame_count', 'create_hresult', 'create_count', 'enumeration_hresult',
                 'enumeration_count', 'enumeration_kind'], range(16, 64, 4), [4]*12))
             + [('pixels', 64, 16777216)]),
            (inputs, common + list(zip(
                ['sequence', 'active', 'cursor_x', 'cursor_y', 'width', 'height'],
                range(16, 40, 4), [4]*6)) + [('reserved', 40, 24), ('keys', 64, 1024)]),
            (media, common + [
                ('request_sequence',16,4), ('request_id',20,4), ('operation',24,4),
                ('flags',28,4), ('path_length',32,4), ('reserved_request_0',36,8),
                ('sound_cancel_generation',44,4), ('cancelled_request_id',48,4),
                ('reserved_request_1',52,12), ('path',64,260), ('reserved_request_2',324,188),
                ('response_id',512,4), ('response_status',516,4), ('heartbeat',520,4),
                ('accepted_id',524,4), ('response_sequence',528,4), ('reserved_response',532,1516)])]
        for module, layout in layouts:
            schema = json.loads((ROOT/'schemas'/f"{module.__name__.split('.')[-1].replace('_', '-')}.json").read_text())
            self.assertEqual([(f['name'],f['offset'],f['size']) for f in schema['fields']], layout)
            for name, offset, size in layout:
                self.assertEqual(getattr(module, name.upper()+'_OFFSET'), offset)
                self.assertEqual(getattr(module, name.upper()+'_SIZE'), size)

    def test_independent_values_and_payloads(self):
        self.assertEqual((frame.PIXEL_FORMAT_RGBA8888, frame.MAX_WIDTH, frame.MAX_HEIGHT), (1,2048,2048))
        statuses = ['NOT_LOADED','FRAME_PUBLISHED','LOCK_FAILED','SURFACE_REJECTED','DLL_LOADED',
                    'HOOK_ARMED','INSIDE_CREATE','INTERFACE_INTERCEPTED','CREATE_FAILED','HOOK_FAILED',
                    'INTERCEPTION_FAILED','ENUMERATING','ENUMERATION_COMPLETE','ENUMERATION_FAILED']
        self.assertEqual([getattr(frame,'STATUS_'+s) for s in statuses], list(range(14)))
        self.assertEqual((frame.ENUMERATION_KIND_ENUMERATE_A,frame.ENUMERATION_KIND_ENUMERATE_EX_A),(1,2))
        self.assertEqual((inputs.KEY_COUNT, inputs.KEY_DOWN_MASK, inputs.KEY_GENERATION_MASK),
                         (256,0x80000000,0x7fffffff))
        self.assertEqual((inputs.HEARTBEAT_MS,inputs.LEASE_MS),(50,500))
        self.assertEqual((media.OPERATION_MOVIE,media.OPERATION_FILE_SOUND,media.OPERATION_STOP_FILE_SOUND),(1,2,3))
        self.assertEqual([getattr(media,'STATUS_'+s) for s in
                          ['LOADING','ACCEPTED','COMPLETE','CANCELLED','DECODER_ERROR','UNSUPPORTED','SOUND_BUSY']],
                         [1,2,3,4,5,6,7])
        self.assertEqual((media.MAX_PATH_LENGTH,media.HEARTBEAT_MS,media.POLL_MS,
                          media.STALE_HOST_MS,media.ACCEPTANCE_TIMEOUT_MS,media.PLAYBACK_TIMEOUT_MS),
                         (259,25,5,5000,10000,600000))
        self.assertEqual((media.SOUND_FLAG_ASYNC,media.SOUND_FLAG_NO_DEFAULT,media.SOUND_FLAG_LOOP,
                          media.SOUND_FLAG_NO_STOP,media.SOUND_FLAG_FILENAME,media.SOUND_ALLOWED_FLAGS),
                         (1,2,8,16,0x20000,0x2001b))
        key_bytes = bytearray(1088)
        struct.pack_into('<I',key_bytes,inputs.KEYS_OFFSET+65*4,inputs.KEY_DOWN_MASK|1)
        self.assertEqual(key_bytes[324:328],b'\x01\x00\x00\x80')
        response = bytearray(2048)
        struct.pack_into('<II',response,media.RESPONSE_ID_OFFSET,7,media.STATUS_ACCEPTED)
        self.assertEqual(response[512:520],b'\x07\x00\x00\x00\x02\x00\x00\x00')


if __name__ == '__main__':
    unittest.main()
