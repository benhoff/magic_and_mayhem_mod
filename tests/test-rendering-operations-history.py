#!/usr/bin/env python3
"""Refuse incomplete or misleading operation/history residual evidence."""
import io
from pathlib import Path
import struct
import sys
import tarfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from rendering_operations_history import (selected_members, pinned_member, refused_copies,
                                         fnv, plane_differences, locate_wave, sha)


def archive(entries):
    out=io.BytesIO()
    with tarfile.open(fileobj=out,mode='w:gz') as tar:
        for name,data,kind in entries:
            info=tarfile.TarInfo(name);info.size=len(data);info.type=kind
            tar.addfile(info,io.BytesIO(data))
    return out.getvalue()


def refused(source=True):
    row=[0]*44;row[0:3]=[3000,0x58c488,0x18c488]
    row[11:16]=[1,0x10,0x88760096,0,0]
    row[16:22]=[640,480,16,40,40,16]
    row[24:28]=[640,512,680,552] if source else [0,0,40,40]
    row[28:32]=[0,0,40,40] if source else [40,0,80,40]
    row[32]=fnv('blit_reject_source_rect' if source else 'blit_reject_target_rect')
    return row


def world():
    data=bytearray(80+64+40+64)
    data[:8]=b'MNMWRLD1'
    for off,value in [(8,1),(12,80),(16,len(data)),(20,1),(24,8),(28,4),(32,8),(36,1),(44,0x40209ca7)]:
        struct.pack_into('<I',data,off,value)
    r=[168,3,0,0,0,0,8,4,40,0,64,0,0,9,0,16]
    struct.pack_into('<16I',data,80,*r)
    struct.pack_into('<16I',data,184,*([2]*16))
    return data


class ResidualReview(unittest.TestCase):
    def test_content_and_logical_archive_names(self):
        raw=archive([('abc',b'one',tarfile.REGTYPE),('working/raw/two',b'two',tarfile.REGTYPE)])
        self.assertEqual(selected_members(raw,lambda n:{'abc':'raw/one','working/raw/two':'raw/two'}.get(n)),{'raw/one':b'one','raw/two':b'two'})

    def test_unsafe_archive_path_even_when_unselected(self):
        for name in ['../outside','/outside','working/../outside','working\\outside']:
            with self.subTest(name=name),self.assertRaises(ValueError):
                selected_members(archive([(name,b'x',tarfile.REGTYPE)]),lambda n:None)

    def test_duplicate_or_colliding_members(self):
        for entries in [[('a',b'x',tarfile.REGTYPE)]*2,[('a',b'x',tarfile.REGTYPE),('b',b'y',tarfile.REGTYPE)]]:
            with self.assertRaises(ValueError):selected_members(archive(entries),lambda n:'same')

    def test_selected_symlink_and_output_escape_refuse(self):
        with self.assertRaises(ValueError):selected_members(archive([('a',b'',tarfile.SYMTYPE)]),lambda n:n)
        with self.assertRaises(ValueError):selected_members(archive([('a',b'x',tarfile.REGTYPE)]),lambda n:'../escape')

    def test_byte_and_member_budgets(self):
        raw=archive([('a',b'abc',tarfile.REGTYPE),('b',b'',tarfile.REGTYPE)])
        with self.assertRaises(ValueError):selected_members(raw,lambda n:n,max_bytes=2)
        with self.assertRaises(ValueError):selected_members(raw,lambda n:None,max_members=1)

    def test_member_identity_requires_hash_and_size(self):
        row=dict(bytes=3,sha256=sha(b'abc'))
        self.assertEqual(pinned_member(b'abc',row),b'abc')
        for data in [b'xyz',b'ab']:
            with self.assertRaises(ValueError):pinned_member(data,row)

    def test_both_invalid_rectangle_branches(self):
        rows=refused_copies([refused(),refused(False)])
        self.assertTrue(all(r['invalid_fast_rectangle_established'] for r in rows))
        self.assertEqual([r['source_outside'] for r in rows],[True,False])

    def test_reason_hresult_or_emitted_output_cannot_be_overlooked(self):
        for index,value in [(13,0),(32,0),(40,9),(43,1),(11,0),(15,1)]:
            row=refused();row[index]=value
            with self.subTest(index=index):self.assertFalse(refused_copies([row])[0]['invalid_fast_rectangle_established'])

    def test_valid_geometry_is_not_explained_by_rectangle_label(self):
        row=refused();row[24:28]=[0,0,40,40]
        self.assertFalse(refused_copies([row])[0]['invalid_fast_rectangle_established'])

    def test_other_failure_stays_unclassified(self):
        row=refused();row[13]=0x80004005
        self.assertFalse(refused_copies([row])[0]['invalid_fast_rectangle_established'])

    def test_full_plane_comparison_not_only_selected_pixel(self):
        left=b'\x00\x00'*40;right=bytearray(left)
        for i in (2,39):struct.pack_into('<H',right,i*2,6338)
        result=plane_differences(left,right,8,5)
        self.assertEqual(result['mismatches'],2)
        self.assertEqual(result['samples'][-1],dict(x=7,y=4,left=0,right=6338))

    def test_pixel_extent_refuses(self):
        with self.assertRaises(ValueError):plane_differences(b'\0',b'\0',1,1)

    def test_locator_is_linear_and_preserves_history_unknowns(self):
        result=locate_wave(world(),1,7,1)
        self.assertEqual(result['source'],[1,2])
        self.assertFalse(result['prior_writer_proved'])
        self.assertFalse(result['footprint_membership_proved'])

    def test_truncated_trailing_refused_or_wrong_operation(self):
        for mutate in [lambda b:b[:-1],lambda b:b+b'X',lambda b:patch(b,40,7),lambda b:patch(b,84,0),lambda b:patch(b,244,17)]:
            with self.assertRaises(ValueError):locate_wave(mutate(world()),1,3,1)

    def test_locator_rejects_unknown_record_target_and_sample(self):
        for record,x,y in [(2,3,1),(1,8,1),(1,7,3)]:
            with self.assertRaises(ValueError):locate_wave(world(),record,x,y)


def patch(data,offset,value):
    struct.pack_into('<I',data,offset,value)
    return data


if __name__=='__main__':unittest.main(verbosity=2)
