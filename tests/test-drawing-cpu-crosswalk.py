#!/usr/bin/env python3
"""Synthetic strict CPU/core pairing and refusal guards; no original execution."""
import copy
from pathlib import Path
import struct
import sys
import tempfile
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_api_journal import plan, trampoline
from drawing_api_contracts import contract
from drawing_census import BUILD_SHA256
from drawing_census_bindings import relocated
from drawing_cpu_crosswalk import review


class CrosswalkTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(); self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.core_path = self.root/'core.bin'; self.cpu_path = self.root/'cpu.bin'
        self.cb = self.root/'core-bindings.bin'; self.ab = self.root/'cpu-bindings.bin'
        self.entry = 0x596cb8; self.site = 0x4cb64d; self.pc = 0x820006
        hook = {'tag':0, 'entry':self.entry, 'length':5, 'pop':0, 'site':False,
                'signature':'90'*5, 'relocations':[], 'route':'synthetic'}
        self.cm = {'schema':1, 'original_sha256':BUILD_SHA256, 'hooks':[hook]}
        instruction = {'address':self.site, 'bytes':'ff15dc036900', 'mnemonic':'call', 'operand':'DWORD PTR ds:0x6903dc'}
        cpu = plan(self.site, {self.site:instruction}); cpu['tag'] = 0; cpu['details'] = contract(cpu, 2)
        self.pm = {'original_sha256':BUILD_SHA256, 'hooks':[cpu], 'journal_version':2,
                   'contract_revision':2, 'cohort':'core_api', 'call_boundary_role':'cpu_backend_dispatch'}
        self.cohort = {'api_observation_enabled':True, 'call_census_enabled':True,
                      'api_only':False, 'call_boundary_role':'cpu_backend_dispatch'}
        self.callers = {'schema':1, 'source_sha256':BUILD_SHA256, 'image_base':0x400000,
                        'input_unchanged':True, 'callers':[]}
        self.inventory = {'source_sha256':BUILD_SHA256, 'functions':[]}
        h = bytearray(128); h[:8] = b'MNMDCB01'; struct.pack_into('<8I', h, 8, 1,128,96,1,0x400000,0x710000,0x10000,0x710100); h[64:96] = bytes.fromhex(BUILD_SHA256)
        row = bytearray(96); struct.pack_into('<12I',row,0,0,self.entry,5,0,0,0x800000,10,0x710200,0,0,0,0)
        row[48:64] = bytes.fromhex(hook['signature']).ljust(16,b'\0'); row[64:88] = relocated(hook,0x800000).ljust(24,b'\0')
        end = bytearray(64); end[:8] = b'MNMDCBEN'; struct.pack_into('<2I',end,8,1,1); self.cb.write_bytes(h+row+end)
        h = bytearray(128); h[:8] = b'MNMDAB01'; struct.pack_into('<12I',h,8,1,128,48,1,0x400000,0x700000,0x10000,0x710000,0x10000,0x710100,0,0); h[64:96] = bytes.fromhex(BUILD_SHA256)
        code, rest = trampoline(cpu,0,0)
        row = struct.pack('<12I',0,self.site,6,6,0x820000,len(code),0x820000+rest,0x700100,0x700200,self.pc,self.site+6,0)
        end = bytearray(64); end[:8] = b'MNMDABEN'; struct.pack_into('<2I',end,8,1,1); self.ab.write_bytes(h+row+end)

    def pair(self, cid=1):
        p = [0]*200; p[:8] = [0,1,cid,0,2,7,self.site,0x100100]
        p[8:13] = [0x123,0x456,0x789,0x202,9]; p[14:18] = [2,0x6903dc,self.entry,1]
        p[18:26] = list(range(8)); p[26:28] = [1,p[7]]; p[32:36] = [0xffffffff,16,1,0]; p[36:52] = list(range(16))
        q = p.copy(); q[1] = 2; q[17:26] = [0]*9; q[26] = 0; q[34] = 0; q[36:52] = [0]*16
        c = [0]*32; c[:9] = [0,1,cid,0,2,7,self.entry,self.pc,p[7]]
        c[9:14] = p[8:13]; c[15:21] = p[18:24]; c[28] = 1
        d = c.copy(); d[1] = 2; d[28] = 0
        return [p,q], [c,d]

    def run_rows(self, cpu, core):
        compact = self.pm['journal_version'] == 1
        if compact:cpu = [row[:32] for row in cpu]
        for path, rows, words, magic, endmagic, version, budget in [
            (self.cpu_path,cpu,32 if compact else 200,b'MNMDAPI1' if compact else b'MNMDAPI2',b'MNMDAEND',1 if compact else 2,4194304 if compact else 1048576),
            (self.core_path,core,32,b'MNMDCN01',b'MNMDCEND',1,4194304)]:
            for sequence,row in enumerate(rows,1): row[0] = sequence
            header = magic + struct.pack('<6I',version,64,words*4,0x400000,1,budget) + bytes.fromhex(BUILD_SHA256)
            terminal = endmagic + struct.pack('<14I',version,len(rows),max((r[2] for r in rows),default=0),0,0,0,*([0]*8))
            path.write_bytes(header+b''.join(struct.pack('<%dI'%words,*r) for r in rows)+terminal)
        return review(self.core_path,self.cm,self.cb,self.cpu_path,self.pm,self.ab,self.cohort,self.callers,self.inventory)

    def compact(self):
        self.pm.update(journal_version=1,compact_cpu_journal=True,diagnostic_stack_words=8,
            formal_arguments_recorded=False,buffer_snapshots_recorded=False)

    def test_compact_cpu_wire_pairs_without_invented_formal_rows(self):
        self.compact();p,c = self.pair();r = self.run_rows(p,c)
        self.assertEqual(r['cpu_journal_version'],1)
        self.assertEqual(r['matched_direct_core_pairs'],1)
        self.assertEqual(self.cpu_path.stat().st_size,128+2*128)
        self.assertFalse(r['caller_attribution_complete'])

    def test_compact_cpu_unreadable_diagnostic_words_refuse_matching(self):
        self.compact();p,c = self.pair();p[0][17:26] = [0]*9;c[0][15:21] = [0]*6
        with self.assertRaises(ValueError):self.run_rows(p,c)

    def test_compact_cpu_cannot_downgrade_without_explicit_scope(self):
        self.compact();self.pm.pop('formal_arguments_recorded');p,c = self.pair()
        with self.assertRaises(ValueError):self.run_rows(p,c)

    def test_direct_pair_preserves_all_pc_and_independent_sequence_identities(self):
        p,c = self.pair(); r = self.run_rows(p,c)
        self.assertEqual(r['matched_direct_core_pairs'],1); self.assertEqual(r['pending_cpu_pairs'],0)
        x = r['counts'][0]; self.assertEqual((x['original_call_site'],x['original_call_return'],x['physical_call_return'],x['observed_core_entry']),(self.site,self.site+6,self.pc,self.entry))
        self.assertEqual(x['proof_samples'][0]['original_return_slot'],p[0][7]-4)
        self.assertFalse(r['caller_attribution_complete']); self.assertFalse(r['draw01_complete'])

    def test_repeated_identical_stack_groups_pair_only_in_full_order(self):
        p,c = self.pair(); p2,c2 = self.pair(2); p2[0][10]+=1; p2[1][10]+=1; c2[0][11]+=1; c2[1][11]+=1
        self.assertEqual(self.run_rows(p+p2,c+c2)['matched_direct_cpu_pairs'],2)
        c2[0][11]-=1
        with self.assertRaises(ValueError): self.run_rows(p+p2,c+c2)

    def test_group_count_mismatch_never_pairs_a_subset(self):
        p,c = self.pair(); p2,_ = self.pair(2); r = self.run_rows(p+p2,c)
        self.assertEqual(r['matched_direct_cpu_pairs'],0); self.assertEqual(r['pending_cpu_pairs'],2)
        self.assertEqual(r['gaps'][0]['reason'],'direct_target_stack_group_count_mismatch')

    def test_target_mismatch_remains_pending(self):
        p,c = self.pair(); p[0][16] = p[1][16] = self.entry+1
        self.assertEqual(self.run_rows(p,c)['matched_direct_core_pairs'],0)

    def test_wrong_thread_and_original_stack_remain_pending(self):
        for index in (5,8):
            p,c = self.pair(); c[0][index]+=4; c[1][index]+=4
            self.assertEqual(self.run_rows(p,c)['matched_direct_core_pairs'],0)

    def test_physical_pc_neighbor_cannot_supply_exact_call(self):
        p,c = self.pair(); c[0][7]+=1; c[1][7]+=1; r = self.run_rows(p,c)
        self.assertEqual(r['matched_direct_cpu_pairs'],0); self.assertEqual(r['gaps'][0]['reason'],'core_caller_outside_cpu_return_map')

    def test_before_register_flags_error_and_argument_mutations_refuse(self):
        for index in list(range(9,14))+list(range(15,21)):
            p,c = self.pair(); c[0][index]^=1
            with self.assertRaises(ValueError): self.run_rows(p,c)

    def test_after_register_flags_error_mutations_refuse(self):
        for index in range(9,14):
            p,c = self.pair(); c[1][index]^=1
            with self.assertRaises(ValueError): self.run_rows(p,c)

    def test_unreadable_stack_cannot_support_direct_attribution(self):
        p,c = self.pair(); p[0][17] = 0; p[0][18:26] = [0]*8
        with self.assertRaises(ValueError): self.run_rows(p,c)

    def test_cleanup_and_phase_disagreements_refuse(self):
        for index in (7,4):
            p,c = self.pair()
            if index==7:p[1][7]+=4
            else:c[0][4]+=1;c[1][4]+=1
            with self.assertRaises(ValueError): self.run_rows(p,c)

    def test_wrong_role_and_cohort_refuse(self):
        p,c = self.pair()
        for field,value in [('call_boundary_role','graphics_api'),('call_census_enabled',False)]:
            old=copy.deepcopy(self.cohort); self.cohort[field]=value
            with self.assertRaises(ValueError): self.run_rows(p,c)
            self.cohort=old
        self.pm['call_boundary_role']='graphics_api'
        with self.assertRaises(ValueError): self.run_rows(p,c)

    def test_cpu_namespace_and_typed_layout_refuse(self):
        p,c = self.pair(); hook=self.pm['hooks'][0]; old=copy.deepcopy(hook)
        hook['target_mode']['address']=0x5c5308
        with self.assertRaises(ValueError): self.run_rows(p,c)
        self.pm['hooks'][0]=old; old['details']['formal_words']=1
        with self.assertRaises(ValueError): self.run_rows(p,c)

    def test_independently_valid_maps_must_agree_on_core_identity(self):
        p,c = self.pair(); raw=bytearray(self.ab.read_bytes()); struct.pack_into('<I',raw,44,0x710101); self.ab.write_bytes(raw)
        with self.assertRaises(ValueError): self.run_rows(p,c)

    def test_independent_allocation_overlap_refuses(self):
        p,c = self.pair(); raw=bytearray(self.ab.read_bytes())
        for offset in (144,152,164):struct.pack_into('<I',raw,offset,struct.unpack_from('<I',raw,offset)[0]-0x20000)
        self.ab.write_bytes(raw)
        with self.assertRaises(ValueError): self.run_rows(p,c)

    def test_original_build_and_binding_bytes_refuse(self):
        p,c = self.pair(); self.cm['original_sha256']='0'*64
        with self.assertRaises(ValueError): self.run_rows(p,c)
        self.cm['original_sha256']=BUILD_SHA256; raw=bytearray(self.cb.read_bytes()); raw[192]^=1; self.cb.write_bytes(raw)
        with self.assertRaises(ValueError): self.run_rows(p,c)

    def test_loss_open_frames_and_active_count_refuse(self):
        p,c = self.pair()
        with self.assertRaises(ValueError): self.run_rows(p[:1],c)
        c[0][28]=2
        with self.assertRaises(ValueError): self.run_rows(p,c)


if __name__ == '__main__': unittest.main()
