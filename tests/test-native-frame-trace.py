#!/usr/bin/env python3
"""Independent authored timestamp and export-join controls; no game artifacts."""
import copy,importlib.util,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
s=importlib.util.spec_from_file_location('frame_trace',ROOT/'tools/analyze-native-frame-trace.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
COLS=['turn','begin_us','world_end_us','commands_end_us','extra_end_us','export_end_us','status_end_us','wait_end_us','deadline_us','source_before','source_after','frames_before','frames_after','commands','presents','imports','export_serial','reason','draw_allowed','paused','pending_after','source_queue_ms','source_ready_ms','source_decode_ms','source_handoff_ms','native_producer_ms','native_prepare_ms','native_submit_ms','original_tick','original_capture_ms','original_ack_wait_ms','original_return_ms','published_us']
def fixture():
    first=dict(zip(COLS,[0]*len(COLS)));first.update(turn='1',begin_us=0,world_end_us=5000,commands_end_us=6000,extra_end_us=6000,export_end_us=7000,status_end_us=7500,wait_end_us=8500,source_before='63',source_after='64',frames_before='0',frames_after='1',export_serial='10',published_us=6500,draw_allowed=True,original_tick=4294967293,original_ack_wait_ms=8)
    second=dict(first,turn='2',begin_us=16667,world_end_us=17000,commands_end_us=17500,extra_end_us=20000,export_end_us=20500,status_end_us=21000,wait_end_us=22000,source_before='64',frames_before='1',frames_after='2',export_serial='11',published_us=20250)
    return {'frame_trace':{'enabled':True,'schema':1,'columns':COLS,'rows':[[r[k] for k in COLS] for r in [first,second]],'gui_columns':['kind','serial','generation','time_us','draw_allowed','paused'],'gui_rows':[[0,'10','64',8000,True,False],[2,'10','64',9000,True,False],[3,'10','64',10000,True,False],[1,'10','64',12000,True,False],[1,'10','64',13000,True,False],[0,'999','64',14000,True,False]],'overwritten_turns':'4','overwritten_gui_events':'2','visible_native_world_fps':58,'visible_owner_seconds':1,'visible_timings':{}},'engine_fps':19.5}
class Controls(unittest.TestCase):
    def test_independent_time_and_identity(self):
        r=m.analyze(fixture());self.assertEqual(r['native_frame_intervals_ms']['mean'],15)
        self.assertEqual(r['export_to_gui_pickup_ms']['mean'],1.5);self.assertEqual(r['gui_pickup_to_qt_swap_ms']['mean'],4)
        self.assertEqual(r['gpu_widget_paint_cpu_ms']['mean'],1);self.assertEqual(r['tail_distinct_swapped_exports'],1);self.assertEqual(r['tail_qt_swap_events'],2)
        self.assertEqual(r['source_ms']['original_ack_wait_ms']['mean'],8);self.assertEqual(r['overwritten_turns'],'4');self.assertEqual(r['visible_native_world_fps'],58)
    def test_paused_and_hidden_reset_intervals(self):
        f=fixture();f['frame_trace']['rows'][0][COLS.index('paused')]=True;r=m.analyze(f);self.assertEqual(r['native_frame_intervals_ms']['samples'],0)
        f=fixture();f['frame_trace']['rows'][0][COLS.index('draw_allowed')]=False;self.assertEqual(m.analyze(f)['native_frame_intervals_ms']['samples'],0)
    def test_malformed_and_impossible_latency(self):
        for mutation in [lambda t:t['rows'][0].append(0),lambda t:t['rows'][0].__setitem__(COLS.index('world_end_us'),9000),lambda t:t['rows'][1].__setitem__(COLS.index('frames_after'),'5'),lambda t:t['rows'][0].__setitem__(COLS.index('published_us'),9000)]:
            f=fixture();mutation(f['frame_trace'])
            with self.assertRaises(AssertionError):m.analyze(f)
if __name__=='__main__':unittest.main()
