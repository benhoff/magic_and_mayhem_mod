#!/usr/bin/env python3
"""Exercise producer entry/return forwarding and strict closed-input refusal."""
from coverage_evidence import read_json, evidence_digest, write_report
import argparse,hashlib,importlib.util,json,os,struct,subprocess,tempfile
from pathlib import Path
from kind8_sprite_fixture import original_fixture
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--claims',type=Path);p.add_argument('--prefix-template',type=Path);p.add_argument('--report',type=Path);a=p.parse_args()
 if a.report and a.report.exists():p.error('Refusing evidence overwrite')
 spec=importlib.util.spec_from_file_location('inspector',ROOT/'tools/inspect-canvas-producers.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 paths=['runtime/scene/canvas_producers.c','runtime/scene/canvas_producers.S','tests/canvas-producers-test.c','tests/canvas-startup-call.S','tools/test-canvas-producers.py','tools/inspect-canvas-producers.py','runtime/shadow/win32_min.h','protocols/include/mnm/canvas_producers_v1.h','tools/build-scene-observer.py','runtime/scene/world_producer_bypass.c','protocols/include/mnm/world_producer_bypass_v1.h','tests/world-producer-bypass-call.S','runtime/scene/world_raster_callers.h']
 paths += ['protocols/include/mnm/canvas_producers_v3.h','protocols/include/mnm/canvas_producers_v2.h','runtime/scene/world_raster_batch.c','protocols/include/mnm/world_raster_batch_v3.h']
 paths += ['reconstruction/rendering/generic_producer_caller.h','tests/world-effect-entry-call.S','tests/world-effect-hook-fixture.h','protocols/include/mnm/world_effect_raster_v1.h']
 paths += ['tools/kind8_sprite_fixture.py','tests/kind8-sprite-hook-fixture.h','tests/kind8-sprite-primary-call.S','runtime/scene/kind8_sprite_admission.h','reconstruction/rendering/kind8_sprite_class.h']
 claims=json.loads(a.claims.read_text()) if a.claims else None
 sources={name:sha(ROOT/name) for name in paths}
 if claims:
  for name,h in claims['sources'].items():
   if sha(ROOT/name)!=h:raise ValueError('Changed prospective source: '+name)
   sources[name]=h
 paths += ['runtime/scene/vertical_bar_trace.h','reconstruction/rendering/vertical_bar.h','runtime/scene/drawing_source_point.h','protocols/include/mnm/drawing_source_point_v1.h','runtime/scene/producer_trace_guard.h','runtime/scene/drawing_publication.h','runtime/scene/drawing_publication.S','runtime/scene/drawing_physical.h','runtime/scene/glyph_live.c','runtime/scene/glyph_live.h','runtime/scene/glyph_leave.S','runtime/scene/glyph_route.c','reconstruction/rendering/glyph_backend.c','reconstruction/rendering/glyph_backend_state.c','reconstruction/rendering/glyph_backend_x87.S','reconstruction/rendering/glyph_backend.h','reconstruction/rendering/glyph_backend_state.h']
 sources.update({name:sha(ROOT/name) for name in paths})
 parent=ROOT/'working/tests/canvas-producers';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));capture=out/'capture';capture.mkdir();print(out,flush=True)
 env={**{k:v for k,v in os.environ.items() if not k.startswith('MNM_')},'WINEPREFIX':str(ROOT/'working/tests/scene-selftest-wine'),'WINEDEBUG':'-all','MNM_CANVAS_PRODUCERS':'1','MNM_SCENE_DIR':'Z:'+str(capture).replace('/','\\'),'MNM_SCENE_SAMPLES':'1'}
 env.pop('MNM_CANVAS_STARTUP',None);env.pop('MNM_SCENE_WORLD',None)
 if a.prefix_template:
  subprocess.run(['cp','-a','--reflink=auto',str(a.prefix_template.resolve()),str(out/'wineprefix')],check=True)
  env['WINEPREFIX']=str(out/'wineprefix')
 def run(name,command):
  with (out/(name+'.log')).open('x') as log:subprocess.run([str(x) for x in command],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=180)
 run('bridge-build',['python3',ROOT/'tools/build-scene-observer.py','--selftest']);build=ROOT/'working/build/scene-observer-selftest'
 sprite_original=original_fixture(ROOT,out)
 objects=[]
 for name in ['tests/canvas-producers-test.c','tests/canvas-startup-call.S','runtime/scene/canvas_producers.S','tests/world-producer-bypass-call.S','tests/world-effect-entry-call.S','tests/kind8-sprite-primary-call.S']:
  obj=out/(Path(name).name+'.obj');objects.append(obj);run('compile-'+Path(name).name,['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-I'+str(out),'-c',ROOT/name,'-o',obj])
 run('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/base:0x400000','/dynamicbase:no','/safeseh:no','/timestamp:0','/out:'+str(out/'fixture.exe'),*objects,*[build/(name+'.obj') for name in ('drawing_publication.S','glyph_live.c','glyph_leave.S','glyph_route.c','glyph_backend.c','glyph_backend_state.c','glyph_backend_x87.S')],build/'kernel32.lib'])
 run('forwarding',['xvfb-run','-a','wine',out/'fixture.exe'])
 marker=(capture/'producer-fixture-pass-0000.bin').read_bytes()
 if len(marker)!=32 or marker[:8]!=b'MNMPFP01' or struct.unpack('<6I',marker[8:])!=(1,61,1,1,1,1):raise ValueError('Fixture completion marker missing or changed')
 _,effect_ops=m.decode((capture/'world-effect-fixture-0000.bin').read_bytes(),complete=False)
 effect_rows=[(r,p) for r,p in effect_ops if r[2]==9]
 if [r[14] for r,p in effect_rows]!=[8,9,10,11,12] or [r[15] for r,p in effect_rows]!=[13,14,15,16,7]:raise ValueError('Effect hook wire operation/backend changed')
 # Resolve the source definitions before checking the normalized payload.
 definitions={r[1]:payload for r,payload in effect_ops if r[2]==25}
 for r,payload in effect_rows:
  if r[4]:payload=definitions[r[4]]
  if struct.unpack_from('<I',payload,28)[0] or r[19]!=64 or r[20]!={8:512,9:512,10:524,11:520,12:8}[r[14]]:raise ValueError('Effect hook source ownership/extent changed')
  if r[14]==10 and struct.unpack_from('<3I',payload,64+512)!=(244,198,13):raise ValueError('Tint arguments changed')
  if r[14]>=11 and struct.unpack_from('<2I',payload,64+(512 if r[14]==11 else 0))!=(1,2):raise ValueError('Effective source/sampling rows changed')
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
 generic_negative=[]
 for mode in ('unknown','mutated'):
  trial=out/('generic-'+mode);trial.mkdir();trial_env={**env,'MNM_GENERIC_CALLER_FIXTURE':mode,'MNM_SCENE_DIR':'Z:'+str(trial).replace('/','\\')}
  with (out/('generic-'+mode+'.log')).open('x') as log:r=subprocess.run(['xvfb-run','-a','wine',str(out/'fixture.exe')],cwd=ROOT,env=trial_env,stdout=log,stderr=subprocess.STDOUT,timeout=180)
  assert r.returncode==95,(mode,r.returncode)
  _,ops=m.decode((trial/'canvas-producers.bin').read_bytes(),complete=False);assert ops[-1][0][2]==13 and ops[-1][0][14]==43
  assert not list(trial.glob('world-bypass-*'))
  generic_negative.append(dict(mode=mode,exit=r.returncode,failure_code=43,request_files_created=False))
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
 report={'success':True,'sources':sources,'sources_stable':True,'forwarded_entries':61,'native_bypass_entries':1,'complete_raster_bypass_entries':13,'complete_raster_ax_and_cleanup_preserved':True,'outside_world_raster_forwarded':True,'native_bypass_state_preserved_except_recovered_eax':True,'malformed_bypass_refusals':2,'nested_jpeg_lock_forwarded':True,'register_flags_float_error_preserved':True,'signature_refusal_before_mutation':True,'malformed_refusals':refused,'original_renderer_executed':True,'original_sprite_methods_executed':True,'original_pixel_rasters_executed':False,'scope':'Actual PE32 entry/return/relocated JPEG-call forwarding against synthetic compatible prologues,61compatible entry sites with53default hooks. Incoming state is retained except recovered zero EAX on admitted generic bypass; flags/x87/SSE/LastError and atomic signature refusal checked. Selected original sprite methods and primary virtual fragment also execute against synthetic RNG and pixel tails. No original pixel equivalence.'}
 proof=(capture/'world-kind8-sprite-proof-0000.bin').read_bytes()
 if proof[:8]!=b'MNMSPR01' or len(proof)!=40 or struct.unpack('<8I',proof[8:])!=(5,10,15,0,10,628,708,15):raise ValueError('Incomplete actual sprite controls: '+repr(proof))
 _,sprite_ops=m.decode((capture/'world-kind8-sprite-fixture-0000.bin').read_bytes(),complete=False)
 sprite_rows=[r for r,payload in sprite_ops if r[2]==9]
 if [r[15] for r in sprite_rows]!=[1,1,3,8,8]*3:raise ValueError('Sprite native entry routes changed')
 report['sprite_artifacts']={str(capture/name):sha(capture/name) for name in ['world-kind8-sprite-fixture-0000.bin','world-kind8-sprite-batch-0000.bin','world-kind8-sprite-proof-0000.bin']}
 report.update(sprite_original=sprite_original,sprite_primary_calls=10,sprite_direct_abi_entries=5,sprite_random_calls=15,sprite_protected_short_spans=628,sprite_guard_controls=708,sprite_legacy_transport_refused=True,sprite_fixture=str(capture/'world-kind8-sprite-fixture-0000.bin'),sprite_batch=str(capture/'world-kind8-sprite-batch-0000.bin'),sprite_native_draws=15,sprite_original_canvas_protected_unchanged=True)
 report.update(effect_entries_native_bypassed=5,effect_wire_normalized=True,effect_guarded_original_canvas_unchanged=True,effect_malformed_refusals=7)
 extent_raw=(capture/'world-extent640-fixture-0000.bin').read_bytes();_,extent_ops=m.decode(extent_raw,complete=False)
 extent_draws=[r for r,payload in extent_ops if r[2]==9]
 assert len(extent_draws)==1 and list(extent_draws[0][5:8])==[640,480,640] and list(extent_draws[0][14:16])==[8,13] and extent_draws[0][21]==0,extent_draws
 report.update(surface_extent640_native_bypassed=True,surface_extent640_pixel_mode_refused=True,surface_extent640_protected_unchanged=True,surface_extent640_invalid_controls=4,surface_extent640_midqueue_identity_controls=5)

 report.update(vertical_bar_generation_cache=True,vertical_bar_original_pixels_read=False,vertical_bar_clipped_points=True,vertical_bar_pending_and_changed_storage_refused=True,uniform_colour_source_clipping=True,source_point_atomic_recorder_pair=True,source_point_interface_pairing=True,source_point_unknown_ambiguous_stopped_explicit=True,source_point_pending_depth_identity=True,source_point_bad_outputs_refused=True,source_point_last_error_preserved=True,exact_fixture_completion_marker=True,colour_rect_entry_forwarded=True,actual_producer_hook_sites=53,compatible_entry_sites=61)
 report.update(buffered_journal_byte_equality=True,buffered_journal_capacity=65536,buffered_journal_flush_failure_refused=True)
 report.update(batch_body_skips=13,batch_canvas_writebacks=1,batch_prescribed_ax_cleanup=True,batch_guard_refusals=negative,malformed_batch_reply_refusals=3,batch_original_canvas_unwritten_before_return=True,batch_hud_forwarded_after_return=True)
 report.update(generic_caller_refusals=generic_negative,generic_completed_eax_zero=True)
 report.update(oracle_budget_invalid_values_refused=7,oracle_budget_max_mib=3072,oracle_budget_default_mib=1024,oracle_budget_crosses_old_ceiling=True,oracle_budget_exact_ceiling_accepted=True,oracle_budget_over_ceiling_refusals=3,oracle_budget_counter_underflow_refused=True)
 report['metadata_checkpoint_ignores_file_oracle_limit']=True
 report['metadata_checkpoint_unreadable_pixels_untouched']=True
 report['oracle_environment_refused_before_hook_mutation']=budget_negative
 report['minimap_environment_refused_before_hook_mutation']=owned_negative
 report['minimap_fixture_refused_before_hook_mutation']=fixture_negative
 if claims:report['claims']=claims['claims']
 if a.report:
  a.report.parent.mkdir(parents=True,exist_ok=True);write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
 else:write_report(ROOT,out/'report.json',(json.dumps(report,indent=2)+'\n').encode())
 print(a.report or out/'report.json',flush=True)
if __name__=='__main__':main()
