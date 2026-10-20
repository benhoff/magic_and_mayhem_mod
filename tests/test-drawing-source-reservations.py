import struct
import sys
import copy
import tempfile
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_source_reservations import decode, analyze
from drawing_gpu_completion import decode as decode_gpu
from drawing_source_version import decode as decode_version


def fixture():
    h = [0]*16
    h[2:8] = [1, 128, 0x40209ca7, 131072, 2048, 4]
    raw = bytearray(struct.pack('<16I', *h));raw[:8] = b'MNMSRG01'
    for phase in range(1, 8):
        r = [0]*32;r[:4] = [phase, phase, 10, 100]
        if phase < 7:
            r[4:11] = [1, 9, 2, 3, 4, 0, 1]
            r[11:15] = [int(phase >= 2), 2*int(phase >= 3), 3*int(phase >= 4), phase]
        else:r[15] = 1
        r[17:19] = [1, int(phase >= 6)]
        raw += struct.pack('<32I', *r)
    return raw


class Reservations(unittest.TestCase):
    def test_complete(self):
        r = decode(fixture());self.assertEqual(r['reservations'], 1)
        self.assertFalse(r['physical_backing_owned'])
        self.assertFalse(r['unobserved_writers_closed'])

    def test_missing_or_reordered_fence(self):
        for phase in (3, 4, 5, 6):
            raw = fixture();del raw[64+(phase-1)*128:64+phase*128]
            with self.assertRaises(ValueError):decode(raw)

    def test_changed_identity(self):
        for phase in range(2, 7):
            for field in (4, 5, 6, 7, 8, 9, 10):
                raw = fixture();offset = 64+(phase-1)*128+field*4
                struct.pack_into('<I', raw, offset, 100)
                with self.assertRaises(ValueError):decode(raw)

    def test_changed_consumer(self):
        for phase, field in ((3, 11), (4, 12), (5, 13), (5, 2), (6, 13)):
            raw = fixture();struct.pack_into('<I', raw, 64+(phase-1)*128+field*4, 100)
            with self.assertRaises(ValueError):decode(raw)

    def test_refused_or_incomplete(self):
        raw = fixture();struct.pack_into('<I', raw, 64+5*128+4, 8)
        with self.assertRaises(ValueError):decode(raw)
        with self.assertRaises(ValueError):decode(fixture()[:-128])

    def test_reserved_and_terminal_counts(self):
        for phase, field in ((1, 19), (6, 18), (7, 17), (7, 4)):
            raw = fixture();struct.pack_into('<I', raw, 64+(phase-1)*128+field*4, 100)
            with self.assertRaises(ValueError):decode(raw)


def gpu_fixture(wait=0x911c):
    h = [0]*16;h[2:7] = [1, 128, 0x40209ca7, 131072, 1000000000]
    raw = bytearray(struct.pack('<16I', *h));raw[:8] = b'MNMWGC01'
    r = [0]*32
    r[:18] = [1, 1, 3, 2, 1, 10, 10, 12, 2, 1, wait, 2, 7, 1, 1, 1, 1000000000, 0]
    t = [0]*32;t[:4] = [2, 4, 1, 1];t[10] = 1
    return raw+struct.pack('<32I', *r)+struct.pack('<32I', *t)


def version_fixture():
    h = [0]*16;h[2:8] = [1, 128, 0x40209ca7, 131072, 2048, 4]
    raw = bytearray(struct.pack('<16I', *h));raw[:8] = b'MNMSVG01'
    for ordinal, phase in enumerate((3, 4, 5, 6, 7), 1):
        r = [0]*32;r[:4] = [ordinal, phase, 10, 100]
        if phase == 7:
            r[4] = r[13] = 1
        else:
            r[4:14] = [1, 9, 2, 3, 4, 0, 1, 1, 0, 1];r[22] = 3
            if phase >= 4:r[14] = 2
            if phase >= 5:r[15] = 3;r[19:22] = [5, 1, 0]
        raw += struct.pack('<32I', *r)
    return raw


class EvidenceAdmission(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.reservations = fixture();self.gpu = gpu_fixture();self.version = version_fixture()
        for name, raw in [('wine-source-reservations.bin', self.reservations),
                          ('wine-gpu-completion.bin', self.gpu),
                          ('wine-source-version.bin', self.version)]:
            (self.root/name).write_bytes(raw)
        self.report = dict(wine_source_version_observation=decode_version(self.version),
                           wine_gpu_completion_observation=decode_gpu(self.gpu))

    def changed_word(self, name, raw, offset, value):
        data = bytearray(raw);struct.pack_into('<I', data, offset, value)
        (self.root/name).write_bytes(data)
        return data

    def test_complete_raw_join_and_both_signal_results(self):
        for wait in (0x911a, 0x911c):
            raw = gpu_fixture(wait);(self.root/'wine-gpu-completion.bin').write_bytes(raw)
            self.report['wine_gpu_completion_observation'] = decode_gpu(raw)
            result = analyze(self.root, self.report)
            self.assertEqual(result['reservations'], 1)
            self.assertEqual(result['gpu_journal_sha256'], decode_gpu(raw)['journal_sha256'])
            self.assertFalse(result['physical_backing_owned'])
            self.assertFalse(result['unobserved_writers_closed'])

    def test_gpu_header_and_terminal_are_validated(self):
        for offset, value in ((0, 0), (8, 2), (28, 1), (64+128+12, 2),
                              (64+128+4, 1), (64+128+40, 0)):
            with self.subTest(offset=offset):
                self.changed_word('wine-gpu-completion.bin', self.gpu, offset, value)
                with self.assertRaises(ValueError):analyze(self.root, self.report)

    def test_gpu_fence_context_wait_and_reserved_fields_are_validated(self):
        for word, value in ((0, 2), (9, 0), (10, 0x911b), (14, 0), (15, 0), (18, 1)):
            with self.subTest(word=word):
                self.changed_word('wine-gpu-completion.bin', self.gpu, 64+word*4, value)
                with self.assertRaises(ValueError):analyze(self.root, self.report)

    def test_gpu_report_must_describe_the_raw_journal(self):
        raw = gpu_fixture(0x911a);(self.root/'wine-gpu-completion.bin').write_bytes(raw)
        with self.assertRaises(ValueError):analyze(self.root, self.report)

    def test_gpu_report_counts_and_hash_cannot_be_forged(self):
        for field, value in (('journal_sha256', 'f'*64), ('signaled', 9), ('closed', False)):
            with self.subTest(field=field):
                report = copy.deepcopy(self.report);report['wine_gpu_completion_observation'][field] = value
                with self.assertRaises(ValueError):analyze(self.root, report)

    def test_gpu_worker_thread_must_match_the_reservation(self):
        raw = self.changed_word('wine-gpu-completion.bin', self.gpu, 64+5*4, 11)
        self.report['wine_gpu_completion_observation'] = decode_gpu(raw)
        with self.assertRaises(ValueError):analyze(self.root, self.report)

    def test_source_version_header_and_terminal_are_validated(self):
        for offset, value in ((0, 0), (8, 3), (64+4*128+13*4, 2)):
            with self.subTest(offset=offset):
                self.changed_word('wine-source-version.bin', self.version, offset, value)
                with self.assertRaises(ValueError):analyze(self.root, self.report)

    def test_source_version_report_must_describe_the_raw_journal(self):
        self.changed_word('wine-source-version.bin', self.version, 64+3*4, 101)
        with self.assertRaises(ValueError):analyze(self.root, self.report)

    def test_source_version_worker_thread_must_match_the_reservation(self):
        raw = bytearray(self.version)
        for row in (2, 3):struct.pack_into('<I', raw, 64+row*128+2*4, 11)
        (self.root/'wine-source-version.bin').write_bytes(raw)
        self.report['wine_source_version_observation'] = decode_version(raw)
        with self.assertRaises(ValueError):analyze(self.root, self.report)

    def test_source_version_report_hash_and_denominator_cannot_be_forged(self):
        for duplicate in (False, True):
            report = copy.deepcopy(self.report)
            if duplicate:report['wine_source_version_observation']['bindings'] *= 2
            else:report['wine_source_version_observation']['journal_sha256'] = 'f'*64
            with self.subTest(duplicate=duplicate), self.assertRaises(ValueError):analyze(self.root, report)


if __name__ == '__main__':unittest.main()
