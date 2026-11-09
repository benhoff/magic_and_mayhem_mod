#!/usr/bin/env python3
"""Current native Qt SFT outline, visible-caption and ownership controls."""
import argparse,hashlib,json,os,subprocess,sys,time
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,required_sources,scenario_contract
from font_native_lifetime import decode,SOURCES
from scene_effects import module
ROOT=Path(__file__).resolve().parents[1];PREFIX='research/runtime/font-dc-lifetime-results-20261108/'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();assert a.report.startswith(PREFIX)and not(ROOT/a.report).exists()
 out=ROOT/'working/font-native-controls'/Path(a.report).stem;out.mkdir(parents=True,exist_ok=False);(out/'.retain').write_text('Tested binaries, current sources and native-font log are durable evidence. Preserve.\n')
 build=ROOT/'working/build/qt-shell';reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes());b=next(b for b in reg['behaviors']if b['id']=='NP.native-menu-font-lifetime-observation');sc=next(s for s in reg['scenarios']if s['id']=='native.menu-font-lifetime-controls')
 names=required_sources(b)|set(sc['tests'])|{'tools/check-font-native-lifetime.py','tools/scene_effects.py'}|set(module('test-native-campaign').linked_compiler_sources(build,ROOT))
 sources={n:sha(ROOT/n)for n in sorted(names)};q=dict(schema=1,kind='native_menu_font_lifetime_controls',success=False,original_game_runs=0,sources=sources,commands=[],artifacts=[],claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={sc['id']:scenario_contract(sc)})],scope='Existing current native outline masks/origins/metrics/fallback and22installed menu variants at three sizes, shared ownership and tooltip role; opt-in bounded actual Qt register/paint/unregister trace plus adversarial reader controls. No original game, original UI pixel equivalence or installed acceptance.')
 def execute(label,cmd,env=None):
  with(out/(label+'.log')).open('xb')as log:r=subprocess.run([str(v)for v in cmd],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=180)
  q['commands'].append(dict(command=[str(v)for v in cmd],returncode=r.returncode));assert r.returncode==0,label
 before=False
 try:
  q['inputs']={n:sha(ROOT/'working/game-nocd'/n)for n in SOURCES}
  execute('original-before',[ROOT/'tools/original-manifest.sh','verify']);before=True
  execute('reader',[sys.executable,'-B',ROOT/'tests/test-font-native-lifetime.py','-v'])
  start_ns=time.monotonic_ns()
  execute('native',[build/'menu-font-test',ROOT/'working/game-nocd'],dict(os.environ,QT_QPA_PLATFORM='offscreen',MNM_FONT_DC_LIFETIME='1'))
  end_ns=time.monotonic_ns()
  native=decode((out/'native.log').read_bytes(),allow_fallback=True);assert native['complete']and native['peak_registered_fonts']==4 and native['paints']and native['reuse']>0
  assert all(start_ns<=int(e['host_ns'])<=end_ns for e in native['events'])
  catalogs={g['key_sha256']for g in native['generations'].values()};assert len(catalogs)==1
  q.update(success=True,reader_controls=12,native_menu_variants=22,menu_sizes=3,painted_captions=len(native['paints']),retired_generations=len(native['generations']),peak_registered_fonts=4,registered_fonts_retired=True,known_catalogs=[dict(key_sha256=g['key_sha256'],registrations=g['registrations'])for g in native['generations'].values()][:1])
 except Exception as e:q['error']=str(e)
 finally:
  if before:execute('original-after',[ROOT/'tools/original-manifest.sh','verify']);q['original_manifest_verified_before_after']=True
  q['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());q['success']&=q['sources_stable']
  keep=[*out.glob('*.log'),build/'menu-font-test',build/'mnm-qt-shell',build/'libmnm-main-menu.a',*list((ROOT/'working/tests/menu-fonts').glob('*.png'))]
  for n,h in sources.items():
   old=subprocess.run(['git','show','HEAD:'+n],cwd=ROOT,capture_output=True)
   if old.returncode or hashlib.sha256(old.stdout).hexdigest()!=h:keep.append(ROOT/n)
  for path in keep:
   if not path.is_file():continue
   raw=path.read_bytes();record=a.report.removesuffix('.json')+'-artifact-'+str(len(q['artifacts']))+'.bin';h=write_report(ROOT,record,raw);q['artifacts'].append(dict(path=str(path),record=record,sha256=h,bytes=len(raw)))
  h=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps({k:q.get(k)for k in ['success','error','reader_controls','painted_captions','retired_generations']}|dict(record=a.report,sha256=h)),flush=True)
 return int(not q['success'])
if __name__=='__main__':raise SystemExit(main())
