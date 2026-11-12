#!/usr/bin/env python3
"""Compare retained real builder RGB/globals against the native table model."""
import argparse,hashlib,json,os,shlex,subprocess,sys
from pathlib import Path
from coverage_evidence import Reader,write_report
from palette_observation import decode,decode_profiles
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/palette-cycling-results-20261108/'
PIN='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--acquisition',required=True);p.add_argument('--report',required=True);p.add_argument('--authored-control',type=lambda s:int(s,0),help='Historical v1 diagnostic only; never actual observed context or row credit');a=p.parse_args()
 require(a.report.startswith(PREFIX)and not(ROOT/a.report).exists(),'Fresh owned result required')
 out=ROOT/'working/palette-cycling'/Path(a.report).stem;out.mkdir(parents=True,exist_ok=False)
 (out/'.retain').write_text('Retain exact tested binary, original input, logs and source. Compiler object/dependency intermediates are temporary after reviewed reconstruction retention.\n')
 def put(suffix,raw):
  n=a.report.removesuffix('.json')+'-'+suffix;return dict(record=n,sha256=write_report(ROOT,n,raw))
 q=dict(schema=1,kind='captured_palette_input_native_comparison',success=False,original_game_runs=0,original_oracle_runs=0,native_player_runs=0,closes_palette_rows=False,commands=[])
 names={n.relative_to(ROOT).as_posix()for directory in('reconstruction','assets')for n in(ROOT/directory).rglob('*')if n.is_file()and n.suffix in('.cpp','.c','.hpp','.h','.inc')}
 names|={'tests/palette-runtime-input-reference.cpp','tools/check-palette-runtime-input.py','tools/palette_observation.py','tools/scene_actor_observation.py','tools/rendering_pilot.py','tools/original-manifest.sh'}
 with Reader(ROOT)as reader:
  prospective={n:reader.digest(n)for n in names};acquired=reader.json(a.acquisition);q['acquisition']=dict(record=a.acquisition,sha256=reader.digest(a.acquisition))
  rows=[r for r in acquired['artifacts']if r['path'].endswith('/palette-observation-0001.bin')];require(len(rows)==1,'Palette input identity missing/ambiguous');source=rows[0];raw=reader.read_bytes(source['record']);require(hashlib.sha256(raw).hexdigest()==source['sha256'],'Changed original observation bytes')
  semantic=acquired['kind']=='palette_original_semantic_probe'
  parsed=decode(raw,complete=not semantic);q['original_input']=source;q['observation_complete']=parsed['complete'];q['observed_nodes']=len(parsed['nodes'])
  q['floating_context_observed']=parsed['version']>=2
  profile_rows=[v for v in acquired['artifacts']if v['path'].endswith('/palette-profiles-0001.bin')];require(len(profile_rows)<=1,'Ambiguous actual profile transcript');profile_raw=None
  if profile_rows:
   profile_ref=profile_rows[0];profile_raw=reader.read_bytes(profile_ref['record']);require(hashlib.sha256(profile_raw).hexdigest()==profile_ref['sha256'],'Changed actual profile transcript');profile=decode_profiles(profile_raw,parsed,complete=not semantic);q['profile_input']=profile_ref;q['profile_queries']=len(profile['rows']);q['profile_inputs_complete']=profile['complete']
  if semantic:
   manifests=[r for r in acquired['artifacts']if r['path']==acquired['experiment']+'/manifest.json'];require(len(manifests)==1,'Original experiment manifest missing');manifest=reader.json(manifests[0]['record']);require(manifest['source_sha256']==PIN,'Original source build differs');q['original_build_binding']=manifests[0]
  else:require(acquired['success']and acquired['sources_stable']and acquired['original_manifest_verified_before_after'],'Incomplete original/native acquisition')
 q['original_sha256']=PIN
 q['source_input']=put('native-model-source.bin',(ROOT/'tests/palette-runtime-input-reference.cpp').read_bytes());q['prospective_sources']=put('prospective-source-map.bin',(json.dumps(prospective,sort_keys=True)+'\n').encode())
 (out/'observed.bin').write_bytes(raw)
 if profile_raw is not None:(out/'profiles.bin').write_bytes(profile_raw)
 def run(label,command):
  q['commands'].append([str(v)for v in command]);result=subprocess.run(command,cwd=ROOT,capture_output=True,timeout=120,env=dict(os.environ,LP_NUM_THREADS='2'))
  log=result.stdout+result.stderr;(out/(label+'.log')).write_bytes(log);q.setdefault('logs',{})[label]=put(label+'.log',log);require(result.returncode==0,label+' failed; retained exact source and diagnostics');return result.stdout
 verified=False
 try:
  run('original-before',[ROOT/'tools/original-manifest.sh','verify']);verified=True
  units=('tests/palette-runtime-input-reference.cpp','reconstruction/rendering/palette_effects.cpp','reconstruction/rendering/palette_shading.cpp','reconstruction/rendering/terrain_selection.cpp')
  for i,n in enumerate(units):run('compile-'+str(i),['c++','-std=c++17','-O2','-Wall','-Wextra','-Werror','-ffunction-sections','-fdata-sections','-MMD','-MF',str(out/(str(i)+'.d')),'-c',str(ROOT/n),'-o',str(out/(str(i)+'.o'))])
  run('link',['c++','-Wl,--gc-sections',*[str(out/(str(i)+'.o'))for i in range(len(units))],'-o',str(out/'native-reference')])
  compiled=set()
  for i in range(len(units)):
   for token in shlex.split((out/(str(i)+'.d')).read_text().replace('\\\n',' ').split(':',1)[1]):
    path=Path(token).resolve()
    if path.is_relative_to(ROOT):compiled.add(path.relative_to(ROOT).as_posix())
  require(compiled<=prospective.keys(),'Undeclared compiler source')
  q['compiler_sources']={n:prospective[n]for n in sorted(compiled)};q['sources']={n:prospective[n]for n in sorted(compiled|{'tools/check-palette-runtime-input.py','tools/palette_observation.py','tools/scene_actor_observation.py','tools/rendering_pilot.py','tools/original-manifest.sh'})}
  q['binary']=put('native-reference.bin',(out/'native-reference').read_bytes());command=[out/'native-reference',out/'observed.bin']
  if a.authored_control is not None:command.append(str(a.authored_control));q['authored_control_word']=a.authored_control;q['observed_floating_context_proved']=False
  elif profile_raw is not None:command+=['--profiles',str(out/'profiles.bin')]
  result=run('comparison',command);q['comparison']=json.loads(result);require(q['comparison']['success']and not q['comparison']['original_tables_used_as_model_input'],'Native table comparison failed')
  q['success']=True
 except Exception as e:q['error']=str(e)
 finally:
  if(out/'native-reference').exists()and'binary'not in q:q['binary']=put('native-reference.bin',(out/'native-reference').read_bytes())
  if verified:
   try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);q['original_manifest_before_after']=True
   except Exception as e:q.update(success=False,original_manifest_error=str(e))
  with Reader(ROOT)as reader:q['sources_stable']=all(reader.digest(n)==h for n,h in prospective.items())
  q['success'] &= q['sources_stable'];q['scope']='Independent native numerical reconstruction from observed real original RGB/count/format/lighting inputs, with directly observed v2 floating context when present. V1 requires explicitly authored diagnostic context and cannot prove actual runtime state. Modes4/5/7 are compared only with their bound actual platform-query transcript; external-service text is a declared supplied input, not a profile-service implementation claim. Missing transcripts remain skipped. No original palette table/output colour feeds native computation. No new game, visible native presentation, complete forced-stop retirement, cycling row, default enablement or whole-player equivalence claim.'
  digest=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),record=a.report,sha256=digest,comparison=q.get('comparison'))),flush=True)
 return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
