import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_region_selection import observe
def fixture():
    before=dict(actor_state=1,actor_active=1,actor_health=400,pool_id=1,capacity=125,slot_plus_one=1,actor_type=0,actor_owner=0,actor_flag_68=0,raw_latch_count=0,mode=0,tick=100)
    after=dict(before,actor_flag_68=1,raw_latch_count=1,mode=1)
    calls=[dict(call=1,mask=1,mouse_record=[400,300],entry=before,returned=after),dict(call=2,mask=2,mouse_record=[400,300],entry=after,returned=after),dict(call=3,mask=32,mouse_record=[440,310],entry=after,returned=dict(after,tick=2100,actor_health=345))]
    return dict(wire_version=2,dropped=0,foreign_thread_calls=0,pending=0,calls=calls),[dict(kind='click',button=1,x=400,y=300,ordinal=3)]
class Latch(unittest.TestCase):
    def test_bound_partial_observation(self):
        r,a=fixture();x=observe(r,a);self.assertEqual(x['matches'][0]['later'][0]['elapsed_ms'],2000);self.assertFalse(x['actor_allocation_generation_proved']);self.assertFalse(x['persistent_selection_semantics_proved'])
    def test_ambiguous_coordinate_match_refuses(self):
        r,a=fixture();r['calls'].append(copy.deepcopy(r['calls'][0]))
        with self.assertRaises(ValueError):observe(r,a)
        r,a=fixture()
        with self.assertRaises(ValueError):observe(r,a+a)
    def test_changed_actor_release_or_missing_later_latch_refuses(self):
        for call,field,value in [(0,'actor_owner',2),(1,'actor_flag_68',0),(2,'tick',500),(2,'pool_id',2)]:
            r,a=fixture();r=copy.deepcopy(r);r['calls'][call]['returned'][field]=value
            with self.assertRaises(ValueError):observe(r,a)
    def test_loss_and_unknown_version_refuse(self):
        for key,value in [('wire_version',1),('dropped',1),('foreign_thread_calls',1),('pending',1)]:
            r,a=fixture();r[key]=value
            with self.assertRaises(ValueError):observe(r,a)
    def test_v3_requires_independent_latch_not_current_hit(self):
        r,a=fixture();r['wire_version']=3
        track=dict(actor_valid=True,pool_id=1,capacity=125,slot_plus_one=1,actor_type=0,actor_owner=0,requested_slot=0,actor_health=345,actor_flag_68=1)
        r['calls'][2]['returned']=dict(r['calls'][2]['returned'],actor_state=0,tracked_actor=track)
        x=observe(r,a);self.assertEqual(x['matches'][0]['independent_tracked_latch'][0]['current_hit_state'],0)
        for field,value in [('pool_id',2),('actor_flag_68',0),('actor_valid',False)]:
            changed=copy.deepcopy(r);changed['calls'][2]['returned']['tracked_actor'][field]=value
            with self.assertRaises(ValueError):observe(changed,a)
if __name__=='__main__':unittest.main()
