#!/usr/bin/env python3
"""Source-bound native Cure/combat/campaign/window integration validation."""
import argparse,hashlib,json,os,re,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--claims',type=Path,required=True);p.add_argument('--assets',type=Path,default=ROOT/'working/game-nocd');a=p.parse_args();a.output=a.output.resolve();a.output.mkdir(parents=True,exist_ok=False);a.assets=a.assets.resolve()
 declaration=json.loads(a.claims.read_text());sources=declaration['sources'];assert all(sha(ROOT/k)==v for k,v in sources.items())
 report=dict(compiler=subprocess.check_output(['g++','--version'],text=True).splitlines()[0],qt_version=subprocess.check_output(['pkg-config','--modversion','Qt6Core'],text=True).strip(),success=False,scope='Native authored arena Cure with shared authoritative health/mana, real movement/combat/effect transaction, generation-safe removal/reenrollment and self-contained campaign/window continuation. Default declared ANI fixture plus installed recipe74 execution; no original balance/cadence/full dispatcher/SPR/physical audio/live replacement claim.',claims=declaration['claims'],source_sha256=sources,commands=[])
 def run(name,argv,env=None):
  report['commands'].append(list(map(str,argv)));r=subprocess.run(list(map(str,argv)),cwd=ROOT,env=env,capture_output=True,text=True,timeout=360);(a.output/(name+'.stdout')).write_text(r.stdout);(a.output/(name+'.stderr')).write_text(r.stderr)
  if r.returncode or 'runtime error:' in r.stderr or 'ERROR: AddressSanitizer' in r.stderr:raise RuntimeError(name+':'+r.stdout[-2000:]+r.stderr[-2000:])
  return r.stdout
 run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
 report['installed_input_sha256']={}
 try:
  inputs=[a.assets/'CFG/Encrypted/effectani.cfg']+sorted(p for p in (a.assets/'Sprites').iterdir() if re.fullmatch(r'effects\d+\.ani',p.name,re.I));assert len(inputs)==14;report['installed_input_sha256']={p.relative_to(a.assets).as_posix():sha(p) for p in inputs}
  for name,flags in [('normal',[]),('sanitized',['-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie','-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
   build=a.output/name;run(name+'-configure',['cmake','-S','apps/skirmish','-B',build,'-DCMAKE_BUILD_TYPE=Debug',*flags]);targets=[] if name=='normal' else ['--target','cure-battle-test','battle-effects-test','skirmish-test','native-health-rules-test','campaign-arena-test'];run(name+'-build',['cmake','--build',build,'-j4',*targets])
   env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1');result=json.loads(run(name+'-contract',[build/'cure-battle-test'],env));assert result['success'];report[name]=result
   run(name+'-ctest',['ctest','--test-dir',build,'--output-on-failure',*(['-R','native-(battle-effects|cure-battle|skirmish-contract|health-rules-contract|campaign-arena-contract)$'] if name=='sanitized' else [])],env)
   report[name+'_installed']=json.loads(run(name+'-installed',[build/'cure-battle-test','installed',a.assets],env));assert report[name+'_installed']['success']
  assert report['normal']==report['sanitized'];run('window-journey',['python3','tests/test-cure-battle.py',a.output/'normal/campaign-window-test',a.output/'normal/mnm-campaign-cli',a.output/'normal/cure-battle-test','--output',a.output/'window-journey']);report['window']=json.loads((a.output/'window-journey/report.json').read_text());assert report['window']['success'];report['success']=True
 finally:
  run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True;report['sources_unchanged']=all(sha(ROOT/k)==v for k,v in sources.items());report['installed_inputs_unchanged']=all(sha(a.assets/k)==v for k,v in report['installed_input_sha256'].items());(a.output/'report.json').write_text(json.dumps(report,indent=2)+'\n')
 assert report['sources_unchanged'] and report['installed_inputs_unchanged'];print(json.dumps(dict(success=True,normal=True,sanitized=True,window=True,installed=True)))
if __name__=='__main__':main()
