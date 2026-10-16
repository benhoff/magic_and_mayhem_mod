#!/usr/bin/env python3
"""Actual Qt map Sphere/Cure controls, dialogs and independent restart/advance."""
import argparse,hashlib,json,os,subprocess
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__)
for name in ('window','cli','fixture','assets'):p.add_argument(name,type=Path)
p.add_argument('--output',type=Path,required=True);a=p.parse_args();a.output=a.output.resolve();a.output.mkdir(parents=True);commands=[]
def run(name,argv):
    argv=list(map(str,argv));r=subprocess.run(argv,capture_output=True,text=True,timeout=100,env=dict(os.environ,QT_SCALE_FACTOR='1',QT_AUTO_SCREEN_SCALE_FACTOR='0'))
    commands.append(dict(argv=argv,exit_code=r.returncode));(a.output/(name+'.stdout')).write_text(r.stdout);(a.output/(name+'.stderr')).write_text(r.stderr)
    assert not r.returncode,(name,r.stdout[-2000:],r.stderr[-2000:]);return r.stdout
def cli(name,command,*args):return run(name,[a.cli,a.assets,command,*args])
def window(name,actions,load):
    script=a.output/(name+'.json');script.write_text(json.dumps(actions,indent=2)+'\n');out=a.output/name
    run(name,['xvfb-run','-a','-s','-screen 0 1280x1200x24',a.window,'--asset-root',a.assets,'--load-campaign',load,'--test-script',script,'--test-output',out]);assert json.loads((out/'report.json').read_text())['success'];return out/'final.mnmc'
seed=a.output/'seed.mnmc';run('seed',[a.fixture,a.assets,'seed',seed])
cut=window('in-flight',[dict(kind='expect',sphere_enabled=True,cure_enabled=True),dict(kind='cell',actor=3),dict(kind='cell',x=1,y=4,z=3,right=True),dict(kind='cell',wizard=0),dict(kind='button',name='sphereButton'),dict(kind='cell',actor=2),dict(kind='button',name='cureButton'),dict(kind='cell',actor=3),dict(kind='save',name='pending.mnmc'),dict(kind='steps',count=1),dict(kind='expect',spheres=1,cures=1,actor=3,health=9),dict(kind='save',name='mid-flight.mnmc'),dict(kind='pause')],seed)
move=a.output/'move.mnmc';sphere=a.output/'sphere.mnmc';queued=a.output/'queued.mnmc';reference_cut=a.output/'reference-cut.mnmc'
cli('move','move',seed,move,1,4,3);cli('sphere','cast',move,sphere,'sphere',2);cli('cure','cast',sphere,queued,'cure',3)
assert queued.read_bytes()==(a.output/'in-flight/pending.mnmc').read_bytes()
cli('cut','resume',queued,reference_cut,1);assert cut.read_bytes()==reference_cut.read_bytes()
info=json.loads(cli('inspect-cut','inspect',cut))['battle'];assert len(info['spheres'])==len(info['cures'])==1 and info['wizards'][0]['mana']==9
reference=a.output/'reference-resumed.mnmc';cli('headless','resume',cut,reference,16)
resumed=window('fresh-process',[dict(kind='expect',spheres=1,cures=1),dict(kind='steps',count=16),dict(kind='expect',spheres=0,cures=0,actor=2,health=0),dict(kind='expect',actor=3,health=17),dict(kind='save',name='single-hit-heal.mnmc')],cut)
assert resumed.read_bytes()==reference.read_bytes()
completed=a.output/'completed.mnmc';result=json.loads(run('single-hit-heal',[a.fixture,a.assets,'resume',cut,completed]));assert result['hits']==1 and result['damage']==20 and result['healed']==8;assert completed.read_bytes()==reference.read_bytes()
forged=a.output/'forged-environment.mnmc';run('forge-environment',[a.fixture,a.assets,'forge-environment',seed,forged])
profile=a.output/'forged-profile.mnmc';run('forge-profile',[a.fixture,a.assets,'forge-profile',seed,profile])
corrupt=a.output/'corrupt.mnmc';raw=bytearray(cut.read_bytes());raw[-1]^=1;corrupt.write_bytes(raw)
won=window('finish',[dict(kind='load',path=str(path),reject=True) for path in (forged,profile,corrupt)]+[dict(kind='finish'),dict(kind='expect',transition=2,battle=False,label='Continue',continue_enabled=True),dict(kind='save',name='victory.mnmc'),dict(kind='pause')],resumed)
info=json.loads(cli('inspect-win','inspect',won));assert info['last_result']['outcome']==1 and info['region']=='arena-finale'
reopened=window('result-reopened',[dict(kind='save',name='unchanged.mnmc'),dict(kind='pause')],won);assert reopened.read_bytes()==won.read_bytes()
advanced=window('advance',[dict(kind='button',name='continueCampaignButton'),dict(kind='expect',transition=3,battle=True,spheres=0,cures=0,sphere_enabled=True,cure_enabled=True),dict(kind='save',name='arena.mnmc')],won)
next_reference=a.output/'reference-next.mnmc';cli('headless-next','start',won,next_reference);assert advanced.read_bytes()==next_reference.read_bytes()
legacy=a.output/'legacy.mnmc';legacy_active=a.output/'legacy-active.mnmc';cli('legacy-create','create',legacy);cli('legacy-start','start',legacy,legacy_active)
old=window('legacy-profile',[dict(kind='expect',sphere_enabled=False,cure_enabled=False),dict(kind='save',name='unchanged.mnmc')],legacy_active);assert old.read_bytes()==legacy_active.read_bytes()
report=dict(success=True,scope='Native terrain-bound Sphere/Cure actual Qt controls and dialogs; exact pending/mid-flight bytes, independent single hit/heal continuation, refused geometry/profile/corruption, victory/reopen/advance and legacy profile load. No original full campaign, physical audio or pixel equivalence.',processes=6,layers=23,single_hit=20,single_heal=8,refused_loads=3,exact_campaign_bytes=True,legacy_profile=True,checkpoint_sha256=hashlib.sha256(cut.read_bytes()).hexdigest(),commands=commands)
(a.output/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
