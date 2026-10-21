#!/usr/bin/env python3
"""Compare owned linked terrain rotation/frame/geometry helpers with pinned original PE32."""
import argparse,hashlib,json,os,subprocess
from pathlib import Path
from coverage_claims import behavior_contract,required_sources
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--executable',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False);(out/'.retain').touch();(out/'tmp').mkdir();os.environ['TMPDIR']=str(out/'tmp')
 r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());behaviors=[b for b in r['behaviors'] if b['id'] in ['RE.terrain-linked-section-copy']];b=behaviors[0];builds={b['id']:b for b in r['builds']};paths=set().union(*(required_sources(v) for v in behaviors));sources={n:sha(ROOT/n) for n in sorted(paths)}
 report=dict(schema=1,success=False,sources=sources,claims=[dict(behavior=v['id'],contract_sha256=behavior_contract(v,builds)) for v in behaviors],scope='; '.join(v['scope'] for v in behaviors),boundary='Whole unchanged PE32 section transform and real linked rotation/first-display helpers versus native32 on admitted nonreset linked/ordinary mixtures. Original temporary source definition mutation is checked; native assets stay immutable. Asset-stage reset, aliases and bounds run in independent native64/ASan-UBSan units; no original reset body, installed/full-scene/live claim.')
 def run(name,cmd):
  result=subprocess.run([str(v) for v in cmd],cwd=ROOT,capture_output=True,text=True,timeout=180);(out/(name+'.log')).write_text(result.stdout+result.stderr);result.check_returncode();return result.stdout
 run('manifest-before',[ROOT/'tools/original-manifest.sh','verify'])
 try:
  if sha(a.executable)!=BUILD:raise ValueError('Original image hash mismatch')
  binary=out/'reference';run('build',['g++','-m32','-std=c++17','-O2','-g','-fno-pie','-no-pie','-I'+str(ROOT/'reconstruction/rendering'),ROOT/'tests/terrain-entity-sections-reference.cpp',ROOT/'reconstruction/rendering/terrain_camera.cpp',ROOT/'reconstruction/rendering/terrain_entity_rotation.cpp',ROOT/'reconstruction/rendering/terrain_entity_sections.cpp',ROOT/'reconstruction/rendering/terrain_sections.cpp','-o',binary]);report['result']=json.loads(run('original-native',[binary,a.executable.resolve()]));report['original_sha256']=BUILD;report['binary_sha256']=sha(binary)
  for suffix,flags in [('native',[]),('sanitized',['-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
   binary=out/('unit-'+suffix);run('build-'+suffix,['g++','-std=c++17','-O1','-g',*flags,'-Wall','-Wextra','-Werror','-I'+str(ROOT/'reconstruction/rendering'),ROOT/'tests/terrain-entity-sections-test.cpp',ROOT/'reconstruction/rendering/terrain_camera.cpp',ROOT/'reconstruction/rendering/terrain_entity_rotation.cpp',ROOT/'reconstruction/rendering/terrain_entity_sections.cpp',ROOT/'reconstruction/rendering/terrain_sections.cpp','-o',binary]);run('unit-'+suffix,[binary])
  report['native_and_sanitized_units_passed']=True;report['success']=report['result']['success']
 except Exception as e:report['error']=str(e)
 finally:
  run('manifest-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True;report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable'];(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=report['success'],result=report.get('result'),error=report.get('error'))))
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
