#!/usr/bin/env python3
"""Check trace clock, identity, tail and visibility boundaries independently."""
import copy
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from native_frame_latency import summarize, original_cadence


def fixture():
    columns=['begin_us','world_end_us','commands_end_us','extra_end_us','export_end_us',
             'status_end_us','wait_end_us','source_before','source_after','export_serial',
             'published_us','draw_allowed','paused','source_queue_ms','source_ready_ms',
             'source_decode_ms','source_handoff_ms','native_producer_ms','native_prepare_ms',
             'native_submit_ms','original_capture_ms','original_ack_wait_ms','original_return_ms']
    gui_columns=['kind','serial','generation','time_us','draw_allowed','paused','activity','duration_us']
    turns=[];gui=[]
    for i in range(7):
        start=i*2000
        turns.append([start,start+100,start+200,start+300,start+400,start+500,start+600,
                      63+i,64+i,str(i+1),start+400,True,False,*([1.]*10)])
        gui.extend([[kind,str(i+1),str(64+i),start+offset,True,False,0,0]
                    for kind,offset in [(0,700),(2,800),(3,900),(8,950),(1,1100),(1,1200),(6,1250)]])
    return dict(engine_fps=19.,native_render_fps=50.,frame_trace=dict(enabled=True,schema=4,
        rows=turns,columns=columns,gui_rows=gui,gui_columns=gui_columns,
        visible_native_world_fps=49.,overwritten_turns='100',overwritten_gui_events='50'))


class Latency(unittest.TestCase):
    def test_known_clock_intervals_and_repeated_swap_deduplication(self):
        result=summarize(fixture())
        self.assertEqual(result['warm_sources'],7)
        self.assertEqual(result['matched_exports'],7)
        for key,expected in [('export_to_gui_pickup_ms',.3),('gui_pickup_to_qt_swap_ms',.4),
                             ('export_to_qt_swap_ms',.7),('widget_paint_cpu_ms',.1),
                             ('distinct_export_swap_interval_ms',2.)]:
            self.assertAlmostEqual(result[key]['median'],expected)
        self.assertEqual(result['overwritten_gui_events'],'50')

    def test_unmatched_tail_pickups_do_not_fabricate_publication_latency(self):
        r=fixture();r['frame_trace']['rows']=r['frame_trace']['rows'][2:]
        self.assertEqual(summarize(r)['matched_exports'],5)
        self.assertEqual(summarize(r)['gui_pickup_to_qt_swap_ms']['samples'],7)

    def test_paused_and_cold_gui_events_reset_swap_interval(self):
        r=fixture();r['frame_trace']['gui_rows'].insert(7,[6,'1','64',1500,False,True,0,0])
        self.assertEqual(summarize(r)['distinct_export_swap_interval_ms']['samples'],5)

    def test_wrong_generation_and_backward_host_time_refuse(self):
        for mutate in [lambda r:r['frame_trace']['gui_rows'][4].__setitem__(2,'99'),
                       lambda r:r['frame_trace']['gui_rows'][4].__setitem__(3,1),
                       lambda r:r['frame_trace']['rows'][0].__setitem__(1,-1)]:
            r=fixture();mutate(r)
            with self.assertRaises(ValueError):summarize(r)

    def test_malformed_and_insufficient_trace_refuse(self):
        for mutate in [lambda t:t.update(enabled=False),lambda t:t.update(schema=99),
                       lambda t:t['columns'].__setitem__(0,t['columns'][1]),
                       lambda t:t['rows'][0].pop(),lambda t:t.update(gui_rows=[]),
                       lambda t:t['gui_rows'][0].__setitem__(0,99)]:
            r=fixture();mutate(r['frame_trace'])
            with self.assertRaises(ValueError):summarize(r)

    def test_original_pacer_partition_wrap_visibility_and_unknown_mask(self):
        def row(g,tick,mask=3):
            return dict(draw_allowed=True,paused=False,source_before=str(g-1),source_after=str(g),
                original_tick=tick,original_capture_ms=2,original_ack_wait_ms=5,original_return_ms=1,
                original_pacer_mask=mask,original_pacer_start=4294967290,
                original_pacer_end=4,original_screen_ms=3)
        a=row(64,4294967270);b=row(65,24);c=row(66,74,0)
        r=original_cadence([a,b,c]);d=r['distributions']
        self.assertEqual(d['entry_interval_ms']['median'],50)
        self.assertEqual(d['world_body_ms']['median'],8)
        self.assertEqual(d['between_world_ms']['median'],42)
        self.assertEqual(d['preceding_pacer_ms']['median'],10)
        self.assertEqual(d['unclassified_between_world_ms']['median'],29)
        self.assertEqual(r['unobserved_pacer_pairs'],1)
        b['paused']=True
        self.assertIsNone(original_cadence([a,b,c])['distributions']['entry_interval_ms'])
        b['paused']=False;b['source_after']='67'
        self.assertEqual(original_cadence([a,b])['nonadjacent_pairs'],1)
        b['source_after']='65';b['original_tick']=4294967271
        with self.assertRaises(ValueError):original_cadence([a,b])

    def test_original_negative_residual_is_retained(self):
        a=dict(draw_allowed=True,paused=False,source_before='63',source_after='64',
            original_tick=0,original_capture_ms=0,original_ack_wait_ms=0,original_return_ms=0)
        b=dict(a,source_before='64',source_after='65',original_tick=20,
            original_pacer_mask=3,original_pacer_start=0,original_pacer_end=30,original_screen_ms=0)
        r=original_cadence([a,b]);self.assertEqual(r['negative_residuals'],1)
        self.assertEqual(r['distributions']['unclassified_between_world_ms']['median'],-10)

    def test_nonfinite_timestamp_refuses(self):
        r=copy.deepcopy(fixture());r['frame_trace']['rows'][0][0]=float('nan')
        with self.assertRaises(ValueError):summarize(r)

    def test_owner_and_gui_generation_must_join(self):
        r=fixture()
        for event in r['frame_trace']['gui_rows'][:7]:event[2]='99'
        with self.assertRaisesRegex(ValueError,'Owner and GUI'):
            summarize(r)

    def test_identity_is_exact_uint64_and_schema_is_current(self):
        for value in [True,1.5,-1,'01',str(2**64)]:
            r=fixture();r['frame_trace']['gui_rows'][0][1]=value
            with self.subTest(value=value),self.assertRaises(ValueError):summarize(r)
        for version in [1,2,3,5,True]:
            r=fixture();r['frame_trace']['schema']=version
            with self.subTest(version=version),self.assertRaises(ValueError):summarize(r)

    def test_duplicate_export_and_first_nonfinite_event_refuse(self):
        r=fixture();column=r['frame_trace']['columns'].index('export_serial')
        r['frame_trace']['rows'][1][column]='1'
        with self.assertRaisesRegex(ValueError,'Duplicate native export'):summarize(r)
        r=fixture();r['frame_trace']['gui_rows'][0][3]=float('nan')
        with self.assertRaises(ValueError):summarize(r)

    def test_unobserved_original_clock_breaks_pair(self):
        a=dict(draw_allowed=True,paused=False,source_before='63',source_after='64',
            original_tick=0,original_capture_ms=0,original_ack_wait_ms=0,original_return_ms=0)
        missing=dict(a,source_before='64',source_after='65');del missing['original_tick']
        b=dict(a,source_before='65',source_after='66',original_tick=100)
        r=original_cadence([a,missing,b])
        self.assertIsNone(r['distributions']['entry_interval_ms'])
        self.assertEqual(r['nonadjacent_pairs'],0)


if __name__=='__main__':unittest.main()
