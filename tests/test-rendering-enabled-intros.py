#!/usr/bin/env python3
"""Refusal controls over real public intro capture and decoded reference data."""
import copy,importlib.util,json,os,struct,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
s=importlib.util.spec_from_file_location('intro',ROOT/'tools/close-rendering-enabled-intros.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m)


@unittest.skipUnless(os.environ.get('MNM_INTRO_RESULT'),'Supply actual intro output result')
class IntroOutputTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.reader=m.Reader(ROOT);cls.report=cls.reader.json(os.environ['MNM_INTRO_RESULT']);cls.cases=cls.report['cases']
        cls.a=cls.reader.json(cls.cases[0]['case']['record']);ref=cls.reader.json(m.PREFIX+'independent-reference-01.json')
        cls.expected={v['name']:v for v in ref['movies']};cls.references={n:cls.reader.read_bytes(v['reference']['record']) for n,v in cls.expected.items()}
        stream=next(v for v in cls.a['artifacts'] if '/capture/' in v['path'] and Path(v['path']).name.startswith('viewport-'))
        cls.rows=m.capture(m.artifact(cls.reader,cls.a,stream['path']))
        cls.index=next(i for i,(r,b) in enumerate(cls.rows) if r['kind']=='paint' and len(b)>0)
        cls.row,body=cls.rows[cls.index];cls.body=body() if callable(body) else body
        cls.e=cls.expected['intro0.avi']['expected'];cls.size=cls.e['width']*cls.e['height']*4
        seq=int(cls.row['image']);inputs=[r for r,b in cls.rows if r['kind']=='image' and r['width']>0]
        image=next(r for r in inputs if int(r['image'])==seq);ordinal=inputs.index(image)
        cls.frame=cls.references['intro0.avi'][ordinal*cls.size:(ordinal+1)*cls.size]
    @classmethod
    def tearDownClass(cls):cls.reader.close()
    def check_paint(self,row=None,body=None,frame=None):
        return m.compare_paint(row or self.row,self.body if body is None else body,self.frame if frame is None else frame,self.e['width'],self.e['height'])
    def refuse_paint(self,**changes):
        row=dict(self.row,**changes)
        with self.assertRaises(ValueError):self.check_paint(row)
    def refuse_policy(self,**changes):
        opened=dict(self.rows[0][0],**changes)
        closed=dict(kind='close',index=1,host_us=opened['host_us'],complete=True,error='',raw_bytes='0',prior_rows=1)
        raw=b'MNMACE01'
        for row in (opened,closed):
            data=json.dumps(row).encode();raw+=struct.pack('<II',len(data),0)+data
        with self.assertRaisesRegex(ValueError,'Unreviewed movie capture policy'):m.capture(raw)
    def test_movie_capture_policy_has_exact_separate_budgets(self):
        r=self.rows[0][0];self.assertEqual((r['version'],r['policy'],r['stored_limit_bytes']),(3,'movie-lossless-viewport-v1',2*1024**3))
        self.assertEqual((r['raw_limit_bytes'],r['row_limit'],r['queue_limit_bytes'],r['queue_limit_rows'],r['keyframe_paints']),(64*1024**3,131072,256*1024**2,4096,128))
    def test_movie_stored_budget_cannot_expand(self):self.refuse_policy(stored_limit_bytes=3*1024**3)
    def test_movie_raw_budget_cannot_expand(self):self.refuse_policy(raw_limit_bytes=65*1024**3)
    def test_movie_row_budget_cannot_expand(self):self.refuse_policy(row_limit=131073)
    def test_movie_queue_budget_cannot_expand(self):self.refuse_policy(queue_limit_bytes=257*1024**2)
    def test_movie_queue_rows_cannot_expand(self):self.refuse_policy(queue_limit_rows=4097)
    def test_movie_codec_key_bound_cannot_expand(self):self.refuse_policy(keyframe_paints=129)
    def test_movie_policy_cannot_change_roles(self):self.refuse_policy(role='commands')
    def test_actual_presentation_spans_both_clips_with_changing_content(self):
        values={n:set(v) for n,v in self.cases[0]['output']['presented_frame_ordinals'].items()}
        m.presentation_coverage(values,self.expected)
    def test_first_and_last_frames_alone_cannot_close_movies(self):
        values={n:{0,len(v['frames'])-1} for n,v in self.expected.items()}
        with self.assertRaises(ValueError):m.presentation_coverage(values,self.expected)
    def test_missing_movie_quarter_cannot_close_output(self):
        values={n:set(v) for n,v in self.cases[0]['output']['presented_frame_ordinals'].items()}
        name='intro0.avi';values[name]={i for i in values[name] if 4*i//len(self.expected[name]['frames'])!=2}
        with self.assertRaises(ValueError):m.presentation_coverage(values,self.expected)
    def single_proof(self):
        ref=self.report['inputs'][0] if self.report.get('reused_current_case_proofs') else None
        return self.reader.json(ref['record']) if ref else copy.deepcopy(self.report)
    def test_single_case_source_identity_is_required_for_reuse(self):
        p=self.single_proof();m.check_single_proof(p,m.analysis_sources(self.reader))
        p['analysis_sources']={}
        with self.assertRaises(ValueError):m.check_single_proof(p,m.analysis_sources(self.reader))
    def test_single_case_failures_cannot_be_reused(self):
        p=self.single_proof();p['success']=False
        with self.assertRaises(ValueError):m.check_single_proof(p,m.analysis_sources(self.reader))
    def test_single_case_pixel_summary_cannot_be_rebound(self):
        p=self.single_proof();p['compared_pixels']+=1
        with self.assertRaises(ValueError):m.check_single_proof(p,m.analysis_sources(self.reader))
    def test_incomplete_or_duplicate_single_case_cannot_be_reused(self):
        for cases in ([],self.single_proof()['cases']*2):
            p=self.single_proof();p['cases']=cases
            with self.assertRaises(ValueError):m.check_single_proof(p,m.analysis_sources(self.reader))
    def gui_row(self):
        row,body=next((r,b) for r,b in self.rows if r['kind']=='paint' and r.get('native_gpu_frame'))
        return dict(row),len(body)
    def test_actual_post_intro_gui_has_no_movie_pixel_credit(self):
        row,size=self.gui_row();m.post_movie_gui(row,size,2781,2781)
        self.assertEqual(self.cases[0]['output']['post_movie_gui_pixel_credit'],0)
        self.assertGreater(len(self.cases[0]['output']['post_movie_gui_paints']),0)
    def test_gpu_output_during_incomplete_movie_cannot_be_excluded(self):
        row,size=self.gui_row()
        with self.assertRaises(ValueError):m.post_movie_gui(row,size,2780,2781)
    def test_world_output_cannot_use_gui_exclusion(self):
        row,size=self.gui_row();row['world']='1'
        with self.assertRaises(ValueError):m.post_movie_gui(row,size,2781,2781)
    def test_unknown_owner_cannot_use_gui_exclusion(self):
        row,size=self.gui_row();row['presentation_owner']='other'
        with self.assertRaises(ValueError):m.post_movie_gui(row,size,2781,2781)
    def test_missing_live_gui_lease_cannot_be_excluded(self):
        row,size=self.gui_row();row['lease']='0'
        with self.assertRaises(ValueError):m.post_movie_gui(row,size,2781,2781)
    def test_actual_whole_viewport_matches_independent_movie(self):self.assertEqual(self.check_paint(),self.row['width']*self.row['height'])
    def test_letterbox_pixel_corruption_refuses(self):
        raw=bytearray(self.body);raw[0]^=1
        with self.assertRaises(ValueError):self.check_paint(body=raw)
    def test_visible_pixel_corruption_refuses(self):
        raw=bytearray(self.body);at=(self.row['top']*self.row['width']+self.row['left'])*4;raw[at]^=1
        with self.assertRaises(ValueError):self.check_paint(body=raw)
    def test_alpha_corruption_refuses(self):
        raw=bytearray(self.body);raw[3]^=1
        with self.assertRaises(ValueError):self.check_paint(body=raw)
    def test_wrong_decoded_frame_refuses(self):
        frame=bytearray(self.frame);frame[0]^=1
        with self.assertRaises(ValueError):self.check_paint(frame=frame)
    def test_missing_body_refuses(self):
        with self.assertRaises(ValueError):self.check_paint(body=b'')
    def test_truncated_body_refuses(self):
        with self.assertRaises(ValueError):self.check_paint(body=self.body[:-1])
    def test_readback_required(self):self.refuse_paint(gpu_read_complete=False)
    def test_pixel_capture_required(self):self.refuse_paint(pixels_captured=False)
    def test_world_source_refuses(self):self.refuse_paint(world='1')
    def test_lease_source_refuses(self):self.refuse_paint(lease='1')
    def test_native_gpu_movie_inference_refuses(self):self.refuse_paint(native_gpu_frame=True)
    def test_raster_substitution_refuses(self):self.refuse_paint(transport='raster')
    def test_unreviewed_smooth_sampling_refuses(self):self.refuse_paint(scaling=1)
    def test_wrong_letterbox_geometry_refuses(self):self.refuse_paint(left=self.row['left']+1)
    def test_wrong_frame_extent_refuses(self):self.refuse_paint(frame_width=self.e['width']+1)
    def test_missing_submitted_sequence_refuses(self):
        rows=list(self.rows);at=next(i for i,(r,b) in enumerate(rows) if r['kind']=='image' and r['width']>0);rows.pop(at)
        with self.assertRaises(ValueError):m.viewport(rows,self.expected,self.references)
    def test_submitted_image_digest_refuses(self):
        rows=list(self.rows);at=next(i for i,(r,b) in enumerate(rows) if r['kind']=='image' and r['width']>0);r,b=rows[at];rows[at]=(dict(r,rgba_sha256='0'*64),b)
        with self.assertRaises(ValueError):m.viewport(rows,self.expected,self.references)
    def test_paint_source_identity_refuses(self):
        rows=list(self.rows);r,b=rows[self.index];rows[self.index]=(dict(r,image='999999'),b)
        with self.assertRaises(ValueError):m.viewport(rows,self.expected,self.references)
    def test_unbound_swap_refuses(self):
        rows=list(self.rows);rows.insert(1,({'kind':'swap','paint':'999999'},b''))
        with self.assertRaises(ValueError):m.viewport(rows,self.expected,self.references)
    def test_both_movie_streams_required(self):
        rows=[v for v in self.rows if v[0]['kind']!='image' or v[0]['width']==0 or v[0]['image']!='2000']
        with self.assertRaises(ValueError):m.viewport(rows,self.expected,self.references)
    def test_natural_eof_required(self):
        args=[copy.deepcopy(self.a),*copy.deepcopy(m.movie.media_inputs(self.reader,self.a)),{n:v['expected'] for n,v in self.expected.items()}];args[1][-1]['status']=4
        with self.assertRaises(ValueError):m.movie.completed_media(*args)
    def test_only_two_rows_change_and_all_four_domains_required(self):
        if len(self.cases)!=4:self.skipTest('Four-domain final join required')
        parent=self.reader.json(m.PARENT);plan=self.reader.json('docs/native-rendering-closure.json');before=copy.deepcopy(parent)
        rows=m.close_rows(plan,parent,self.cases);self.assertEqual(parent,before)
        for old,row in zip(parent['requirements'],rows):
            if old['id']!=m.LEAF:self.assertEqual(old,row)
        self.assertEqual(sum(r['state']=='passed' for r in rows),76)
        for cases in (self.cases[:-1],self.cases+[self.cases[0]],self.cases[:-1]+[self.cases[0]]):
            with self.assertRaises(ValueError):m.close_rows(plan,parent,cases)


if __name__=='__main__':unittest.main()
