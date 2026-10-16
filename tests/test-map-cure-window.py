#!/usr/bin/env python3
"""Plains movement/Cure/save/restart/win/advance through actual Qt controls."""
import argparse,hashlib,json,os,subprocess,tempfile
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__)
for name in ('window','cli','fixture','assets'):p.add_argument(name,type=Path)
p.add_argument('--output',type=Path,required=True);a=p.parse_args();a.output=a.output.resolve();a.output.mkdir(parents=True);commands=[]
def run(name,argv):
    argv=list(map(str,argv));r=subprocess.run(argv,capture_output=True,text=True,timeout=90,env=dict(os.environ,QT_SCALE_FACTOR='1',QT_AUTO_SCREEN_SCALE_FACTOR='0'))
    commands.append(dict(argv=argv,exit_code=r.returncode));(a.output/(name+'.stdout')).write_text(r.stdout);(a.output/(name+'.stderr')).write_text(r.stderr)
    assert not r.returncode,(name,r.stdout[-2000:],r.stderr[-2000:]);return r.stdout
def cli(name,command,*args):return run(name,[a.cli,a.assets,command,*args])
def window(name,actions,load):
    script=a.output/(name+'.json');script.write_text(json.dumps(actions,indent=2)+'\n');out=a.output/name
    run(name,['xvfb-run','-a','-s','-screen 0 1280x1200x24',a.window,'--asset-root',a.assets,'--load-campaign',load,'--test-script',script,'--test-output',out])
    assert json.loads((out/'report.json').read_text())['success'];return out/'final.mnmc'
seed=a.output/'seed.mnmc';run('seed',[a.fixture,a.assets,'seed',seed])
pending_actions=[dict(kind='cell',actor=2),dict(kind='cell',x=3,y=5,z=3,right=True),dict(kind='cell',wizard=0),dict(kind='button',name='cureButton'),dict(kind='cell',actor=2),dict(kind='save',name='pending.mnmc')]
cut=window('moving-cure',pending_actions+[dict(kind='steps',count=2),dict(kind='expect',cures=1,actor=2,health=9),dict(kind='save',name='mid-ani.mnmc'),dict(kind='pause')],seed)
# Compare semantic clicks and every serialized owner, including pending commands.
move=a.output/'move.mnmc';queued=a.output/'queued.mnmc';refcut=a.output/'reference-cut.mnmc'
cli('move','move',seed,move,3,5,3);cli('cast','cast',move,queued,'cure',2)
assert queued.read_bytes()==(a.output/'moving-cure/pending.mnmc').read_bytes()
cli('cut','resume',queued,refcut,2);assert cut.read_bytes()==refcut.read_bytes()
meta=json.loads(cli('inspect-cut','inspect',cut))['battle'];unit=next(u for u in meta['units'] if u['slot']==2)
assert len(meta['cures'])==1 and unit['position']!=[2,5,3] and meta['cures'][0]['position']==unit['position']
reference=a.output/'reference-resumed.mnmc';cli('headless','resume',cut,reference,16)
resumed=window('fresh-process',[dict(kind='expect',cures=1,actor=2,health=9),dict(kind='steps',count=16),dict(kind='expect',cures=0,actor=2,health=17),dict(kind='save',name='healed.mnmc')],cut)
assert resumed.read_bytes()==reference.read_bytes()
completed=a.output/'completed.mnmc';result=json.loads(run('single-heal',[a.fixture,a.assets,'resume',cut,completed]));assert result['healed']==8 and result['health']==17
won=window('finish',[dict(kind='finish'),dict(kind='expect',transition=2,battle=False,label='Continue',continue_enabled=True),dict(kind='save',name='victory.mnmc'),dict(kind='pause')],resumed)
info=json.loads(cli('inspect-win','inspect',won));assert info['last_result']['outcome']==1 and info['region']=='arena-finale'
reopened=window('result-reopened',[dict(kind='save',name='unchanged.mnmc'),dict(kind='pause')],won);assert reopened.read_bytes()==won.read_bytes()
advanced=window('advance',[dict(kind='button',name='continueCampaignButton'),dict(kind='expect',transition=3,battle=True,cures=0),dict(kind='save',name='next.mnmc')],won)
nextref=a.output/'reference-next.mnmc';cli('headless-next','start',won,nextref);assert nextref.read_bytes()==advanced.read_bytes()
report=dict(success=True,scope='Authored Cure runtime on installed Plains crop; real Qt clicks/dialogs, exact pending and mid-ANI bytes, fresh-process single heal, victory/reopen/advance. Native policy; no original campaign or pixels equivalence.',processes=5,layers=23,single_heal=8,health_before=9,health_after=17,exact_campaign_bytes=True,checkpoint_sha256=hashlib.sha256(cut.read_bytes()).hexdigest(),commands=commands)
(a.output/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
