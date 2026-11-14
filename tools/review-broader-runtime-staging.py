"""Reuse20 unchanged staged DLL proofs; rebuild only the changed scene DLL."""
import hashlib,json,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from coverage_evidence import Reader,write_report
P='research/runtime/broader-player-acceptance-results-20261110/';PAL='research/runtime/palette-cycling-results-20261108/'
source='research/runtime/surface-loss-restore-results-20261109/bp10-loss-default640-01-acquisition.json'
scene=ROOT/'working/build/scene-observer';mp=scene/'manifest.json';dll=scene/'MnmScene.dll'
def current_scene():
 if not mp.is_file()or not dll.is_file():return False
 m=json.loads(mp.read_bytes())
 return hashlib.sha256(dll.read_bytes()).hexdigest()==m['sha256']and all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in m['sources'].items())
rebuilt=False
if not current_scene():
 subprocess.run(['python3','tools/build-scene-observer.py'],check=True);rebuilt=True
assert current_scene()
with Reader(ROOT)as r:
 a=r.json(source);assert a['success']and a['private_wine_stopped']and a['prefix_pool']['returned']
 manifests=[v for v in a['artifacts']if v['path'].endswith('-build.json')];assert len(manifests)==22
 dlls={v['sha256']:v for v in a['artifacts']if v['path'].lower().endswith('.dll')};receipts=[];sources={}
 mn=PAL+'bp11-scene-build-manifest-01.json';dn=PAL+'bp11-scene-staging-01.bin'
 mr=dict(record=mn,sha256=write_report(ROOT,mn,mp.read_bytes()));dr=dict(record=dn,sha256=write_report(ROOT,dn,dll.read_bytes()))
 sm=json.loads(mp.read_bytes());assert sm['sha256']==dr['sha256']
 changed=0
 for ref in manifests:
  assert r.digest(ref['record'])==ref['sha256'];m=r.json(ref['record']);inputs=m.get('compiled_sources',m['sources'])
  bad=[n for n,h in inputs.items()if r.digest(n)!=h]
  if bad:
   assert Path(ref['path']).name=='scene-bridge-build.json'and set(bad)=={'runtime/scene/canvas_producers.c','runtime/scene/observer.c','runtime/scene/world_raster_callers.h','tools/build-scene-observer.py'}
   inputs=sm['sources'];manifest=mr;binary=dr;role='Current production scene DLL; former exact DLL/manifest remains historical';changed+=1
  else:
   assert m['sha256']in dlls;binary=dlls[m['sha256']];assert r.digest(binary['record'])==m['sha256'];manifest=ref;role='Unchanged exact staged DLL and every recorded compiler/declared input'
  assert all(r.digest(n)==h for n,h in inputs.items());sources.update(inputs)
  receipts.append(dict(historical_slot=ref,manifest=manifest,binary=binary,recorded_source_inputs=inputs,role=role))
 assert changed==2
 basis=r.json('research/runtime/surface-loss-restore-results-20261109/bp11-input-plan-01.json')
 sources.update(basis['compiler_sources']|basis['runtime_sources']|basis['oracle']['native_sources'])
 sources['tools/review-broader-runtime-staging.py']=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
 assert all(r.digest(n)==h for n,h in sources.items())
 q=dict(schema=1,kind='current_broader_selective_runtime_staging_preflight',success=True,sources_stable=True,original_game_runs=0,
  native_player_runs=0,historical_staging_basis=dict(record=source,sha256=r.digest(source)),
  manifest_count=22,unchanged_manifest_slots=20,current_scene_manifest_slots=2,scene_rebuilt_this_invocation=rebuilt,manifests=receipts,
  sources=sources,player_binary=basis['player_binary'],oracle=basis['oracle'],
  scope='Exact staging provenance only. All20unchanged helper slots retain original manifests, binary hashes and full recorded input fingerprints. Only the two historical scene slots use the newly verified current production scene DLL and its new manifest; historical bytes remain immutable. Full current native/runtime/player/oracle source maps are independent requirements. No original-game, healthy palette, complete output or global player credit.')
 name=PAL+'bp11-staging-preflight-01.json';h=write_report(ROOT,name,(json.dumps(q,indent=2)+'\n').encode())
 print(json.dumps(dict(success=True,sha256=h,unchanged_slots=20,new_scene_slots=2,original_game_runs=0)),flush=True)
