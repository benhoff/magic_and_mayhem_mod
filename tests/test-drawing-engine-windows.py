#!/usr/bin/env python3
from pathlib import Path
import struct,sys,tempfile,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_engine_windows import analyze


def trace(specs):
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMCAD01');h[2:11]=[1,96,1000,0,0,0,0,7,262144];rows=[]
    for seq,counter,mode,advance in specs:
        for event in ([2,3,8] if advance else [2,8]):
            r=[0]*24;r[:5]=[len(rows)+1,event,7,len(rows)+1,0];r[5]=counter+int(event!=2 and advance)&0xffffffff;r[6]=seq;r[10]=mode;r[13]=0x4fd4b0;rows.append(r)
    h[6]=len(rows);return struct.pack('<16I',*h)+b''.join(struct.pack('<24I',*r) for r in rows)


class Windows(unittest.TestCase):
    def run_trace(self,data):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'trace.bin';p.write_bytes(data);return analyze(p)
    def test120_and_final_exclusion(self):
        x=self.run_trace(trace([(i,i,0,True) for i in range(121)]));self.assertEqual(x['longest_contiguous_advancing_run'],120);self.assertEqual(x['complete_update_windows'],120);self.assertFalse(x['producer_update_identity_bound'])
    def test_pause_breaks_consecutive_run(self):
        x=self.run_trace(trace([(1,1,0,True),(2,2,1,False),(3,2,0,True),(4,3,0,True)]));self.assertEqual(x['longest_contiguous_advancing_run'],1);self.assertEqual(x['non_advancing_update_windows'],1)
    def test_wrap_is_exact(self):
        x=self.run_trace(trace([(0xffffffff,0xffffffff,0,True),(0,0,0,True),(1,1,0,True)]));self.assertEqual(x['longest_contiguous_advancing_run'],2)
    def test_counter_store_proves_advancement_without_mode_policy(self):
        x=self.run_trace(trace([(1,1,2,True),(2,2,2,True)]));self.assertEqual(x['advancing_update_windows'],1)
    def test_sequence_gap_breaks_run(self):
        x=self.run_trace(trace([(1,1,0,True),(3,2,0,True),(4,3,0,True)]));self.assertEqual(x['longest_contiguous_advancing_run'],1)
    def test_bad_tick_caller_and_unpaired_call_refuse(self):
        for offset,value in ((64+96+5*4,3),(64+192+13*4,9),(64+1*4,11)):
            raw=bytearray(trace([(1,1,0,True),(2,2,0,True)]));struct.pack_into('<I',raw,offset,value)
            with self.assertRaises(ValueError):self.run_trace(raw)


if __name__=='__main__':
    if '--claims' not in sys.argv:unittest.main()
    else:
        import argparse,json,hashlib
        parser=argparse.ArgumentParser();parser.add_argument('--claims',type=Path,required=True);parser.add_argument('--report',type=Path,required=True);a=parser.parse_args()
        if a.report.exists():parser.error('Refusing evidence overwrite')
        root=Path(__file__).resolve().parents[1];claims=json.loads(a.claims.read_text());sources=claims['sources']
        def stable():return all(hashlib.sha256((root/p).read_bytes()).hexdigest()==h for p,h in sources.items())
        if not stable():raise ValueError('Prospective sources changed')
        result=unittest.TextTestRunner().run(unittest.defaultTestLoader.loadTestsFromTestCase(Windows))
        report=dict(schema=1,success=result.wasSuccessful() and stable(),sources=sources,sources_stable=stable(),claims=claims['claims'],tests_run=result.testsRun,original_game_executed=False,scope='Synthetic actual update-window decoding only:120complete windows/final exclusion,wrap,nonadvancing breaks,sequence gaps,counter stores without inferred mode policy,malformed counter/caller/nesting refusals. No original source or presentation binding.')
        a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');sys.exit(0 if report['success'] else 1)
