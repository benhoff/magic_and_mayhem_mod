import importlib.util
from pathlib import Path
import unittest
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import drawing_update_accounting as m


def fixture():
    windows=[];points=[];queues=[];records=[];calls=[];frames=[];source_frames=[];comparisons=[];qid=0
    for sequence in range(1,122):
        points.append(dict(sequence=sequence,cadence_record=sequence*10,producer_prefix=len(records),queue_prefix=qid,thread=7,counter_before=sequence-1,mode=0,pending_frames=0))
        if sequence==121:break
        present=sequence!=96
        windows.append(dict(sequence=sequence,advancing=True,counter_before=sequence-1,begin_record=sequence*10,end_record=(sequence+1)*10,
                            begin_qpc=sequence*100,end_qpc=(sequence+1)*100,queue_count=int(present),queue_absence_or_route='selected_world_queue' if present else 'original_transition_branch',route_points=[],results_push_observed=not present,branch_accounted=True))
        if not present:continue
        qid+=1;before=len(records)
        for op in (11,12):
            r=[0]*24;r[2]=op;r[14]=qid;records.append((r,b''))
        queues.append(dict(queue=qid,accepted_sequence=sequence,complete_update_window=True,advancing_update_window=True,update_queue_record_matches=True,producer_before=before,producer_after=len(records)))
        calls.append(dict(entry=sequence,type=1,method=7,thread=7,enter_qpc=sequence*100+10,return_qpc=sequence*100+20,complete_qpc=sequence*100+25))
        frames.append(dict(entry=sequence,ordinal=qid))
        value=[64,1,19,before,0,0,1,800,600,800,0,0,0,0,7,0]
        source_frames.append(dict(entry=sequence,both_generations_known=True,producer_live_at_all_points=True,points=[dict(phase=p,source=list(value),destination=list(value)) for p in (1,2,3)]))
        comparisons.append(dict(frame_entry=sequence,egl_pid=99,egl_call=sequence,physical_ordinal=qid,submission=sequence,worker=sequence,command=sequence,source_sha256='a'*64,physical_sha256='b'*64,pixels_match=True,whole_source_image_agreement=dict(pixels_match=True)))
    return [dict(frequency=1000,windows=windows),dict(frequency=1000,calls=calls),dict(points=points),dict(queues=queues),records,frames,dict(comparisons=comparisons),source_frames]


class Accounting(unittest.TestCase):
    def test_queue_free_update_counts_toward120_but_global_gate_stays_open(self):
        r=m.build(*fixture());self.assertEqual(r['source_queues'],119);self.assertEqual(r['queue_free_updates'],[96]);self.assertEqual(len(r['windows']),120)
        self.assertTrue(r['complete_observed_route_120_accounting']);self.assertFalse(r['complete_presented_interval'])
    def test_consumed_input_straddles_and_is_retained_in_both_windows(self):
        args=fixture();args[1]['calls'].append(dict(entry=1000,type=2,method=1,thread=8,enter_qpc=9590,return_qpc=9610,complete_qpc=9620,result=32768,arguments=[27]))
        r=m.build(*args);self.assertEqual(r['input_calls'],1);self.assertEqual(r['calls_crossing_boundaries'],[1000]);self.assertEqual([w['sequence'] for w in r['windows'] if w['input_calls']],[95,96])
    def test_missing_egl_unknown_source_and_pending_frames_are_gaps(self):
        for kind in ('egl','source','pending'):
            args=fixture()
            if kind=='egl':args[6]['comparisons'].pop(0)
            if kind=='source':args[7][0]['producer_live_at_all_points']=False
            if kind=='pending':args[2]['points'][1]['pending_frames']=1
            r=m.build(*args);self.assertFalse(r['complete_observed_route_120_accounting']);self.assertFalse(r['complete_presented_interval'])
    def test_pixel_mismatch_and_unexplained_branch_do_not_disappear(self):
        args=fixture();args[6]['comparisons'][0]['pixels_match']=False;args[0]['windows'][95]['branch_accounted']=False
        r=m.build(*args);self.assertEqual(r['unresolved_selected_branches'],[96]);self.assertEqual(len(r['reference_pixel_gaps']),1)
    def test_clock_counter_and_source_boundaries_cannot_be_waived(self):
        for kind in ('clock','sequence','counter','source','queue','time'):
            args=fixture()
            if kind=='clock':args[1]['frequency']=999
            if kind=='sequence':args[0]['windows'][80]['sequence']=1000
            if kind=='counter':args[0]['windows'][80]['counter_before']=999
            if kind=='source':args[2]['points'][80]['cadence_record']=999
            if kind=='queue':args[3]['queues'][0]['producer_after']=9999
            if kind=='time':args[0]['windows'][0]['end_qpc']=50
            with self.assertRaises(ValueError):m.build(*args)
    def test_duplicates_and_orphan_primary_frames_refuse(self):
        for kind in ('call','source','egl','frame'):
            args=fixture()
            if kind=='call':args[1]['calls'].append(args[1]['calls'][0])
            if kind=='source':args[2]['points'].append(args[2]['points'][0])
            if kind=='egl':args[6]['comparisons'].append(args[6]['comparisons'][0])
            if kind=='frame':args[5][0]['entry']=99999
            with self.assertRaises(ValueError):m.build(*args)
    def test_source_points_crossing_prefix_and_non120_denominator_fail(self):
        args=fixture();args[7][0]['points'][2]['source'][3]=99999
        self.assertFalse(m.build(*args)['complete_observed_route_120_accounting'])
        with self.assertRaises(ValueError):m.build(*fixture(),count=119)


if __name__=='__main__':unittest.main()
