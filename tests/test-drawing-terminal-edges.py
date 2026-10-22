#!/usr/bin/env python3
"""Reject false inherited edges and overclaims from cleanup observations."""
import copy
import json
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from coverage_evidence import read_text as evidence_text
from drawing_terminal_edges import match, verified_contracts, WRAPPERS, OUTER, LEAF


class TerminalEdgesTest(unittest.TestCase):
    def fixture(self, target=0x4bdd10):
        row = [0]*32
        row[4:14] = [14,3,target,0x1234,0x3000,9,10,0x12345670,0x202,81]
        row[15:21] = [21,22,23,24,25,26]
        ps = row.copy();ps[0:4] = [1,1,1,0]
        pe = row.copy();pe[0:4] = [4,2,1,0]
        cs = row.copy();cs[0:4] = [2,1,2,1];cs[6:8] = [LEAF,0x76540000]
        receiver,bit = WRAPPERS.get(target,(0x641fc8,0))
        cs[9] = receiver
        if bit:cs[11] = (ps[11]&0xffffff00)|bit
        ce = cs.copy();ce[0:4] = [3,2,2,1];ce[9:14] = [100,101,102,0x246,103]
        pe[9:14] = ce[9:14]
        pairs = {1:(ps,pe),2:(cs,ce)}
        if target == OUTER:
            cs[0] = 4;ce[0] = 5;pe[0] = 6
            fs = ps.copy();fs[0:4] = [2,1,3,1];fs[6:10] = [LEAF,0x4a37f4,ps[8]-4,0x642000]
            fe = fs.copy();fe[0:4] = [3,2,3,1];fe[10:14] = [41,42,0x202,43]
            cs[10:14] = fe[10:14]
            pairs[3] = (fs,fe)
        return pairs

    def check(self,pairs):
        return match(pairs,{2:1},{**WRAPPERS,OUTER:(0x641fc8,0)})

    def test_all_five_tail_contracts_keep_call_edge_false(self):
        for target in [*WRAPPERS,OUTER]:
            with self.subTest(target=hex(target)):
                r = self.check(self.fixture(target))[0]
                self.assertFalse(r['original_call_edge_to_tail_child'])
                self.assertEqual(r['tail_target'],LEAF)
                self.assertEqual(r['tail_site'],target+25 if WRAPPERS.get(target,(0,0))[1] else 0x4a37f9 if target==OUTER else target+5)

    def test_child_thread_phase_stack_or_receiver_cannot_be_substituted(self):
        for key,value in [(5,4),(4,2),(8,0x3004),(9,0x642000)]:
            p = self.fixture();p[2][0][key] = value
            with self.assertRaises(ValueError):self.check(p)

    def test_parent_must_enclose_both_child_events(self):
        for sequence in [0,4,5]:
            p = self.fixture();p[2][0][0] = sequence
            with self.assertRaises(ValueError):self.check(p)

    def test_wrapper_preserves_edx_eax_error_and_diagnostic_words(self):
        for key in [10,11,13,15]:
            p = self.fixture();p[2][0][key] ^= 1
            with self.assertRaises(ValueError):self.check(p)

    def test_mov_wrapper_cannot_change_flags(self):
        p = self.fixture();p[2][0][12] ^= 4
        with self.assertRaises(ValueError):self.check(p)

    def test_one_shot_eax_low_byte_is_exact_and_upper_bytes_preserved(self):
        for change in [1,0x100]:
            p = self.fixture(0x4a2f70);p[2][0][11] ^= change
            with self.assertRaises(ValueError):self.check(p)

    def test_or_flags_require_clear_cf_of_zf_and_preserved_other_flags(self):
        for flag in [1,0x40,0x800,0x200]:
            p = self.fixture(0x4a2f00);p[2][0][12] ^= flag
            with self.assertRaises(ValueError):self.check(p)

    def test_uncaptured_flag_byte_does_not_fix_pf_sf_or_undefined_af(self):
        p = self.fixture(0x4a2f00);p[2][0][12] ^= 0x94
        self.assertEqual(len(self.check(p)),1)

    def test_child_return_must_equal_parent_return(self):
        for key in range(9,14):
            p = self.fixture();p[2][1][key] ^= 1
            with self.assertRaises(ValueError):self.check(p)

    def test_outer_uses_actual_first_cleanup_return_not_outer_entry_state(self):
        p = self.fixture(OUTER);p[1][0][10:14] = [1,2,3,4]
        self.assertEqual(self.check(p)[0]['first_cleanup_call_id'],3)

    def test_outer_requires_one_exact_prior_call_and_receiver(self):
        for key,value in [(7,0x4a37f5),(8,0x3000),(9,0x641fc8),(5,4),(4,2)]:
            p = self.fixture(OUTER);p[3][0][key] = value
            with self.assertRaises(ValueError):self.check(p)
        p = self.fixture(OUTER);p[4] = copy.deepcopy(p[3])
        with self.assertRaises(ValueError):self.check(p)

    def test_outer_call_after_state_must_reach_tail_unchanged(self):
        for key in range(10,14):
            p = self.fixture(OUTER);p[3][1][key] ^= 1
            with self.assertRaises(ValueError):self.check(p)

    def test_missing_pairs_or_unreviewed_contracts_are_rejected(self):
        p = self.fixture();p.pop(1)
        with self.assertRaises(ValueError):self.check(p)
        with self.assertRaises(ValueError):match(self.fixture(),{2:1},WRAPPERS)

    def test_exact_original_outer_body_must_match_retained_staged_bytes(self):
        proof = Path(__file__).resolve().parents[1]/'research/runtime/drawing-canvas-release-original-range-20261011.json'
        record = json.loads(evidence_text(proof));staged = bytearray(0x200000)
        offset = lambda raw,entry,size:entry-0x400000
        for entry in [*WRAPPERS,OUTER]:
            body = bytes.fromhex(record['original_entry_anchors'][hex(entry)])
            start = offset(staged,entry,len(body));staged[start:start+len(body)] = body
        self.assertEqual(len(verified_contracts(proof,staged,offset)),5)
        staged[OUTER-0x400000+12] ^= 1
        with self.assertRaises(ValueError):verified_contracts(proof,staged,offset)


if __name__ == '__main__':unittest.main()
