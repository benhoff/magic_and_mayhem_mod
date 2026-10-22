#!/usr/bin/env python3
"""Execute real CLI entry points using owned synthetic journals and SQLite refs."""
import hashlib,importlib.util,json,os,shutil,struct,subprocess,sys,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import write_report,read_json,identity
from drawing_api_session import BUILD
from drawing_copy_order import decode


def sha(raw):return hashlib.sha256(raw).hexdigest()
def load(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def producer(rows):
    h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMPRO01');h[2:7]=[1,64,0x40209ca7,65536,1]
    out=[]
    for i,row in enumerate(rows,1):
        r=list(row);r[0]=96;r[1]=i;out.append(struct.pack('<24I',*r))
    return struct.pack('<16I',*h)+b''.join(out)

def row(kind,**words):
    r=[0]*24;r[2]=kind
    for key,value in words.items():r[int(key[1:])]=value
    return r


class Pipelines(unittest.TestCase):
    def setUp(self):
        parent=ROOT/'working/tests/ninth-cli-inputs';parent.mkdir(parents=True,exist_ok=True)
        self.temp=tempfile.TemporaryDirectory(dir=parent);self.addCleanup(self.temp.cleanup);self.out=Path(self.temp.name)
        self.refs=tempfile.TemporaryDirectory(dir=ROOT/'research/runtime',prefix='.ninth-cli-');self.addCleanup(self.refs.cleanup);self.reports=Path(self.refs.name)
    def save(self,name,value):
        path=self.reports/name;write_report(ROOT,path,(json.dumps(value)+'\n').encode());self.assertIsNotNone(identity(path.read_bytes()));return path
    def cli(self,tool,*args,code=0):
        p=subprocess.run([sys.executable,'-B',str(ROOT/tool),*map(str,args)],cwd=ROOT,capture_output=True,text=True,timeout=60)
        self.assertEqual(p.returncode,code,p.stdout+p.stderr);return p

    def test_request_cli_rejoins_every_copy_and_refuses_corrupt_producers(self):
        fixture=load('ninth_order_fixture','tests/test-drawing-copy-order.py')
        raw=fixture.journal();observation=decode(raw);capture=self.out/'capture';capture.mkdir()
        (capture/'copy-order-0000.bin').write_bytes(raw)
        rows=[row(3) for _ in range(13)]
        for returned in observation['returns']:
            r=row(6);r[3:15]=[returned['destination'],returned['source'],*returned['destination_extent_stride'],*returned['destination_position'],*returned['source_rectangle'],returned['raw_key']]
            rows[returned['producer_return_sequence']-1]=r
        rows += [row(11,w14=1),row(12,w14=1)]
        wire=producer(rows);(capture/'canvas-producers.bin').write_bytes(wire)
        reference=self.save('copy-reference.json',dict(success=True,normal_original_quit_verified=True,synthetic_protocol_fixture=True,experiment=str(self.out),copy_order_observation=observation))
        report=self.reports/'copy-result.json'
        self.cli('tools/analyze-drawing-copy-requests.py','--reference',reference,'--report',report)
        result=read_json(report,root=ROOT);self.assertEqual(result['producer_copies'],2);self.assertEqual(result['adjacent_entry_reversals'],[[1,2]]);self.assertFalse(result['driver_order_proved'])
        self.assertIsNotNone(identity(report.read_bytes()))
        self.cli('tools/analyze-drawing-copy-requests.py','--reference',reference,'--report',report,code=2)
        for bad in (b'INVALID!'+wire[8:],wire[:-1],wire[:64]+wire[160:]):
            (capture/'canvas-producers.bin').write_bytes(bad)
            p=subprocess.run([sys.executable,'-B',str(ROOT/'tools/analyze-drawing-copy-requests.py'),'--reference',str(reference),'--report',str(self.reports/'bad.json')],capture_output=True)
            self.assertNotEqual(p.returncode,0);self.assertFalse((self.reports/'bad.json').exists())

    def test_local_order_cli_retains_exact_failure_and_refuses_changed_plane(self):
        capture=self.out/'capture';capture.mkdir();native_dir=self.out/'native';native_dir.mkdir()
        before=bytes(960000);after=bytearray(before)
        for y in range(310,350):
            for x in range(440,480):struct.pack_into('<H',after,2*(y*800+x),0x1234)
        after=bytes(after);(native_dir/'frame-0001.565').write_bytes(after)
        for name in ('source.565','producer-oracle-0001.565','producer-oracle-0002.565'):(capture/name).write_bytes(before)
        save=row(6,w3=5,w4=1,w5=40,w6=40,w10=440,w11=310,w12=480,w13=350,w14=0xffffffff)
        full=row(6,w3=1,w4=2,w5=800,w6=600,w12=800,w13=600,w14=0xffffffff)
        (capture/'canvas-producers.bin').write_bytes(producer([row(10,w3=2,w5=800,w6=600,w14=1),save,full,row(10,w3=2,w5=800,w6=600,w14=2),row(11,w14=1),row(12,w14=1)]))
        points=[];calls=[]
        for entry,copy_record in ((10,full),(11,save)):
            r=[0]*48;r[1:3]=[3,entry];r[22]=copy_record[3];r[35]=1;r[38]=copy_record[4];points.append(struct.pack('<48I',*r))
            r=[0]*32;r[1]=1;r[2]=7;r[6]=entry+100;r[21]=16;r[24:28]=copy_record[10:14];r[29]=entry;calls.append(struct.pack('<32I',*r))
        (capture/'api-producer-points.bin').write_bytes(bytes(64)+b''.join(points));(capture/'session.bin').write_bytes(bytes(64)+b''.join(calls))
        bindings=[dict(command=1,submission=1,worker_entry_record=9,worker_return_record=10),dict(command=2,submission=2,worker_entry_record=11,worker_return_record=12)]
        reference=self.save('local-reference.json',dict(success=True,synthetic_protocol_fixture=True,source_sha256=BUILD,experiment=str(self.out),wine_submission_observation=dict(calls=[dict(call=1,api_entry=10),dict(call=2,api_entry=11)]),wine_blit_packet_observation=dict(bindings=bindings)))
        mismatch=dict(source_different_pixels=1600,different_pixels=1600,source_path='source.565',source_sha256=sha(before),boundary=dict(canvas=2,sequence=4))
        native=self.save('local-native.json',dict(success=False,native_exit=0,reference=dict(sha256=sha((json.dumps(read_json(reference,root=ROOT))+'\n').encode())),artifacts=str(self.out.relative_to(ROOT)),comparisons=[mismatch],native=dict(checkpoints=[dict(canvas=2,sequence=1,ordinal=1,sha256=sha(before)),dict(canvas=2,sequence=4,ordinal=2,sha256=sha(after))])))
        report=self.reports/'local-result.json';self.cli('tools/analyze-drawing-copy-order.py','--reference',reference,'--native',native,'--report',report)
        result=read_json(report,root=ROOT);self.assertEqual(result['mismatch_box'],[440,310,480,350]);self.assertTrue(result['ordering_reversal_confirmed']);self.assertFalse(result['mismatch_cause_fully_proved']);self.assertTrue(result['original_cpu_checkpoint_unchanged'])
        (native_dir/'frame-0001.565').write_bytes(before)
        p=subprocess.run([sys.executable,'-B',str(ROOT/'tools/analyze-drawing-copy-order.py'),'--reference',str(reference),'--native',str(native),'--report',str(self.reports/'no-mismatch.json')],capture_output=True,text=True)
        self.assertNotEqual(p.returncode,0);self.assertIn('No source-plane mismatch',p.stderr)

    def test_timing_cli_uses_sqlite_peer_and_exact_raw_delivery(self):
        fixture=load('ninth_physical_fixture','tests/test-drawing-api-physical.py');case=fixture.WitnessTest();case.setUp();self.addCleanup(case.doCleanups)
        experiment=self.reports/'timing-experiment';experiment.mkdir();capture=self.out/'capture';capture.mkdir();(experiment/'capture').symlink_to(capture,target_is_directory=True)
        for p in case.root.iterdir():
            if p.suffix!='.json':shutil.copyfile(p,capture/p.name)
        frame=case.peer['frames'][0];frame.update(clock_domain='linux_CLOCK_MONOTONIC',x11_target='32',root_x=4,root_y=-6,first_capture=dict(begin_ns='10',end_ns='11'),second_capture=dict(begin_ns='20',end_ns='21'),settling_samples=[])
        peer=experiment/'physical-report.json';peer_raw=(json.dumps(case.peer)+'\n').encode();write_report(ROOT,peer,peer_raw)
        h=[0]*16;h[:2]=struct.unpack('<2I',b'MNMX1101');h[2:5]=[3,96,65536]
        rows=[]
        for phase,t in ((1,30),(2,40)):
            r=[0]*24;r[:6]=[phase,1,11,3,4,t];r[7]=phase-1;r[9:12]=[1,1,1];rows.append(struct.pack('<24I',*r))
        raw=struct.pack('<16I',*h)+b''.join(rows);(capture/'x11-delivery.bin').write_bytes(raw)
        from drawing_api_physical import analyze
        reference=self.save('timing-reference.json',dict(success=True,synthetic_protocol_fixture=True,source_sha256=BUILD,original_drawing_active=True,experiment=str(experiment),physical_peer_report_sha256=sha(peer_raw),physical_api_observation=analyze(capture,peer),x11_delivery_observation=dict(sha256=sha(raw))))
        claims=self.save('timing-claims.json',dict(claims=[],sources={}))
        report=self.reports/'timing-result.json';self.cli('tools/analyze-drawing-delivery-timing.py','--reference',reference,'--claims',claims,'--report',report)
        result=read_json(report,root=ROOT);self.assertEqual(result['analysis']['physical_pairs_before_first_egl'],1);self.assertEqual(result['analysis']['frames'][0]['root_y'],-6)
        self.assertFalse(result['coherent_presented_frames']);self.assertIsNotNone(identity(report.read_bytes()))

    def test_artifact_cli_records_all_independent_failures_in_sqlite(self):
        helper=load('ninth_saved_policy','tools/check-native-campaign-artifacts.py')
        experiment=self.out/'experiment';experiment.mkdir()
        for n in ('events.bin','channel.bin','gameplay-events.bin','spell-events.bin'):(experiment/n).write_bytes(b'')
        self.save('flow.json',dict(success=False,error='retained native worst-window failure',gameplay_rates=[]))
        self.save('input-0.json',dict(actions=[],captures=[]))
        reference=self.save('campaign.json',dict(success=False,error='retained engine FPS failure',experiment=str(experiment),sources={p:helper.sha(ROOT/p) for p in helper.validator_sources()},menu_cycles=1,difficulty=0,normal_quit_required=False,portrait_stress_seconds=10,minimum_fps=20,stress_seconds_per_phase=10,ordinary_resident_gates=dict(worlds_passed=True,fps_passed=False,minimum_fps=20)))
        report=self.reports/'artifact-result.json';self.cli('tools/check-native-campaign-artifacts.py','--report',reference,'--output',report,code=1)
        result=read_json(report,root=ROOT);self.assertFalse(result['success']);self.assertTrue(result['sources_stable']);self.assertTrue(result['inputs_stable']);self.assertGreaterEqual(len(result['errors']),6);self.assertEqual(result['campaign_error'],'retained engine FPS failure');self.assertIn('resident_performance',result['errors']);self.assertIsNotNone(identity(report.read_bytes()))


if __name__=='__main__':unittest.main()
