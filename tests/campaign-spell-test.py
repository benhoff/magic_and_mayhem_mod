#!/usr/bin/env python3
from pathlib import Path
import struct
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_spell import HEADER, read_spell_rows, validate_spell_observation


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


if __name__ == '__main__':unittest.main()
