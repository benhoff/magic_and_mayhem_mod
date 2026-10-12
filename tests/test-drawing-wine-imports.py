#!/usr/bin/env python3
"""Selected installed Wine imports require exact pins and bounded PE metadata."""
import importlib.util
from pathlib import Path
import struct
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('wine_imports', ROOT/'tools/inspect-drawing-wine-imports.py')
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


class WineImports(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.ddraw = Path('/usr/lib/wine/i386-windows/ddraw.dll').read_bytes()
        cls.wined3d = Path('/usr/lib/wine/i386-windows/wined3d.dll').read_bytes()

    def test_selected_actual_import_export_boundary(self):
        x = m.inspect(self.ddraw, self.wined3d)
        self.assertEqual([r['iat_rva'] for r in x['selected']], [0x6d58c, 0x6d6ac])
        self.assertFalse(x['runtime_abi_validated'])
        self.assertFalse(x['source_submission_identity_bound'])

    def test_hash_drift_refuses_before_admission(self):
        for name in ('ddraw', 'wined3d'):
            blobs = dict(ddraw=self.ddraw, wined3d=self.wined3d)
            blobs[name] = blobs[name][:-1] + bytes([blobs[name][-1] ^ 1])
            with self.subTest(module=name), self.assertRaisesRegex(ValueError, 'pin mismatch'):
                m.inspect(**blobs)

    def test_malformed_headers_and_rva_refuse(self):
        pe = struct.unpack_from('<I', self.ddraw, 60)[0]
        for offset, value, fmt in ((0, 0, '<H'), (60, 0xffffffff, '<I'),
                                   (pe+4, 0x8664, '<H'), (pe+24, 0x20b, '<H'),
                                   (pe+6, 97, '<H')):
            raw = bytearray(self.ddraw)
            struct.pack_into(fmt, raw, offset, value)
            with self.subTest(offset=offset), self.assertRaises(ValueError):
                m.Pe32(raw)
        parsed = m.Pe32(self.ddraw)
        with self.assertRaises(ValueError):
            parsed.offset(0xffffffff, 4)

    def test_import_and_export_bounds_refuse(self):
        raw = bytearray(self.ddraw)
        pe = struct.unpack_from('<I', raw, 60)[0]
        struct.pack_into('<I', raw, pe+24+104, 0xffffffff)
        with self.assertRaises(ValueError):
            m.Pe32(raw).imports()
        raw = bytearray(self.wined3d)
        parsed = m.Pe32(raw)
        export = parsed.offset(parsed.directories[0][0], 40)
        struct.pack_into('<I', raw, export+24, 0xffffffff)
        with self.assertRaises(ValueError):
            m.Pe32(raw).exports()


if __name__ == '__main__':
    unittest.main()
