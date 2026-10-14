import sys
from pathlib import Path
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_backing_provenance import classify


class Cases(unittest.TestCase):
    def run_case(self, prefixes):
        events=[dict(canvas=3,tag=2,phase=2,producer_prefix=p,returned_address=7,interface_addresses=[1,2]) for p in prefixes]
        return classify([dict(lease=1,sequence=5,canvases=[dict(canvas=3,layout_known=False,has_pixels=False),dict(canvas=2,layout_known=True,has_pixels=True)])],dict(events=events))
    def test_no_lock_is_unknown(self):
        r=self.run_case([]);self.assertEqual(r['entries'][0]['classification'],'no-lock-return-in-captured-window');self.assertFalse(r['physical_backing_owned']);self.assertEqual(r['unknown_entries'],1)
    def test_earlier_observation_is_not_current_pointer(self):
        r=self.run_case([4,5,6]);self.assertEqual(r['entries'][0]['classification'],'earlier-lock-return-observed');self.assertFalse(r['entries'][0]['has_recorded_pixels'])
    def test_equal_prefix_has_no_order(self):
        r=self.run_case([5]);self.assertEqual(r['entries'][0]['classification'],'equal-prefix-lock-order-unknown');self.assertFalse(r['equal_prefix_event_order_known'])
    def test_later_lock(self):
        r=self.run_case([6]);self.assertEqual(r['entries'][0]['classification'],'later-lock-return-observed');self.assertEqual(r['canvases'][0]['nonzero_returned_address_ids'],[7])


if __name__=='__main__':unittest.main()
