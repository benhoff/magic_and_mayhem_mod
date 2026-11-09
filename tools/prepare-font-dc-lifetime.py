#!/usr/bin/env python3
"""Freeze a bounded installed font/DC pilot before any original-game launch."""
import argparse,ast,hashlib,json,subprocess,shlex
from pathlib import Path
from coverage_evidence import Reader,write_report
from scene_effects import module
from rendering_build_binding import native_runtime_source_paths
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];PREFIX='research/runtime/font-dc-lifetime-results-20261108/'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);p.add_argument('--controls',required=True);p.add_argument('--repair-of');p.add_argument('--previous-plan');p.add_argument('--dc-controls',required=True);p.add_argument('--failure-retirement');p.add_argument('--budget-amendment',required=True);p.add_argument('--hidden-controls',required=True);p.add_argument('--native-font-controls',required=True);p.add_argument('--incomplete-pilot');p.add_argument('--incomplete-comparison');a=p.parse_args();require(a.report.startswith(PREFIX)and not(ROOT/a.report).exists(),'Fresh owned plan required')
 build=(ROOT/'working/build/qt-shell').resolve();linked=set(module('test-native-campaign').linked_compiler_sources(build,ROOT))
 tree=ast.parse((ROOT/'tools/run-font-dc-lifetime.py').read_text());assignment=next(n for n in ast.walk(tree)if isinstance(n,ast.Assign)and any(isinstance(t,ast.Name)and t.id=='names'for t in n.targets))
 names=eval(compile(ast.Expression(assignment.value),'<collector-source-domain>','eval'),dict(linked=linked,native_runtime_source_paths=native_runtime_source_paths,ROOT=ROOT))
 oracle=set()
 for dep in [ROOT/'working/original-reference.d',*build.glob('CMakeFiles/mnm-acceptance-world-history.dir/**/*.o.d')]:
  for item in shlex.split(dep.read_text().replace('\\\n',' ')):
   path=Path(item).resolve()
   if path.is_file()and path.is_relative_to(ROOT)and not path.is_relative_to(ROOT/'working'):oracle.add(path.relative_to(ROOT).as_posix())
 require({'tests/acceptance-world-reference.cpp','tests/acceptance-world-history.cpp'}<=oracle,'Independent oracle compiler closure missing')
 names|=oracle
 names|={'tools/check_inputs.py','tools/native_music_launch.py','tools/check-installed-glyph-kernels.py','tools/prepare-font-dc-lifetime.py','tools/font_lifetime_acceptance.py','tools/palette_cycle_journey.py','tools/resident_capture.py'}
 with Reader(ROOT)as r:
  control=r.json(a.controls);require(control['success']and control['sources_stable']and all(r.digest(n)==h for n,h in control['sources'].items()),'Current production observer controls required')
  report=dict(schema=1,kind='font_dc_lifetime_prospective_plan',success=True,maximum_launch_attempts=None,maximum_actual_game_starts=None,domains=[dict(software=s,size=list(z))for s in(False,True)for z in((640,480),(800,600))],pilot=dict(software=False,size=[640,480]),stop_after_failed_acquisition_or_comparison=True,case_deadline_seconds=600,pilot_review_required_before_expansion=True,automatic_repairs_allowed=False,sources={n:sha(ROOT/n)for n in sorted(names)},controls=dict(record=a.controls,sha256=r.digest(a.controls)),player_binary=dict(path='working/build/qt-shell/mnm-qt-shell',sha256=sha(build/'mnm-qt-shell')),source_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),retention='Exact tested sources/binary, inputs, all positive and negative reports, semantic font/DC trace, lossless complete native viewport and independent World source history; no old task budget is reopened.',installation_executable_sha256=sha(ROOT/'working/game-nocd/Chaos.exe'),prefix_seed=str((ROOT/'working/fixtures/prefix-seed').resolve()),original_game_runs=0)
 from check_inputs import paths
 from native_music_launch import catalog_inputs
 music=paths(ROOT).get('assets');require(music is not None,'Verified prepared music catalog required before freezing')
 report['music_inputs'],report['music_toc']=catalog_inputs(music)
 with Reader(ROOT)as d:
  amendment=d.json(a.budget_amendment);require(amendment['success']and amendment['authorized']and (amendment['maximum_launch_attempts'],amendment['maximum_actual_game_starts'])in((9,7),(10,8)),'Explicit user-approved prospective amendment required');report.update(maximum_launch_attempts=amendment['maximum_launch_attempts'],maximum_actual_game_starts=amendment['maximum_actual_game_starts']);report['budget_amendment']=dict(record=a.budget_amendment,sha256=d.digest(a.budget_amendment))
  hidden=d.json(a.hidden_controls);require(hidden['success']and hidden['sources_stable']and all(d.digest(n)==h for n,h in hidden['sources'].items()),'Current controlled offscreen-copy policy required');report['hidden_controls']=dict(record=a.hidden_controls,sha256=d.digest(a.hidden_controls))
 with Reader(ROOT)as d:
  dc=d.json(a.dc_controls);require(dc['success']and dc['sources_stable']and all(d.digest(n)==h for n,h in dc['sources'].items()),'Current passive DC controls required');report['dc_controls']=dict(record=a.dc_controls,sha256=d.digest(a.dc_controls))
 with Reader(ROOT)as d:
  native=d.json(a.native_font_controls);require(native['success']and native['sources_stable']and all(d.digest(n)==h for n,h in native['sources'].items()),'Current native Qt font controls required');report['native_font_controls']=dict(record=a.native_font_controls,sha256=d.digest(a.native_font_controls))
 if a.incomplete_pilot:
  require(not a.repair_of and a.incomplete_comparison,'Explicit incomplete-pilot boundary review required')
  with Reader(ROOT)as d:
   pilot=d.json(a.incomplete_pilot);comparison=d.json(a.incomplete_comparison)
   require(pilot['success']and pilot['sources_stable']and pilot['private_wine_stopped']and pilot['original_game_runs']==1,'Incomplete pilot must preserve its successful stopped acquisition')
   require(comparison['acquisition']==dict(record=a.incomplete_pilot,sha256=d.digest(a.incomplete_pilot))and not comparison['success']and comparison['error']=='Required actual presented World has no independently verified font contribution'and comparison['independent_output_verified'],'Only retained wrong-provider comparison is reviewed')
   require('player_pid'not in pilot,'This review cannot replace an already observed native Qt font pilot')
   parent=d.json(pilot['input_plan']['record']);report['reviewed_failures']=parent['reviewed_failures'];report['reviewed_incomplete_pilots']=[a.incomplete_pilot]
   report['incomplete_pilot_review']=dict(acquisition=dict(record=a.incomplete_pilot,sha256=d.digest(a.incomplete_pilot)),comparison=dict(record=a.incomplete_comparison,sha256=d.digest(a.incomplete_comparison)),reason='Successful original-font/DC acquisition and normal whole-World comparison remain historical. Exact oracle-only glyph omission changed zero World planes; hidden original-menu text does not prove visible native Qt fonts. A fresh pilot must bind actual native registration, caption paints before/after reload, cache reuse and retirement to source-matched offscreen controls. Preserve every failed request, prior caps and the conservatively reserved unknown start. No acceptance credit from this boundary review.')
 if a.repair_of:
  with Reader(ROOT)as d:
   failure=d.json(a.repair_of)
   admitted=failure['admitted_original_sessions']
   require(not failure['success']and failure['sources_stable'],'Fresh retained failed source phase required')
   if admitted:
    require(admitted==failure['original_game_runs']==1,'Only one preserved failed original start is reviewed')
    if failure['error']=='Original inventory lacks unique Iron Skin recipe':
     require(a.failure_retirement,'Independent failed-prefix retirement receipt required');retired=d.json(a.failure_retirement)
     require(retired['success']and retired['private_wine_stopped']and retired['pool_recovered']and retired['acquisition']==dict(record=a.repair_of,sha256=d.digest(a.repair_of)),'Failed prefix retirement not verified')
     report['failure_retirement']=dict(record=a.failure_retirement,sha256=d.digest(a.failure_retirement))
    else:
     require(failure['error']=='Bounded attachment observation deadline'and failure['private_wine_stopped']and len(failure['font_lifetime_route']['worlds'])==1 and len(failure['font_lifetime_route']['reloads'])==2,'Only retained post-reload hidden-copy failure is reviewed')
   else:require(failure.get('private_wine_stopped')or failure.get('original_game_runs')==failure.get('charged_original_games')==0,'Unadmitted setup did not stop')
   report['repair_of']=dict(record=a.repair_of,sha256=d.digest(a.repair_of));parent=d.json(failure.get('input_plan',{}).get('record')or a.previous_plan);report['reviewed_failures']=list(dict.fromkeys(parent.get('reviewed_failures',[])+[a.repair_of]))
   report['driver_repair']='Preserve the originally offered Spellbox inventory/draft and press its public OK; no spell recipe is required or selected, and no human cast is requested. Add the separately controlled explicit passive nominal font/DC journal gate; it changes metadata initialization only, not pixels, requests or leases.'
   report['repair_reason']='Bind missing verified music input and preflight its complete catalog before opening the player; retain the independently observed prelaunch store-wait cancellation and use an isolated verified evidence pair. Preserve raw unknown game-start field; conservatively reserve one actual-start slot for the prior charged launch. A third zero-game invocation refused because the prior plan writer had not finished; preserve that report and require completed plans before dependent calls. No renderer, font observer, source admission, original pixels, gameplay or deadline change. The user explicitly amended only future capacity to9requests/7reserved starts; preserve every historical8/6plan, result and unknown count. Imported exact f369afac0hidden-copy guard passed fresh bounded controls; version2 passive glyph records add exact World/source identities without altering rendering.'
 report['oracle_sources']={n:sha(ROOT/n)for n in sorted(oracle)}
 report['oracle_binaries']=[]
 for path in ['working/original-reference','working/build/qt-shell/mnm-acceptance-world-history','working/build/qt-shell/mnm-qt-shell']:
  raw=(ROOT/path).read_bytes();record=a.report.removesuffix('.json')+'-'+Path(path).name+'.bin';h=write_report(ROOT,record,raw);report['oracle_binaries'].append(dict(path=path,record=record,sha256=h))
 require(report['installation_executable_sha256']=='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168','Private executable differs')
 report['source_snapshot']=dict(revision=report['source_revision'],unique_changes=[])
 for name,expected in report['sources'].items():
  old=subprocess.run(['git','show',report['source_revision']+':'+name],cwd=ROOT,capture_output=True)
  if old.returncode==0 and hashlib.sha256(old.stdout).hexdigest()==expected:continue
  raw=(ROOT/name).read_bytes();require(hashlib.sha256(raw).hexdigest()==expected,'Source drift before freeze: '+name);record=a.report.removesuffix('.json')+'-source-'+hashlib.sha256(name.encode()).hexdigest()[:20]+'.bin';h=write_report(ROOT,record,raw);report['source_snapshot']['unique_changes'].append(dict(path=name,record=record,sha256=h))
 print(write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode()),flush=True)
if __name__=='__main__':main()
