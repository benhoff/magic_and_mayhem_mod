#!/usr/bin/env python3
"""Pinned original spectator/single-cell light comparison and native integration."""
import argparse,hashlib,json,os,subprocess
from pathlib import Path
from coverage_claims import behavior_contract,required_sources
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--executable',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False);(out/'.retain').touch();(out/'tmp').mkdir();os.environ['TMPDIR']=str(out/'tmp')
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());builds={b['id']:b for b in register['builds']};behaviors=[b for b in register['behaviors'] if b['id'] in ['RE.terrain-spectator-camera-light','RE.terrain-single-cell-light']]
 paths={'tools/test-terrain-light-boundaries.py','tests/sprite-binary-reference.cpp','tests/terrain-static-lighting-reference.cpp','reconstruction/rendering/terrain_static_lighting.hpp','assets/palette_lighting.cpp','assets/palette_lighting.hpp','assets/config_loader.cpp','assets/persistence.hpp','assets/persistence_internal.hpp','assets/asset_file.hpp','assets/path_resolver.hpp'}
 for b in behaviors:paths.update(required_sources(b))
 sources={n:sha(ROOT/n) for n in sorted(paths)};report=dict(schema=1,success=False,sources=sources,claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,builds)) for b in behaviors],results={},unit_checks=[],original_sha256=BUILD,scope='Whole original spectator/ordinary creature refresh and explicit inherited index-one kernel/stamps on guarded owned fields; native static/cycle/parser integration. Installed entity producers and live replacement remain separate.')
 def run(name,cmd,env=None):
  r=subprocess.run([str(v) for v in cmd],cwd=ROOT,capture_output=True,text=True,env=env,timeout=180);(out/(name+'.log')).write_text(r.stdout+r.stderr);r.check_returncode();return r.stdout
 run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
 try:
  if sha(a.executable)!=BUILD:raise ValueError('Executable hash mismatch')
  model=ROOT/'reconstruction/rendering';common=['g++','-std=c++17','-I'+str(model),'-I'+str(ROOT/'apps/terrain-preview'),'-I'+str(ROOT/'assets')]
  light=model/'terrain_lighting.cpp';creature=model/'terrain_creature_lighting.cpp';static=model/'terrain_static_lighting.cpp';cycle=model/'terrain_lighting_cycle.cpp'
  sanitizer=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'];env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1')
  for name,files in [('creature',[ROOT/'tests/terrain-creature-lighting-reference.cpp',light,creature]),('single-cell',[ROOT/'tests/terrain-single-cell-lighting-reference.cpp',light]),('static',[ROOT/'tests/terrain-static-lighting-reference.cpp',light,creature,static])]:
   results=[]
   for variant,flags in [('original',['-O2','-m32','-fno-pie','-no-pie','-DORIGINAL_REFERENCE']),('native',['-O2']),('sanitized',['-O1','-g',*sanitizer])]:
    label=name+'-'+variant;binary=out/label;run(label+'-build',[*common,*flags,*files,'-o',binary]);data=out/(label+'.bin');stats=json.loads(run(label,[binary,*([a.executable.resolve()] if variant=='original' else []),data],env));results.append(dict(stats=stats,data_sha256=sha(data),binary_sha256=sha(binary)))
   if any((x['stats'],x['data_sha256'])!=(results[0]['stats'],results[0]['data_sha256']) for x in results):raise ValueError(name+' original/native/sanitized mismatch')
   report['results'][name]=dict(all_match=True,builds=results)
  for name,extra in [('terrain-creature-lighting',[]),('terrain-static-lighting',[]),('terrain-lighting-cycle',[ROOT/'apps/terrain-preview/light_fixture.cpp']),('terrain-lighting',[ROOT/'assets/palette_lighting.cpp',ROOT/'assets/config_loader.cpp','-ffunction-sections','-fdata-sections','-Wl,--gc-sections'])]:
   for variant,flags in [('native',['-O2']),('sanitized',['-O1','-g',*sanitizer])]:
    label='unit-'+name+'-'+variant;binary=out/label;run(label+'-build',[*common,*flags,ROOT/('tests/'+name+'-test.cpp'),light,creature,static,cycle,*extra,'-o',binary]);run(label,[binary],env);report['unit_checks'].append(dict(name=label,passed=True,binary_sha256=sha(binary)))
  report['success']=True
 except Exception as e:report['error']=str(e)
 finally:
  run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True;report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable'];(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=report['success'],fixtures={n:v['builds'][0]['stats'] for n,v in report['results'].items()},unit_checks=len(report['unit_checks']),error=report.get('error'))))
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
