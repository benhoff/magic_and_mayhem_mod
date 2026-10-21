import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_physical_owners import decode,pin_prefix_ntdll,join_reservation


def journal(rows):
    h=[0]*16;h[2:8]=[2,128,0x40209ca7,262144,2048,51]
    raw=bytearray(struct.pack('<16I',*h));raw[:8]=b'MNMPOW02'
    for ordinal,values in enumerate(rows,1):
        r=[ordinal,*values];r += [0]*(32-len(r));raw += struct.pack('<32I',*r)
    return raw


def rows(client=False):
    r=[[1,10,1,0,2 if client else 1,3200,0]]
    if client:r.append([9,10,1,0,77,8192,0])
    return r+[[2,10,1,1,0,1,0],[7,20,0,0,42,1,0],[3,20,1,1,0,0,0],[8,20,0,0,42,0,0],[6,10,0,0,0,1,0]]


class Ownership(unittest.TestCase):
    def test_live_capture_refuses_borrowed_heap_page_mode(self):
        script=Path(__file__).resolve().parents[1]/'tools/capture-drawing-interactive.py'
        r=subprocess.run([sys.executable,str(script),'--source-game','/unused','--prefix-template','/unused','--report','/unused','--wine-physical-owners'],capture_output=True,text=True)
        self.assertEqual(r.returncode,2)
        self.assertIn('Live physical ownership requires --wine-source-handoff',r.stderr)

    def test_independent_journal_ordinals_survive_the_join(self):
        r=rows();r[1:1]=[[7,20,0,0,0,1,0],[8,20,0,0,0,0,0]]*20
        physical=decode(journal(r))['bindings'][0]
        reservation=dict(submission=1,command=1,worker=1,acquisition_record=1,release_record=6)
        joined=join_reservation(physical,reservation)
        self.assertEqual(joined['release_record'],44)
        self.assertEqual(joined['reservation_release_record'],6)
        self.assertEqual(joined['reservation_acquisition_record'],1)
        self.assertLess(joined['acquire_record'],joined['release_record'])

    def test_complete_owned_and_client(self):
        for client in (False,True):
            result=decode(journal(rows(client)));self.assertEqual(result['reservations'],1)
            self.assertEqual(result['bindings'][0]['release_thread'],20)
            self.assertFalse(result['global_host_driver_alias_ownership'])
            self.assertFalse(result['native_replacement'])
            if client:self.assertEqual(result['bindings'][0]['heap_generation'],77)

    def test_version_three_external_owner(self):
        for kind in range(3,7):
            r=[[1,10,1,0,kind,3200,0],[10,10,1,1,71,3,0,101,0,1,9,1,9,1,7],[2,10,1,1,0,1,0],[3,20,1,1,0,0,0],[4,20,1,0,kind,3200,0],[6,10,0,0,0,1,0]]
            raw=journal(r);raw[:8]=b'MNMPOW03';struct.pack_into('<I',raw,8,3)
            got=decode(raw);self.assertEqual(got['external_provider_reservations'],1);self.assertFalse(got['owned_private_sysmem'])
            for row,field in [(1,5),(1,6),(1,10),(1,15),(2,4),(4,1)]:
                bad=bytearray(raw);at=64+row*128+field*4;struct.pack_into('<I',bad,at,0)
                with self.assertRaises(ValueError):decode(bad)

    def test_missing_heap_generation(self):
        r=rows(True);del r[1]
        with self.assertRaises(ValueError):decode(journal(r))

    def test_generation_reuse_and_early_free(self):
        for inserted in ([1,10,1,0,1,3200,0],[4,10,1,0,0,3200,0]):
            r=rows();r.insert(2,inserted)
            with self.assertRaises(ValueError):decode(journal(r))

    def test_wrong_reference_counts(self):
        for index,value in ((1,2),(3,1)):
            r=rows();r[index][5]=value
            with self.assertRaises(ValueError):decode(journal(r))

    def test_unsupported_worker_with_lease(self):
        r=rows();r[2][4]=46;r[4][4]=46
        with self.assertRaises(ValueError):decode(journal(r))

    def test_active_worker_during_acquisition(self):
        r=rows();r[1],r[2]=r[2],r[1]
        with self.assertRaises(ValueError):decode(journal(r))

    def test_wrong_owner_submission_and_worker_order(self):
        for field,value in ((2,2),(3,2)):
            r=rows();r[3][field]=value
            with self.assertRaises(ValueError):decode(journal(r))
        r=rows();r[4][1]=21
        with self.assertRaises(ValueError):decode(journal(r))

    def test_missing_terminal_fault_and_reserved(self):
        with self.assertRaises(ValueError):decode(journal(rows()[:-1]))
        r=rows();r.insert(3,[5,20,1,1,0,0,18])
        with self.assertRaises(ValueError):decode(journal(r))
        raw=journal(rows());struct.pack_into('<I',raw,64+8*4,1)
        with self.assertRaises(ValueError):decode(raw)

    def test_prefix_pe32_selection_and_bad_image(self):
        installed=Path('/usr/lib/wine/i386-windows/ntdll.dll')
        if not installed.exists():self.skipTest('Pinned Wine is not installed')
        with tempfile.TemporaryDirectory() as tmp:
            prefix=Path(tmp);windows=prefix/'drive_c/windows';(windows/'system32').mkdir(parents=True);(windows/'syswow64').mkdir()
            fake=bytearray(128);fake[:2]=b'MZ';struct.pack_into('<I',fake,60,64);fake[64:68]=b'PE\0\0';struct.pack_into('<H',fake,68,0x8664)
            (windows/'system32/ntdll.dll').write_bytes(fake);(windows/'syswow64/ntdll.dll').symlink_to(installed)
            result=pin_prefix_ntdll(prefix);self.assertEqual(result['pe32_images'][0]['path'],'syswow64/ntdll.dll')
            (windows/'syswow64/ntdll.dll').unlink()
            with self.assertRaises(ValueError):pin_prefix_ntdll(prefix)
            struct.pack_into('<H',fake,68,0x14c);(windows/'system32/ntdll.dll').write_bytes(fake)
            with self.assertRaises(ValueError):pin_prefix_ntdll(prefix)
            (windows/'system32/ntdll.dll').unlink();(windows/'system32/ntdll.dll').symlink_to(installed)
            self.assertEqual(pin_prefix_ntdll(prefix)['pe32_images'][0]['path'],'system32/ntdll.dll')


if __name__=='__main__':unittest.main()
