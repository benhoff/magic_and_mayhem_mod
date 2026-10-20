#!/usr/bin/env python3
"""Compare all palette effect builders with original PE32 and native64/sanitizers."""
import argparse,hashlib,json,os,subprocess
from pathlib import Path
from coverage_claims import behavior_contract,required_sources
ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--executable',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False);(out/'.retain').touch();(out/'tmp').mkdir();os.environ['TMPDIR']=str(out/'tmp')
 register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(b for b in register['behaviors'] if b['id']=='RE.palette-effect-modes');builds={b['id']:b for b in register['builds']};sources={n:sha(ROOT/n) for n in sorted(required_sources(b))};report=dict(schema=1,success=False,sources=sources,claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,builds))],scope=b['scope'],boundary='Whole unchanged PE32 builders, original CRT and RNG; byte-checked private allocation/IAT adapters, supplied ASCII profile strings and private SEH head. Native64 and ASan/UBSan are separate executions. No Windows profile service, scene/hardware or live replacement claim.')
 def run(name,cmd):
  result=subprocess.run([str(v) for v in cmd],cwd=ROOT,capture_output=True,text=True,timeout=600);(out/(name+'.log')).write_text(result.stdout+result.stderr);result.check_returncode();return result.stdout
 run('manifest-before',[ROOT/'tools/original-manifest.sh','verify'])
 try:
  if sha(a.executable)!=BUILD:raise ValueError('Pinned original executable hash mismatch')
  sources_cpp=[ROOT/'tests/palette-effects-reference.cpp',ROOT/'reconstruction/rendering/palette_effects.cpp',ROOT/'reconstruction/rendering/palette_shading.cpp',ROOT/'reconstruction/rendering/terrain_selection.cpp'];results={}
  for name,flags in [('original',['-m32','-DORIGINAL_REFERENCE','-fno-pie','-no-pie']),('native64',[]),('sanitized',['-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
   binary=out/name;run('build-'+name,['g++','-std=c++17','-O1','-g',*flags,'-I'+str(ROOT/'reconstruction/rendering'),*sources_cpp,'-o',binary]);results[name]=json.loads(run(name,[binary,*([a.executable.resolve()] if name=='original' else [])]));report['results']=results
  if not results['original']==results['native64']==results['sanitized']:raise ValueError('Original/native64/sanitized word/source hashes differ')
  for name,flags in [('unit',[]),('unit-sanitized',['-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
   binary=out/name;run('build-'+name,['g++','-std=c++17','-O1','-g','-Wall','-Wextra','-Werror',*flags,'-I'+str(ROOT/'reconstruction/rendering'),ROOT/'tests/palette-effects-test.cpp',*sources_cpp[1:],'-o',binary]);run(name,[binary])
  report['native_and_sanitized_units_passed']=True;report['original_sha256']=BUILD;report['success']=True
 except Exception as e:report['error']=str(e)
 finally:
  run('manifest-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True;report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());report['success'] &= report['sources_stable'];(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=report['success'],results=report.get('results'),error=report.get('error'))))
 return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
