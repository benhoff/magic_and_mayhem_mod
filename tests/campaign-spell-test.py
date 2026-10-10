#!/usr/bin/env python3
from pathlib import Path
import struct
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_spell import HEADER, HEADER_V2, read_spell_rows, validate_spell_observation, validate_spell_cases, player_fireball_damage


class SpellTests(unittest.TestCase):
    def fixture(self):
        rows = [[1,1,400,20,14,0,0,0,12800,8448,0xffffffff,1,2,4,6,1]]
        gameplay = [[1,1,400,10]+[0]*12, [2,1,400,30]+[0]*12]
        inputs = dict(summon=dict(before_sequence=1,after_sequence=2),wizard_slot=0,player_owner=0)
        return rows, gameplay, inputs

    def test_debit_requires_matching_original_caster_and_cast_interval(self):
        rows, gameplay, inputs = self.fixture()
        self.assertEqual(validate_spell_observation(rows,gameplay,inputs)['observed_mana_debit'],4352)
        for field,value in [(2,401),(3,9),(4,71),(5,99),(6,14),(7,2),(9,12800),(15,0)]:
            bad=[list(rows[0])];bad[0][field]=value
            with self.assertRaises(ValueError):validate_spell_observation(bad,gameplay,inputs)
        rows[0][14]=0 # Opaque original EAX cannot determine acceptance.
        self.assertEqual(validate_spell_observation(rows,gameplay,inputs)['observed_mana_debit'],4352)

    def test_tick_wraparound_is_bounded(self):
        rows, gameplay, inputs = self.fixture()
        gameplay[0][3]=0xfffffff0;rows[0][3]=0;gameplay[1][3]=10
        validate_spell_observation(rows,gameplay,inputs)
        rows[0][3]=11
        with self.assertRaises(ValueError):validate_spell_observation(rows,gameplay,inputs)

    def test_torn_stale_wrong_protocol_and_exhausted_traces_fail(self):
        rows,_,_ = self.fixture();body=struct.pack('<16I',*rows[0])
        with tempfile.TemporaryDirectory() as directory:
            p=Path(directory)/'spell-events.bin'
            for data in [b'',b'MNMGP001'+HEADER[8:]+body,HEADER+body+b'x',HEADER+struct.pack('<16I',2,*rows[0][1:]),HEADER+body*65536]:
                p.write_bytes(data)
                with self.assertRaises(ValueError):read_spell_rows(p,complete=True)
            p.write_bytes(HEADER+body+b'x')
            self.assertEqual(len(read_spell_rows(p)),1) # Only live append tolerates a partial tail.

    def cases_fixture(self):
        def actor(seq,tick,mana,slot=0,kind=0,owner=0):
            return [seq,1,400,tick,slot,kind,owner,1,200,10,70,0,0,0,0xffffffff,mana]
        gp=[actor(1,0,12800),actor(2,2000,12800),actor(3,2100,12800),
            actor(4,2400,8448),actor(5,2500,8448,1,14),
            actor(6,4000,8448),actor(7,5000,2560),actor(8,5100,2560),actor(9,7200,2560)]
        cast=[1,1,400,2200,14,0,0,0,12800,8448,0xffffffff,1,2,4,6,1]
        fire=[2,1,400,4500,71,0,0,0,8448,6912,2,1,2,4,6,1]
        damage=[3,3,400,4600,2,10,2,105,85,1,20,0,71,0x48ed24,0,0]
        inp=dict(player_owner=0,wizard_slot=0,summon=dict(before_sequence=3,after_sequence=5),
                 blocked_casts=[dict(kind='invalid-target',before_sequence=1,after_sequence=2,mana_before=12800,mana_after=12800,created_slots=[]),
                               dict(kind='insufficient-mana',before_sequence=8,after_sequence=9,mana_before=2560,mana_after=2560,created_slots=[])])
        return [cast,fire,damage],gp,inp

    def test_v2_defended_damage_requires_matching_player_effect(self):
        rows,gp,inp=self.cases_fixture()
        self.assertTrue(validate_spell_cases(rows,gp,inp)['verified'])
        for field,value in [(6,0),(7,0),(8,105),(9,0),(11,2),(12,94),(14,99),(15,2),(13,0x50c0d0),(10,0)]:
            bad=[list(r) for r in rows];bad[-1][field]=value
            self.assertEqual(player_fireball_damage(bad,0,0),[])
            with self.assertRaises(ValueError):validate_spell_cases(bad,gp,inp)
        for bad in [rows[:1]+rows[2:],[rows[0],list(rows[1]),rows[2]]]:
            if len(bad)==3:bad[1][3]=rows[2][3]-10001
            with self.assertRaises(ValueError):validate_spell_cases(bad,gp,inp)
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'trace.bin';p.write_bytes(HEADER_V2+b''.join(struct.pack('<16I',*r) for r in rows))
            self.assertEqual(len(read_spell_rows(p,complete=True)),3)
            p.write_bytes(HEADER+struct.pack('<16I',*rows[-1]))
            with self.assertRaises(ValueError):read_spell_rows(p,complete=True)

    def test_refusals_reject_short_missing_funded_or_spending_cases(self):
        import copy
        rows,gp,inp=self.cases_fixture()
        mutations=[lambda i:i['blocked_casts'].pop(),lambda i:i['blocked_casts'][0].update(kind='insufficient-mana'),
                   lambda i:i['blocked_casts'][0].update(created_slots=[3]),lambda i:i['blocked_casts'][0].update(mana_after=1),
                   lambda i:i['blocked_casts'][1].update(before_sequence=6,mana_before=8448)]
        for mutate in mutations:
            bad=copy.deepcopy(inp);mutate(bad)
            with self.assertRaises(ValueError):validate_spell_cases(rows,gp,bad)
        for seq,field,value in [(2,3,100),(2,15,12799),(9,8,0),(9,6,2)]:
            bad=copy.deepcopy(gp);bad[seq-1][field]=value
            with self.assertRaises(ValueError):validate_spell_cases(rows,bad,inp)


if __name__ == '__main__':unittest.main()
