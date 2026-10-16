import sys
from pathlib import Path
import unittest
import struct
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from importlib.machinery import SourceFileLoader
m=SourceFileLoader('quick_battle',str(Path(__file__).resolve().parents[1]/'tools/compare-drawing-quick-battle.py')).load_module()


class Cases(unittest.TestCase):
    def test_entire_plane_and_quantization(self):
        native=struct.pack('<H',0xf800)*(800*600);original=bytes([255,0,0,255])*(800*600)
        self.assertEqual(m.compare_plane(native,original)['mismatching_pixels'],0)
        bad=bytearray(original);bad[-4:]=bytes([0,0,0,255]);self.assertEqual(m.compare_plane(native,bad)['mismatching_pixels'],1)
    def test_stopped_and_unlocated_sources_refuse(self):
        context=dict(pid=1,call=2,stable_active_context=True,points=[dict(context=dict(worker=3,command=4,submission=5))])
        original=dict(egl_worker_context_observation=dict(calls=[context]),wine_blit_packet_observation=dict(bindings=[dict(worker=3,command=4,submission=5)]),wine_source_map_observation=dict(samples=[dict(submission=5,api_entry=6,status=0,descriptor=[0]*6+[800,600])]),egl_physical_observation=dict(frames=[dict(pid=1,entry=2,ordinal=1,width=800,height=600,path='oracle',sha256='hash')]))
        point=(64,1,19,10,0,0,7,800,600,800,0,0,1,0,9,0)
        good,refused=m.frame_bindings(original,{6:point},10);self.assertEqual(len(good),1);self.assertFalse(refused)
        for flags in (23,3):
            changed=list(point);changed[2]=flags;good,refused=m.frame_bindings(original,{6:tuple(changed)},10);self.assertFalse(good);self.assertEqual(len(refused),1)
    def test_rectangle_history_key_and_refresh(self):
        previous=struct.pack('<H',0x1234)*(800*600)
        source=struct.pack('<4H',31,0xf800,0x7e0,31)
        f=dict(refresh=False,width=2,height=2,transfer=dict(source_box=[0,0,2,2],destination_box=[7,9,9,11],key=31))
        actual=m.compose_native(previous,source,f)
        words=struct.unpack('<480000H',actual)
        self.assertEqual(words[9*800+7:9*800+9],(0x1234,0xf800))
        self.assertEqual(words[10*800+7:10*800+9],(0x7e0,0x1234))
        self.assertEqual(words[-1],0x1234)
        self.assertEqual(m.compose_native(actual,None,dict(refresh=True)),actual)
        with self.assertRaises(ValueError):m.compose_native(None,source,f)
        with self.assertRaises(ValueError):m.compose_native(None,None,dict(refresh=True))
        f['transfer']['destination_box']=[799,599,801,601]
        with self.assertRaises(ValueError):m.compose_native(previous,source,f)
    def test_pending_primary_fill(self):
        source_free=dict(fill=0,refresh=False)
        result=m.compose_native(None,None,source_free)
        self.assertEqual(m.compare_plane(result,bytes([0,0,0,255])*(800*600))['mismatching_pixels'],0)
    def test_partial_planes_refuse(self):
        with self.assertRaises(ValueError):m.compare_plane(b'\0\0',b'\0'*4)


if __name__=='__main__':unittest.main()
