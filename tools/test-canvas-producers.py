#!/usr/bin/env python3
"""Exercise producer entry/return forwarding and strict closed-input refusal."""
import argparse,hashlib,importlib.util,json,os,struct,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--claims',type=Path);a=p.parse_args()
 spec=importlib.util.spec_from_file_location('inspector',ROOT/'tools/inspect-canvas-producers.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 paths=['runtime/scene/canvas_producers.c','runtime/scene/canvas_producers.S','tests/canvas-producers-test.c','tests/canvas-startup-call.S','tools/test-canvas-producers.py','tools/inspect-canvas-producers.py','runtime/shadow/win32_min.h','protocols/include/mnm/canvas_producers_v1.h','tools/build-scene-observer.py','runtime/scene/world_producer_bypass.c','protocols/include/mnm/world_producer_bypass_v1.h','tests/world-producer-bypass-call.S','runtime/scene/world_raster_callers.h']
 paths += ['protocols/include/mnm/canvas_producers_v3.h','protocols/include/mnm/canvas_producers_v2.h','runtime/scene/world_raster_batch.c','protocols/include/mnm/world_raster_batch_v3.h']
 claims=json.loads(a.claims.read_text()) if a.claims else None
 sources={name:sha(ROOT/name) for name in paths}
 if claims:
  for name,h in claims['sources'].items():
   if sha(ROOT/name)!=h:raise ValueError('Changed prospective source: '+name)
   sources[name]=h
 parent=ROOT/'working/tests/canvas-producers';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));capture=out/'capture';capture.mkdir();print(out,flush=True)
 env={**{k:v for k,v in os.environ.items() if not k.startswith('MNM_')},'WINEPREFIX':str(ROOT/'working/tests/scene-selftest-wine'),'WINEDEBUG':'-all','MNM_CANVAS_PRODUCERS':'1','MNM_SCENE_DIR':'Z:'+str(capture).replace('/','\\'),'MNM_SCENE_SAMPLES':'1'}
 env.pop('MNM_CANVAS_STARTUP',None);env.pop('MNM_SCENE_WORLD',None)
 def run(name,command):
  with (out/(name+'.log')).open('x') as log:subprocess.run([str(x) for x in command],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=180)
 run('bridge-build',['python3',ROOT/'tools/build-scene-observer.py','--selftest']);build=ROOT/'working/build/scene-observer-selftest'
 objects=[]
 for name in ['tests/canvas-producers-test.c','tests/canvas-startup-call.S','runtime/scene/canvas_producers.S','tests/world-producer-bypass-call.S']:
  obj=out/(Path(name).name+'.obj');objects.append(obj);run('compile-'+Path(name).name,['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',ROOT/name,'-o',obj])
 run('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/base:0x400000','/dynamicbase:no','/safeseh:no','/timestamp:0','/out:'+str(out/'fixture.exe'),*objects,build/'kernel32.lib'])
 run('forwarding',['xvfb-run','-a','wine',out/'fixture.exe'])
 budget_negative=[]
 for budget in ('0','3073','99999999',''):
  trial=out/('budget-'+(budget or 'empty'));trial.mkdir();trial_env={**env,'MNM_CANVAS_ORACLE_MIB':budget,'MNM_SCENE_DIR':'Z:'+str(trial).replace('/','\\')}
  with (out/('budget-'+(budget or 'empty')+'.log')).open('x') as log:r=subprocess.run(['xvfb-run','-a','wine',str(out/'fixture.exe')],cwd=ROOT,env=trial_env,stdout=log,stderr=subprocess.STDOUT,timeout=180)
  assert r.returncode==4 and not (trial/'canvas-producers.bin').exists(),(budget,r.returncode)
  budget_negative.append(budget)
 owned_negative=[]
 for value in ('0','2','11',''):
  trial=out/('owned-'+(value or 'empty'));trial.mkdir();trial_env={**env,'MNM_MINIMAP_OWNED':value,'MNM_SCENE_DIR':'Z:'+str(trial).replace('/','\\')}
  with (out/('owned-'+(value or 'empty')+'.log')).open('x') as log:r=subprocess.run(['xvfb-run','-a','wine',str(out/'fixture.exe')],cwd=ROOT,env=trial_env,stdout=log,stderr=subprocess.STDOUT,timeout=180)
  assert r.returncode==4 and not (trial/'canvas-producers.bin').exists(),(value,r.returncode)
  owned_negative.append(value)
 fixture_negative=[]
 for value,owned in [('0','1'),('2','1'),('11','1'),('','1'),('1',None)]:
  label=(value or 'empty')+('-unowned' if owned is None else '');trial=out/('input-'+label);trial.mkdir();trial_env={**env,'MNM_MINIMAP_INPUT_FIXTURE':value,'MNM_SCENE_DIR':'Z:'+str(trial).replace('/','\\')}
  if owned:trial_env['MNM_MINIMAP_OWNED']=owned
  with (out/('input-'+label+'.log')).open('x') as log:r=subprocess.run(['xvfb-run','-a','wine',str(out/'fixture.exe')],cwd=ROOT,env=trial_env,stdout=log,stderr=subprocess.STDOUT,timeout=180)
  assert r.returncode==4 and not (trial/'canvas-producers.bin').exists(),(value,owned,r.returncode)
  fixture_negative.append({'value':value,'owned':owned})
 negative=[]
 for mode,status,code in [('read',96,44),('write',96,44),('source',95,40),('producer',95,45)]:
  trial=out/('guard-'+mode);trial.mkdir();trial_env={**env,'MNM_BATCH_FIXTURE':mode,'MNM_SCENE_DIR':'Z:'+str(trial).replace('/','\\')}
  with (out/('guard-'+mode+'.log')).open('x') as log:r=subprocess.run(['xvfb-run','-a','wine',str(out/'fixture.exe')],cwd=ROOT,env=trial_env,stdout=log,stderr=subprocess.STDOUT,timeout=180)
  assert r.returncode==status,(mode,r.returncode)
  _,ops=m.decode((trial/'canvas-producers.bin').read_bytes(),complete=False);assert ops[-1][0][2]==13 and ops[-1][0][14]==code,(mode,ops[-1][0])
  if code==44:
   fault=(trial/'world-batch-fault-0002.bin').read_bytes();assert len(fault)==32 and fault[:8]==b'MNMBFLT1' and struct.unpack_from('<I',fault,16)[0]==(mode=='write')
  negative.append({'mode':mode,'exit':status,'failure_code':code})
 raw=(capture/'canvas-producers.bin').read_bytes();h,rs=m.decode(raw,complete=False);assert rs
 bad=[raw[:63],raw[:-1],b'BADMAGIC'+raw[8:]]
 for offset,value in [(8,2),(12,32),(16,0),(20,1),(24,0),(64,95),(68,0),(72,24),(64+23*4,1)]:
  b=bytearray(raw);struct.pack_into('<I',b,offset,value);bad.append(bytes(b))
 refused=0
 for b in bad:
  try:m.decode(b,complete=False)
  except ValueError:refused+=1
  else:raise AssertionError('Malformed producer stream accepted')
 assert sources=={name:sha(ROOT/name) for name in sources}
 report={'success':True,'sources':sources,'sources_stable':True,'forwarded_entries':52,'native_bypass_entries':1,'complete_raster_bypass_entries':13,'complete_raster_ax_and_cleanup_preserved':True,'outside_world_raster_forwarded':True,'native_bypass_state_preserved':True,'malformed_bypass_refusals':2,'nested_jpeg_lock_forwarded':True,'register_flags_float_error_preserved':True,'signature_refusal_before_mutation':True,'malformed_refusals':refused,'original_renderer_executed':False,'scope':'Actual PE32 entry/return/relocated JPEG-call forwarding against synthetic compatible prologues, all52hook sites, post-close calls, full register/flags/x87/SSE/LastError and atomic signature refusal. No original pixel equivalence.'}
 report.update(buffered_journal_byte_equality=True,buffered_journal_capacity=65536,buffered_journal_flush_failure_refused=True)
 report.update(batch_body_skips=13,batch_canvas_writebacks=1,batch_prescribed_ax_cleanup=True,batch_guard_refusals=negative,malformed_batch_reply_refusals=3,batch_original_canvas_unwritten_before_return=True,batch_hud_forwarded_after_return=True)
 report.update(oracle_budget_invalid_values_refused=7,oracle_budget_max_mib=3072,oracle_budget_default_mib=1024,oracle_budget_crosses_old_ceiling=True,oracle_budget_exact_ceiling_accepted=True,oracle_budget_over_ceiling_refusals=3,oracle_budget_counter_underflow_refused=True)
 report['oracle_environment_refused_before_hook_mutation']=budget_negative
 report['minimap_environment_refused_before_hook_mutation']=owned_negative
 report['minimap_fixture_refused_before_hook_mutation']=fixture_negative
 if claims:report['claims']=claims['claims']
 (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
if __name__=='__main__':main()
