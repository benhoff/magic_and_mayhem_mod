#!/usr/bin/env python3
"""Negative controls over actual retained native movie-to-Main executions."""
import copy,importlib.util,os,struct,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
spec=importlib.util.spec_from_file_location('movie_return',ROOT/'tools/close-rendering-movie-return.py');movie=importlib.util.module_from_spec(spec);spec.loader.exec_module(movie)


@unittest.skipUnless(os.environ.get('MNM_MOVIE_RETURN_RESULT'),'Supply the four-domain retained result for actual-case controls')
class RetainedMovieReturnTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        from coverage_evidence import Reader
        cls.reader=Reader(ROOT);cls.report=cls.reader.json(os.environ['MNM_MOVIE_RETURN_RESULT']);cls.execution=cls.report['executions'][0]
        cls.a=cls.reader.json(cls.execution['case']['record']);cls.media_inputs=movie.media_inputs(cls.reader,cls.a)
        cls.expected,_=movie.references(cls.reader);cls.menu=cls.execution['menu_proof']
    @classmethod
    def tearDownClass(cls):cls.reader.close()
    def media_args(self):return copy.deepcopy([self.a,*self.media_inputs,self.expected])
    def return_args(self):return copy.deepcopy([self.a,self.menu,self.execution['proof']['media']])
    def refuse_media(self,args):
        with self.assertRaises(ValueError):movie.completed_media(*args)
    def refuse_return(self,args):
        with self.assertRaises(ValueError):movie.return_state(*args)
    def test_actual_four_cases_rederive_media_and_initial_main_return(self):
        self.assertEqual(len(self.report['executions']),4)
        for e in self.report['executions']:
            a=self.reader.json(e['case']['record']);media=movie.completed_media(a,*movie.media_inputs(self.reader,a),self.expected)
            self.assertEqual(movie.return_state(a,e['menu_proof'],media),e['proof'])
        self.assertEqual(self.report['matched_decoded_frames'],11124);self.assertEqual(self.report['movie_gpu_pixels_compared'],0)
    def test_both_intros_and_exact_request_order_are_required(self):
        for transform in [lambda rows:rows[:-1],lambda rows:rows+[rows[-1]],lambda rows:[rows[0],rows[2],rows[1]]]:
            args=self.media_args();args[1]=transform(args[1]);self.refuse_media(args)
    def test_native_probe_and_audio_factory_admission_cannot_be_assumed(self):
        args=self.media_args();args[1][0]['native_pipeline_available']=False;self.refuse_media(args)
        for field in ['native_audio_factory','native_media','native_media_startup_probe','wine_directsound_disabled']:
            args=self.media_args();args[3][field]=False
            with self.subTest(field=field):self.refuse_media(args)
    def test_skipped_rejected_or_failed_movie_is_not_natural_return(self):
        for field,value in [('status',4),('accepted',False),('result',4),('error','decoder failure')]:
            args=self.media_args();args[1][1][field]=value
            with self.subTest(field=field):self.refuse_media(args)
    def test_full_frame_counts_shape_and_ordered_hash_are_required(self):
        for field,value in [('video_frames',1776),('presented_video_frames',1776),('width',639),('height',359),('video_rgba_sha256','0'*64)]:
            args=self.media_args();args[1][1][field]=value
            with self.subTest(field=field):self.refuse_media(args)
    def test_encoded_input_must_match_the_independent_reference(self):
        args=self.media_args();args[0]['installation_inputs']['FMV/Intro0.avi']['sha256']='0'*64;self.refuse_media(args)
        args=self.media_args();args[4].pop('intro1.avi');self.refuse_media(args)
    def test_hidden_or_caller_paused_video_does_not_pass(self):
        for field,value in [('video_presentation_requested',False),('caller_paused',True)]:
            args=self.media_args();args[1][1][field]=value
            with self.subTest(field=field):self.refuse_media(args)
    def test_unadmitted_silence_loss_or_missing_pcm_is_rejected(self):
        for field,value in [('audible_output_requested',False),('audio_device_available',False),('initial_silent_output',True),('waiting_for_audio_output',True),('audio_frames',0),('audio_bytes',1),('output_losses',1),('output_recoveries',1),('default_output_changes',1)]:
            args=self.media_args();args[1][1][field]=value
            with self.subTest(field=field):self.refuse_media(args)
    def test_channel_magic_version_and_complete_extent_are_required(self):
        for change in [lambda raw:b'badmagic'+raw[8:],lambda raw:raw[:64],lambda raw:raw+b'\0']:
            args=self.media_args();args[2]=change(args[2]);self.refuse_media(args)
        args=self.media_args();raw=bytearray(args[2]);struct.pack_into('<I',raw,8,2);args[2]=bytes(raw);self.refuse_media(args)
    def test_publication_sequences_must_be_stable_and_complete(self):
        for name,value in [('REQUEST_SEQUENCE',5),('REQUEST_SEQUENCE',8),('RESPONSE_SEQUENCE',13),('RESPONSE_SEQUENCE',0)]:
            args=self.media_args();raw=bytearray(args[2]);struct.pack_into('<I',raw,getattr(movie.wire,name+'_OFFSET'),value);args[2]=bytes(raw)
            with self.subTest(name=name,value=value):self.refuse_media(args)
    def test_acknowledged_identity_completion_and_no_timeout_cancel_are_required(self):
        for name,value in [('REQUEST_ID',4),('RESPONSE_ID',2),('ACCEPTED_ID',0),('RESPONSE_STATUS',4),('OPERATION',2),('FLAGS',1),('CANCELLED_REQUEST_ID',3),('SOUND_CANCEL_GENERATION',1),('HEARTBEAT',0)]:
            args=self.media_args();raw=bytearray(args[2]);struct.pack_into('<I',raw,getattr(movie.wire,name+'_OFFSET'),value);args[2]=bytes(raw)
            with self.subTest(name=name):self.refuse_media(args)
    def test_last_intro_path_and_reserved_wire_bytes_are_bound(self):
        args=self.media_args();args[2]=args[2].replace(b'intro1.avi',b'intro0.avi');self.refuse_media(args)
        args=self.media_args();raw=bytearray(args[2]);raw[532]=1;args[2]=bytes(raw);self.refuse_media(args)
    def test_initial_main_requires_painted_visible_enabled_focus(self):
        for field in ['focused','enabled','showing']:
            args=self.return_args();args[1]['main_ready'][0]['target'][field]=False
            with self.subTest(field=field):self.refuse_return(args)
        args=self.return_args();args[1]['visible_main_presentations']=[v for v in args[1]['visible_main_presentations'] if v['label']!='main-ready-initial'];self.refuse_return(args)
    def test_initial_public_input_original_callbacks_and_nominal_retirement_are_required(self):
        args=self.return_args();next(v for v in args[0]['actions'] if v['action']=='click' and v['name']=='Preferences')['physical']['down_up']=False;self.refuse_return(args)
        args=self.return_args();args[1]['original_callbacks'][0]=[3,2];self.refuse_return(args)
        for field,value in [('success',False),('sources_stable',False),('original_game_runs',0),('private_wine_stopped',False),('original_manifest_verified_before_after',False),('forced_private_prefix_stop',True)]:
            args=self.return_args();args[0][field]=value
            with self.subTest(field=field):self.refuse_return(args)
        args=self.return_args();args[0]['resident']['imports']='1';self.refuse_return(args)
    def test_raw_media_artifact_tampering_is_not_masked_by_success_metadata(self):
        target=next(v['record'] for v in self.a['artifacts'] if v['path'].endswith('/native-media.bin'));reader=self.reader
        class Changed:
            def read_bytes(self,name):
                raw=reader.read_bytes(name);return bytes([raw[0]^1])+raw[1:] if name==target else raw
        with self.assertRaises(ValueError):movie.media_inputs(Changed(),self.a)
    def test_only_two_return_rows_change_and_all_contexts_are_required(self):
        parent=self.reader.json(self.report['accepted_parent']['binding']['record']);previous=self.reader.json(parent['inputs']['progress']['record']);before=copy.deepcopy(previous)
        plan=self.reader.json('docs/native-rendering-closure.json');runs=self.report['executions'];rows=movie.close_rows(plan,previous,runs)
        self.assertEqual(previous,before);self.assertEqual(sum(v['state']=='passed' for v in rows),38)
        for a,b in zip(previous['requirements'],rows):
            if a['id']!=movie.LEAF:self.assertEqual(a,b)
        for bad in [runs[:-1],runs[:-1]+[runs[0]],runs+[runs[0]]]:
            with self.assertRaises(ValueError):movie.close_rows(plan,previous,bad)
        bad=copy.deepcopy(runs);bad[0]['configuration']['software_flag']=0
        with self.assertRaises(ValueError):movie.close_rows(plan,previous,bad)


if __name__=='__main__':unittest.main()
