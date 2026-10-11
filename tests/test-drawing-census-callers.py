#!/usr/bin/env python3
"""Exact relocation and observed return-chain admission, with pending gaps."""
import copy
from pathlib import Path
import struct
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_census import BUILD_SHA256
from drawing_census_bindings import relocated
from drawing_census_callers import review


class CallerReviewTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.path=Path(self.tmp.name)/'census.bin';self.binding=Path(self.tmp.name)/'core-bindings.bin'
        self.hooks=[]
        for tag,entry in enumerate([0x500100,0x500900,0x500a00]):
            signature=b'\xe8'+struct.pack('<i',0x500900-entry-5) if tag==0 else b'\x90'*5
            self.hooks.append({'tag':tag,'entry':entry,'length':5,'pop':8 if tag<2 else 0,'site':tag==2,'signature':signature.hex(),'relocations':[0] if tag==0 else [],'route':'test-'+str(tag)})
        self.manifest={'schema':1,'original_sha256':BUILD_SHA256,'hooks':self.hooks}
        self.inventory={'source_sha256':BUILD_SHA256,'functions':[{'entry':0x601000,'ranges':[[0x601000,0x601005]]},{'entry':0x500100,'ranges':[[0x500100,0x500105]]}]}
        self.callers={'schema':1,'source_sha256':BUILD_SHA256,'image_base':0x400000,'input_unchanged':True,'callers':[]}
        for site,target in [(0x601000,0x500100),(0x500100,0x500900)]:
            self.callers['callers'].append({'call_va':site,'return_pc':site+5,'entry':target,'bytes':(b'\xe8'+struct.pack('<i',target-site-5)).hex(),'discovered_owners':[]})
        h=bytearray(128);h[:8]=b'MNMDCB01';struct.pack_into('<8I',h,8,1,128,96,3,0x400000,0x700000,0x10000,0x700100);h[64:96]=bytes.fromhex(BUILD_SHA256)
        rows=[]
        for tag,item in enumerate(self.hooks):
            base=0x800000+tag*0x10000;r=bytearray(96);struct.pack_into('<12I',r,0,tag,item['entry'],5,item['pop'],item['site'],base,10,0x700200+tag*0x100,len(item['relocations']),0,0,0);r[48:64]=bytes.fromhex(item['signature']).ljust(16,b'\0');r[64:88]=relocated(item,base).ljust(24,b'\0');rows.append(r)
        e=bytearray(64);e[:8]=b'MNMDCBEN';struct.pack_into('<2I',e,8,1,3);self.binding.write_bytes(h+b''.join(rows)+e)

    def entry(self,cid,tag,caller,sp=0x100100,thread=7,phase=2,active=1):
        row=[0,1,cid,tag,phase,thread,self.hooks[tag]['entry'],caller,sp]+[0]*23;row[28]=active;return row

    def leave(self,row,active=0):
        r=row.copy();r[1]=2;r[28]=active;return r

    def run_rows(self,rows):
        h=b'MNMDCN01'+struct.pack('<6I',1,64,128,0x400000,3,4194304)+bytes.fromhex(BUILD_SHA256)
        for seq,row in enumerate(rows,1):row[0]=seq
        count=max(r[2] for r in rows);e=b'MNMDCEND'+struct.pack('<14I',1,len(rows),count,0,0,0,*([0]*8))
        self.path.write_bytes(h+b''.join(struct.pack('<32I',*r) for r in rows)+e)
        return review(self.path,self.manifest,self.binding,self.callers,self.inventory)

    def test_relocated_original_call_preserves_both_pc_identities(self):
        root=self.entry(1,0,0x601005);child=self.entry(2,1,0x800005,sp=0x100080,active=2)
        r=self.run_rows([root,child,self.leave(child,1),self.leave(root)])
        self.assertEqual(r['exact_original_pairs'],1);self.assertEqual(r['exact_relocated_pairs'],1);self.assertEqual(r['gaps'],[])
        row=next(x for x in r['counts'] if x['classification']=='exact_relocated_original_call');self.assertEqual((row['physical_caller'],row['original_return_continuation'],row['original_call_target']),(0x800005,0x500105,0x500900));self.assertEqual(row['continuation_discovered_owners'],[0x500100]);self.assertFalse(r['caller_attribution_complete']);self.assertFalse(r['draw01_complete'])

    def test_same_slot_inherits_root_but_never_an_original_call_edge(self):
        root=self.entry(1,0,0x601005);child=self.entry(2,1,0x700100,active=2,phase=14)
        r=self.run_rows([root,child,self.leave(child,1),self.leave(root)]);row=next(x for x in r['counts'] if x['inherited_depth']==1)
        self.assertEqual(row['classification'],'same_return_slot_inherited_continuation');self.assertEqual(row['original_return_continuation'],0x601005);self.assertIsNone(row['original_call_site']);self.assertIsNone(row['original_call_target']);self.assertEqual(row['inherited_parent_entry'],0x500100);self.assertEqual(row['proof_samples'][0]['inherited_parent_call_id'],1);self.assertEqual(r['inherited_continuation_pairs'],1)

    def test_three_level_same_slot_chain_preserves_root_and_immediate_parent(self):
        a=self.entry(1,0,0x601005);b=self.entry(2,1,0x700100,active=2);c=self.entry(3,1,0x700100,active=3)
        r=self.run_rows([a,b,c,self.leave(c,2),self.leave(b,1),self.leave(a)]);row=next(x for x in r['counts'] if x['inherited_depth']==2);self.assertEqual(row['proof_samples'][0]['continuation_root_call_id'],1);self.assertEqual(row['proof_samples'][0]['inherited_parent_call_id'],2);self.assertEqual(row['inherited_parent_entry'],0x500900)

    def test_unknown_root_stays_unknown_through_inherited_chain(self):
        a=self.entry(1,0,0x650005);b=self.entry(2,1,0x700100,active=2)
        r=self.run_rows([a,b,self.leave(b,1),self.leave(a)]);row=next(x for x in r['counts'] if x['inherited_depth']);self.assertIsNone(row['original_return_continuation']);self.assertEqual(row['continuation_discovered_owners'],[]);self.assertTrue(any(x['reason']=='inherited_continuation_root_unresolved' for x in r['gaps']))

    def test_wrong_thread_slot_or_orphan_leave_does_not_choose_nearest_parent(self):
        for thread,sp in [(8,0x100100),(7,0x100104),(7,0x1000fc)]:
            a=self.entry(1,0,0x601005);b=self.entry(2,1,0x700100,thread=thread,sp=sp,active=2);r=self.run_rows([a,b,self.leave(b,1),self.leave(a)]);self.assertEqual(r['inherited_continuation_pairs'],0);self.assertEqual(r['gaps'][0]['reason'],'unmatched_core_leave')
        a=self.entry(1,1,0x700100);self.assertEqual(self.run_rows([a,self.leave(a)])['gaps'][0]['reason'],'unmatched_core_leave')

    def test_differing_cleanup_does_not_infer_shared_return_continuation(self):
        self.hooks[1]['pop']=12;raw=bytearray(self.binding.read_bytes());struct.pack_into('<I',raw,128+96+12,12);self.binding.write_bytes(raw)
        a=self.entry(1,0,0x601005);b=self.entry(2,1,0x700100,sp=0x100104,active=2);r=self.run_rows([a,b,self.leave(b,1),self.leave(a)]);self.assertEqual(r['gaps'][0]['reason'],'unmatched_core_leave')

    def test_original_and_relocated_call_targets_must_name_observed_callee(self):
        a=self.entry(1,0,0x800005);r=self.run_rows([a,self.leave(a)]);self.assertEqual(r['exact_relocated_pairs'],0);self.assertIn('pending',r['counts'][0]['classification'])
        a=self.entry(1,1,0x601005);r=self.run_rows([a,self.leave(a)]);self.assertEqual(r['exact_original_pairs'],0);self.assertIsNone(r['counts'][0]['original_return_continuation'])

    def test_missing_static_call_does_not_complete_relocated_attribution(self):
        self.callers['callers'].pop();a=self.entry(1,1,0x800005);r=self.run_rows([a,self.leave(a)]);self.assertEqual(r['exact_relocated_pairs'],0);self.assertTrue(r['gaps'])

    def test_module_neighbors_and_noncall_allocations_stay_unresolved(self):
        for pc in [0x700101,0x800004,0x800006,0x810005,0x601006]:
            a=self.entry(1,1,pc);r=self.run_rows([a,self.leave(a)]);self.assertIsNone(r['counts'][0]['original_return_continuation']);self.assertEqual(r['counts'][0]['continuation_discovered_owners'],[])

    def test_active_counts_cannot_fabricate_valid_return_chains(self):
        for index in [0,1]:
            a=self.entry(1,0,0x601005);rows=[a,self.leave(a)];rows[index][28]+=1
            with self.assertRaises(ValueError):self.run_rows(rows)

    def test_out_of_order_same_stack_returns_refuse_even_exact_pair_identity(self):
        a=self.entry(1,0,0x601005);b=self.entry(2,1,0x700100,active=2)
        with self.assertRaises(ValueError):self.run_rows([a,b,self.leave(a,1),self.leave(b)])

    def test_sites_never_supply_return_caller_or_parent_frames(self):
        a=self.entry(1,0,0x601005);site=self.entry(2,2,0,active=1);site[1]=3;r=self.run_rows([a,site,self.leave(a)]);self.assertEqual(r['site_events'],1);self.assertEqual(r['paired_calls'],1);self.assertEqual(len(r['counts']),1)
        site[28]=2
        with self.assertRaises(ValueError):self.run_rows([a,site,self.leave(a)])

    def test_binding_byte_mismatch_and_missing_terminal_refuse(self):
        a=self.entry(1,0,0x601005);raw=bytearray(self.binding.read_bytes());raw[128+64]^=1;self.binding.write_bytes(raw)
        with self.assertRaises(ValueError):self.run_rows([a,self.leave(a)])

    def test_empty_discovered_owner_does_not_discard_exact_call_or_fill_gap(self):
        self.inventory['functions']=[];a=self.entry(1,0,0x601005);r=self.run_rows([a,self.leave(a)]);self.assertEqual(r['exact_original_pairs'],1);self.assertEqual(r['counts'][0]['continuation_discovered_owners'],[])


if __name__=='__main__':unittest.main()
