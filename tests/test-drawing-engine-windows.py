#!/usr/bin/env python3
from pathlib import Path
import struct,sys,tempfile,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_engine_windows import analyze,analyze_session


def trace(specs):
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMCAD01');h[2:11]=[1,96,1000,0,0,0,0,7,262144];rows=[]
    for seq,counter,mode,advance in specs:
        for event in ([2,3,8] if advance else [2,8]):
            r=[0]*24;r[:5]=[len(rows)+1,event,7,len(rows)+1,0];r[5]=counter+int(event!=2 and advance)&0xffffffff;r[6]=seq;r[10]=mode;r[13]=0x4fd4b0;rows.append(r)
    h[6]=len(rows);return struct.pack('<16I',*h)+b''.join(struct.pack('<24I',*r) for r in rows)


class Windows(unittest.TestCase):
    def run_trace(self,data,session=False):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'trace.bin';p.write_bytes(data);return (analyze_session if session else analyze)(p)
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


    def test_duplicate_or_reversed_accepted_sequence_refuses(self):
        for sequences in ((1,1,2),(2,1,3)):
            with self.assertRaises(ValueError):self.run_trace(trace([(s,i,0,True) for i,s in enumerate(sequences)]))

    def test_trace_hash_and_windows_describe_one_snapshot(self):
        from drawing_api_session import sha
        raw=trace([(1,1,0,True),(2,2,0,True)])
        self.assertEqual(self.run_trace(raw)['journal_sha256'],sha(raw))

    def session_restart(self):
        raw=trace([(7,10,0,True),(8,11,0,True),(1,3,0,True),(2,4,0,True),(3,5,0,True)])
        h=list(struct.unpack_from('<16I',raw));rows=[list(w) for w in struct.iter_unpack('<24I',raw[64:])]
        pair=[]
        for event in (1,11):
            w=[0]*24;w[1]=event;w[2]=7;w[5]=3;w[13]=0x557408;pair.append(w)
        rows[6:6]=pair;h[6]=len(rows)
        for i,w in enumerate(rows,1):w[0]=w[3]=i
        return bytearray(struct.pack('<16I',*h)+b''.join(struct.pack('<24I',*w) for w in rows))

    def test_session_restart_has_separate_epoch_and_never_continuity(self):
        raw=self.session_restart()
        with self.assertRaises(ValueError):self.run_trace(raw)
        x=self.run_trace(raw,session=True)
        self.assertEqual(x['epochs'],2);self.assertFalse(x['global_sequence_identity'])
        self.assertEqual([w['epoch'] for w in x['windows']],[1,1,2,2])
        self.assertEqual(x['longest_contiguous_advancing_run'],2)
        self.assertFalse(x['sequence_restarts'][0]['original_load_causality_verified'])
        self.assertFalse(x['producer_update_identity_bound'])

    def test_session_restart_requires_immediately_preceding_zero_pacer_pair(self):
        # Nonzero entry/return, mismatched counter, non-sequence1 restart and
        # stale rather than immediately preceding pacer all retain refusal.
        for row,field,value in ((6,6,1),(7,6,1),(7,5,4),(8,6,2),(8,1,1)):
            raw=self.session_restart();struct.pack_into('<I',raw,64+row*96+field*4,value)
            with self.subTest(row=row,field=field):
                with self.assertRaises(ValueError):self.run_trace(raw,session=True)

    def test_session_without_observed_reset_still_refuses_duplicate_reversal(self):
        for sequences in ((1,1,2),(2,1,3)):
            with self.assertRaises(ValueError):self.run_trace(trace([(s,i,0,True) for i,s in enumerate(sequences)]),session=True)

if __name__=='__main__':
    if '--claims' not in sys.argv:unittest.main()
    else:
        import argparse,json,hashlib
        from coverage_evidence import write_report,evidence_digest
        parser=argparse.ArgumentParser();parser.add_argument('--claims',type=Path,required=True);parser.add_argument('--report',type=Path,required=True);a=parser.parse_args()
        if a.report.exists():parser.error('Refusing evidence overwrite')
        root=Path(__file__).resolve().parents[1];claims=json.loads(a.claims.read_text());sources=claims['sources']
        def stable():return all(evidence_digest(root/p,root=root)==h for p,h in sources.items())
        if not stable():raise ValueError('Prospective sources changed')
        result=unittest.TextTestRunner().run(unittest.defaultTestLoader.loadTestsFromTestCase(Windows))
        report=dict(schema=1,success=result.wasSuccessful() and stable(),sources=sources,sources_stable=stable(),claims=claims['claims'],tests_run=result.testsRun,original_game_executed=False,scope='Synthetic actual update-window decoding only:120complete windows/final exclusion,wrap,nonadvancing breaks,sequence gaps,counter stores without inferred mode policy,malformed counter/caller/nesting refusals. No original source or presentation binding.')
        a.report.parent.mkdir(parents=True,exist_ok=True);write_report(root,a.report,(json.dumps(report,indent=2)+'\n').encode());sys.exit(0 if report['success'] else 1)
