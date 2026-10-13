from pathlib import Path
import copy
import importlib.util
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_source_worker_egl import bind, delivery_boundary, whole_image_agreement

# Reuse the independent four-colour input, not the model's colour conversion.
spec = importlib.util.spec_from_file_location('source_frames', Path(__file__).with_name('test-drawing-source-frame-binding.py'))
fixture = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixture)


class SourceWorkerEGL(unittest.TestCase):
    def setUp(self):
        fixture.SourceFrames.setUp(self)
        self.calls[0].update(source_subresource=0, destination_subresource=0)
        self.samples[0].update(subresource=0, lifetime_after=5)
        self.packets = [dict(command=2, submission=1, emit_thread=7, worker_thread=8, lifetime_prefix=5, worker=3, emit_record=1, worker_entry_record=2, worker_return_record=3)]
        self.workers = [dict(call=3, thread=8, lifetime_prefix=5, source=3, source_generation=1, destination=4, destination_generation=2, source_subresource=0, destination_subresource=0, source_box=[0,0,2,2,0,1], destination_box=[1,1,3,3,0,1], flags=0x41000000, effects=[0]*6, filter=1)]
        value = dict(sequence=6, active=True, worker=3, command=2, submission=1, win_thread=8, lifetime_prefix=5, flags=0)
        self.contexts = [dict(pid=11, call=12, stable_active_context=True, points=[dict(status=0, pid=11, unix_thread=13, context=copy.deepcopy(value)), dict(status=0, pid=11, unix_thread=13, context=copy.deepcopy(value))])]
        self.frames[0].update(pid=11, entry=12, thread=13)
        self.api_calls = [dict(entry=9, thread=7, caller=0x58c000, method=7, hresult=0, last_error=4)]

    def tearDown(self):
        fixture.SourceFrames.tearDown(self)

    def run_model(self):
        return bind(self.calls, self.samples, self.payloads, self.packets, self.workers, self.contexts, self.frames, self.directory, self.api_calls)

    def test_exact_identity_and_separate_threads(self):
        r = self.run_model()
        self.assertTrue(r['all_observed_sources_worker_bound'])
        self.assertTrue(r['all_physical_copy_regions_bound'])
        self.assertEqual(r['matched_pixels'], 4)
        c = r['comparisons'][0]
        self.assertEqual((c['submission'], c['command'], c['worker'], c['egl_call'], c['win_thread'], c['unix_thread']), (1,2,3,12,8,13))
        self.assertEqual((c['api_caller'], c['api_method'], c['api_hresult'], c['api_last_error']), (0x58c000,7,0,4))
        for k in ('original_source_atomicity_proved', 'all_alias_writers_accounted', 'scanout_proved', 'native_renderer_parity_performed', 'complete_presented_journey'):
            self.assertFalse(r[k])

    def test_source_worker_generation_subresource_geometry_and_order(self):
        changes = [(self.workers[0], 'source_generation', 6), (self.workers[0], 'destination_generation', 6), (self.workers[0], 'source_subresource', 1), (self.workers[0], 'source_box', [1,0,2,2,0,1]), (self.workers[0], 'flags', 1), (self.packets[0], 'emit_thread', 9), (self.packets[0], 'worker_thread', 9), (self.packets[0], 'worker_return_record', 1), (self.packets[0], 'lifetime_prefix', 4), (self.samples[0], 'lifetime_after', 6), (self.samples[0], 'source_texture', 8), (self.samples[0], 'status', 3)]
        for obj, key, value in changes:
            with self.subTest(key=key):
                old = obj[key]; obj[key] = value
                r = self.run_model()
                self.assertEqual(len(r['source_refusals']), 1)
                self.assertEqual(r['compared_regions'], 0)
                self.assertFalse(r['all_observed_sources_worker_bound'])
                obj[key] = old

    def test_context_command_generation_thread_and_unavailable(self):
        point = self.contexts[0]['points'][0]
        for obj, key, value in [(point['context'], 'command', 9), (point['context'], 'win_thread', 9), (point['context'], 'lifetime_prefix', 9), (point, 'status', 1), (self.frames[0], 'thread', 99), (self.workers[0], 'filter', 2), (self.workers[0], 'effects', [1]*6)]:
            old = obj[key]; obj[key] = value
            self.assertEqual(len(self.run_model()['refusals']), 1)
            obj[key] = old
        self.contexts[0]['stable_active_context'] = False
        self.assertEqual(self.run_model()['compared_regions'], 0)

    def test_all_missing_sources_and_frames_remain_explicit(self):
        self.packets.clear()
        r = self.run_model()
        self.assertEqual(len(r['source_refusals']), 1)
        self.assertEqual(len(r['refusals']), 1)
        self.assertFalse(r['all_physical_copy_regions_bound'])
        self.contexts.clear()
        self.assertEqual(self.run_model()['refusals'][0]['reason'], 'missing_unstable_or_inactive_worker_context')

    def test_duplicate_identities_refuse(self):
        for items in (self.calls, self.samples, self.payloads, self.packets, self.workers, self.contexts, self.frames, self.api_calls):
            items.append(copy.deepcopy(items[0]))
            with self.assertRaises(ValueError):
                self.run_model()
            items.pop()

    def test_enclosing_api_identity_and_failed_outcome_remain_explicit(self):
        self.api_calls[0]['thread'] = 99
        self.assertEqual(self.run_model()['source_refusals'][0]['reason'], 'missing_or_changed_enclosing_api')
        self.api_calls[0]['thread'] = 7
        self.api_calls[0]['hresult'] = -1
        self.assertEqual(self.run_model()['source_worker_bindings'][0]['api_hresult'], -1)
        self.api_calls.clear()
        self.assertEqual(len(self.run_model()['source_refusals']), 1)

    def test_delivery_boundary_uses_exact_identity_and_one_clock(self):
        egl = self.run_model()['comparisons'][0]
        api = dict(egl, frame_ordinal=6, physical_sha256='a'*64, pixels_match=False)
        timing = dict(entry=9, ordinal=6, sha256='a'*64, clock_domain='linux_CLOCK_MONOTONIC', first_capture=dict(begin_ns='10', end_ns='11'), second_capture=dict(begin_ns='12', end_ns='13'))
        context = copy.deepcopy(self.contexts[0])
        context['points'][0]['time_ns']=20; context['points'][1]['time_ns']=30
        r = delivery_boundary(api, egl, timing, context)
        self.assertTrue(r['api_mismatch_before_matching_egl'])
        context['points'][0]['time_ns']=12
        self.assertFalse(delivery_boundary(api, egl, timing, context)['api_mismatch_before_matching_egl'])
        context['points'][0]['time_ns']=20
        for obj, key, value in [(api,'submission',99), (api,'source_sha256','b'*64), (timing,'entry',99), (context,'call',99), (timing,'clock_domain','Windows_QPC')]:
            old=obj[key]; obj[key]=value
            with self.assertRaises(ValueError):delivery_boundary(api,egl,timing,context)
            obj[key]=old

    def test_whole_image_is_distinct_from_matching_copy_region(self):
        r=self.run_model()
        self.assertEqual(r['matched_regions'],1)
        self.assertEqual(r['whole_source_images_compared'],0)
        self.assertFalse(r['all_physical_whole_images_agree'])
        self.assertEqual(r['comparisons'][0]['whole_source_image_agreement']['reason'],'source_and_physical_extents_differ')

    def test_whole_image_independent_colours_mismatch_and_hash(self):
        from drawing_api_session import sha
        raw=bytes([0,255,0,255,255,0,0,255,0,0,255,255,255,255,255,255])
        path=self.directory/'whole.rgba';path.write_bytes(raw)
        frame=dict(width=2,height=2,path=path.name,sha256=sha(raw))
        r=whole_image_agreement(self.payloads[0],frame,self.directory)
        self.assertTrue(r['pixels_match']);self.assertEqual(r['pixels'],4)
        path.write_bytes(bytes(16));frame['sha256']=sha(bytes(16))
        self.assertEqual(whole_image_agreement(self.payloads[0],frame,self.directory)['pixels_over_one'],4)
        frame['sha256']='0'*64
        with self.assertRaises(ValueError):whole_image_agreement(self.payloads[0],frame,self.directory)

    def test_mismatch_hash_and_unwitnessed_context_preserved(self):
        from drawing_api_session import sha
        raw = bytes(4*4*4)
        (self.directory/'front.rgba').write_bytes(raw)
        self.frames[0]['sha256'] = sha(raw)
        self.contexts.append(dict(self.contexts[0], call=99))
        r = self.run_model()
        self.assertEqual(r['mismatched_regions'], 1)
        self.assertEqual(r['comparisons'][0]['pixels_over_one'], 4)
        self.assertEqual(len(r['contexts_without_physical_witness']), 1)
        self.frames[0]['sha256'] = '0'*64
        with self.assertRaises(ValueError):
            self.run_model()


if __name__ == '__main__':
    unittest.main()
