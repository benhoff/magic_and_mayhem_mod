#!/usr/bin/env python3
"""Actual Magic Sphere button, moving target, mid-flight save and fresh-window resume."""
import argparse,hashlib,json,os,subprocess,tempfile
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__);p.add_argument('window',type=Path);p.add_argument('cli',type=Path);p.add_argument('fixture',type=Path);p.add_argument('--output',type=Path);a=p.parse_args();temporary=tempfile.TemporaryDirectory(prefix='sphere-window-') if not a.output else None;a.output=a.output.resolve() if a.output else Path(temporary.name);a.output.mkdir(parents=True,exist_ok=True);assert not any(a.output.iterdir());commands=[]
def run(name,argv,ok=True):
 argv=list(map(str,argv));result=subprocess.run(argv,capture_output=True,text=True,timeout=90,env=dict(os.environ,QT_SCALE_FACTOR='1',QT_AUTO_SCREEN_SCALE_FACTOR='0'));commands.append(argv);(a.output/(name+'.stdout')).write_text(result.stdout);(a.output/(name+'.stderr')).write_text(result.stderr)
 assert (result.returncode==0)==ok,(name,result.stdout[-2000:],result.stderr[-2000:]);return result.stdout

def window(name,actions,load):
 script=a.output/(name+'.json');script.write_text(json.dumps(actions,indent=2)+'\n');out=a.output/name
 run(name,['xvfb-run','-a','-s','-screen 0 1280x1200x24',a.window.resolve(),'--test-script',script,'--test-output',out,'--load-campaign',load]);assert json.loads((out/'report.json').read_text())['success'];return out/'final.mnmc'
seed=a.output/'seed.mnmc';run('seed',[a.fixture.resolve(),'seed-campaign',seed]);cut=window('flight',[dict(kind='cell',wizard=0),dict(kind='button',name='sphereButton'),dict(kind='cell',actor=2),dict(kind='save',name='pending.mnmc'),dict(kind='steps',count=1),dict(kind='expect',spheres=1,actor=2,health=24),dict(kind='save',name='flight.mnmc'),dict(kind='pause')],seed)
state=json.loads(run('inspect-cut',[a.cli.resolve(),'inspect',cut]))['battle'];assert len(state['spheres'])==1
reference=a.output/'reference.mnmc';run('headless',[a.cli.resolve(),'resume',cut,reference,8]);resumed=window('restart',[dict(kind='expect',spheres=1,actor=2,health=24),dict(kind='steps',count=8),dict(kind='expect',spheres=1,actor=2,health=4),dict(kind='save',name='impact.mnmc')],cut);assert resumed.read_bytes()==reference.read_bytes()
completed=a.output/'completed.mnmc';impact=json.loads(run('count-impact',[a.fixture.resolve(),'resume-campaign',cut,completed]));assert impact['impacts']==1 and impact['damage']==20 and impact['health']==4;assert completed.read_bytes()==reference.read_bytes()
final=json.loads(run('inspect-final',[a.cli.resolve(),'inspect',resumed]))['battle'];unit=next(u for u in final['units'] if u['slot']==2);assert unit['position']!=[5,5,1]
# The exact prior Cure-only profile remains loadable and cannot acquire Sphere.
legacy=a.output/'cure-only.mnmc';run('seed-cure-only',[a.fixture.resolve(),'seed-cure-only',legacy]);run('inspect-cure-only',[a.cli.resolve(),'inspect',legacy]);refused=a.output/'refused.mnmc';before=legacy.read_bytes();run('reject-sphere-on-cure-profile',[a.cli.resolve(),'cast',legacy,refused,'sphere',1],ok=False);assert not refused.exists() and legacy.read_bytes()==before
report=dict(success=True,scope='Actual Qt Magic Sphere button/enemy picker/save dialogs, moving native creature, mid-flight save, fresh window and headless process resume with byte-identical campaigns and one20-point impact; legacy Cure-only profile retained.',moving_target=True,fresh_process=True,exact_campaign_bytes=True,impacts=1,damage=20,health_before=24,health_after=4,cure_only_profile=True,checkpoint_sha256=hashlib.sha256(cut.read_bytes()).hexdigest(),result_sha256=hashlib.sha256(resumed.read_bytes()).hexdigest(),commands=commands)
(a.output/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
