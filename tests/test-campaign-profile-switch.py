#!/usr/bin/env python3
"""Switch resource-bound campaign profiles through real Qt load/new controls."""
import argparse, hashlib, json, os, subprocess
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__)
for name in ['window','arena_cli','map_cli','creature_fixture','assets','output']:p.add_argument(name,type=Path)
a=p.parse_args();a.output=a.output.resolve();a.output.mkdir(parents=True,exist_ok=False)
commands=[]
def run(name,args):
    args=list(map(str,args));r=subprocess.run(args,capture_output=True,text=True,timeout=120,env=dict(os.environ,QT_SCALE_FACTOR='1',QT_AUTO_SCREEN_SCALE_FACTOR='0'))
    (a.output/(name+'.stdout')).write_text(r.stdout);(a.output/(name+'.stderr')).write_text(r.stderr)
    commands.append(dict(argv=args,exit_code=r.returncode));assert not r.returncode,(name,r.stderr[-2000:])
def seed(cli,prefix,name):
    idle=a.output/(name+'-idle.mnmc');active=a.output/(name+'.mnmc')
    run(name+'-create',[cli,*prefix,'create-sphere' if name=='map' else 'create',idle]);run(name+'-start',[cli,*prefix,'start',idle,active]);return active
map_save=seed(a.map_cli,[a.assets],'map');arena_save=seed(a.arena_cli,[],'arena')
creature_save=a.output/'creature.mnmc';run('creature-seed',[a.creature_fixture,'seed-campaign',a.assets,creature_save])
inputs={str(f):hashlib.sha256(f.read_bytes()).hexdigest() for f in [map_save,arena_save,creature_save]}
def expect(terrain,name):return dict(kind='expect',terrain=terrain,creature_name=name,battle=True)
def load(path,reject=False):return dict(kind='load',path=str(path),reject=reject)
def button(name):return dict(kind='button',name=name+'Button')
def window(name,initial,flags,actions):
    script=a.output/(name+'.json');script.write_text(json.dumps(actions,indent=2)+'\n');out=a.output/name
    run(name,['xvfb-run','-a','-s','-screen 0 1280x1200x24',a.window,*flags,'--load-campaign',initial,'--test-script',script,'--test-output',out])
    assert json.loads((out/'report.json').read_text())['success'];return out
both=['--asset-root',a.assets,'--installation',a.assets]
out=window('switch',map_save,both,[expect(True,'Sentinel'),load(creature_save),expect(False,'Crocodile'),dict(kind='save',name='creature.mnmc'),button('newMapCampaign'),expect(True,'Sentinel'),dict(kind='save',name='map.mnmc'),load(arena_save),expect(False,'Sentinel'),dict(kind='save',name='arena.mnmc'),button('newCampaign'),expect(False,'Crocodile'),dict(kind='save',name='new-creature.mnmc'),load(map_save),expect(True,'Sentinel')])
for name,source in [('map',map_save),('creature',creature_save),('arena',arena_save)]:assert (out/(name+'.mnmc')).read_bytes()==source.read_bytes(),name
assert (out/'final.mnmc').read_bytes()==map_save.read_bytes()
expected=seed(a.arena_cli,['--installation',a.assets],'installed')
assert (out/'new-creature.mnmc').read_bytes()==expected.read_bytes()
# Refused profile changes preserve the active campaign and its presentation.
for name,initial,flags,target,terrain,label in [('no-installation',map_save,['--asset-root',a.assets],creature_save,True,'Sentinel'),('no-terrain',creature_save,['--installation',a.assets],map_save,False,'Crocodile')]:
    out=window(name,initial,flags,[load(target,True),expect(terrain,label)])
    assert (out/'final.mnmc').read_bytes()==initial.read_bytes()
assert all(hashlib.sha256(Path(f).read_bytes()).hexdigest()==h for f,h in inputs.items())
report=dict(success=True,scope='Three real Qt processes switch installed Crocodile, authored arena and Plains campaign saves via dialogs and New controls, verify terrain/profile selection and byte-exact saves, and retain the previous campaign on missing resource bindings. No original campaign equivalence.',inputs=inputs,commands=commands)
(a.output/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=True,processes=3)))
