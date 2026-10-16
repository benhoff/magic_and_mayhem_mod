#!/usr/bin/env python3
"""Actual campaign window Cure action, save dialogs and fresh-process continuation."""
import argparse,hashlib,json,os,subprocess,tempfile
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__);p.add_argument('window',type=Path);p.add_argument('cli',type=Path);p.add_argument('fixture',type=Path);p.add_argument('--output',type=Path);a=p.parse_args();temporary=tempfile.TemporaryDirectory(prefix='cure-battle-window-') if not a.output else None;a.output=a.output.resolve() if a.output else Path(temporary.name);a.output.mkdir(parents=True,exist_ok=True);assert not any(a.output.iterdir())
commands=[]
def run(name,argv):
 argv=list(map(str,argv));r=subprocess.run(argv,capture_output=True,text=True,timeout=90,env=dict(os.environ,QT_SCALE_FACTOR='1',QT_AUTO_SCREEN_SCALE_FACTOR='0'));commands.append(argv);(a.output/(name+'.stdout')).write_text(r.stdout);(a.output/(name+'.stderr')).write_text(r.stderr)
 assert not r.returncode,(name,r.stdout[-2000:],r.stderr[-2000:]);return r.stdout
def window(name,actions,load):
 script=a.output/(name+'.json');script.write_text(json.dumps(actions,indent=2)+'\n');out=a.output/name
 run(name,['xvfb-run','-a','-s','-screen 0 1280x1200x24',a.window.resolve(),'--test-script',script,'--test-output',out,'--load-campaign',load]);assert json.loads((out/'report.json').read_text())['success'];return out/'final.mnmc'
legacy=a.output/'legacy.mnmc';run('seed-legacy',[a.fixture.resolve(),'seed-legacy-campaign',legacy]);legacy_meta=json.loads(run('inspect-legacy',[a.cli.resolve(),'inspect',legacy]));assert not legacy_meta['battle']['cures']
seed=a.output/'seed.mnmc';run('seed',[a.fixture.resolve(),'seed-campaign',seed])
cut=window('in-flight',[dict(kind='cell',actor=2),dict(kind='cell',x=4,y=5,right=True),dict(kind='cell',wizard=0),dict(kind='button',name='cureButton'),dict(kind='cell',actor=2),dict(kind='save',name='pending.mnmc'),dict(kind='steps',count=2),dict(kind='expect',cures=1,actor=2,health=9),dict(kind='save',name='moving-cure.mnmc'),dict(kind='pause')],seed)
meta=json.loads(run('inspect',[a.cli.resolve(),'inspect',cut]))['battle'];assert len(meta['cures'])==1;unit=next(u for u in meta['units'] if u['slot']==2);assert unit['position']!=[2,5,1] and meta['cures'][0]['position']==unit['position']
reference=a.output/'reference.mnmc';run('headless',[a.cli.resolve(),'resume',cut,reference,16])
resumed=window('fresh-process',[dict(kind='expect',cures=1,actor=2,health=9),dict(kind='steps',count=16),dict(kind='expect',cures=0,actor=2,health=17),dict(kind='save',name='healed.mnmc')],cut)
assert resumed.read_bytes()==reference.read_bytes()
# A separately restarted headless consumer completes exactly the pending effect.
completed=a.output/'completed.mnmc';result=json.loads(run('resume-fixture',[a.fixture.resolve(),'resume-campaign',cut,completed]));assert result['healed']==8 and result['health']==17
report=dict(success=True,scope='Real Qt campaign Cure button/friendly target/move and save dialogs; fresh-window and headless continuation share exact campaign bytes.',moving_target=True,health_before=9,health_after=17,single_heal=8,fresh_process=True,exact_campaign_bytes=True,checkpoint_sha256=hashlib.sha256(cut.read_bytes()).hexdigest(),result_sha256=hashlib.sha256(resumed.read_bytes()).hexdigest(),commands=commands)
(a.output/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
