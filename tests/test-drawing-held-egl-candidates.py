#!/usr/bin/env python3
import copy,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_held_egl_candidates import candidates


def reference():
    before=[64,1,19,3,1,0,7,800,600,800,1,1,0,0,10,0]
    pair=[dict(api_entry=9,phase=p,lease=4,thread=10,sha256='same',before=before,oracle=p,path=f'{p}.565') for p in (1,2)]
    worker=dict(call=2,thread=20,destination=11,source=12,destination_generation=3,source_generation=4,source_box=[0,0,800,600,0,1],destination_box=[0,0,800,600,0,1],source_subresource=0,destination_subresource=0,filter=1,effects=[0]*6)
    submission=dict(call=5,api_entry=9,hresult=0,method=1,source_rect=[0,0,800,600],destination_rect=[0,0,800,600],first_object=11,second_object=12,first_generation=3,second_generation=4)
    context=dict(command=6,worker=2,submission=5,win_thread=20)
    return dict(source_held_snapshot_observation=dict(frames=pair),api_source_worker_lease_observation=dict(calls=[dict(api_entry=9,lease=4,owner=10,commands={'6':dict(worker=2,thread=20)})]),wine_blit_packet_observation=dict(bindings=[dict(command=6,worker=2,submission=5,worker_thread=20)]),wine_blit_worker_observation=dict(calls=[worker]),wine_submission_observation=dict(calls=[submission]),egl_physical_observation=dict(frames=[dict(entry=8,ordinal=1,width=800,height=600)]),egl_worker_context_observation=dict(calls=[dict(call=8,stable_active_context=True,points=[dict(context=context)])]))


def records(active=False):
    rows=[]
    for sequence,kind in ((1,11),(2,12),(3,5)):
        r=[0]*24;r[1]=sequence;r[2]=11 if active and sequence==3 else kind;r[14]=1;rows.append((r,b''))
    return rows


class Cohort(unittest.TestCase):
    def test_exact_full_command_admitted(self):
        selected,excluded=candidates(reference(),records());self.assertEqual(len(selected),1);self.assertFalse(excluded);self.assertEqual(selected[0]['boundary']['canvas'],7)
    def test_lease_context_hash_command_and_generation_refuse(self):
        changes=[lambda x:x['api_source_worker_lease_observation']['calls'][0]['commands'].clear(),lambda x:x['egl_worker_context_observation']['calls'][0].update(stable_active_context=False),lambda x:x['source_held_snapshot_observation']['frames'][1].update(sha256='changed'),lambda x:x['wine_blit_packet_observation']['bindings'][0].update(worker=99),lambda x:x['wine_blit_worker_observation']['calls'][0].update(source_generation=0),lambda x:x['wine_submission_observation']['calls'][0].update(second_generation=5)]
        for mutate in changes:
            x=reference();mutate(x);s,e=candidates(x,records());self.assertFalse(s);self.assertEqual(len(e),1)
    def test_partial_geometry_error_and_active_queue_refuse(self):
        for mutate in (lambda x:x['wine_blit_worker_observation']['calls'][0].update(destination_box=[0,0,40,40,0,1]),lambda x:x['wine_submission_observation']['calls'][0].update(hresult=1),lambda x:x['egl_physical_observation']['frames'][0].update(width=640)):
            x=reference();mutate(x);self.assertFalse(candidates(x,records())[0])
        self.assertFalse(candidates(reference(),records(True))[0])
    def test_repeated_prefix_retained_as_exclusion(self):
        x=reference();other=copy.deepcopy(x['egl_worker_context_observation']['calls'][0]);other['call']=9;x['egl_worker_context_observation']['calls'].append(other);x['egl_physical_observation']['frames'].append(dict(entry=9,ordinal=2,width=800,height=600));s,e=candidates(x,records());self.assertEqual(len(s),1);self.assertEqual(e[0]['reason'],'repeated-or-reordered-source-prefix')


if __name__=='__main__':unittest.main()
