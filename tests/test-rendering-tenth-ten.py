#!/usr/bin/env python3
"""Adversarial owned journals for the ten bounded observation contracts."""
import importlib.util
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from drawing_x11_delivery import decode_delivery
from drawing_lifetime_index import LifetimeIndex
from drawing_wine_submission import decode as submissions
from drawing_source_byte_witness import decode as source_bytes,analyze as source_artifacts
from drawing_source_lease import decode as leases
from drawing_source_held_snapshot import decode as held
from drawing_egl_physical import analyze_egl_physical


def load(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path)
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value


def rows(data,width):return [list(r) for r in struct.iter_unpack('<'+str(width)+'I',data[64:])]
def pack(header,items):return bytes(header[:64])+b''.join(struct.pack('<'+str(len(r))+'I',*r) for r in items)


class Observers(unittest.TestCase):
    def test_each_delivery_route_rejects_cross_call_clock_regression(self):
        fixture=load('tenth_x11_fixture','tests/test-drawing-x11-delivery.py')
        for version,method in ((1,1),(2,10),(3,11)):
            header=bytearray(fixture.header());struct.pack_into('<I',header,8,version)
            items=[]
            for phase,identity,time in ((1,1,10),(2,1,20),(1,2,19),(2,2,21)):
                r=rows(fixture.header()+fixture.row(phase,identity,time),24)[0];r[2]=method
                if method==10:r[9:17]=[1,1,0,0,0,0,0,0]
                if method==11:r[9:17]=[1,1,1,0,0,0,0,0]
                items.append(r)
            with self.subTest(method=method),self.assertRaisesRegex(ValueError,'clock regressed'):decode_delivery(pack(header,items))
            items[2][5]=20
            self.assertEqual(len(decode_delivery(pack(header,items))['calls']),2)

    def test_glx_and_egl_retain_unknown_routes_and_opaque_aliases(self):
        # These observers intentionally do not infer source identity from swaps.
        fixture=load('tenth_delivery_scope','tests/test-drawing-x11-delivery.py')
        for version,method,args in ((2,10,[99,16,0,0,0,0,0,0]),(3,11,[256,256,16,0,0,0,0,0])):
            header=bytearray(fixture.header());struct.pack_into('<I',header,8,version);items=[]
            for phase in (1,2):
                r=rows(fixture.header()+fixture.row(phase),24)[0];r[2]=method;r[9:17]=args;items.append(r)
            result=decode_delivery(pack(header,items));self.assertFalse(result['producer_identity_bound']);self.assertFalse(result['all_display_routes_observed'])

    def test_x11_unused_arguments_refuse(self):
        f=load('tenth_x11_unused','tests/test-drawing-x11-delivery.py')
        for method,word in ((1,16),(5,14),(6,11),(7,15),(8,9),(9,10)):
            items=rows(f.header()+f.row()+f.row(2),24)
            for r in items:r[2]=method;r[9:17]=[0]*8;r[word]=1
            with self.subTest(method=method),self.assertRaisesRegex(ValueError,'unused argument'):decode_delivery(pack(f.header(),items))

    def test_submission_terminal_owner_refuses(self):
        f=load('tenth_submission','tests/test-drawing-wine-submission.py');data=f.wire();items=rows(data,48);items[-1][4]=0
        with self.assertRaisesRegex(ValueError,'terminal'):submissions(pack(data,items))

    def test_lifecycle_terminal_is_not_a_submission_prefix(self):
        header=b'MNMWLF01'+struct.pack('<14I',1,96,0x40209ca7,131072,2048,*([0]*9))
        terminal=[1,4,1,0,7]+[0]*19
        index=LifetimeIndex(pack(header,[terminal]));self.assertTrue(index.valid_prefix(0));self.assertFalse(index.valid_prefix(1))
        with self.assertRaisesRegex(ValueError,'prefix'):index.generation(1,1,1)

    def test_source_entry_return_owner_and_generations(self):
        f=load('tenth_bytes_identity','tests/test-drawing-source-byte-witness.py');data=f.wire()
        for word in (3,5,6):
            items=rows(data,48);items[1][word]+=1
            with self.subTest(word=word),self.assertRaisesRegex(ValueError,'owner/generations'):source_bytes(pack(data,items))

    def test_source_acceptance_canvas_stride_and_plane_budget(self):
        f=load('tenth_bytes_extent','tests/test-drawing-source-byte-witness.py');data=f.wire();items=rows(data,48)
        point=[64,1,19,0,0,0,1,800,600,800,1,0,0,0,7,0]
        for number,r in enumerate(items[:-1],1):r[7]=0;r[8:10]=[number,960000];r[16:32]=point;r[32:48]=point
        items[-1][8:10]=[2,1920000];self.assertEqual(source_bytes(pack(data,items))['admitted_planes'],2)
        for word,value in ((22,0),(25,799),(8,1025)):
            bad=[r[:] for r in items];bad[0][word]=value
            if word>=16:bad[0][word+16]=value
            with self.subTest(word=word),self.assertRaises(ValueError):source_bytes(pack(data,bad))

    def test_source_oracle_artifact_set_is_closed(self):
        f=load('tenth_bytes_artifacts','tests/test-drawing-source-byte-witness.py')
        with tempfile.TemporaryDirectory() as name:
            directory=Path(name);(directory/'api-source-bytes.bin').write_bytes(f.wire());self.assertTrue(source_artifacts(directory)['success'])
            (directory/'api-source-oracle-000001.565').write_bytes(bytes(960000))
            with self.assertRaisesRegex(ValueError,'Unbound'):source_artifacts(directory)

    def test_lease_cannot_reuse_completed_api_identity(self):
        f=load('tenth_lease','tests/test-drawing-source-lease.py');data=f.wire();items=rows(data,32);second=[r[:] for r in items[:2]]
        for ordinal,r in enumerate(second,3):r[0]=ordinal;r[7]=2
        second[0][6]=0x80000002;terminal=items[-1][:];terminal[0]=5
        with self.assertRaisesRegex(ValueError,'Duplicateleaseentry'):leases(pack(data,items[:2]+second+[terminal]))

    def test_held_snapshot_cannot_reuse_completed_api_identity(self):
        f=load('tenth_held','tests/test-drawing-source-held-pending.py');data=f.journal(version=1,pending=1,admission=False);items=rows(data,48);second=[r[:] for r in items[:2]]
        for ordinal,r in enumerate(second,3):r[0]=ordinal;r[8]=ordinal;r[10]=2
        terminal=items[-1][:];terminal[0]=5;terminal[8:10]=[4,3840000]
        with self.assertRaisesRegex(ValueError,'acquisition'):held(pack(data,items[:2]+second+[terminal]))

    def test_held_snapshot_enforces_advertised_plane_budget(self):
        f=load('tenth_held_budget','tests/test-drawing-source-held-pending.py');data=f.journal(version=1,pending=1,admission=False);template=rows(data,48);items=[]
        for pair in range(513):
            for phase in (1,2):
                r=template[phase-1][:];r[0]=len(items)+1;r[2]=pair+1;r[8]=len(items)+1;r[10]=pair+1;items.append(r)
        terminal=template[-1][:];terminal[0]=len(items)+1;terminal[8:10]=[len(items),len(items)*960000]
        with self.assertRaisesRegex(ValueError,'extent/ordinal'):held(pack(data,items+[terminal]))

    def test_source_oracle_identity_set_has_no_missing_ordinal(self):
        f=load('tenth_bytes_gap','tests/test-drawing-source-byte-witness.py');data=f.wire();items=rows(data,48)
        point=[64,1,19,0,0,0,1,800,600,800,1,0,0,0,7,0]
        for ordinal,r in enumerate(items[:-1],1):r[7]=0;r[8:10]=[ordinal if ordinal==1 else 3,960000];r[16:32]=point;r[32:48]=point
        items[-1][8:10]=[2,1920000]
        with self.assertRaisesRegex(ValueError,'closure/count'):source_bytes(pack(data,items))

    def aux_fixture(self):
        f=load('tenth_aux_fixture','tests/test-drawing-aux-save-readback.py');case=f.GuardTests();case.setUp();self.addCleanup(case.doCleanups);return case,f.aux

    def test_auxiliary_cannot_sample_same_api_twice(self):
        case,aux=self.aux_fixture();second=case.row[:];second[0]=2;second[15]=2;end=case.end[:];end[0]=3;end[5:7]=[2,2]
        (case.root/'aux-save-oracle-000002.565').write_bytes(bytes(3200))
        (case.root/'aux-save-readback.bin').write_bytes(pack(case.header,[case.row,second,end]))
        with self.assertRaisesRegex(ValueError,'Duplicate auxiliary'):aux.analyze(case.root)

    def test_auxiliary_refuses_symlinked_or_oversized_plane(self):
        for mode in ('symlink','oversize'):
            case,aux=self.aux_fixture();case.run_data();plane=case.root/'aux-save-oracle-000001.565'
            if mode=='symlink':
                target=case.root/'other.565';plane.rename(target);plane.symlink_to(target)
            else:plane.write_bytes(bytes(3201))
            with self.subTest(mode=mode),self.assertRaises((ValueError,OSError)):aux.analyze(case.root)

    def test_egl_close_marker_names_last_witnessed_call(self):
        f=load('tenth_physical_fixture','tests/test-drawing-egl-physical-binding.py')
        with tempfile.TemporaryDirectory() as name:
            import json
            directory=Path(name);peer=f.fixture(directory);path=directory/'peer.json';path.write_text(json.dumps(peer))
            self.assertTrue(analyze_egl_physical(directory,path)['success'])
            (directory/'egl-physical.done').write_bytes(b'MNMEGD01'+struct.pack('<6I',1,3,1,2,1,0))
            with self.assertRaisesRegex(ValueError,'last witnessed'):analyze_egl_physical(directory,path)

    def test_egl_physical_refuses_symlinked_plane(self):
        f=load('tenth_physical_symlink','tests/test-drawing-egl-physical-binding.py')
        with tempfile.TemporaryDirectory() as name:
            import json
            directory=Path(name);peer=f.fixture(directory);path=directory/'peer.json';path.write_text(json.dumps(peer))
            plane=directory/peer['frames'][0]['path'];target=directory/'other.rgba';plane.rename(target);plane.symlink_to(target)
            with self.assertRaises((ValueError,OSError)):analyze_egl_physical(directory,path)


if __name__=='__main__':unittest.main()
