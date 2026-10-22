import sys
from pathlib import Path
import struct
import unittest

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from original_canvas_backing import analyze


class BackingGenerationJournal(unittest.TestCase):
    def fixture(self,reuse=False):
        events=[(100,0,0x5000,1,112,0x100100),(110,0,0x5000,0,0,0x100110),(120,0,0x5000,1,0,0x100110)]
        rows=[(81,99,0,0x100000,0x1000,1),(81,99,1,0x200000,0x1000,1)]
        for phase,start in [(0,0)]+([(1,3)] if reuse else []):
            if phase:events.extend([(100,1,0x5000,1,112,0x100100),(110,1,0x5000,0,0,0x100110),(120,1,0x5000,4,0,0x100110)])
            rows.extend((83,phase,stage,start+(0 if stage==0 else 1 if stage<3 else 3),0,0) for stage in range(5))
            rows.append((84,phase,3,start+1,16,0x5010))
        rows.append((82,99,0,0,len(events),0))
        return rows,events

    def run_journal(self,rows,events):return analyze(rows,b''.join(struct.pack('<6I',*row) for row in events))

    def test_same_numeric_address_in_two_completed_allocations_is_two_generations(self):
        rows,events=self.fixture(True);result=self.run_journal(rows,events)
        self.assertEqual([g['generation'] for g in result['matched_cpu_backing_generations']],[1,2])
        self.assertEqual(result['backing_generations_retained_after_wrapper_retirement'],2)

    def test_completed_free_not_just_release_or_free_entry_is_required(self):
        rows,events=self.fixture();events.pop();rows[-1]=(82,99,0,0,len(events),0)
        with self.assertRaisesRegex(ValueError,'Incomplete provider free'):self.run_journal(rows,events)

    def test_lock_after_free_cannot_reuse_dead_generation(self):
        rows,events=self.fixture();rows[-2]=(84,0,3,3,16,0x5010)
        with self.assertRaisesRegex(ValueError,'No actual provider allocations'):self.run_journal(rows,events)

    def test_caller_outside_pinned_provider_is_rejected(self):
        rows,events=self.fixture();events[0]=(*events[0][:5],0x300100)
        with self.assertRaisesRegex(ValueError,'escaped pinned provider'):self.run_journal(rows,events)


if __name__=='__main__':unittest.main()
