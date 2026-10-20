"""Shutdown acceptance must distinguish original Quit from process reclamation."""
from pathlib import Path
import struct
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from native_draw_retirement import read_retirement, require_retired, read_startup_retirement


class Retirement(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.path = Path(self.temp.name)/'retirement.bin'
        self.header = [0x444d4e4d,0x31544552,1,64,1,0,0,0,0,0,0,2,69,0,0,0]

    def write(self, header=None, rows=()):
        values = list(header or self.header)+[word for row in rows for word in row]
        self.path.write_bytes(struct.pack('<'+'I'*len(values), *values))

    def test_actual_clean_shutdown_has_positive_startup_and_no_survivors(self):
        self.write()
        self.assertTrue(require_retired(self.path)['resources_released'])
        self.assertTrue(read_retirement(self.path)['process_exit'])

    def test_normal_exit_with_device_and_surface_leak_is_not_retirement(self):
        header = list(self.header);header[5:7] = [1,1];header[10] = 960000
        rows = [[1,0x1234]+[0]*14, [2,0x5678]+[0]*14]
        self.write(header, rows)
        self.assertFalse(read_retirement(self.path)['resources_released'])
        with self.assertRaisesRegex(ValueError, 'survived normal Quit'):
            require_retired(self.path)

    def test_each_owner_class_and_pixel_allocation_blocks_acceptance(self):
        for index in range(7,11):
            header = list(self.header);header[index] = 1;self.write(header)
            with self.subTest(index=index), self.assertRaises(ValueError):
                require_retired(self.path)

    def test_missing_truncated_stale_schema_and_hidden_owner_inventory_refuse(self):
        with self.assertRaises(ValueError):require_retired(self.path)
        for raw in (b'', b'\0'*64, b'\0'*65):
            self.path.write_bytes(raw)
            with self.assertRaises(ValueError):require_retired(self.path)
        for index,value in ((2,2),(4,2),(5,65),(6,513),(11,0),(13,1),(14,1),(15,1)):
            header = list(self.header);header[index] = value;self.write(header)
            with self.subTest(index=index), self.assertRaises(ValueError):require_retired(self.path)
        self.write(rows=[[2,0x1234]+[0]*14])
        with self.assertRaises(ValueError):read_retirement(self.path)

    def test_symlink_cannot_substitute_another_session_receipt(self):
        self.write();other=self.path.with_name('other.bin');self.path.rename(other);self.path.symlink_to(other)
        with self.assertRaises(ValueError):require_retired(self.path)

    def test_startup_cleanup_needs_real_alias_release_and_original_result(self):
        header=[0x444d4e4d,0x31425250,1,64,0xbdb6f,2,0,1,1,1,0,1,0,1,0,0]
        self.write(header)
        self.assertEqual(read_startup_retirement(self.path)['surface_references_released'],2)
        for index,value in ((4,0),(5,0),(6,1),(10,1),(12,1),(13,0),(14,1),(15,1)):
            bad=list(header);bad[index]=value;self.write(bad)
            with self.subTest(index=index),self.assertRaises(ValueError):read_startup_retirement(self.path)


if __name__ == '__main__':unittest.main()
