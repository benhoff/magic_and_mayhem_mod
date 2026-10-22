import copy
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_two_stage_source import bind,FULL_RECT,FULL_BOX


def fixture():
    source=[64,1,19,50,0,0,7,800,600,800,3,3,1,0,18,0]
    m=source.copy();m[1]=2;m[15]=4
    first=dict(call=1,method=1,thread=18,api_entry=9,first_object=2,first_generation=20,second_object=1,second_generation=10,hresult=0,source_rect=FULL_RECT,destination_rect=FULL_RECT)
    final=dict(first,call=2,first_object=3,first_generation=30,second_object=2,second_generation=20)
    ps=[dict(submission=1,worker=1,command=1,worker_thread=22,worker_entry_record=1,worker_return_record=2),dict(submission=2,worker=2,command=2,worker_thread=22,worker_entry_record=3,worker_return_record=4)]
    ws=[dict(call=1,thread=22,destination=2,destination_generation=20,source=1,source_generation=10,source_box=FULL_BOX,destination_box=FULL_BOX,source_subresource=0,destination_subresource=0,filter=1,effects=[0]*6),dict(call=2,thread=22,destination=3,destination_generation=30,source=2,source_generation=20,source_box=FULL_BOX,destination_box=FULL_BOX,source_subresource=0,destination_subresource=0,filter=1,effects=[0]*6)]
    hs=[dict(api_entry=9,phase=i,before=source,thread=18,sha256='a'*64,lease=8) for i in (1,2)]
    r=dict(success=True,wine_submission_observation=dict(calls=[first,final]),wine_retained_map_source_observation=dict(points=[dict(submission=1,metadata=m,status=1,matched=True)]),canvas_storage_transition_observation=dict(queries=[dict(query=4,epoch=6,epoch_known=True,metadata=m)]),wine_blit_packet_observation=dict(bindings=ps),wine_blit_worker_observation=dict(calls=ws),api_source_worker_lease_observation=dict(calls=[dict(api_entry=9,lease=8,owner=18,commands={'1':dict(worker=1,thread=22),'2':dict(worker=2,thread=22)})]),source_held_snapshot_observation=dict(frames=hs))
    n=dict(success=True,comparisons=[dict(entry=9,submission=2,egl_call=11,ordinal=1,command=2,worker=2,lease=8,source_sha256='a'*64,boundary=dict(sequence=50,canvas=7),different_pixels=0,source_different_pixels=0)])
    return r,n


class TwoStage(unittest.TestCase):
    def test_exact_chain_is_scoped_observation(self):
        r=bind(*fixture());self.assertEqual(r['accepted_frames'],1)
        self.assertFalse(r['allocation_ownership_proved']);self.assertFalse(r['unobserved_uploads_proved'])

    def test_missing_epoch_state_or_texture_generation_excludes(self):
        for change in ('epoch','prefix','generation'):
            r,n=fixture()
            if change=='epoch':r['canvas_storage_transition_observation']['queries'][0]['epoch_known']=False
            elif change=='prefix':r['wine_retained_map_source_observation']['points'][0]['metadata'][3]=49
            else:r['wine_submission_observation']['calls'][0]['first_generation']=21
            self.assertEqual(bind(r,n)['accepted_frames'],0)

    def test_missing_command_or_worker_order_excludes(self):
        for change in ('lease','order','geometry','pixels'):
            r,n=fixture()
            if change=='lease':del r['api_source_worker_lease_observation']['calls'][0]['commands']['1']
            elif change=='order':r['wine_blit_packet_observation']['bindings'][0]['worker_return_record']=3
            elif change=='geometry':r['wine_blit_worker_observation']['calls'][0]['source_box']=[0,0,400,600,0,1]
            else:n['comparisons'][0]['different_pixels']=1
            self.assertEqual(bind(r,n)['accepted_frames'],0)

    def test_ambiguous_initial_link_and_duplicate_prediction_refuse(self):
        r,n=fixture();extra=copy.deepcopy(r['wine_submission_observation']['calls'][0]);extra['call']=3;r['wine_submission_observation']['calls'][1]['call']=4;n['comparisons'][0]['submission']=4;r['wine_submission_observation']['calls'].append(extra)
        self.assertEqual(bind(r,n)['accepted_frames'],0)
        r,n=fixture();n['comparisons'].append(copy.deepcopy(n['comparisons'][0]))
        with self.assertRaises(ValueError):bind(r,n)


    def test_duplicate_tables_and_held_phases_are_not_overwritten(self):
        for table,key in [('wine_submission_observation','calls'),('wine_retained_map_source_observation','points'),('canvas_storage_transition_observation','queries'),('wine_blit_worker_observation','calls'),('api_source_worker_lease_observation','calls'),('source_held_snapshot_observation','frames')]:
            r,n=fixture();r[table][key].append(copy.deepcopy(r[table][key][0]))
            with self.assertRaises(ValueError):bind(r,n)

    def test_zero_lease_changed_held_owner_and_internal_worker_interval(self):
        for change in ('lease','owner','canvas','interval'):
            r,n=fixture()
            if change=='lease':n['comparisons'][0]['lease']=0
            elif change=='owner':r['source_held_snapshot_observation']['frames'][1]['thread']=99
            elif change=='canvas':r['source_held_snapshot_observation']['frames'][1]['before']=list(r['source_held_snapshot_observation']['frames'][1]['before']);r['source_held_snapshot_observation']['frames'][1]['before'][6]=99
            else:r['wine_blit_packet_observation']['bindings'][0]['worker_entry_record']=2
            self.assertEqual(bind(r,n)['accepted_frames'],0)

if __name__=='__main__':unittest.main()
