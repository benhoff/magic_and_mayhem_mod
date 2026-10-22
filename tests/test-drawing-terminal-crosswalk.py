#!/usr/bin/env python3
"""Exact fixed wrapper state admission; never pair a partial group."""
import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from coverage_evidence import read_text as evidence_text
from drawing_terminal_crosswalk import match, contracts, WRAPPERS


def pair(target=0x4bdd10, cid=1):
    receiver,bit=WRAPPERS[target]
    p=[0]*200;p[:8]=[cid*2-1,1,cid,0,2,7,0x59d98a,0x100100]
    p[8:13]=[0x123,0x456,target,0x202,9];p[14:18]=[1,0,target,1]
    p[18:26]=range(8);p[27]=p[7];p[34]=1
    q=p.copy();q[0]+=1;q[1]=2;q[8]=receiver;q[17:26]=[0]*9;q[34]=0
    c=[0]*32;c[:9]=[cid*2-1,1,cid,0,2,7,0x58b3e0,0x820002,p[7]]
    c[9:14]=[receiver,p[9],target if not bit else (target&0xffffff00)|bit,p[11],p[12]];c[15:21]=p[18:24]
    d=c.copy();d[0]+=1;d[1]=2;d[9:14]=q[8:13]
    return (p,q),(c,d)


class TerminalTests(unittest.TestCase):
    def run_pair(self,p,c):return match([p],[c],0x820002,WRAPPERS)

    def test_four_exact_wrappers_keep_original_root_and_tail_identities(self):
        for target in WRAPPERS:
            p,c=pair(target);r=self.run_pair(p,c);x=r['counts'][0]
            self.assertEqual(r['matched_wrapper_tail_pairs'],1);self.assertEqual(x['observed_wrapper_target'],target);self.assertEqual(x['reviewed_tail_target'],0x58b3e0)
            self.assertEqual(x['original_cpu_call_site'],0x59d98a);self.assertFalse(x['initial_flag_byte_observed'])

    def test_compact_wrapper_cpu_rows_need_no_fabricated_formals(self):
        for target in WRAPPERS:
            p,c = pair(target);p = tuple(row[:32] for row in p)
            self.assertEqual(self.run_pair(p,c)['matched_wrapper_tail_pairs'],1)

    def test_malformed_compact_wrapper_record_extent_refuses(self):
        p,c = pair();p = tuple(row[:31] for row in p)
        with self.assertRaises(ValueError):self.run_pair(p,c)

    def test_unknown_receiver_and_stack_thread_or_pc_stay_pending(self):
        for index in [5,7,8,9]:
            p,c=pair();c[0][index]+=4;self.assertEqual(self.run_pair(p,c)['matched_wrapper_tail_pairs'],0)

    def test_full_group_cardinality_prevents_subset_pairing(self):
        p,c=pair();p2,_=pair(cid=2);r=match([p,p2],[c],0x820002,WRAPPERS)
        self.assertEqual(r['matched_wrapper_tail_pairs'],0);self.assertEqual(r['gaps'][0]['reason'],'complete_wrapper_group_count_mismatch')

    def test_unreadable_or_wrong_register_target_refuses(self):
        for index in [10,14,15,17,34]:
            p,c=pair();p[0][index]^=1
            with self.assertRaises(ValueError):self.run_pair(p,c)

    def test_fixed_wrapper_entry_register_flags_error_or_arguments_refuse(self):
        for index in [10,11,12,13,15,16,17,18,19,20]:
            p,c=pair();c[0][index]^=1
            with self.assertRaises(ValueError):self.run_pair(p,c)

    def test_bit_wrappers_allow_only_documented_arithmetic_flag_changes(self):
        for target in [0x4a2f00,0x4a2f70]:
            p,c=pair(target);c[0][12]^=0x94 # PF/AF/SF depend on uncaptured input; AF undefined.
            self.assertEqual(self.run_pair(p,c)['matched_wrapper_tail_pairs'],1)
            for mask in [1,0x40,0x800,0x200]:
                p,c=pair(target);c[0][12]^=mask
                with self.assertRaises(ValueError):self.run_pair(p,c)

    def test_bit_wrapper_preserves_upper_eax_and_sets_exact_low_bit(self):
        p,c=pair(0x4a2f70)
        for mask in [1,0x100]:
            bad=copy.deepcopy(c);bad[0][11]^=mask
            with self.assertRaises(ValueError):self.run_pair(p,bad)

    def test_saved_phase_cleanup_and_after_state_refuse(self):
        for group,index in [('core',4),('cpu',7),('after',9),('after',10),('after',11),('after',12),('after',13)]:
            p,c=pair()
            if group=='core':c[0][index]+=1
            elif group=='cpu':p[1][index]+=4
            else:c[1][index]^=1
            with self.assertRaises(ValueError):self.run_pair(p,c)

    def test_unknown_contract_map_refuses(self):
        p,c=pair();bad=dict(WRAPPERS);bad[0x4bdd10]=(0x6a49e4,0)
        with self.assertRaises(ValueError):match([p],[c],0x820002,bad)

    def test_repeated_group_orders_use_each_journal_entry_order(self):
        p,c=pair();p2,c2=pair(cid=2);p2[0][9]+=1;c2[0][10]+=1
        self.assertEqual(match([p2,p],[c,c2],0x820002,WRAPPERS)['matched_wrapper_tail_pairs'],2)
        c2[0][10]-=1
        with self.assertRaises(ValueError):match([p,p2],[c,c2],0x820002,WRAPPERS)

    def test_pinned_original_contract_and_each_staged_wrapper_byte(self):
        proof=Path(__file__).resolve().parents[1]/'research/runtime/drawing-canvas-release-original-range-20261011.json'
        record=json.loads(evidence_text(proof));staged=bytearray();offsets={}
        for target in WRAPPERS:
            offsets[target]=len(staged);staged.extend(bytes.fromhex(record['original_entry_anchors'][hex(target)]))
        offset=lambda raw,target,length:offsets[target]
        self.assertEqual(contracts(proof,staged,offset),WRAPPERS)
        for target in WRAPPERS:
            bad=staged.copy();bad[offsets[target]]^=1
            with self.assertRaises(ValueError):contracts(proof,bad,offset)

    def test_altered_original_proof_cannot_supply_transfer_contracts(self):
        with tempfile.TemporaryDirectory() as directory:
            proof=Path(directory)/'proof.json';proof.write_text('{}')
            with self.assertRaises(ValueError):contracts(proof,b'',lambda raw,target,length:0)


if __name__=='__main__':unittest.main()
