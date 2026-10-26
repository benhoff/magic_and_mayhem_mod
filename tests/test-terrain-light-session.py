#!/usr/bin/env python3
"""Actual worker/channel controls using a retained original-active source/result.

This executes the persistent native worker, not Wine or the original executable.
Original-active installed execution is a separate runner and evidence kind.
"""
import argparse,ctypes,hashlib,json,mmap,os,shutil,signal,sqlite3,struct,sys,time,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
import terrain_light_session as session
from coverage_evidence import read_json,read_bytes,write_report
from coverage_claims import behavior_contract,required_sources,scenario_contract
BUILD=None;OUT=None;GOLD=None;CASE=0
atom=ctypes.CDLL('libatomic.so.1');store=atom.__atomic_store_4;store.argtypes=[ctypes.c_void_p,ctypes.c_uint32,ctypes.c_int];load=atom.__atomic_load_4;load.argtypes=[ctypes.c_void_p,ctypes.c_int];load.restype=ctypes.c_uint32

def publish(mm,value):store(ctypes.addressof(ctypes.c_char.from_buffer(mm))+16,value,3)
def phase(mm):return load(ctypes.addressof(ctypes.c_char.from_buffer(mm))+16,2)
def until(predicate,seconds=3):
 end=time.monotonic()+seconds
 while time.monotonic()<end:
  value=predicate()
  if value:return value
  time.sleep(.001)
 raise AssertionError('Channel peer timed out')
def checksum(data):
 h=2166136261
 for i,v in enumerate(data):h=((h^(0 if 120<=i<124 else v))*16777619)&0xffffffff
 return h

def fixture(mode='native',capture='first32'):
 global CASE
 CASE+=1;root=OUT/('case-%02d'%CASE);(root/'game').mkdir(parents=True);(root/'lighting-build').mkdir();directory=root/'terrain-light-session';directory.mkdir()
 shutil.copy2(BUILD/'bridge/MnmLight.dll',root/'game/MnmLight.dll');shutil.copy2(BUILD/'mnm-terrain-light-session',root/'lighting-build/mnm-terrain-light-session')
 build=json.loads((BUILD/'manifest.json').read_text());nonce=GOLD[0][24:32];header=bytearray(128);header[:8]=b'MNMLS001';struct.pack_into('<3I',header,8,1,session.CAPACITY,7);header[36:44]=nonce;struct.pack_into('<2I',header,44,int(mode=='native'),{'none':0,'first32':1,'reload32':2}[capture])
 with (directory/'channel.bin').open('xb') as f:f.write(header);f.truncate(session.SIZE)
 policy=dict(schema=1,mode=mode,capture=capture,channel=str(directory/'channel.bin'),nonce=nonce.hex(),dll_sha256=build['dll_sha256'],worker_sha256=build['worker_sha256'],sources=build['sources'],scope='Actual native worker controls over a retained recorded input; no original executable executed.')
 return root,dict(terrain_light_session=policy,directplay_dx6={'control_fixture':True}),dict(MNM_TERRAIN_LIGHTING=mode,MNM_TERRAIN_LIGHT_CAPTURE=capture)

class SessionControls(unittest.TestCase):
 def test_continuous_peer_exceeds_old_capture_budget_with_fixed_storage(self):
  root,metadata,env=fixture();before_size=(root/'terrain-light-session/channel.bin').stat().st_size
  with session.Session(root,metadata,env) as peer:
   mm=peer.mapping
   for seq in range(1,97):
    data=bytearray(GOLD[0]);struct.pack_into('<I',data,20,seq);struct.pack_into('<I',data,112,1);struct.pack_into('<I',data,120,checksum(data));mm[128:128+len(data)]=data;struct.pack_into('<I',mm,20,len(data));struct.pack_into('<I',mm,32,seq);publish(mm,1);until(lambda:phase(mm)==2)
    n=struct.unpack_from('<I',mm,24)[0];reply=bytes(mm[128+session.CAPACITY:128+session.CAPACITY+n]);self.assertEqual(reply[32:],GOLD[1]);self.assertEqual(struct.unpack_from('<I',reply,16)[0],seq);self.assertEqual(struct.unpack_from('<I',reply,28)[0],struct.unpack_from('<I',data,120)[0])
    at=128+2*session.CAPACITY;mm[at:at+len(GOLD[1])]=GOLD[1];struct.pack_into('<I',mm,28,len(GOLD[1]));publish(mm,3);until(lambda:phase(mm)==4);publish(mm,0)
   self.assertEqual(struct.unpack_from('<I',mm,76)[0],96);self.assertEqual(struct.unpack_from('<I',mm,84)[0],96);self.assertEqual(struct.unpack_from('<I',mm,116)[0],32);self.assertEqual((root/'terrain-light-session/channel.bin').stat().st_size,before_size);self.assertLessEqual((root/'terrain-light-session/corpus.bin').stat().st_size,64*1024*1024)
  self.assertTrue(peer.result['success']);self.assertIsNone(peer.child)
  records=0
  with (root/'terrain-light-session/corpus.bin').open('rb') as f:
   self.assertEqual(f.read(8),b'MNMLSC01')
   while lengths:=f.read(12):
    n,r,o=struct.unpack('<3I',lengths);data=f.read(n);reply=f.read(r);actual=f.read(o);self.assertEqual(len(data),n);self.assertEqual(reply[32:],actual);self.assertEqual(actual,GOLD[1]);records+=1
  self.assertEqual(records,32)
 def test_production_mode_has_no_per_update_recording(self):
  root,metadata,env=fixture(capture='none')
  with session.Session(root,metadata,env) as peer:
   self.assertFalse((root/'terrain-light-session/corpus.bin').exists());childenv=dict(env,MNM_LIGHT_DIR='unapproved',MNM_LIGHT_LIMIT='64');session.authorize(root,metadata,childenv);self.assertNotIn('MNM_LIGHT_DIR',childenv);self.assertNotIn('MNM_LIGHT_LIMIT',childenv);self.assertEqual(childenv['MNM_LIGHT_SESSION_FILE'],'Z:'+str(root/'terrain-light-session/channel.bin').replace('/','\\'))
   mm=peer.mapping;data=bytearray(GOLD[0]);struct.pack_into('<I',data,20,1);struct.pack_into('<I',data,112,1);struct.pack_into('<I',data,120,checksum(data));mm[128:128+len(data)]=data;struct.pack_into('<I',mm,20,len(data));struct.pack_into('<I',mm,32,1);publish(mm,1);until(lambda:phase(mm)==2);self.assertEqual(struct.unpack_from('<I',mm,76)[0],1);self.assertEqual(struct.unpack_from('<I',mm,84)[0],0);publish(mm,0)
  self.assertTrue(peer.result['success'])
 def test_invalid_length_refuses_without_writing_reply_lane(self):
  root,metadata,env=fixture()
  with self.assertRaisesRegex(RuntimeError,'refused/stopped'):
   with session.Session(root,metadata,env) as peer:
    mm=peer.mapping;at=128+session.CAPACITY;mm[at:at+32]=b'Z'*32;struct.pack_into('<I',mm,20,session.CAPACITY+1);struct.pack_into('<I',mm,32,1);publish(mm,1);until(lambda:phase(mm)==5);self.assertEqual(bytes(mm[at:at+32]),b'Z'*32);self.assertEqual(struct.unpack_from('<I',mm,80)[0],1);publish(mm,6);until(lambda:peer.child.poll() is not None)
  self.assertFalse(peer.result['success'])
 def test_sequence_gap_is_declined(self):
  root,metadata,env=fixture()
  with self.assertRaises(RuntimeError):
   with session.Session(root,metadata,env) as peer:
    mm=peer.mapping;data=GOLD[0];mm[128:128+len(data)]=data;struct.pack_into('<I',mm,20,len(data));struct.pack_into('<I',mm,32,3);publish(mm,1);until(lambda:phase(mm)==5);publish(mm,6);until(lambda:peer.child.poll() is not None)
 def test_startup_rejects_stale_channel_and_wrong_binary(self):
  root,metadata,env=fixture()
  with (root/'terrain-light-session/channel.bin').open('r+b') as f:f.seek(16);f.write(struct.pack('<I',0))
  with self.assertRaisesRegex(ValueError,'stale'):session.Session(root,metadata,env).__enter__()
  root,metadata,env=fixture();(root/'lighting-build/mnm-terrain-light-session').write_bytes(b'wrong')
  with self.assertRaisesRegex(ValueError,'identity'):session.Session(root,metadata,env).__enter__()
 def test_source_receipt_is_not_renewed_for_changed_source(self):
  root,metadata,env=fixture();metadata['terrain_light_session']['sources']={'runtime/lighting/observer.c':'0'*64}
  with self.assertRaisesRegex(ValueError,'source changed'):session.Session(root,metadata,env).__enter__()
 def test_staged_mode_cannot_be_changed_at_launch(self):
  root,metadata,env=fixture();env['MNM_TERRAIN_LIGHTING']='observe'
  with self.assertRaisesRegex(ValueError,'selection differs'):session.Session(root,metadata,env).__enter__()
 def test_truncated_channel_refuses_before_worker_start(self):
  root,metadata,env=fixture()
  with (root/'terrain-light-session/channel.bin').open('r+b') as f:f.truncate(127)
  with self.assertRaisesRegex(ValueError,'identity'):session.Session(root,metadata,env).__enter__()
 def test_worker_loss_is_reaped_and_recorded_as_failure(self):
  root,metadata,env=fixture()
  with self.assertRaises(RuntimeError):
   with session.Session(root,metadata,env) as peer:
    os.kill(peer.child.pid,signal.SIGKILL);until(lambda:peer.child.poll() is not None)
  self.assertTrue(peer.result['worker_exited_early']);self.assertFalse(peer.result['success'])
  with sqlite3.connect(root/'terrain-light-session/outcome.sqlite3') as db:self.assertEqual(len(db.execute('SELECT * FROM results').fetchall()),1)
 def test_original_default_does_not_start_or_stage_a_worker(self):
  self.assertEqual(session.selection({}),('original','none'));self.assertEqual(session.stage(OUT,b'not original',b'unchanged',{}),(b'unchanged',{}))
  with session.Session(OUT,{},{}):pass
  for mode in ['auto','', 'replace']:
   with self.assertRaises(ValueError):session.selection({'MNM_TERRAIN_LIGHTING':mode})


def main():
 global BUILD,OUT,GOLD
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args();BUILD=a.build.resolve();OUT=a.output.resolve();OUT.mkdir(parents=True,exist_ok=False);(OUT/'.retain').write_text('Actual native worker controls and bounded source/result corpus; preserve.\n')
 r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(b for b in r['behaviors'] if b['id']=='TL.session-light-policy');builds={v['id']:v for v in r['builds']};scenario=next(s for s in r['scenarios'] if s['id']=='TLSN25.controls');sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(required_sources(b))}
 old=read_json(ROOT/'research/runtime/terrain-light-closure-results-20261025/observe12.json',root=ROOT)['results'][0];GOLD=(read_bytes(ROOT/old['input'],root=ROOT),read_bytes(ROOT/old['result'],root=ROOT));result=unittest.TextTestRunner(verbosity=2).run(unittest.defaultTestLoader.loadTestsFromTestCase(SessionControls));stable=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in sources.items())
 report=dict(schema=1,success=result.wasSuccessful() and stable,sources=sources,sources_stable=stable,claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,builds),scenarios={scenario['id']:scenario_contract(scenario)})],tests=result.testsRun,failures=[text for _,text in result.failures],errors=[text for _,text in result.errors],recorded_inputs={old['input']:old['input_sha256'],old['result']:old['result_sha256']},original_artifacts_consumed=False,original_game_executed=False,scope=__doc__)
 print(write_report(ROOT,a.report.absolute(),(json.dumps(report,indent=2)+'\n').encode()));return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
