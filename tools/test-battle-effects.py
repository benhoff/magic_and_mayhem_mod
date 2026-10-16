#!/usr/bin/env python3
"""Source-bound shared battle section and runtime integration validation."""
import argparse,hashlib,json,os,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--claims',type=Path,required=True);a=p.parse_args();a.output=a.output.resolve();a.output.mkdir(parents=True,exist_ok=False)
 declaration=json.loads(a.claims.read_text());sources=declaration['sources'];assert all(sha(ROOT/k)==v for k,v in sources.items())
 report=dict(success=False,scope='Shared native battle sections, explicit runtime binding and transactional combat/movement/effects ownership with exact legacy continuation. Synthetic runtime only; no Cure implementation or original equivalence.',claims=declaration['claims'],source_sha256=sources,commands=[])
 def run(name,argv,env=None):
  report['commands'].append(list(map(str,argv)));r=subprocess.run(list(map(str,argv)),cwd=ROOT,env=env,capture_output=True,text=True,timeout=300);(a.output/(name+'.stdout')).write_text(r.stdout);(a.output/(name+'.stderr')).write_text(r.stderr)
  if r.returncode:raise RuntimeError(name+':'+r.stdout[-1500:]+r.stderr[-2000:])
  return r.stdout
 try:
  for name,flags in [('normal',[]),('sanitized',['-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie','-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
   build=a.output/name;run(name+'-configure',['cmake','-S','apps/skirmish','-B',build,'-DCMAKE_BUILD_TYPE=Debug',*flags]);targets=[] if name=='normal' else ['--target','battle-effects-test','skirmish-test','native-health-rules-test','campaign-arena-test'];run(name+'-build',['cmake','--build',build,'-j4',*targets])
   env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
   result=json.loads(run(name+'-contract',[build/'battle-effects-test'],env));assert result['success'];report[name]=result
   run(name+'-ctest',['ctest','--test-dir',build,'--output-on-failure',*(['-R','native-(battle-effects|skirmish-contract|health-rules-contract|campaign-arena-contract)$'] if name=='sanitized' else [])],env)
  assert report['normal']==report['sanitized'];report['success']=True
 finally:
  report['sources_unchanged']=all(sha(ROOT/k)==v for k,v in sources.items());(a.output/'report.json').write_text(json.dumps(report,indent=2)+'\n')
 assert report['sources_unchanged'];print(json.dumps(dict(success=True,normal=True,sanitized=True)))
if __name__=='__main__':main()
