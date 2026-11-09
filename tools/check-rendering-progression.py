#!/usr/bin/env python3
"""Compare a real public-player acquisition with an independent original/history oracle."""
import argparse,hashlib,importlib.util,json,os,shlex,subprocess,tempfile
from pathlib import Path
import numpy as np
from coverage_evidence import Reader,write_report
from rendering_pilot import acquisition,compare,inflate,require,sha,input_acknowledgements
ROOT=Path(__file__).resolve().parents[1]
from rendering_progression import PREFIX,validate_flow,validate_callbacks
def digest(path):
 with Path(path).open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--acquisition',required=True);p.add_argument('--report',required=True);p.add_argument('--temporary-reference-frames',action='store_true',help='Classify successful reproducible oracle qz planes as temporary; retain hashes, index, commands and tested executables. Failures retain the full diagnostic outputs.');a=p.parse_args()
 normal=True
 if not a.report.startswith(PREFIX) or '..' in Path(a.report).parts or (ROOT/a.report).exists():p.error('New immutable owned report required')
 output=ROOT/'working/pilot'/(Path(a.report).stem+'-verification');output.mkdir(parents=True,exist_ok=False)
 report=dict(schema=1,kind='public_native_progression_comparison',success=False,player_accepted=False,original_pixels_used_as_native_inputs=False,original_history_deadline_seconds=900,scope=__doc__)
 from comparison_process import Processes
 processes=Processes()
 before=False;reference=None;index=None;reconstruction=None
 report['temporary_output_policy']=dict(reference_frames_requested=a.temporary_reference_frames,classification='Successful reproducible oracle qz planes only; raw acquired observations and all failure outputs remain durable',removal='Separate reviewed handoff required; this producer deletes nothing')
 def execute(label,command,timeout=300):
  with (output/(label+'.log')).open('xb') as log:processes.run([str(v) for v in command],cwd=ROOT,stdout=log,timeout=timeout)
 with Reader(ROOT) as reader:
  acquired=reader.json(a.acquisition);report['acquisition']=dict(record=a.acquisition,sha256=reader.digest(a.acquisition));report['acquired_source_fingerprints']=dict(acquired['sources'])
  if acquired.get('kind') in ('public_native_watchdog_diagnostic_acquisition','public_native_progression_diagnostic_acquisition'):
   report['kind']='public_native_progression_diagnostic_comparison'
   report['scope']+=' Injected/diagnostic execution only; ineligible for nominal acceptance.'
  sources=dict(acquired['sources']);sources.update(acquired.get('native_admission',{}).get('native_only',{}).get('sources',{}))
  sources.update({n:digest(ROOT/n) for n in ['tools/rendering_progression.py','tools/rendering_pilot.py','tools/rendering_capture.py','tools/check-rendering-progression.py','tests/acceptance-world-history.cpp','tests/acceptance-world-reference.cpp','tests/sprite-binary-reference.cpp','tests/world-raster-reference.hpp','tests/clipped-word-grayscale-cases.hpp','tools/native_player_quick_acceptance.py','tools/test-menu-observer.py','tools/oracle_frame_retention.py']});report['sources']=sources
  try:
   from check_capture_policy import require_full
   require_full(acquired)
   sources.update({n:digest(ROOT/n) for n in ['tools/check_capture_policy.py','tools/comparison_process.py','tools/evidence_workspace.py']})
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

   from rendering_normal_acceptance import DRIVER_SOURCES,reviewed_driver_sources
   drift={n for n,h in sources.items() if digest(ROOT/n)!=h}
   require(not drift,'Changed current engine/capture/driver sources')
   report['original_execution_sources']=dict(sources)
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
    require(report['world_domain']['extents']==[list(map(int,acquired['configuration']['requested_size'].split('x')))],'Actual imported RGB565 World mode differs')
    report['driver_scope_facts']=dict(world_extents=report['world_domain']['extents'],presented_world_image_extents=[list(p) for p in sorted({(r['image_width'],r['image_height']) for r,b in observed['paints'].values() if int(r['world'])>0})])
    sources.update(report['lighting']['policy']['sources'])
   require(acquired['kind'] in ('public_native_progression_acquisition','public_native_progression_diagnostic_acquisition'),'Wrong acquisition type')
   report['semantic_route']=validate_flow(acquired['flow'],acquired['scenario'])
   report['original_menu_actions']=validate_callbacks((session/'events.bin').read_bytes(),acquired['flow'])
   report['transition_worlds']=[]
   for world in acquired['flow']['worlds']:
    matching=[i for i in observed['swaps'] if int(observed['paints'][i][0]['world'])==world['world']]
    require(matching,'Reported transition World was not independently captured')
    report['transition_worlds'].append(dict(region=world['region'],realm=world['realm'],world=world['world'],swaps=matching))
   report['populated_handoff_lifetime_closure']='pending original object/vptr/generation acquire/release proof; final zero owners alone is insufficient'
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
   if a.temporary_reference_frames:
    temporary_parent=ROOT/'working/oracle-temporary';temporary_parent.mkdir(parents=True,exist_ok=True)
    temporary_root=Path(tempfile.mkdtemp(prefix=Path(a.report).stem+'-',dir=temporary_parent))
    (temporary_root/'.retain').write_text('Prospectively temporary reproducible oracle planes. Hold through reviewed durable input/result/tested-executable handoff; preserve this pin. These planes are not a second durable evidence store. No automatic deletion.\n')
    reference=temporary_root/'reference';report['temporary_oracle_directory']=str(reference)
   reconstruction=[history,source,Path(acquired['session'])/'game',original,pe,reference]
   execute('original-history',reconstruction,timeout=900)
   index=json.loads((reference/'worlds.json').read_text());require(index['success'] is True and index['native_player_pixels_read'] is False and index['original_destination_capture_read'] is False,'Oracle history provenance')
   worlds={}
   for row in index['worlds']:
    g=int(row['generation']);require(g==len(worlds)+1 and Path(row['path']).name==row['path'],'Original World order/path')
    raw=inflate((reference/row['path']).read_bytes());require(len(raw)==row['width']*row['height']*2,'Original World full plane');worlds[g]=np.frombuffer(raw,dtype='<u2').astype('uint32').reshape(row['height'],row['width'])
   report['original_reference']=dict(worlds=len(worlds),source_chunks=int(index.get('source_chunks','0')),draws=index['draws'],native_player_pixels_read=False,original_destination_capture_read=False,continuous_from_owned_initial_history=True)
   require(all(digest(assets/n)==h for n,h in asset_hashes.items()),'Reference asset changed');report['asset_inputs_stable']=True
   report['comparison']=compare(observed,worlds);report['success']=report['comparison']['success']
  except Exception as e:report['error']=str(e)
  finally:
   if before:
    try:execute('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
    except Exception as e:report.update(success=False,original_verification_error=str(e))
   report['sources_stable']=all(digest(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable']
   if a.temporary_reference_frames and report['success'] and report.get('original_manifest_verified_before_after') is True and processes.stopped():
    try:
     from oracle_frame_retention import frame_manifest
     import platform,zlib
     frames=frame_manifest(reference,index)
     qt=subprocess.run(['pkg-config','--modversion','Qt6Core'],capture_output=True,text=True,check=True).stdout.strip()
     report['temporary_reference_frames']=dict(policy='successful-oracle-qz-temporary-v1',frames=frames,
        reconstruction_command=[str(v) for v in reconstruction],source_capture=artifacts[str(source.relative_to(ROOT))],
        tested_binaries=report['binaries'],tool_versions=dict(python=platform.python_version(),numpy=np.__version__,qt6_core=qt,zlib=zlib.ZLIB_RUNTIME_VERSION),
        retained_index='reference/worlds.json',directory=str(reference),physical_frames_still_held=True,reconstruction_output_policy='Use a new empty directory for the last argument, then verify every encoded and raw frame hash.',
        scope='Every generated plane was independently compared before classification. Only reproducible derived qz files are temporary; encoded/raw hashes, all original input captures, index, result and tested executables stay durable. Separate current-consumer/pin/hash review and verified durable input handoff are required before removal. No old output is reclassified.')
     retained_reference=output/'reference';retained_reference.mkdir(exist_ok=False)
     (retained_reference/'worlds.json').write_bytes((reference/'worlds.json').read_bytes())
    except Exception as error:
     report.update(success=False,error='Temporary oracle classification failed: '+str(error),temporary_classification_error=str(error));report.pop('temporary_reference_frames',None)
   if a.temporary_reference_frames and reference is not None and reference.exists() and not report.get('temporary_reference_frames'):
    # A failed/partial oracle remains durable negative evidence. Move its exact
    # files into the normal retained output; no source file is removed.
    retained_reference=output/'reference'
    if retained_reference.exists():retained_reference=output/'reference-failed'
    require(not retained_reference.exists(),'Failure reference destination already exists')
    reference.rename(retained_reference)
   report['artifacts']=[]
   for f in sorted(output.rglob('*')):
    if f.is_file():
     name=str(Path(a.report).with_suffix(''))+'-'+hashlib.sha256(str(f.relative_to(output)).encode()).hexdigest()[:20]+'.bin';data=f.read_bytes();h=write_report(ROOT,name,data);report['artifacts'].append(dict(path=str(f.relative_to(output)),record=name,sha256=h,bytes=len(data)))
   report.update(output_directory=str(output),comparison_processes_stopped=processes.stopped(),artifact_retention_complete=True,artifact_retention_scope='All durable observations/results; prospectively classified successful oracle qz files are held temporary and named separately')
   h=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps({k:report.get(k) for k in ['success','error','observations','original_reference']}|{'record':a.report,'sha256':h}),flush=True)
 from evidence_objects import settings
 if settings(ROOT) and report['comparison_processes_stopped']:
  from evidence_workspace import register_completed
  register_completed(ROOT,[output],dict(record=a.report,sha256=h),producer='comparison')
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
