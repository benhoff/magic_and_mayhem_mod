#!/usr/bin/env python3
"""Independent-process installed crop campaign continuation and resource refusals."""
import json,os,shutil,subprocess,sys,tempfile
from pathlib import Path
cli,contract,root=map(Path,sys.argv[1:4])
output=Path(os.environ.get('MNM_MAP_CAMPAIGN_OUTPUT',tempfile.mkdtemp(prefix='mnm-map-campaign-')));output.mkdir(parents=True,exist_ok=True)
index=0
commands=[]
def run(*args,assets=root,ok=True):
    command=[str(cli),str(assets),*map(str,args)]
    result=subprocess.run(command,text=True,capture_output=True)
    commands.append({'argv':command,'exit_code':result.returncode,'stderr':result.stderr})
    assert (result.returncode==0)==ok,(command,result.stderr)
    return json.loads(result.stdout) if ok else None

def nextfile():
    global index
    index+=1;return output/f'state-{index:03}.mnmc'
def action(source,kind,*args):
    target=nextfile();v=run(kind,source,target,*args);return target,v
initial=nextfile();run('create',initial)
current,v=action(initial,'start');assert v['battle']['tick']==0 and v['region']=='plains-crossing'
current,v=action(current,'cast','summon',2,5,3)
pending=current
current,v=action(current,'resume',3)
current,v=action(current,'move',3,5,3)
current,v=action(current,'cast','bolt')
current,v=action(current,'resume',3)
flight=current
one,v=action(flight,'resume',6)
half,v=action(flight,'resume',2);split,v=action(half,'resume',4)
assert one.read_bytes()==split.read_bytes()
# Five more bolts on an unchanged rule cadence win the first mission.
current=one
for tick in range(5):
    current,v=action(current,'cast','bolt');current,v=action(current,'resume',3)
    if not v['battle']:break
current,v=action(current,'resume',100)
assert v['last_result']['outcome']==1 and v['region']=='arena-finale' and v['transition']==2
settled=current;again,v=action(settled,'resume',10);assert settled.read_bytes()==again.read_bytes() and v['settlements']==0
current,v=action(settled,'start');assert v['attempt']==3 and v['battle']['tick']==0
# Each resource participates in revision binding. Appended bytes either fail
# strict decoding or change the revision; neither may resume the old save.
relative=['Realms/Celtic/Plains/CPsec01.map','Realms/Celtic/Plains/Terrain.ttd','Realms/Celtic/Plains/Terrain.spr']
for n,name in enumerate(relative):
    changed=output/f'changed-{n}'
    for source in relative:
        target=changed/source;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(root/source,target)
    with (changed/name).open('ab') as f:f.write(b'changed-content')
    run('inspect',flight,assets=changed,ok=False)
run('inspect',flight,assets=output/'missing-root',ok=False)
result=subprocess.run([str(contract),str(root)],text=True,capture_output=True);assert result.returncode==0,result.stderr
checks=json.loads(result.stdout);assert checks['success']
report={'success':True,'scope':'Bounded installed Plains projection and native campaign policy only. No original campaign or full-map equivalence.','pending':str(pending),'flight':str(flight),'settled':str(settled),'next':str(current),'checks':checks,'refused_resources':4,'commands':commands}
(output/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'success':True,'report':str(output/'report.json')}))
