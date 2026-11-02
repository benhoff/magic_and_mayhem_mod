#!/usr/bin/env python3
"""Refuse stale identities, changed replies, counter gaps and sparse-log overclaims."""
from copy import deepcopy
import json
from pathlib import Path
import struct
import sys
import unittest
import zlib
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from storage_cpu_read_binding import counters, join_reads, reply_header, sha


def cap(rows):
    raw = bytearray(b'MNMACE01'); total = 0
    rows = [('open', dict(version=1, role='commands', session='fixture'), b''), *rows]
    for index, (kind, fields, body) in enumerate(rows):
        row = dict(index=index, host_us=str(index), kind=kind, **fields)
        metadata = json.dumps(row).encode(); packed = struct.pack('>I',len(body))+zlib.compress(body) if body else b''
        raw.extend(struct.pack('<II',len(metadata),len(packed))+metadata+packed); total += len(body)
    end = dict(index=len(rows), host_us=str(len(rows)), kind='close', complete=True,
               error='', prior_rows=len(rows), raw_bytes=str(total))
    data = json.dumps(end).encode();raw.extend(struct.pack('<II',len(data),0)+data)
    return bytes(raw)


def fixture():
    request = [1,1,0,77,2,2,1,0,9,0]
    def cmd(op,seq,payload):return struct.pack('<III',op,seq,len(payload))+payload
    stream = b'MNMCMD05'+struct.pack('<II',5,16)
    stream += cmd(1,1,struct.pack('<7I',1,2,2,16,0xf800,0x7e0,31)+b'\0'*8)
    stream += cmd(19,2,struct.pack('<5I',1,1,0,9,0))
    stream += cmd(21,3,struct.pack('<10I',*request))
    stream += cmd(7,4,struct.pack('<I',1))+cmd(8,5,b'')
    body = reply_header(request)+b'\0'*8
    rows = [('fragment',{},stream),('cpu-read',dict(sequence=3),body),('ended',dict(commands=5),b'')]
    count = b'MNMRDC01'+struct.pack('<10I',1,48,1,1,0,0,0,0,0,0)
    compared = [dict(sequence=3,bytes=len(body),sha256=sha(body))]
    values = [0x1234,316,0,1,1,0,8]+[0]*12
    log = ('resident_cpu_materialized '+' '.join(f'{v:08x}' for v in values)+'\n').encode()
    return rows,count,compared,log,body


def run(data):
    rows,count,compared,log,body = data
    return join_reads(cap(rows),'fixture',count,compared,log,body)


class ReadTests(unittest.TestCase):
    def test_exact_identity_join_preserves_missing_original_caller(self):
        result = run(fixture())
        self.assertEqual(result['complete_reply_matches'],1)
        self.assertEqual(result['exact_original_callers_recovered'],0)
        self.assertIsNone(result['requests'][0]['original_caller_address'])
        self.assertEqual(result['requests'][0]['completion_kind_from_uniform_aggregate'],0)

    def test_sparse_log_never_becomes_complete_callback_history(self):
        data = list(fixture()); data[3] = b''; result = run(data)
        self.assertEqual(result['requests_without_lifecycle_detail'],1)
        self.assertFalse(result['lifecycle']['complete_original_callback_history'])
        self.assertFalse(result['requests'][0]['per_request_original_completion_recorded'])

    def test_every_reply_identity_word_is_checked(self):
        for offset in range(0,64,4):
            data = list(deepcopy(fixture())); body = bytearray(data[0][1][2]); body[offset] ^= 1
            data[0][1] = ('cpu-read',dict(sequence=3),bytes(body))
            with self.subTest(offset=offset), self.assertRaises(ValueError):run(data)

    def test_changed_payload_cannot_reuse_old_comparison(self):
        data = list(deepcopy(fixture())); body = bytearray(data[0][1][2]); body[-1] ^= 1
        data[0][1] = ('cpu-read',dict(sequence=3),bytes(body))
        with self.assertRaisesRegex(ValueError,'comparison hash'):run(data)

    def test_duplicate_or_orphan_reply_refuses(self):
        data = list(deepcopy(fixture())); data[0].insert(2,data[0][1])
        with self.assertRaisesRegex(ValueError,'Duplicate'):run(data)
        data = list(deepcopy(fixture())); data[0][1] = ('cpu-read',dict(sequence=4),data[0][1][2])
        with self.assertRaisesRegex(ValueError,'gap'):run(data)

    def test_counter_sum_and_reserved_words_refuse(self):
        data = bytearray(fixture()[1]); data[20] = 2
        with self.assertRaisesRegex(ValueError,'sum'):counters(data)
        data = bytearray(fixture()[1]); data[44] = 1
        with self.assertRaisesRegex(ValueError,'reserved'):counters(data)

    def test_lifecycle_wrong_surface_extent_and_duplicates_refuse(self):
        for index,value in ((4,2),(6,10)):
            data = list(fixture()); parts = data[3].decode().split();parts[index+1] = f'{value:08x}'
            data[3] = (' '.join(parts)+'\n').encode()
            with self.assertRaises(ValueError):run(data)
        data = list(fixture());data[3] *= 2
        with self.assertRaisesRegex(ValueError,'ambiguity'):run(data)

    def test_terminal_reply_and_capture_session_are_bound(self):
        data = list(fixture());data[4] = data[4][:-1]+b'\1'
        with self.assertRaisesRegex(ValueError,'Terminal'):run(data)
        data = fixture()
        with self.assertRaisesRegex(ValueError,'role/session'):
            join_reads(cap(data[0]),'different',*data[1:])

    def test_counter_generic_log_kind_does_not_override_completion_enum(self):
        data = list(fixture()); count = bytearray(data[1]);count[20] = 0;count[32] = 1;data[1] = bytes(count)
        self.assertEqual(run(data)['requests'][0]['completion_kind_from_uniform_aggregate'],3)

    def test_request_high_ticket_zero_and_nonce_required(self):
        for index in (0,1,3,4,5,6,8):
            w = [1,1,0,77,2,2,1,0,9,0];w[index] = 0
            with self.assertRaises(ValueError):reply_header(w)
        w = [1,1,1,77,2,2,1,0,9,0]
        with self.assertRaises(ValueError):reply_header(w)


if __name__ == '__main__':unittest.main()
