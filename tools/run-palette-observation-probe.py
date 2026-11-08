#!/usr/bin/env python3
"""One bounded original-active semantic probe; never grants cycling rows."""
import argparse,hashlib,json,os,sqlite3,subprocess,sys,time
from collections import Counter
from pathlib import Path
from coverage_evidence import Reader,write_report
from palette_observation import decode
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/palette-cycling-results-20261108/'

def probe_command():
 return [sys.executable,str(ROOT/'tools/capture-scene-game.py'),'--canvas-producers','--palette-observation','--samples','4','--startup-queues','4','--producer-oracle-mib','512','--skip-window-screenshot','--source-game',str(ROOT/'working/game-nocd'),'--prefix-template',str(ROOT/'working/fixtures/prefix-seed'),'--admitted-prefix-template']

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--plan',required=True);p.add_argument('--report',required=True);a=p.parse_args()
 require(a.report.startswith(PREFIX)and '..'not in Path(a.report).parts and not(ROOT/a.report).exists(),'Fresh owned report required')
 out=ROOT/'working/palette-cycling'/Path(a.report).stem;out.mkdir(parents=True,exist_ok=False)
 (out/'.retain').write_text('Palette probe and every negative; no native or cycling row credit. Preserve.\n')
 def put(suffix,raw):
  name=a.report.removesuffix('.json')+'-'+suffix;return dict(record=name,sha256=write_report(ROOT,name,raw),bytes=len(raw))
 q=dict(schema=1,kind='palette_original_semantic_probe',success=False,original_game_runs=0,native_player_runs=0,closes_palette_rows=False,artifacts=[])
 initial=set((ROOT/'working/experiments/scene-observer').glob('run-*'));verified=False
 try:
  with Reader(ROOT)as reader:
   plan=reader.json(a.plan);require(plan['success']is True and plan['maximum_launch_attempts']==6,'Missing palette task plan')
   require(all(reader.digest(n)==h for n,h in plan['sources'].items()),'Palette plan source drift')
   control=reader.json(plan['controls']['record']);require(reader.digest(plan['controls']['record'])==plan['controls']['sha256']and control['success']and control['sources_stable'],'Palette control admission missing')
   require(all(reader.digest(n)==h for n,h in control['sources'].items()),'Palette production control source drift')
   q.update(plan=dict(record=a.plan,sha256=reader.digest(a.plan)),sources=plan['sources'])
  subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True);verified=True
  preflight=subprocess.run([*probe_command(),'--validate-only'],capture_output=True,timeout=10)
  q['option_preflight']=put('option-preflight.log',preflight.stdout+preflight.stderr)
  require(preflight.returncode==0 and json.loads(preflight.stdout)['validated']is True,'Palette probe option preflight failed')
  budget=ROOT/'working/palette-cycling/attempts.sqlite3'
  with sqlite3.connect(budget)as db:
   db.execute('CREATE TABLE IF NOT EXISTS attempts(id INTEGER PRIMARY KEY,report TEXT UNIQUE,kind TEXT,started_ns INTEGER,plan_sha256 TEXT)');db.execute('BEGIN IMMEDIATE')
   require(db.execute('SELECT COUNT(*) FROM attempts').fetchone()[0]<6,'Six-attempt palette task cap exhausted')
   db.execute('INSERT INTO attempts(report,kind,started_ns,plan_sha256)VALUES(?,?,?,?)',(a.report,'semantic-original-active',time.monotonic_ns(),q['plan']['sha256']));q['attempt']=db.execute('SELECT last_insert_rowid()').fetchone()[0];db.commit()
  command=['xvfb-run','-a','-s','-screen 0 1024x768x24',*probe_command()]
  q['command']=command
  with(out/'capture.log').open('xb')as log:run=subprocess.run(command,cwd=ROOT,env=dict(os.environ,LP_NUM_THREADS='2'),stdout=log,stderr=subprocess.STDOUT,timeout=240)
  q['exit_code']=run.returncode
  roots=set((ROOT/'working/experiments/scene-observer').glob('run-*'))-initial;require(len(roots)==1,'Ambiguous/missing palette probe experiment; producer exit '+str(run.returncode)+': '+(out/'capture.log').read_text(errors='replace').splitlines()[-1]);root=next(iter(roots));q['experiment']=str(root.relative_to(ROOT))
  q['original_game_runs']=int((root/'game.log').exists())
  paths=list((root/'capture').glob('palette-observation-*.bin'));require(len(paths)==1,'Palette sidecar missing/ambiguous')
  parsed=decode(paths[0].read_bytes(),complete=False)
  q['observation']=dict(complete=parsed['complete'],owner=parsed['owner'],nodes=len(parsed['nodes']),records=len(parsed['rows']),kinds=dict(Counter(r['header'][2]for r in parsed['rows'])),modes=dict(Counter(n['mode']for n in parsed['nodes'].values())),terminal=parsed['terminal'],runtime_cycling_proved=False)
  require(run.returncode==0,'Original semantic probe failed; retain and diagnose before another launch');q['success']=True
 except Exception as e:q['error']=str(e)
 finally:
  roots=set((ROOT/'working/experiments/scene-observer').glob('run-*'))-initial
  candidates=list(out.glob('*'))
  for root in roots:
   candidates.extend(f for f in root.rglob('*')if f.is_file()and 'wineprefix'not in f.relative_to(root).parts and 'game'not in f.relative_to(root).parts)
   candidates.extend(root/'game'/n for n in ('Chaos.exe','MnmScene.dll','MnmMenu.dll'))
   candidates.extend(f for f in (root/'game/CFG').rglob('*')if f.is_file()and f.name.lower()in('prefs.cfg','tables.cfg'))
   if(root/'game.log').exists():q['original_game_runs']=1
  for i,f in enumerate(sorted(set(candidates))):
   if f.is_file():q['artifacts'].append(dict(path=str(f.relative_to(ROOT)),**put('artifact-'+str(i)+'.bin',f.read_bytes())))
  if verified:
   check=subprocess.run([ROOT/'tools/original-manifest.sh','verify'],capture_output=True);q['original_manifest_before_after']=check.returncode==0;q['original_after']=put('original-after.log',check.stdout+check.stderr)
   if check.returncode:q.update(success=False,original_manifest_error=True)
  q['scope']='Bounded original-active discovery only. Passive guarded builder/selection/free observation; four startup World queues, forced private experiment stop. No native execution, normal retirement, palette-cycle requirement or full-game equivalence claim. All charged attempts and failures remain retained.'
  digest=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],error=q.get('error'),record=a.report,sha256=digest,observation=q.get('observation'))),flush=True)
 return int(not q['success'])

if __name__=='__main__':raise SystemExit(main())
