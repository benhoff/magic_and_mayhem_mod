from pathlib import Path
import struct
import subprocess
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from copy_caller_provenance import decode, CAPACITY
from copy_caller_output import join_resolved_gpu


def fixture():
    h = [0]*32
    h[2:6] = [1, 128, 64, CAPACITY]
    h[6] = h[8] = h[10] = h[16] = h[19] = 1
    h[17:19] = [0x400000, 0x310000]
    header = bytearray(struct.pack('<32I', *h));header[:8] = b'MNMCCL01'
    row = [1, 0x58c05d, 0x18c05d, 0x18bf40, 92, 0x5597fd, 0x1597fd,
           0x6a4fe0, 0x12340, 1, 2, 0, 1, 0x23450, 0x34560, 0]
    raw = header+struct.pack('<16I', *row)+bytes((CAPACITY-1)*64)
    rh = [0]*32;rh[15:17] = h[17:19]
    route = [0]*44
    route[0:3] = row[0:3];route[4] = row[8];route[6:8] = [row[14], row[13]]
    route[10] = 4;route[11] = 1;route[15] = 2
    layout = [dict(entry_rva=row[3], return_slot_distance=92, apis=[dict(return_rva=row[2], fast=True)])]
    return raw, tuple(rh), [tuple(route)], layout


class CallerTests(unittest.TestCase):
    def test_v3_cursor_handoff_requires_version_and_exact_profile(self):
        args = list(self.v2());raw = bytearray(args[0]);raw[:8] = b'MNMCCL03'
        struct.pack_into('<I',raw,8,3);struct.pack_into('<I',raw,128+17*4,3)
        with self.assertRaisesRegex(ValueError,'five wrapper'):decode(raw,*args[1:])
        for word,value in ((1,0x58c279),(2,0x18c279),(3,0x18c140)):struct.pack_into('<I',raw,128+word*4,value)
        route=list(args[2][0]);route[1:3]=[0x58c279,0x18c279];args[2]=[tuple(route)]
        args[3][0].update(entry_rva=0x18c140,apis=[dict(return_rva=0x18c279,fast=True)])
        self.assertEqual(decode(raw,*args[1:])[1][0][17],3)
        for index,value in ((16,0),(17,4),(18,6),(19,0)):
            bad=bytearray(raw);struct.pack_into('<I',bad,128+index*4,value)
            with self.subTest(index=index),self.assertRaises(ValueError):decode(bad,*args[1:])
        raw[:8]=b'MNMCCL02';struct.pack_into('<I',raw,8,2)
        with self.assertRaises(ValueError):decode(raw,*args[1:])

    def test_cursor_route_requires_caller_diagnostic(self):
        tool=Path(__file__).resolve().parents[1]/'tools/run-rendering-acceptance-pilot.py'
        result=subprocess.run([sys.executable,str(tool),'--cursor-copy-callers','--report','invalid.json'],capture_output=True,text=True)
        self.assertEqual(result.returncode,2)
        self.assertIn('requires COPY caller diagnostic',result.stderr)

    def test_output_credit_requires_both_resolved_caller_and_complete_gpu_proof(self):
        _, _, routes, _ = fixture()
        _, callers = decode(*fixture())
        proven = {1: dict(call=1, first=10, last=12)}
        self.assertEqual(len(join_resolved_gpu(callers, routes, proven)), 1)
        self.assertEqual(join_resolved_gpu(callers, routes, {}), [])
        unknown = list(callers[0]);unknown[9] = 3
        self.assertEqual(join_resolved_gpu([unknown], routes, proven), [])
        cpu = list(routes[0]);cpu[15] = 1
        cpu_caller = list(callers[0]);cpu_caller[10] = 1
        self.assertEqual(join_resolved_gpu([cpu_caller], [cpu], {}), [])
        with self.assertRaisesRegex(ValueError, 'no GPU'):join_resolved_gpu([cpu_caller], [cpu], proven)
        with self.assertRaisesRegex(ValueError, 'bijection'):join_resolved_gpu([], routes, {})
        wrong = list(callers[0]);wrong[0] = 2
        with self.assertRaisesRegex(ValueError, 'ordinal'):join_resolved_gpu([wrong], routes, proven)

    def v2(self):
        raw, rh, routes, layouts = fixture()
        header = bytearray(raw[:128]);header[:8] = b'MNMCCL02'
        struct.pack_into('<I', header, 8, 2);struct.pack_into('<I', header, 16, 80)
        layouts[0]['hook_length'] = 5
        extended = struct.pack('<4I', 0x6a49e0, 2, 5, 0x12001000)
        return header+raw[128:192]+extended+bytes((CAPACITY-1)*80), rh, routes, layouts

    def test_v2_scene_handoff_and_identity_refusals(self):
        args = list(self.v2());h, rows = decode(*args)
        self.assertEqual((h[2], rows[0][17]), (2, 2))
        for index, value in ((16, 0), (17, 0), (17, 3), (18, 8), (19, 0)):
            raw = bytearray(args[0]);struct.pack_into('<I', raw, 128+index*4, value)
            with self.subTest(index=index), self.assertRaises(ValueError):decode(raw, *args[1:])

    def test_diagnostic_route_reaches_its_own_report_guard(self):
        tool = Path(__file__).resolve().parents[1]/'tools/run-rendering-acceptance-pilot.py'
        result = subprocess.run([sys.executable, str(tool), '--copy-callers', '--scenario', 'normal-quick',
                                 '--skip-intros', '--report', 'invalid.json'], capture_output=True, text=True)
        self.assertEqual(result.returncode, 2)
        self.assertIn('New owned immutable report required', result.stderr)

    def test_mixed_nominal_diagnostic_route_refuses(self):
        tool = Path(__file__).resolve().parents[1]/'tools/run-rendering-acceptance-pilot.py'
        result = subprocess.run([sys.executable, str(tool), '--copy-callers', '--operation-census',
                                 '--scenario', 'normal-quick', '--report', 'invalid.json'], capture_output=True, text=True)
        self.assertEqual(result.returncode, 2)
        self.assertIn('requires its own diagnostic route', result.stderr)

    def check_mutation(self, at, value):
        args = list(fixture());raw = bytearray(args[0]);struct.pack_into('<I', raw, at, value);args[0] = raw
        with self.assertRaises(ValueError):decode(*args)

    def test_exact_companion(self):
        h, rows = decode(*fixture())
        self.assertEqual(rows[0][5], 0x5597fd)
        self.assertEqual(h[10], 1)

    def test_unclosed_missed_overflow_and_counter_refusal(self):
        for index, value in ((16, 0), (7, 1), (9, 2), (8, 2), (19, 2), (10, 0), (20, 1)):
            with self.subTest(index=index):self.check_mutation(index*4, value)

    def test_module_identity(self):
        self.check_mutation(17*4, 0x500000)

    def test_ordinal_api_and_destination_identity(self):
        for index in (0, 1, 2, 8, 10, 11, 12, 13, 14, 15):
            with self.subTest(index=index):self.check_mutation(128+index*4, 9)

    def test_frame_layout_and_return_address(self):
        for index, value in ((3, 0x18c140), (4, 68), (5, 0x300000), (6, 0x150000), (7, 0), (9, 7)):
            with self.subTest(index=index):self.check_mutation(128+index*4, value)

    def test_wrong_known_api_method(self):
        args = list(fixture());args[3][0]['apis'][0]['fast'] = False
        with self.assertRaisesRegex(ValueError, 'method'):decode(*args)

    def test_unsupported_site_keeps_unknown(self):
        raw, rh, routes, layouts = fixture();raw = bytearray(raw)
        struct.pack_into('<II', raw, 10*4, 0, 1)
        for index in range(3, 8):struct.pack_into('<I', raw, 128+index*4, 0)
        struct.pack_into('<I', raw, 128+9*4, 2)
        _, rows = decode(raw, rh, routes, layouts)
        self.assertEqual(rows[0][9], 2)
        struct.pack_into('<I', raw, 128+5*4, 0x5597fd)
        with self.assertRaises(ValueError):decode(raw, rh, routes, layouts)

    def test_unused_tail_refusal(self):
        args = list(fixture());raw = bytearray(args[0]);raw[-1] = 1;args[0] = raw
        with self.assertRaises(ValueError):decode(*args)

    def test_truncated_extent(self):
        args = list(fixture());args[0] = args[0][:-1]
        with self.assertRaises(ValueError):decode(*args)


if __name__ == '__main__':unittest.main()
