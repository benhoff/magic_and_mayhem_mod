#!/usr/bin/env python3
"""Run a prepared finite EP05 case; semantic completion never grants pixel acceptance."""
import argparse,hashlib,importlib.util,json,os,secrets,signal,subprocess,sys,time
from pathlib import Path
from coverage_evidence import write_report
ROOT=Path(__file__).resolve().parents[1]
from rendering_progression import PREFIX, CASES, recipes, require, validate_flow, validate_callbacks
from coverage_evidence import Reader
def sha(path):
 with Path(path).open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def module(name):
 spec=importlib.util.spec_from_file_location(name,ROOT/'tools'/name);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('--evidence-mode',choices=['diagnostic','full'],default='diagnostic',help='Default: bounded semantic diagnostics without pixel/World captures; full is required for rendering evidence')
 p.add_argument('--deadline-seconds',type=int,help='15..900; default diagnostic120/full900')
 p.add_argument('--checkpoint',choices=['capture','restore'],help='Separate diagnostic Save/Load; never nominal progression acceptance')
 p.add_argument('--checkpoint-probe',choices=['orders','paused-orders','paused-cast','combat','portal'],help='Bounded physical selection/move, combat-order or portal-command probe after verified original checkpoint restore')
 p.add_argument('--checkpoint-name',help='New unique checkpoint- name for capture or exact existing name for restore')
 p.add_argument('--checkpoint-seed',help='Capture from an unchanged original Save/Load candidate report; new Save/Load witnesses are still required')
 p.add_argument('--checkpoint-reference',help='Successful capture acquisition report required for restore')
 p.add_argument('--report',required=True);p.add_argument('--case',choices=CASES,required=True)
 p.add_argument('--size',choices=['640x480','800x600'],default='800x600')
 p.add_argument('--preparation',required=True);p.add_argument('--software',action='store_true')
 p.add_argument('--observe-input-admission',action='store_true',help='Extra passive original mouse/hover observer; diagnostic, never nominal acceptance')
 p.add_argument('--observe-startup-timing',action='store_true',help='Existing bounded host/resident timing trace; diagnostic, never nominal acceptance')
 p.add_argument('--startup-main-dwell-ms',type=int,default=0,help='Diagnostic dwell before and after Preferences (0..30000ms); requires --observe-startup-timing')
 p.add_argument('--prefix-template',type=Path);p.add_argument('--skip-intros',action='store_true');p.add_argument('--storage-policy',type=Path)
 a=p.parse_args()
 require(not a.checkpoint_probe or a.checkpoint=='restore','Checkpoint probe requires restore')
 from check_capture_policy import policy
 capture_policy=policy(a.evidence_mode,a.deadline_seconds)
 full=capture_policy['full_capture']
 if a.checkpoint:
  from campaign_checkpoint import save_name
  save_name(a.checkpoint_name or '')
  require(a.case=='victory' and a.skip_intros and not a.observe_startup_timing,'Checkpoint diagnostics require --case victory --skip-intros without startup timing')
  store=ROOT/'working/user-saves/native-menu';saved=store/(a.checkpoint_name+'.sav')
  if a.checkpoint=='capture':
   require(not saved.exists() and not a.checkpoint_reference,'Capture requires a new save name')
   if a.checkpoint_seed:
    with Reader(ROOT) as reader:seed=reader.json(a.checkpoint_seed);seed_sha=reader.digest(a.checkpoint_seed)
    f=seed.get('flow',{});seed_name=save_name(f.get('saved_name',''));seed_file=store/(seed_name+'.sav')
    require(seed.get('kind')=='public_native_checkpoint_diagnostic_acquisition' and seed.get('exit_code')==0 and seed.get('sources_stable') is True and seed.get('compiler_closure_stable') is True and seed.get('player_binary_stable') is True and seed.get('private_wine_stopped') is True and seed.get('original_manifest_verified_before_after') is True and seed.get('artifact_retention_complete') is True and f.get('success') is True and f.get('normal_quit') is True and f.get('save_load') is True,'Seed requires a retained normally completed original Save/Load flow; its old acquisition status is not promoted')
    require(seed_file.is_file() and not seed_file.is_symlink() and sha(seed_file)==f.get('saved_sha256'),'Seed candidate hash mismatch')
    from campaign_storage import validate_storage_flow
    validate_storage_flow(f)
    with Reader(ROOT) as reader:
     matches=[v for v in seed['artifacts'] if v['path']==str((Path(seed['temporary_root'])/'checkpoint-route.json').relative_to(ROOT))]
     require(len(matches)==1,'Missing retained seed route')
     seed_route=reader.json(matches[0]['record']);require(reader.digest(matches[0]['record'])==matches[0]['sha256'],'Seed route hash mismatch')
    require(seed_route.get('checkpoint_ready') is True and seed_route.get('success') is True and seed_route.get('simulation_writes') is False,'Seed route never reached the checkpoint')

  else:
   require(not a.checkpoint_seed,'Seed is only an input to a new capture')
   require(a.checkpoint_reference and saved.is_file() and not saved.is_symlink(),'Restore requires a pinned capture and private existing save')
   with Reader(ROOT) as reader:reference=reader.json(a.checkpoint_reference);reference_sha=reader.digest(a.checkpoint_reference)
   require(reference.get('success') is True and reference.get('checkpoint',{}).get('verified') is True and sha(saved)==reference['checkpoint']['save_sha256'],'Restore save/reference hash mismatch')
 else:require(not a.checkpoint_name and not a.checkpoint_reference and not a.checkpoint_seed,'Checkpoint options require --checkpoint')
 require(0<=a.startup_main_dwell_ms<=30000 and (not a.startup_main_dwell_ms or a.observe_startup_timing),'Bounded startup dwell requires timing diagnostic')
 require(a.report.startswith(PREFIX) and '..' not in Path(a.report).parts and not (ROOT/a.report).exists(),'New owned report required')
 with Reader(ROOT) as reader:
  prepared=reader.json(a.preparation);preparation_sha=reader.digest(a.preparation)
 require(prepared['kind']=='rendering_progression_preparation' and prepared['success'] is True,'Valid preparation required')
 require(all(sha(ROOT/n)==h for n,h in prepared['sources'].items()),'Prepared driver sources changed; prepare a new result')
 from check_inputs import paths as shared_paths
 shared_inputs=shared_paths(ROOT)
 game=Path(prepared['source_game'])
 require(recipes(game)==prepared['maps'],'Prepared map inputs changed')
 require(all(sha(game/r['path'])==r['sha256'] for r in prepared['configuration_inputs']),'Prepared executable/configuration changed')
 require(prepared.get('launch_manifest') and sha(ROOT/prepared['launch_manifest']['path'])==prepared['launch_manifest']['sha256'],'Missing or changed prepared launcher manifest')
 normal=True
 token=Path(a.report).stem;out=ROOT/'working/pilot'/token;out.mkdir(parents=True,exist_ok=False);(out/'.retain').write_text('Actual public pilot and all negative attempts. Retain before RAM teardown.\n')
 launcher=module('run-native-owned.py');campaign=module('test-native-campaign.py')
 # The public launcher can rebuild. Freeze dependencies only after that build,
 # then require the same executable and dependency graph after execution.
 build=(ROOT/'working/build/qt-shell').resolve()
 build_tmp=build.parent/'tmp';build_tmp.mkdir(parents=True,exist_ok=True)
 build_commands=[['cmake','-S',str(ROOT/'apps/qt-shell'),'-B',str(build),'-DCMAKE_BUILD_TYPE=Release','-DBUILD_TESTING=OFF'],['cmake','--build',str(build),'--target','mnm-qt-shell','--parallel','4']]
 build_error=None
 try:
  with (out/'player-build.log').open('xb') as log:
   for command in build_commands:subprocess.run(command,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=300,env=dict(os.environ,TMPDIR=str(build_tmp)))
 except Exception as e:build_error=str(e)
 build_record=str(Path(a.report).with_suffix(''))+'-player-build.bin'
 build_sha=write_report(ROOT,build_record,(out/'player-build.log').read_bytes())
 if build_error:
  result=dict(schema=1,kind='native_player_build_preparation_failure',success=False,error=build_error,commands=build_commands,log=dict(record=build_record,sha256=build_sha),original_artifacts_consumed=False)
  digest=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=False,error=build_error,record=a.report,sha256=digest)),flush=True);return 1
 compiled=set(campaign.linked_compiler_sources(build,ROOT))
 binary=build/'mnm-qt-shell';binary_record=str(Path(a.report).with_suffix(''))+'-player-binary.bin'
 binary_sha=write_report(ROOT,binary_record,binary.read_bytes())
 names=set(compiled)
 names.update(['run-native.sh','tools/run-native-game.py','tools/run-native-owned.py','tools/run-qt-shell.sh','tools/native_only_contract.py','tools/native_prefix_template.py','tools/experiment_environment.py','tools/directplay_dx6_launch.py','tools/working-retention.py','tools/run-rendering-progression.py','tools/test-native-campaign.py'])
 names.update(['tools/rendering_capture.py','tools/rendering_pilot.py']);names.update(prepared['sources']);names.update(['tools/menu-game-runner.py','tools/run-game.sh','tools/owned-menu-game-runner.py','tools/prepare-menu-observer.py']);names.update(['tools/menu-storage-input.py','tools/campaign_storage.py','tools/run-rendering-progression.py','tools/campaign_gameplay.py'])
 # Freeze native runtime inputs as well as the linked Qt compiler closure.
 # Native DLLs are built by the launcher after the public player build. Their
 # retained build manifests alone must not hide runtime source drift here.
 from rendering_build_binding import native_runtime_source_paths
 runtime_names=native_runtime_source_paths(ROOT)
 names.update(['tools/rendering_build_binding.py','tools/check_capture_policy.py','tools/check_inputs.py'])
 names.update(['tools/campaign_checkpoint.py','apps/qt-shell/campaign_storage_test.cpp','tools/campaign_order_observation.py','tools/campaign-order-input.py','tools/campaign_target_admission.py'])
 names.update(runtime_names)
 sources={n:sha(ROOT/n) for n in sorted(names)}
 report=dict(schema=1,kind='public_native_progression_diagnostic_acquisition' if a.skip_intros else 'public_native_progression_acquisition',success=False,player_accepted=False,scenario=a.case,preparation=dict(record=a.preparation,sha256=preparation_sha),
  sources=sources,native_runtime_sources=sorted(runtime_names),source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),session_identity=secrets.token_hex(16),
  configuration=dict(id='player-software' if a.software else 'player-default',software_flag=a.software,skip_intros=a.skip_intros,terrain_lighting='original',native_lighting_capture=False,viewport_scaling='integer-sharp'),
  original_pixels_used_as_native_inputs=False,original_reference_executed=False,scope=__doc__)
 report['shared_inputs']={k:str(v) for k,v in shared_inputs.items()}
 report['player_build']=dict(commands=build_commands,log=dict(record=build_record,sha256=build_sha),binary=dict(path='working/build/qt-shell/mnm-qt-shell',record=binary_record,sha256=binary_sha),compiler_sources={n:sources[n] for n in sorted(compiled)})
 report['configuration'].update(evidence_mode=a.evidence_mode,capture_policy=capture_policy,diagnostic_only=a.skip_intros or not full)
 env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE')) and k not in ['DISPLAY','XAUTHORITY','LIBGL_ALWAYS_SOFTWARE','GALLIUM_DRIVER','MESA_LOADER_DRIVER_OVERRIDE','QT_XCB_GL_INTEGRATION','__GLX_VENDOR_LIBRARY_NAME','__EGL_VENDOR_LIBRARY_FILENAMES']}
 env.update(QT_QPA_PLATFORM='xcb',LP_NUM_THREADS='4',QT_SCALE_FACTOR='1',WINEDEBUG='-all',TMPDIR=str(out),
  MNM_QT_BUILD_TYPE='Release',MNM_QT_BUILD_TESTS='OFF',MNM_MENU_GAME_SOURCE=str(game),
  MNM_LIVE_MENU_TEST_QT_JOURNEY='1',MNM_QUICK_BATTLE_MAP_ROW='0',MNM_ACCEPTANCE_PILOT='1',MNM_ACCEPTANCE_SKIP_INTROS=str(int(a.skip_intros)),
  MNM_TERRAIN_LIGHTING='original',MNM_NO_ROUTE_SCREENSHOTS='1',MNM_RESIDENT_STAGE_PROFILE='1',MNM_RESIDENT_PROFILE_CAPTURE=str(out/'worlds.bin'),
  MNM_ACCEPTANCE_CAPTURE_DIRECTORY=str(out/'capture'),MNM_ACCEPTANCE_CAPTURE_ID=report['session_identity'])
 if normal:
  env.update(MNM_ACCEPTANCE_NORMAL='1',MNM_ACCEPTANCE_CAMPAIGN='1',MNM_TERRAIN_LIGHTING='native',MNM_RESIDENT_CAPTURE_SOURCE_CHUNKS='1',XDG_CONFIG_HOME=str(out/'config'))
  report['configuration']['terrain_lighting']='native'
  report['configuration']['source_chunks']=True
  report['configuration'].update(viewport_capture_stored_limit_bytes=1024**3,viewport_capture_policy='campaign-lossless-viewport-v1',viewport_capture_raw_limit_bytes=64*1024**3,world_capture_retained_limit_bytes=16*1024**3,world_capture_format=4,world_capture_policy='zstd-prefix-sha256-v1')
  report['configuration']['configuration_home']=str(out/'config')
 if a.storage_policy:
  policy=a.storage_policy.resolve(strict=True)
  env['MNM_WORKING_STORAGE_POLICY']=str(policy)
  name=str(Path(a.report).with_suffix(''))+'-storage-policy.bin'
  report['storage_policy']=dict(path=str(policy),record=name,sha256=write_report(ROOT,name,policy.read_bytes()))
 deadline_ms=capture_policy['deadline_seconds']*1000
 report['configuration'].update(deadline_ms=deadline_ms,difficulty=1,experimental_campaign_menus=True,requested_size=a.size)
 env.pop('MNM_LIVE_MENU_TEST_QT_JOURNEY')
 env.update(MNM_CAMPAIGN_TIMEOUT_MS=str(deadline_ms-5000),MNM_CAMPAIGN_SMOKE_TEST='1',MNM_CAMPAIGN_PROGRESSION_CASE=a.case,MNM_CAMPAIGN_PROGRESSION_SIZE=a.size,MNM_EXPERIMENTAL_CAMPAIGN_MENUS='1',MNM_CAMPAIGN_CASTING_COMBAT='1')
 if a.case!='defeat':env['MNM_CAMPAIGN_FOLLOW_OBSERVE']='1'
 report['configuration']['follow_reference_observation']=a.case!='defeat'
 baseline_orders=a.case!='defeat'
 report['configuration']['input_admission_observation']=a.observe_input_admission or baseline_orders
 report['configuration']['order_admission_observation']=baseline_orders or bool(a.checkpoint_probe)
 if baseline_orders:
  env.update(MNM_CAMPAIGN_INPUT_ADMISSION='1',MNM_CAMPAIGN_ORDER_ADMISSION='1')
  report['configuration']['input_readiness']='original-callback-then-original-World-return-v3'
  report['configuration']['order_acknowledgement']='original-selected-actor-opcode7-buffer-submission'

 report['configuration']['startup_timing_observation']=a.observe_startup_timing
 report['configuration']['startup_main_dwell_ms']=a.startup_main_dwell_ms
 if a.observe_startup_timing:
  env['MNM_RESIDENT_POLL_TIMING']='1'
  env['MNM_CAMPAIGN_STARTUP_DWELL_MS']=str(a.startup_main_dwell_ms)
  report['kind']='public_native_progression_diagnostic_acquisition'
  report['configuration']['diagnostic_only']=True
 if a.observe_input_admission or a.checkpoint_probe:
  env['MNM_CAMPAIGN_INPUT_ADMISSION']='1'
  report['kind']='public_native_progression_diagnostic_acquisition'
  report['configuration'].update(input_readiness='original-callback-then-original-World-return-v3',diagnostic_only=True)
 if a.checkpoint_probe:
  env.update(MNM_CAMPAIGN_ORDER_ADMISSION='1',MNM_CAMPAIGN_CHECKPOINT_PROBE=a.checkpoint_probe)
  report['configuration']['checkpoint_probe']=a.checkpoint_probe
 if a.checkpoint:
  report.update(kind='public_native_checkpoint_diagnostic_acquisition',scenario='checkpoint-'+a.checkpoint)
  report['configuration'].update(diagnostic_only=True,checkpoint_mode=a.checkpoint,checkpoint_name=a.checkpoint_name)
  env.pop('MNM_CAMPAIGN_PROGRESSION_CASE',None)
  env.update(MNM_CAMPAIGN_CHECKPOINT=a.checkpoint,MNM_CAMPAIGN_STORAGE='1',MNM_CAMPAIGN_DIFFICULTY='1',MNM_CAMPAIGN_STORAGE_NAME=a.checkpoint_name,MNM_CAMPAIGN_TIMEOUT_MS=str(deadline_ms-5000))
  if a.checkpoint=='capture' and a.checkpoint_seed:
   report['checkpoint_seed_reference']=dict(record=a.checkpoint_seed,sha256=seed_sha,save_name=seed_name,save_sha256=seed['flow']['saved_sha256'],scope='Unchanged original candidate input only; this acquisition must establish a new Save/Load witness')
   (out/'checkpoint-route.json').write_bytes((json.dumps(dict(seed_route,seed_reference=report['checkpoint_seed_reference']),indent=2)+'\n').encode())
   env.update(MNM_CAMPAIGN_CHECKPOINT_SEED=seed_name,MNM_CAMPAIGN_CHECKPOINT_SEED_SHA256=seed['flow']['saved_sha256'])
  if a.checkpoint=='restore':
   env['MNM_CAMPAIGN_STORAGE_RESTART']='1'
   report['checkpoint_reference']=dict(record=a.checkpoint_reference,sha256=reference_sha)
 if not full:
  report['kind']='public_native_campaign_lightweight_diagnostic'
  report['configuration'].update(source_chunks=False,viewport_capture_stored_limit_bytes=0,viewport_capture_raw_limit_bytes=0,world_capture_retained_limit_bytes=0)
  for key in ('MNM_ACCEPTANCE_CAPTURE_DIRECTORY','MNM_ACCEPTANCE_CAPTURE_ID','MNM_RESIDENT_PROFILE_CAPTURE','MNM_RESIDENT_CAPTURE_SOURCE_CHUNKS'):
   env.pop(key,None)
 report['installation_inputs']={str(f.relative_to(game)):dict(sha256=sha(f),bytes=f.stat().st_size) for f in sorted(game.rglob('*')) if f.is_file()}
 before_experiments=set((ROOT/'working/experiments/menu-observer').glob('run-*'))
 server=child=None;prefix_lease=None;verified=False;before=set((ROOT/'working/tests/native-output-owner').glob('run-*'))
 def execute_log(label,command):
  with (out/(label+'.log')).open('xb') as log:subprocess.run([str(v) for v in command],stdout=log,stderr=subprocess.STDOUT,check=True,timeout=180)
 try:
  execute_log('original-before',[ROOT/'tools/original-manifest.sh','verify']);verified=True
  host=out/'host';host.mkdir();server,owner,_=launcher.start_display(host,allow_glx=True);env.update(DISPLAY=owner['display'],XAUTHORITY=owner['authority'])
  from evidence_objects import settings
  template=a.prefix_template or shared_inputs.get('prefix') or ROOT/'working/fixtures/prefix-seed'
  if settings(ROOT) and template.is_dir():
   from native_prefix_template import borrow
   prefix_lease,report['prefix_pool']=borrow(template,out/'wineprefix',env)
  command=[str(ROOT/'run-native.sh'),'--prefix',str(out/'wineprefix')]
  if prefix_lease is None and template.is_dir():command+=['--prefix-template',str(template)]
  command+=['--','--live-menu-test',str(out/'flow.json')]
  if shared_inputs.get('assets'):command+=['--music-assets',str(shared_inputs['assets'])]
  if a.software:command.append('--software-rendering')
  if a.skip_intros:command.append('--skip-movies')
  report['command']=command
  with (out/'public-launch.log').open('xb') as log:
   child=subprocess.Popen(command,cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
   child.wait(timeout=deadline_ms/1000+40)
  report['exit_code']=child.returncode
  subprocess.run(['wineserver','-w'],env=dict(env,WINEPREFIX=str(out/'wineprefix')),check=True,timeout=10)
  report['private_wine_stopped']=True
  flow=json.loads((out/'flow.json').read_text()) if (out/'flow.json').is_file() else None
  if flow is not None:report['flow']=flow
  if child.returncode:raise ValueError('Public launcher failed: '+str(flow.get('error') if flow else 'retained its actual output'))
  require(flow is not None,'Missing public driver flow')
  if not a.checkpoint:report['semantic_route']=validate_flow(flow,a.case)
  owners=set((ROOT/'working/tests/native-output-owner').glob('run-*'))-before
  if len(owners)!=1:raise ValueError('Ambiguous public launch owner')
  owner=owners.pop();report['owner']=str(owner);sessions=sorted((owner/'sessions').glob('*.json'))
  if len(sessions)!=1:raise ValueError('Pilot requires one normal player generation')
  session=Path(json.loads(sessions[0].read_text())['session']);report['session']=str(session)
  from native_only_contract import outcome
  report['retirement']=outcome(session)
  if report['retirement']['state']!='completed':raise ValueError('Normal original Quit/resources missing')
  retired=report['retirement']['graphics_retirement']
  require(retired['process_exit'] is True and retired['resources_released'] is True and not retired['surviving_owners'] and all(v==0 for v in retired['counts'].values()),'Surviving native/original owners')
  report['native_admission']=json.loads((session/'native-output-admission.json').read_text())
  report['resident']=json.loads((session/'resident-world-report.json').read_text())
  if full:
   source_capture=report['resident'].get('stage_profile',{}).get('capture',{})
   require(source_capture.get('complete') is True and source_capture.get('truncated') is False,'Incomplete continuous World/source capture')
   require(source_capture.get('format')==4 and source_capture.get('codec')=='zstd-prefix-sha256-v1','Campaign World/source capture policy missing')
   from rendering_pilot import capture
   report['capture_completion']=[]
   for path in sorted((out/'capture').glob('*.bin')):
    rows=capture(path.read_bytes());first,last=rows[0][0],rows[-1][0]
    if first['role']=='viewport':require(first['version']==2 and first.get('policy')=='campaign-lossless-viewport-v1','Campaign viewport capture policy missing')
    report['capture_completion'].append(dict(path=str(path.relative_to(ROOT)),role=first['role'],version=first['version'],rows=len(rows),raw_bytes=last['raw_bytes'],complete=last['complete']))
    del rows
   require({r['role'] for r in report['capture_completion']} in ({'viewport','commands'},{'viewport','commands','exports'}),'Incomplete capture role set')
  if a.checkpoint:
   from campaign_checkpoint import validate_capture,validate_restore
   raw_save=(session/'game/Save'/(a.checkpoint_name+'.sav')).read_bytes()
   if a.checkpoint=='capture':
    report['checkpoint']=validate_capture(flow,json.loads((out/'checkpoint-route.json').read_text()),(session/'checkpoint-events.bin').read_bytes(),raw_save)
   else:report['checkpoint']=validate_restore(flow,(session/'checkpoint-events.bin').read_bytes(),raw_save,reference['checkpoint'])
   require(saved.is_file() and sha(saved)==report['checkpoint']['save_sha256'],'Normal exit did not preserve exact checkpoint in private owned store')
  if a.checkpoint_probe:
   from campaign_order_observation import decode as decode_orders
   report['checkpoint_probe']=json.loads((out/'checkpoint-orders.json').read_text())
   observed=decode_orders((session/'order-admission.bin').read_bytes())
   report['order_observation']=dict(rows=len(observed['rows']),pending_callbacks=observed['pending_callbacks'],command_delivery_proved=False)
   require(not observed['pending_callbacks'],'Order observation ended within a callback')
   require(report['checkpoint_probe'].get('success') is True,'Bounded checkpoint order probe failed: '+str(report['checkpoint_probe'].get('error')))
  if not a.checkpoint:report['original_callbacks']=validate_callbacks((session/'events.bin').read_bytes(),flow)
  require(shared_paths(ROOT)==shared_inputs,'Shared input binding changed during execution')
  report['map_inputs_stable']=recipes(game)==prepared['maps']
  require(report['map_inputs_stable'],'Input map drift during execution')
  report['independent_history_admission']='pending separate comparison of every first/later World swap and populated owner handoff' if full else 'ineligible lightweight diagnostic; no World/pixel capture requested'
  report['success']=True
 except Exception as e:report['error']=str(e)
 finally:
  if child is not None and child.poll() is None:
   try:os.killpg(child.pid,signal.SIGTERM);child.wait(timeout=15)
   except (ProcessLookupError,subprocess.TimeoutExpired):
    try:os.killpg(child.pid,signal.SIGKILL);child.wait(timeout=10)
    except ProcessLookupError:pass
  if child is not None and child.poll() is not None and (out/'wineprefix').is_dir():
   try:
    from check_capture_policy import stop_private_prefix
    report['prefix_shutdown']=stop_private_prefix(out/'wineprefix',env)
    report['private_wine_stopped']=True
   except Exception as e:report.update(success=False,private_wine_stopped=False,prefix_shutdown_error=str(e))
  if server is not None:server.terminate();server.wait(timeout=10)
  if prefix_lease is not None:
   try:
    prefix_lease.close()
    report['prefix_pool']['returned']=True
    report['prefix_pool']['receipt']=str(prefix_lease.receipt)
   except Exception as e:report.update(success=False,prefix_pool_return_error=str(e))
  if verified:
   try:execute_log('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
   except Exception as e:report.update(success=False,original_verification_error=str(e))
  report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable']
  try:
   report['compiler_closure_stable']=set(campaign.linked_compiler_sources(build,ROOT))==compiled
   report['player_binary_stable']=sha(binary)==binary_sha
  except Exception as e:
   report.update(compiler_closure_stable=False,player_binary_stable=False,build_binding_error=str(e))
  report['success'] &= report['compiler_closure_stable'] and report['player_binary_stable']
  if a.storage_policy:
   report['storage_policy_stable']=sha(policy)==report['storage_policy']['sha256'];report['success'] &= report['storage_policy_stable']
  # Preserve exact execution/exit/source assertions before importing large raw
  # captures. An interrupted artifact collector must not lose this boundary or
  # require a recovery pass to invent the former process's in-memory claims.
  checkpoint=str(Path(a.report).with_suffix(''))+'-execution.json'
  execution=dict(report,kind='public_native_progression_execution_checkpoint',
      acquisition_kind=report['kind'],semantic_acquisition_success=report['success'],
      success=False,artifact_retention_complete=False)
  checkpoint_sha=write_report(ROOT,checkpoint,(json.dumps(execution,indent=2)+'\n').encode())
  report['execution_checkpoint']=dict(record=checkpoint,sha256=checkpoint_sha)
  # Physical files here are bounded execution interchange. Citable bytes go
  # directly to SQLite before this RAM namespace may retire.
  report['artifacts']=[]
  candidates=[f for f in out.rglob('*') if f.is_file() and 'wineprefix' not in f.relative_to(out).parts and f.name!='Xauthority']
  for owner in set((ROOT/'working/tests/native-output-owner').glob('run-*'))-before:
   candidates += [f for f in owner.rglob('*') if f.is_file() and f.name!='Xauthority']
  experiments=set((ROOT/'working/experiments/menu-observer').glob('run-*'))-before_experiments
  report['experiments']=[str(v) for v in sorted(experiments)]
  for session in sorted(experiments):
   candidates += [f for f in session.rglob('*') if f.is_file() and ('game' not in f.relative_to(session).parts or f.suffix.lower() in ('.dll','.cfg','.sav','.ini') or (f.name.startswith('native-') and f.suffix=='.bin'))]
  for f in sorted(set(candidates)):
   limit=16*1024**3 if f==out/'worlds.bin' else 1024**3 if f.parent==out/'capture' and normal else 512*1024**2
   try:
    if f.is_symlink() or f.stat().st_size>limit:continue
    relative=str(f.relative_to(ROOT));name=str(Path(a.report).with_suffix(''))+'-raw-'+hashlib.sha256(relative.encode()).hexdigest()[:20]+'.bin'
    raw=f.read_bytes();digest=write_report(ROOT,name,raw);report['artifacts'].append(dict(path=relative,record=name,sha256=digest,bytes=len(raw)))
   except Exception as e:
    report.update(success=False,artifact_retention_error=dict(path=str(f),error=str(e)))
    break
  report['artifact_retention_complete']='artifact_retention_error' not in report
  report['artifact_count']=len(report['artifacts']);report['temporary_root']=str(out)
  digest=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps({k:report.get(k) for k in ['success','error','exit_code','artifact_count']}|{'record':a.report,'sha256':digest}),flush=True)
  from evidence_objects import settings
  if settings(ROOT) and report.get('private_wine_stopped') is True and (prefix_lease is None or report.get('prefix_pool',{}).get('returned') is True):
   from evidence_workspace import register_completed
   register_completed(ROOT,[out,*experiments],dict(record=a.report,sha256=digest))
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
