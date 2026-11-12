import hashlib
import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from campaign_checkpoint import HEADER,decode,compare,save_name,validate_capture,validate_restore


def stream():
    rows=[]
    for i,screen in enumerate((13,12)):
        rows.append([len(rows)+1,1,99,200+i,i+1,screen,2,1]+[0]*8)
        rows.append([len(rows)+1,2,99,200+i,0,0,0,1,180,23,57,6,0,0,0xffffffff,9216])
        rows.append([len(rows)+1,2,99,200+i,1]+[0]*11)
    return HEADER+b''.join(struct.pack('<16I',*r) for r in rows)


def capture_fixture(seeded=False):
    raw=b'original-save';sha=hashlib.sha256(raw).hexdigest()
    states=[]
    for i,(action,control) in enumerate(zip((29,28,15),('miniMenuButton2','miniMenuButton1','miniMenuButton4'))):
        states.extend([dict(event='mini-request',action=action,accepted=True,reason='',screen=17,generation=i+1,time_ms=10*i+1),
                       dict(event='click',action=control,time_ms=10*i,capture_begin_ms=10*i+2,capture_end_ms=10*i+2,injected_capture_delay_ms=0,capture_duration_ms=0)])
    states.append(dict(event='checkpoint-seed-loaded',name='checkpoint-seed',sha256=sha) if seeded else dict(event='checkpoint-ready'))
    flow=dict(success=True,save_load=True,stage=12,normal_quit=True,resident_world_requested=True,native_command_fallback=False,
              native_recoveries=0,native_error='',native_command_frames=1000,saved_sha256=sha,states=states,checkpoints=[])
    for i,phase in enumerate(('before-save','after-save','after-load')):
        flow['checkpoints'].append(dict(phase=phase,native_active=True,fallback=False,error='',source_generation=str(i*10+5),source_base=str(i*10),frames=i*100+30,frame_base=i*100))
    route=dict(success=True,checkpoint_ready=True,simulation_writes=False,semantic_outcome_accepted=False,route=dict(points={'CF52 PoP':dict(x=23,y=57,z=6)}))
    if seeded:route['seed_reference']=dict(save_name='checkpoint-seed',save_sha256=sha)
    rows=[]
    for i,screen in enumerate((12,13,12) if seeded else (13,12)):
        rows.append([len(rows)+1,1,99,200+i,i+1,screen,3,1]+[0]*8)
        for slot,kind,owner,hp,x,y,z,mana in ((0,0,0,180,23,57,6,9216),(1,14,0,90,22,57,6,0),(2,24,1,100,43,27,5,12800)):
            rows.append([len(rows)+1,2,99,200+i,slot,kind,owner,1,hp,x,y,z,0,0,0xffffffff,mana])
    return flow,route,HEADER+b''.join(struct.pack('<16I',*r) for r in rows),raw


class CheckpointTest(unittest.TestCase):
    def test_fresh_and_seeded_capture_require_new_original_save_load(self):
        for seeded in (False,True):
            flow,route,data,raw=capture_fixture(seeded)
            result=validate_capture(flow,route,data,raw)
            self.assertTrue(result['verified']);self.assertFalse(result['fresh_campaign_progression_accepted'])
            self.assertEqual(result['saved_snapshot']['screen'],13)

    def test_seed_identity_cannot_replace_new_storage_witness(self):
        flow,route,data,raw=capture_fixture(True)
        flow['states'][-1]['sha256']='0'*64
        with self.assertRaises(ValueError):validate_capture(flow,route,data,raw)
        flow,route,data,raw=capture_fixture(True)
        with self.assertRaises(ValueError):validate_capture(flow,route,data[:16+4*64],raw)

    def test_changed_saved_bytes_and_primary_outcome_are_refused(self):
        flow,route,data,raw=capture_fixture()
        with self.assertRaises(ValueError):validate_capture(flow,route,data,raw+b'changed')
        route['semantic_outcome_accepted']=True
        with self.assertRaises(ValueError):validate_capture(flow,route,data,raw)

    def test_fresh_process_restore_needs_fresh_world_and_selected_fields(self):
        flow,route,data,raw=capture_fixture();reference=validate_capture(flow,route,data,raw)
        flow['checkpoints']=flow['checkpoints'][-1:]
        flow['states']=[s for s in flow['states'] if s.get('event')=='mini-request' and s['action']==15]
        loaded=data[16+4*64:];rows=[list(r) for r in struct.iter_unpack('<16I',loaded)]
        for i,row in enumerate(rows):row[0]=i+1
        rows[0][4]=1
        loaded=HEADER+b''.join(struct.pack('<16I',*r) for r in rows)
        self.assertTrue(validate_restore(flow,loaded,raw,reference)['verified'])
        flow['checkpoints'][0]['source_generation']=flow['checkpoints'][0]['source_base']
        with self.assertRaises(ValueError):validate_restore(flow,loaded,raw,reference)

    def test_checked_fields_and_separately_reported_action_difference(self):
        a,b=decode(stream());b['actors'][0]['action']=20
        result=compare(a,b)
        self.assertTrue(result['verified']);self.assertEqual(result['other_observed_differences'][0]['field'],'action')

    def test_damage_mana_position_identity_and_presence_fail(self):
        for field in ('health','mana','x','type','owner','active'):
            with self.subTest(field=field):
                a,b=decode(stream());b['actors'][0][field]+=1
                self.assertFalse(compare(a,b)['verified'])

    def test_partial_exhausted_sequence_and_boundary_refused(self):
        data=stream()
        for bad in (b'',data[:-1],data[:-64],data[:16],data[:20]+struct.pack('<I',2)+data[24:],data[:32]+struct.pack('<I',7)+data[36:]):
            with self.subTest(size=len(bad)):
                with self.assertRaises(ValueError):decode(bad)

    def test_distinct_name_and_no_path_or_overwrite_alias(self):
        self.assertEqual(save_name('checkpoint-camulos-01'),'checkpoint-camulos-01')
        for name in ('native-menu-live','../checkpoint-x','checkpoint-X','checkpoint-','checkpoint-x.sav'):
            with self.assertRaises(ValueError):save_name(name)


if __name__=='__main__':unittest.main()
