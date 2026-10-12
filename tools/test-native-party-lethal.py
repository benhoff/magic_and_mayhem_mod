#!/usr/bin/env python3
"""Source-bound same-party lethal oracle controls, never retrospective gameplay success."""
import hashlib,json,subprocess,tempfile
from pathlib import Path
from campaign_gameplay import read_rows,damage_rows
from campaign_spell import read_spell_rows,player_engaged_fireball_lethal
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 parent=ROOT/'working/tests/native-party-lethal';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print('Party lethal controls:',out,flush=True)
 subprocess.run(['python3',str(ROOT/'tools/draft-coverage-claims.py'),'--behavior','HOST.native-campaign-party-lethal-oracle','--scenario','HOST.native-campaign-party-lethal-oracle.fixture','--output',str(out/'claims.json')],cwd=ROOT,check=True)
 d=json.loads((out/'claims.json').read_text());r=dict(success=False,claims=d['claims'],sources=d['sources'],scope=__doc__)
 try:
  with (out/'controls.log').open('x') as log:
   for script in ['tests/campaign-spell-test.py','tests/test-native-campaign.py']:subprocess.run(['python3',str(ROOT/script)],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True)
  experiment=ROOT/'working/experiments/menu-observer/run-ucokwheb';game=experiment/'gameplay-events.bin';spells=experiment/'spell-events.bin';rows=read_rows(game,complete=True);sr=read_spell_rows(spells,complete=True);hits=damage_rows(rows,0,[1,3],0);lethal=player_engaged_fireball_lethal(sr,rows,0,0,hits)
  assert lethal and lethal[0][4:9]==(2,10,2,11,0)
  r.update(controls_passed=True,historical_diagnosis=dict(gameplay=str(game),gameplay_sha256=sha(game),spell=str(spells),spell_sha256=sha(spells),lethal_event=list(lethal[0]),scope='Historical trace proves the scoped enemy effect; original failed gameplay/Quit result remains negative.'),sources_unchanged=all(sha(ROOT/p)==h for p,h in d['sources'].items()));r['success']=r['sources_unchanged']
 except (OSError,AssertionError,ValueError,subprocess.SubprocessError) as e:r['error']=str(e)
 (out/'report.json').write_text(json.dumps(r,indent=2)+'\n');print('Report:',out/'report.json',flush=True);return 0 if r['success'] else 1
if __name__=='__main__':raise SystemExit(main())
