#!/usr/bin/env python3
"""Compare the bounded installed font/DC case with the unchanged independent history oracle."""
import argparse,hashlib,importlib.util,json,os,shlex,subprocess
from pathlib import Path
import numpy as np
from coverage_evidence import Reader,write_report
from rendering_pilot import acquisition,compare,inflate,require,sha,input_acknowledgements
from rendering_provenance_policy import drawing_only
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-acceptance-pilot-results-20261031/'
def digest(path):
 with Path(path).open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--acquisition',required=True);p.add_argument('--driver-review');p.add_argument('--palette-reader-review');p.add_argument('--drawing-only',action='store_true',help='Explicit cursor diagnostic only; withhold Right-key ACK admission');p.add_argument('--report',required=True);p.add_argument('--analysis-review');p.add_argument('--native-font-controls');a=p.parse_args()
 normal=True
 if not a.report.startswith('research/runtime/font-dc-lifetime-results-20261108/') or (ROOT/a.report).exists():p.error('New immutable owned font report required')
 output=ROOT/'working/pilot'/(Path(a.report).stem+'-verification');output.mkdir(parents=True,exist_ok=False)
 report=dict(schema=1,kind='public_native_normal_comparison' if normal else 'public_native_pilot_comparison',success=False,player_accepted=False,original_pixels_used_as_native_inputs=False,scope=__doc__)
 from comparison_process import Processes
 processes=Processes()
 before=False
 def execute(label,command):
  with (output/(label+'.log')).open('xb') as log:processes.run([str(v) for v in command],cwd=ROOT,stdout=log,timeout=300)
 with Reader(ROOT) as reader:
  acquired=reader.json(a.acquisition);report['acquisition']=dict(record=a.acquisition,sha256=reader.digest(a.acquisition))
  if acquired.get('kind')=='public_native_watchdog_diagnostic_acquisition':
   report['kind']='public_native_watchdog_diagnostic_comparison'
   report['scope']+=' Injected/diagnostic execution only; ineligible for nominal acceptance.'
  elif acquired.get('kind')=='public_native_cpu_reader_diagnostic_acquisition':
   report['kind']='public_native_cpu_reader_diagnostic_comparison'
   report['scope']+=' CPU-reader diagnostic execution only; ineligible for nominal acceptance.'
  sources=dict(acquired['sources']);sources.update(acquired.get('native_admission',{}).get('native_only',{}).get('sources',{}))
  sources.update({n:digest(ROOT/n) for n in ['tools/rendering_pilot.py','tools/rendering_capture.py','tools/check-font-lifetime.py','tests/acceptance-world-history.cpp','tests/acceptance-world-reference.cpp','tests/sprite-binary-reference.cpp','tests/world-raster-reference.hpp','tests/clipped-word-grayscale-cases.hpp','tools/native_player_quick_acceptance.py','tools/test-menu-observer.py','tools/rendering_normal_acceptance.py','tools/rendering_provenance_policy.py']});report['sources']=sources
  sources.update({n:digest(ROOT/n) for n in ['tools/comparison_process.py','tools/evidence_workspace.py']})
  try:
   input_policy=drawing_only(acquired,a.drawing_only)
   if input_policy is not None:
    report['input_admission_withheld']=True
    sources['tools/rendering_provenance_policy.py']=digest(ROOT/'tools/rendering_provenance_policy.py')
   require(acquired['success'] is True and acquired['sources_stable'] is True and acquired['private_wine_stopped'] is True and acquired['original_manifest_verified_before_after'] is True,'Incomplete actual public acquisition')
   from rendering_build_binding import require_player_source_closure
   campaign_spec=importlib.util.spec_from_file_location('binding_campaign',ROOT/'tools/test-native-campaign.py');campaign=importlib.util.module_from_spec(campaign_spec);campaign_spec.loader.exec_module(campaign)
   linked=set(campaign.linked_compiler_sources(ROOT/'working/build/qt-shell',ROOT))
   require_player_source_closure(acquired['sources'],linked)
   sources['tools/rendering_build_binding.py']=digest(ROOT/'tools/rendering_build_binding.py')
   if acquired.get('player_build'):
    binding=acquired['player_build'];binary=binding['binary']
    require(acquired.get('compiler_closure_stable') is True and acquired.get('player_binary_stable') is True,'Unstable as-built player')
    require(set(binding['compiler_sources'])==linked and all(acquired['sources'][n]==h for n,h in binding['compiler_sources'].items()),'As-built compiler closure differs')
    require(reader.digest(binary['record'])==binary['sha256']==digest(ROOT/binary['path']),'As-built executable differs')
    report['player_build_binding']=dict(verified=True,build=binding)

   if a.analysis_review:
    review=reader.json(a.analysis_review);require(review['success']and review['acquisition']==report['acquisition'],'Wrong retained-analysis source review')
    require(set(review['changes'])<={'tools/font_lifetime_acceptance.py','tools/check-font-lifetime.py'},'Analysis review cannot replace engine or driver source identity')
    for name,change in review['changes'].items():
     require(change['before']==acquired['sources'][name]and change['after']==digest(ROOT/name),'Analysis review source mismatch');sources[name]=change['after']
    report['retained_analysis_source_review']=dict(record=a.analysis_review,sha256=reader.digest(a.analysis_review))
   report['recorded_acquisition_sources']=dict(acquired['sources'])
   from rendering_normal_acceptance import DRIVER_SOURCES,reviewed_driver_sources
   drift={n for n,h in sources.items() if digest(ROOT/n)!=h}
   require(not drift or (a.driver_review and drift<=DRIVER_SOURCES),'Changed current engine/capture sources')
   report['original_execution_sources']=dict(acquired['sources'])
   artifacts={v['path']:v for v in acquired['artifacts']}
   capture_files=[v for path,v in artifacts.items() if '/capture/' in path and path.endswith('.bin')]
   streams=[]
   for artifact in capture_files:
    raw=reader.read_bytes(artifact['record']);require(sha(raw)==artifact['sha256'],'Capture pointer no longer binds the acquired bytes');streams.append(raw)
   observed=acquisition(streams,acquired['session_identity']);report['observations']=observed['observations']
   del streams
   def bound(path):
    relative=path.relative_to(ROOT).as_posix();require(relative in artifacts,'Unretained verification input: '+relative)
    artifact=artifacts[relative];require(digest(path)==artifact['sha256']==reader.digest(artifact['record']),'Changed acquired verification input: '+relative)
   session=Path(acquired['session']);bound(session/'events.bin')
   report['captured_command_stream_sha256']=sha(observed['commands'])
   if normal:
    from rendering_normal_acceptance import lighting,world_domain,preferences_boundary
    sources['tools/rendering_normal_acceptance.py']=digest(ROOT/'tools/rendering_normal_acceptance.py')
    report['lighting']=lighting(ROOT,acquired,artifacts)
    report['world_domain']=world_domain(observed)
    report['driver_scope_facts']=dict(world_extents=report['world_domain']['extents'],presented_world_image_extents=[list(p) for p in sorted({(r['image_width'],r['image_height']) for r,b in observed['paints'].values() if int(r['world'])>0})])
    if a.driver_review:
     reference=dict(record=a.driver_review,sha256=reader.digest(a.driver_review))
     effective=reviewed_driver_sources(reader,reference,report['acquisition'],acquired,report['driver_scope_facts'])
     for name in DRIVER_SOURCES:sources[name]=effective[name]
     sources.update({n:h for n,h in effective.items() if n not in acquired['sources']})
     report['driver_source_applicability']=reference

    sources.update(report['lighting']['policy']['sources'])
   require(acquired.get('scenario')=='normal-font-dc','Font checker requires its own ordinary scenario')
   from font_lifetime_acceptance import route
   sources.update({n:digest(ROOT/n)for n in ['tools/font_lifetime_acceptance.py','tools/font_lifetime_observation.py','tools/resident_capture.py','tools/cpu_reader_journal.py','tools/storage_cpu_read_binding.py','tools/font_native_lifetime.py']})
   from coverage_claims import behavior_contract,scenario_contract,required_sources
   register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());behavior=next(b for b in register['behaviors']if b['id']=='NP.installed-font-dc-acceptance');scenario=next(s for s in register['scenarios']if s['id']=='native.installed-font-dc-acceptance')
   sources.update({n:digest(ROOT/n)for n in required_sources(behavior)|set(scenario['tests'])})
   report['claims']=[dict(behavior=behavior['id'],contract_sha256=behavior_contract(behavior,{b['id']:b for b in register['builds']}),scenarios={scenario['id']:scenario_contract(scenario)})]
   report['font_dc_lifetime']=route(ROOT,reader,acquired,session,bound)
   report['font_lifetimes_verified']=report['font_dc_lifetime']['verified']
   require(report['world_domain']['extents']==[list(map(int,acquired['configuration']['requested_size'].split('x')))],'Actual font World extent differs')
   root=Path(acquired['temporary_root']);source=root/'worlds.bin';require(digest(source)==artifacts[str(source.relative_to(ROOT))]['sha256'],'World source capture differs')
   history=ROOT/'working/build/qt-shell/mnm-acceptance-world-history';original=ROOT/'working/original-reference';pe=ROOT/'original/Arcane_Nocd/Chaos.exe'
   require(digest(pe)=='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168','Original build mismatch')
   report['binaries']={str(v.relative_to(ROOT)):digest(v) for v in [history,original]}
   # These are oracle-side executables. Their actual compiler closure is kept
   # distinct from the already stopped public player and its runtime adapters.
   dependencies=[ROOT/'working/original-reference.d',*list((ROOT/'working/build/qt-shell').glob('CMakeFiles/mnm-acceptance-world-history.dir/**/*.o.d'))]
   compiled=set()
   for f in dependencies:
    for word in shlex.split(f.read_text().replace('\\\n',' ')):
     candidate=Path(word).resolve()
     if candidate.is_file() and candidate.is_relative_to(ROOT) and not candidate.is_relative_to(ROOT/'working'):compiled.add(str(candidate.relative_to(ROOT)))
   require('tests/acceptance-world-history.cpp' in compiled and 'tests/acceptance-world-reference.cpp' in compiled,'Independent compiler source closure missing')
   sources.update({n:digest(ROOT/n) for n in sorted(compiled)})
   execute('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
   assets=Path(acquired['session'])/'game'
   extensions={'.spr','.jpg','.jpeg','.bmp','.pcx','.sft','.fp','.ttd','.wav'}
   asset_hashes={str(f.relative_to(assets)):digest(f) for f in sorted(assets.rglob('*')) if f.is_file() and f.suffix.lower() in extensions}
   require(all(n in acquired['installation_inputs'] and h==acquired['installation_inputs'][n]['sha256'] for n,h in asset_hashes.items()),'Reference asset differs from the named installation')
   report['asset_inputs']=asset_hashes
   reference=output/'reference'
   execute('original-history',[history,source,Path(acquired['session'])/'game',original,pe,reference])
   index=json.loads((reference/'worlds.json').read_text());require(index['success'] is True and index['native_player_pixels_read'] is False and index['original_destination_capture_read'] is False,'Oracle history provenance')
   worlds={}
   for row in index['worlds']:
    g=int(row['generation']);require(g==len(worlds)+1 and Path(row['path']).name==row['path'],'Original World order/path')
    raw=inflate((reference/row['path']).read_bytes());require(len(raw)==row['width']*row['height']*2,'Original World full plane');worlds[g]=np.frombuffer(raw,dtype='<u2').astype('uint32').reshape(row['height'],row['width'])
   report['original_reference']=dict(worlds=len(worlds),source_chunks=int(index.get('source_chunks','0')),draws=index['draws'],native_player_pixels_read=False,original_destination_capture_read=False,continuous_from_owned_initial_history=True)
   require(all(digest(assets/n)==h for n,h in asset_hashes.items()),'Reference asset changed');report['asset_inputs_stable']=True
   report['comparison']=compare(observed,worlds);report['success']=report['comparison']['success'];report['independent_output_verified']=report['comparison']['success']
   from font_native_lifetime import installed
   require(a.native_font_controls,'Current native font controls required')
   controls=reader.json(a.native_font_controls)
   require(controls['success'] and controls['sources_stable'] and all(digest(ROOT/n)==h for n,h in controls['sources'].items()),'Native font controls no longer applicable')
   log=root/'qt.log';bound(log)
   native_fonts=installed(acquired,log.read_bytes())
   expected=controls['known_catalogs'][0]
   require(all(g['key_sha256']==expected['key_sha256'] and all(g['registrations'][i]['ttf_sha256']==expected['registrations'][str(i)]['ttf_sha256'] for i in range(4))for g in native_fonts['source_catalogs']),'Installed native font catalog differs from independently tested menu fonts')
   require(all(acquired['installation_inputs'][n]['sha256']==h for n,h in controls['inputs'].items()),'Installed SFT sources differ from native font controls')
   report['native_menu_fonts']=native_fonts
   report['native_font_controls']=dict(record=a.native_font_controls,sha256=reader.digest(a.native_font_controls))
   if normal:
    from rendering_player_acceptance import first_presented
    sources.update({n:digest(ROOT/n) for n in ['tools/rendering_player_acceptance.py','tools/rendering_storage_acceptance.py','tools/rendering_acceptance.py','tools/coverage_claims.py']})
    report['first_presented_world']=first_presented(observed,report['comparison'],report['original_reference'])
  except Exception as e:report.update(success=False,error=str(e))
  finally:
   if before:
    try:execute('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
    except Exception as e:report.update(success=False,original_verification_error=str(e))
   report['sources_stable']=all(digest(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable']
   report['artifacts']=[]
   for f in sorted(output.rglob('*')):
    if f.is_file():
     name=str(Path(a.report).with_suffix(''))+'-'+hashlib.sha256(str(f.relative_to(output)).encode()).hexdigest()[:20]+'.bin';data=f.read_bytes();h=write_report(ROOT,name,data);report['artifacts'].append(dict(path=str(f.relative_to(output)),record=name,sha256=h,bytes=len(data)))
   report.update(output_directory=str(output),comparison_processes_stopped=processes.stopped(),artifact_retention_complete=True)
   h=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps({k:report.get(k) for k in ['success','error','observations','original_reference']}|{'record':a.report,'sha256':h}),flush=True)
 from evidence_objects import settings
 if settings(ROOT) and report['comparison_processes_stopped']:
  from evidence_workspace import register_completed
  register_completed(ROOT,[output],dict(record=a.report,sha256=h),producer='comparison')
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
