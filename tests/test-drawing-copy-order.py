"""Driver order must come from identities, independently of oracle pixels."""
from pathlib import Path
import sys,struct,unittest
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from drawing_copy_order import normalize

def fixture():
    records=[]
    for sequence,dst,src in ((1,2,1),(2,1,3)):
        r=[0]*24;r[:7]=[96,sequence,6,dst,src,2,2];r[12:15]=[2,2,0xffffffff];records.append((tuple(r),b''))
    raw=b'X'*64+b''.join(struct.pack('<24I',*r) for r,_ in records)
    points=[];calls=[];workers=[];samples=[];packets=[]
    # Earlier command's wrapper returns second, with another thread's copy in between.
    for api,thread,dst,src,worker in ((1,10,1,3,1),(2,11,2,1,2)):
        calls.append(dict(entry=api,thread=thread,method=7,related=1,hresult=0))
        for phase in (1,2,3):
            row=[0]*48;row[1:3]=[phase,api];row[22]=dst;row[38]=src;points.append(row)
        workers.append(dict(call=worker,source_box=[0,0,2,2,0,1],destination_box=[0,0,2,2,0,1],flags=0x41000000,effects=[0]*6,filter=1,source_subresource=0,destination_subresource=0))
        samples.append(dict(submission=worker,api_entry=api));packets.append(dict(worker=worker,submission=worker))
    # Limit each candidate to its own endpoint pair; both return before emission.
    original=dict(observation=dict(calls=calls),wine_blit_worker_observation=dict(calls=workers),wine_source_map_observation=dict(samples=samples),wine_blit_packet_observation=dict(bindings=packets))
    point_raw=b'X'*64+b''.join(struct.pack('<48I',*r) for r in points)+b'X'*192
    return raw,records,point_raw,original

class Cases(unittest.TestCase):
    def test_copy_effect_follows_driver_order(self):
        args=fixture();result,proof=normalize(*args,[])
        self.assertEqual([struct.unpack_from('<24I',result,64+i*96)[3:5] for i in range(2)],[(1,3),(2,1)])
        self.assertEqual([struct.unpack_from('<I',result,68+i*96)[0] for i in range(2)],[1,2])
        self.assertEqual(len(proof['moved_copies']),2);self.assertFalse(proof['original_pixels_used_as_native_inputs'])
    def test_read_uses_command_prefix(self):
        query=dict(sequence=1,canvas=1,api_entry=2);_,proof=normalize(*fixture(),[query])
        self.assertEqual(query['sequence'],1);self.assertEqual(proof['rebound_reads'],[])
        query=dict(sequence=1,canvas=3,api_entry=1);_,proof=normalize(*fixture(),[query])
        self.assertEqual(query['sequence'],0);self.assertEqual(query['original_sequence'],1)
    def test_missing_read_identity_refuses(self):
        with self.assertRaises(ValueError):normalize(*fixture(),[dict(sequence=1,canvas=1)])
    def test_unsupported_copy_is_a_barrier(self):
        raw,records,points,original=fixture();original['wine_blit_worker_observation']['calls'][0]['flags']=123
        result,proof=normalize(raw,records,points,original,[]);self.assertEqual(result,raw);self.assertEqual(proof['moved_copies'],[])
    def test_no_original_pixels_can_select_order(self):
        args=list(fixture());args[3]['expected_pixels']=b'arbitrary diagnostic bytes'
        self.assertEqual(normalize(*args,[])[0],normalize(*fixture(),[])[0])
    def test_ambiguous_api_bounds_refuse(self):
        raw,records,points,original=fixture();r=list(records[1][0]);r[3:5]=[2,1];records[1]=(tuple(r),b'');raw=raw[:160]+struct.pack('<24I',*r)
        with self.assertRaises(ValueError):normalize(raw,records,points,original,[])
if __name__=='__main__':unittest.main()
