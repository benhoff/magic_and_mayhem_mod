import copy,struct,sys,tempfile,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_api_session import sha
from drawing_cursor_map_composition import evaluate


def fixture(directory):
    def call(n,dest,dgen,src,sgen,srect,drect,flags,entry):
        return dict(call=n,method=1,thread=10,api_entry=entry,first_object=dest,first_generation=dgen,second_object=src,second_generation=sgen,source_rect=srect,destination_rect=drect,flags=flags,hresult=0)
    area=[30,20,70,60];tile=[0,0,40,40];full=[0,0,800,600]
    calls=[call(1,2,6,9,10,area,tile,0,10),call(2,9,10,3,7,tile,area,0x8000,20),call(3,4,8,9,10,full,full,0,30),call(4,9,10,2,6,tile,area,0,40)]
    samples=[dict(submission=2,status=0,shape=2,source_texture=3,source_generation=7,payload_ordinal=1),dict(submission=4,status=0,shape=1,source_texture=2,source_generation=6,payload_ordinal=2)]
    atlas=bytearray(struct.pack('<H',31)*(400*280));bg=struct.pack('<H',0xf800)*1600;physical=bytearray(bytes([0,0,0,255])*(800*600))
    for y in range(40):
        for x in range(40):
            green=x%7==0 or y%7==0
            if green:struct.pack_into('<H',atlas,2*(y*400+x),0x07e0)
            at=4*((20+y)*800+30+x);physical[at:at+4]=bytes([0,255,0,255] if green else [255,0,0,255])
    payloads=[]
    for ordinal,data,width,height in [(1,bytes(atlas),400,280),(2,bg,40,40)]:
        name=f'wine-source-map-{ordinal:06d}.565';(directory/name).write_bytes(data);payloads.append(dict(ordinal=ordinal,path=name,width=width,height=height,sha256=sha(data)))
    name='api-physical-oracle-000001.rgba';(directory/name).write_bytes(physical)
    frames=[dict(entry=30,ordinal=1,thread=10,width=800,height=600,path=name,sha256=sha(physical))]
    return calls,samples,payloads,frames


class CursorBinding(unittest.TestCase):
    def setUp(self):self.temp=tempfile.TemporaryDirectory();self.directory=Path(self.temp.name);self.data=fixture(self.directory)
    def tearDown(self):self.temp.cleanup()
    def run_model(self):return evaluate(*self.data,self.directory)
    def test_independent_red_background_green_keyed_overlay(self):
        r=self.run_model();self.assertEqual(r['matched_cycles'],1);self.assertEqual(r['matched_pixels'],1600);self.assertEqual(r['candidates'][0]['max_channel_difference'],0);self.assertFalse(r['native_renderer_parity_performed'])
    def test_generation_and_latest_writer_are_not_guessed(self):
        self.data[0][0]['second_generation']=99;r=self.run_model();self.assertEqual(r['matched_cycles'],0);self.assertEqual(r['refusals'][0]['reason'],'latest_writer_does_not_bind_save')
    def test_intervening_writer_and_ambiguous_draw_refuse(self):
        calls=self.data[0];interloper=copy.deepcopy(calls[0]);interloper.update(call=2,second_object=88)
        for c in calls[1:]:c['call']+=1
        self.data[1][0]['submission']=3;self.data[1][1]['submission']=5;calls.insert(1,interloper)
        self.assertEqual(self.run_model()['matched_cycles'],0)
        self.data=fixture(self.directory);calls=self.data[0];second=copy.deepcopy(calls[1]);second['call']=3
        for c in calls[2:]:c['call']+=1
        self.data[1][1]['submission']=5;calls.insert(2,second)
        self.assertEqual(self.run_model()['refusals'][0]['reason'],'missing_or_ambiguous_keyed_draw')
    def test_missing_sample_physical_thread_and_geometry(self):
        self.data[1].pop(0);self.assertEqual(self.run_model()['refusals'][0]['reason'],'missing_sampled_overlay')
        self.data=fixture(self.directory);self.data[3][0]['thread']=11;self.assertEqual(self.run_model()['refusals'][0]['reason'],'physical_thread')
        self.data=fixture(self.directory);self.data[0][1]['source_rect']=[0,0,39,40];self.assertEqual(self.run_model()['refusals'][0]['reason'],'unsupported_or_clipped_geometry')
    def test_hash_and_path_corruption_cannot_pass(self):
        self.data[2][0]['sha256']='0'*64
        with self.assertRaises(ValueError):self.run_model()
        self.data=fixture(self.directory);self.data[3][0]['path']='../bad.rgba'
        with self.assertRaises(ValueError):self.run_model()
    def test_black_physical_mismatch_is_preserved(self):
        frame=self.data[3][0];raw=bytes([0,0,0,255])*(800*600);(self.directory/frame['path']).write_bytes(raw);frame['sha256']=sha(raw)
        r=self.run_model();self.assertEqual(r['mismatched_cycles'],1);self.assertFalse(r['all_candidate_pixels_match']);self.assertEqual(r['candidates'][0]['pixels_over_one'],1600)
    def test_calls_need_unique_monotone_order(self):
        self.data[0][1]['call']=1
        with self.assertRaises(ValueError):self.run_model()


    def test_duplicate_samples_payloads_and_missing_payload_refuse(self):
        for index in (1,2):
            self.data=fixture(self.directory);self.data[index].append(copy.deepcopy(self.data[index][0]))
            with self.assertRaises(ValueError):self.run_model()
        self.data=fixture(self.directory);self.data[2].pop()
        self.assertEqual(self.run_model()['refusals'][0]['reason'],'missing_sampled_payload')

    def test_intervening_destination_write_and_multiple_presents_refuse(self):
        for duplicate_present in (False,True):
            self.data=fixture(self.directory);calls=self.data[0]
            extra=copy.deepcopy(calls[2] if duplicate_present else calls[1])
            # An unkeyed destination writer would previously escape draw counting.
            extra.update(call=3,flags=0)
            if not duplicate_present:extra['source_rect']=[0,0,1,1];extra['destination_rect']=[0,0,1,1]
            calls[2]['call']=4;calls[3]['call']=5;calls.insert(2,extra)
            self.data[1][1]['submission']=5
            r=self.run_model();self.assertEqual(r['matched_cycles'],0)
            self.assertIn(r['refusals'][0]['reason'],('intervening_composed_surface_write','missing_or_ambiguous_physical_present'))

if __name__=='__main__':unittest.main()
