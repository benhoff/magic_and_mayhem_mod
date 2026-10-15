from pathlib import Path
import copy,struct,sys,tempfile,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_api_session import sha
from drawing_source_frame_binding import evaluate


class SourceFrames(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.directory=Path(self.temp.name)
        raw=struct.pack('<4H',0x07e0,0xf800,31,0xffff);front=bytearray(bytes([11,12,13,255])*16)
        for (x,y),rgb in zip([(1,1),(2,1),(1,2),(2,2)],[(0,255,0),(255,0,0),(0,0,255),(255,255,255)]):front[4*(y*4+x):4*(y*4+x)+3]=bytes(rgb)
        (self.directory/'source.565').write_bytes(raw);(self.directory/'front.rgba').write_bytes(front)
        self.calls=[dict(call=1,method=1,api_entry=9,thread=7,hresult=0,flags=0x41000000,first_object=4,first_generation=2,second_object=3,second_generation=1,lifetime_prefix=5,source_rect=[0,0,2,2],destination_rect=[1,1,3,3])]
        self.samples=[dict(submission=1,status=0,api_entry=9,thread=7,source_texture=3,source_generation=1,lifetime_before=5,payload_ordinal=1,descriptor=[116,0,0,0,0,13,2,2,1,8])]
        self.payloads=[dict(ordinal=1,path='source.565',sha256=sha(raw),width=2,height=2)]
        self.frames=[dict(entry=9,ordinal=1,thread=7,path='front.rgba',sha256=sha(front),width=4,height=4)]
    def tearDown(self):self.temp.cleanup()
    def run_model(self):return evaluate(self.calls,self.samples,self.payloads,self.frames,self.directory)
    def test_independent_copy_colors_and_crop(self):
        r=self.run_model();self.assertEqual(r['matched_pixels'],4);self.assertTrue(r['all_witness_copy_regions_bound']);self.assertFalse(r['native_renderer_parity_performed']);self.assertFalse(r['complete_presented_journey'])
        self.calls[0]['source_rect']=[1,0,2,2];self.calls[0]['destination_rect']=[2,1,3,3];self.assertEqual(self.run_model()['matched_pixels'],2)
    def test_rgb_tolerance_and_alpha_policy(self):
        raw=bytearray((self.directory/'front.rgba').read_bytes());raw[4*5+1]=254;raw[4*5+3]=1;(self.directory/'front.rgba').write_bytes(raw);self.frames[0]['sha256']=sha(raw)
        r=self.run_model();self.assertEqual(r['matched_regions'],1);self.assertEqual(r['comparisons'][0]['max_channel_difference'],1);self.assertFalse(r['alpha_compared'])
    def test_identity_generation_thread_lifetime(self):
        for obj,key,value in [(self.samples[0],'source_generation',2),(self.samples[0],'source_texture',5),(self.samples[0],'api_entry',8),(self.samples[0],'lifetime_before',4),(self.frames[0],'thread',8)]:
            old=obj[key];obj[key]=value;r=self.run_model();self.assertEqual(len(r['refusals']),1);self.assertFalse(r['all_witness_copy_regions_bound']);obj[key]=old
    def test_missing_source_unsupported_copy_and_later_write(self):
        for field,value in [('hresult',0x80000000),('flags',0x41008000)]:
            old=self.calls[0][field];self.calls[0][field]=value;self.assertEqual(len(self.run_model()['refusals']),1);self.calls[0][field]=old
        later=copy.deepcopy(self.calls[0]);later['call']=2;self.calls.append(later);r=self.run_model();self.assertEqual(r['refusals'][0]['reason'],'source_pixels_not_retained');self.assertEqual(r['compared_regions'],0)
    def test_scaled_clipped_and_wrong_payload_geometry(self):
        for key,value in [('source_rect',[0,0,3,2]),('destination_rect',[1,1,4,3]),('destination_rect',[-1,0,1,2])]:
            old=self.calls[0][key];self.calls[0][key]=value;self.assertEqual(len(self.run_model()['refusals']),1);self.calls[0][key]=old
        self.samples[0]['descriptor'][6]=3;self.assertEqual(len(self.run_model()['refusals']),1)
    def test_duplicates_and_call_order(self):
        for items in (self.calls,self.samples,self.payloads,self.frames):
            items.append(copy.deepcopy(items[0]))
            with self.assertRaises(ValueError):self.run_model()
            items.pop()
    def test_bound_hash_size_paths_and_symlinks(self):
        for payload in (self.payloads[0],self.frames[0]):
            old=payload['sha256'];payload['sha256']='0'*64
            with self.assertRaises(ValueError):self.run_model()
            payload['sha256']=old
        old=self.payloads[0]['path'];self.payloads[0]['path']='../source.565'
        with self.assertRaises(ValueError):self.run_model()
        self.payloads[0]['path']=old;source=self.directory/old;target=self.directory/'target.565';source.rename(target);source.symlink_to(target)
        with self.assertRaises(ValueError):self.run_model()
    def test_black_mismatch_and_missing_witness_are_retained(self):
        raw=bytes(4*4*4);(self.directory/'front.rgba').write_bytes(raw);self.frames[0]['sha256']=sha(raw);self.frames.append(dict(self.frames[0],entry=10,ordinal=2));r=self.run_model();self.assertEqual(r['mismatched_regions'],1);self.assertEqual(r['comparisons'][0]['pixels_over_one'],4);self.assertEqual(len(r['refusals']),1);self.assertFalse(r['all_compared_regions_match'])


if __name__=='__main__':unittest.main()
