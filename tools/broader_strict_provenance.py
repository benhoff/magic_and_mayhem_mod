"""Join the strict acquisition to its predeclared exact runtime build inputs."""
import copy,hashlib,json
from pathlib import Path
from broader_player_acceptance import native_phase as original_phase,checked
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261110/'
HEADER='runtime/scene/fading_particle_admission.h'
BUILD=P+'build-current-07.json'
def merged_sources(acquired,phase,build):
 require(phase['runtime_sources']==build['runtime_sources'],'Strict phase runtime inputs differ from tested build')
 sources=dict(acquired['sources'])
 for n,h in phase['runtime_sources'].items():
  require(n not in sources or sources[n]==h,'Conflicting predeclared runtime hash');sources[n]=h
 return sources

def derive(root,reader,acquisition):
 from coverage_evidence import write_report
 from evidence_objects import configured
 root=Path(root);a=checked(reader,acquisition);p=checked(reader,a['broader_phase']);build=checked(reader,dict(record=BUILD,sha256=reader.digest(BUILD)));cid=next(c['id']for c in p['broader_cases']if c['report']==acquisition['record']);raw=original_phase(root,reader,a,p['player_binary']['sha256'])
 require(a['success']and a['sources_stable']and a['actual_start_count_complete']and a['actual_provider_verified']and a['original_game_runs']==1 and a['original_verified_after'],'Incomplete strict acquisition')
 require(raw['exact_current_binary']and raw['changed_native_sources']=={}and raw['missing_runtime_sources']==[HEADER],'Join is only for the one omitted predeclared header')
 sources=merged_sources(a,p,build);require(all(reader.digest(n)==h for n,h in sources.items()),'Current strict source drift')
 scene=build['scene'];manifest=build['scene-manifest'];sm=json.loads(reader.read_bytes(manifest['record']));require(reader.digest(scene['record'])==scene['sha256']and reader.digest(manifest['record'])==manifest['sha256'],'Retained scene build changed')
 require(sm['sources'][HEADER]==sources[HEADER]and sm['sha256']==scene['sha256'],'Scene compiler manifest does not bind header')
 pre=build['source_preimages']
 with configured(root)as pair:
  tree=json.loads(pair.read(pre['sha256'],pre['size']));item=tree['sources'][HEADER];body=pair.read(item['sha256'],item['size']);require(hashlib.sha256(body).hexdigest()==sources[HEADER]==item['sha256'],'Exact build header preimage missing')
 session=root/a['session'];literal=(session/'manifest.json').read_bytes();observed=json.loads(literal);require(observed['resident_scene_sha256']==scene['sha256'],'Actual strict runtime DLL differs from tested build')
 dll=session/'game/MnmScene.dll';require(hashlib.sha256(dll.read_bytes()).hexdigest()==scene['sha256'],'Actual staged scene DLL changed')
 n=P+cid+'-provenance-session-manifest-01.bin';mh=write_report(root,n,literal)
 virtual=dict(a,sources=sources);joined=original_phase(root,reader,virtual,p['player_binary']['sha256']);require(joined['current_native_source_candidate'],'Full joined strict runtime provenance remains incomplete')
 return dict(schema=1,kind='strict_predeclared_runtime_provenance_join',success=True,original_game_runs=0,new_native_executions=0,new_request_reservations=0,case=cid,acquisition=acquisition,phase=a['broader_phase'],build=dict(record=BUILD,sha256=reader.digest(BUILD)),raw_acquisition_provenance=raw,joined_provenance=joined,added_source_hashes={HEADER:sources[HEADER]},preexisting_phase_runtime_hash=p['runtime_sources'][HEADER],preexisting_scene_compiler_manifest=manifest,exact_source_preimage=item,source_preimage_root=pre,actual_session_manifest=dict(record=n,sha256=mh),actual_scene_sha256=scene['sha256'],physical_scene_path=str(dll),sources={n:reader.digest(n)for n in ['tools/broader_strict_provenance.py','tools/broader_player_acceptance.py']},Native_source_waivers=False,raw_acquisition_rewritten=False,scope='Only join an omitted header from the unchanged pre-run prospective phase, tested build07 compiler manifest and exact retained source preimage. Actual same-session manifest and staged DLL match that tested scene binary. Original acquisition remains immutable and incomplete when read alone; no new execution or inferred historical hash.')

def native_phase(root,reader,acquired,current_binary):
 root=Path(root);phase=checked(reader,acquired['broader_phase']);case=next(c for c in phase['broader_cases']if c['report'].endswith('-acquisition.json')and c['report']==P+c['id']+'-acquisition.json'and c['stress_seconds']==acquired['stress_seconds']and(c['profile']=='player-software')==acquired['configuration']['software_flag']);name=P+case['id']+'-provenance-join-01.json';join=checked(reader,dict(record=name,sha256=reader.digest(name)));require(join['case']==case['id']and join['phase']==acquired['broader_phase']and join['acquisition']==dict(record=case['report'],sha256=reader.digest(case['report']))and reader.json(case['report'])==acquired,'Foreign strict provenance join')
 require(join['Native_source_waivers']is False and join['raw_acquisition_rewritten']is False and join['original_game_runs']==join['new_native_executions']==0 and all(reader.digest(n)==h for n,h in join['sources'].items()),'Changed strict join source policy')
 build=checked(reader,join['build']);sources=merged_sources(acquired,phase,build);require(join['added_source_hashes']=={HEADER:sources[HEADER]}and join['preexisting_phase_runtime_hash']==sources[HEADER]and reader.digest(join['preexisting_scene_compiler_manifest']['record'])==join['preexisting_scene_compiler_manifest']['sha256'],'Unbound runtime header join')
 observed=json.loads(reader.read_bytes(join['actual_session_manifest']['record']));require(reader.digest(join['actual_session_manifest']['record'])==join['actual_session_manifest']['sha256']and observed['resident_scene_sha256']==join['actual_scene_sha256']==build['scene']['sha256'],'Actual retained scene identity changed')
 result=original_phase(root,reader,dict(acquired,sources=sources),current_binary);require(result==join['joined_provenance']and result['current_native_source_candidate'],'Joined strict source drift');result['predeclared_runtime_join']=dict(record=name,sha256=reader.digest(name));return result
