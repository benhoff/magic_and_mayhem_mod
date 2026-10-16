#!/usr/bin/env python3
"""Fresh-process native campaign mission journey and independent v2 wire checks."""
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

CLI=Path(sys.argv[1]).resolve()
MUTATOR=Path(sys.argv[2]).resolve()
retained=os.environ.get('MNM_CAMPAIGN_JOURNEY_OUTPUT')
temporary=None if retained else tempfile.TemporaryDirectory(prefix='mnm-campaign-arena-')
ROOT=Path(retained or temporary.name)
ROOT.mkdir(parents=True,exist_ok=True)
assert not any(ROOT.iterdir()),'Refusing occupied campaign journey output'
if retained:(ROOT/'.retain').write_text('Native campaign fresh-process journey.\n')
commands=[]
def run(*args,good=True):
    result=subprocess.run([str(CLI),*map(str,args)],capture_output=True,text=True,timeout=30)
    commands.append({'argv':[str(a) for a in args],'exit_code':result.returncode,'stdout':result.stdout,'stderr':result.stderr})
    assert result.returncode==(0 if good else 1),(args,result.stdout,result.stderr)
    return json.loads(result.stdout) if good else None
def rewrite(source,destination,kind):
    result=subprocess.run([str(MUTATOR),'rewrite',str(source),str(destination),kind],capture_output=True,text=True,timeout=30)
    assert not result.returncode,result.stderr
def wire(path):
    # The outer snapshot campaign blob directly follows its map string.
    data=path.read_bytes()
    offset=44
    size=struct.unpack_from('<I',data,offset)[0];offset+=4+size
    size=struct.unpack_from('<I',data,offset)[0];offset+=4
    data=data[offset:offset+size];pos=8
    assert data[:8]==b'MNMCAMP\0'
    def word(fmt='<I'):
        nonlocal pos
        value=struct.unpack_from(fmt,data,pos)[0];pos+=struct.calcsize(fmt);return value
    def string():
        nonlocal pos
        size=word();value=data[pos:pos+size].decode('ascii');pos+=size;return value
    def strings():return [string() for _ in range(word())]
    version=word();identity=(string(),string());difficulty=word();transition=word('<Q')
    player,realm,region=string(),string(),string();completed=word('<B')
    for _ in range(word()):
        string();word('<Q')
        for _ in range(6):word('<i')
        strings();strings()
    owners=[]
    for _ in range(word()):
        realm_id=string()
        for _ in range(word()):
            region_id=string();owner=string() if word('<B') else None
            owners.append((realm_id,region_id,owner,word('<B')))
    battle=None
    if word('<B'):
        battle={'realm':string(),'region':string(),'map':string(),'participants':strings()}
        battle['attempt']=word('<Q') if version>=2 else 0
    result=None
    if version>=2 and word('<B'):
        result={'attempt':word('<Q'),'realm':string(),'region':string(),'outcome':word()}
    assert pos==len(data)
    return {'version':version,'identity':identity,'difficulty':difficulty,'transition':transition,
            'player':player,'realm':realm,'region':region,'completed':completed,'owners':owners,'battle':battle,'result':result}

seed=ROOT/'seed.mnc'
initial=run('create',seed)
assert initial['battle'] is None and initial['last_result'] is None
assert wire(seed)['version']==1
seed_bytes=seed.read_bytes()
run('create',seed,good=False)
assert seed.read_bytes()==seed_bytes
active=ROOT/'active.mnc';started=run('start',seed,active)
assert started['attempt']==1 and started['transition']==1
assert started['battle']['healthRules']==0x4e53323348454131
assert wire(active)['version']==2 and wire(active)['battle']['attempt']==1
run('start',active,ROOT/'double-start.mnc',good=False)
assert not (ROOT/'double-start.mnc').exists()
# Existing native arena actions, retaining full pending summon and bolt state.
path=active
path2=ROOT/'summon-pending.mnc';queued=run('cast',path,path2,'summon',2,5);path=path2
assert queued['battle']['pending']==1
path2=ROOT/'summoned.mnc';run('resume',path,path2,3);path=path2
for tick in [3,6,9,12]:
    path2=ROOT/f'bolt-{tick}-pending.mnc';run('cast',path,path2,'bolt');path=path2
    path2=ROOT/f'bolt-{tick}-launched.mnc';state=run('resume',path,path2,3);path=path2
    assert state['battle'] is not None
    if tick==3:
        cut=path
        assert state['battle']['projectiles'],'no in-flight checkpoint'
        assert run('inspect',path)['battle']==state['battle']
last_active=path
victory=ROOT/'victory.mnc';won=run('resume',last_active,victory,100)
assert won['settlements']==1 and won['last_result']['outcome']==1 and won['last_result']['attempt']==1
assert won['battle'] is None and won['region']=='second-duel' and won['transition']==2 and not won['completed']
record=wire(victory)
assert record['result']=={'attempt':1,'realm':'arena','region':'first-duel','outcome':1}
assert record['owners']==[('arena','first-duel','player',1),('arena','second-duel','rival',0)]
# Same saved projectile/input state, split at another boundary in a new process.
split=ROOT/'split-flight.mnc';run('resume',last_active,split,1)
continued=ROOT/'continued-victory.mnc';run('resume',split,continued,100)
assert continued.read_bytes()==victory.read_bytes(),'split continuation differs'
idle=ROOT/'idle-result.mnc';repeat=run('resume',victory,idle,100)
assert repeat['settlements']==0 and idle.read_bytes()==victory.read_bytes()
assert run('inspect',idle)['settlements']==0
for kind in ['navigation','health','revision','participants']:
    invalid=ROOT/f'bad-{kind}.mnc';rewrite(cut,invalid,kind)
    run('inspect',invalid,good=False)
for kind in ['replay','owner']:
    invalid=ROOT/f'bad-{kind}.mnc';rewrite(victory,invalid,kind)
    run('inspect',invalid,good=False)
before=active.read_bytes()
run('resume',last_active,active,100,good=False)
assert active.read_bytes()==before,'overwrite refusal changed old checkpoint'
run('resume',cut,ROOT/'negative.mnc',-1,good=False)
run('resume',cut,ROOT/'excess.mnc',4097,good=False)
bad=ROOT/'corrupt.mnc';raw=bytearray(cut.read_bytes());raw[-1]^=1;bad.write_bytes(raw)
run('resume',bad,ROOT/'corrupt-out.mnc',1,good=False)
assert not (ROOT/'corrupt-out.mnc').exists()
# Defeat retains the second mission, then retry gets a fresh identity.
second=ROOT/'second.mnc';second_start=run('start',victory,second)
assert second_start['attempt']==3
defeat=ROOT/'defeat.mnc';lost=run('resume',second,defeat,200)
assert lost['last_result']['outcome']==2 and lost['last_result']['attempt']==3
assert lost['region']=='second-duel' and lost['transition']==4 and not lost['completed']
reopened=ROOT/'defeat-reopened.mnc';quiet=run('resume',defeat,reopened,20)
assert quiet['settlements']==0 and reopened.read_bytes()==defeat.read_bytes()
retry=ROOT/'retry.mnc';retry_state=run('start',defeat,retry)
assert retry_state['attempt']==5
path=retry
path2=ROOT/'retry-summon.mnc';run('cast',path,path2,'summon',2,5);path=path2
path2=ROOT/'retry-summoned.mnc';run('resume',path,path2,3);path=path2
for tick in [3,6,9,12]:
    path2=ROOT/f'retry-bolt-{tick}.mnc';run('cast',path,path2,'bolt');path=path2
    path2=ROOT/f'retry-step-{tick}.mnc';run('resume',path,path2,3);path=path2
complete=ROOT/'complete.mnc';end=run('resume',path,complete,100)
assert end['completed'] and end['transition']==6 and end['last_result']['attempt']==5 and end['last_result']['outcome']==1
run('start',complete,ROOT/'restart-completed.mnc',good=False)
terminal=ROOT/'complete-reopened.mnc';run('resume',complete,terminal,100)
assert terminal.read_bytes()==complete.read_bytes()
artifacts={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in ROOT.iterdir() if p.is_file()}
report={'success':True,'scope':'Actual authored frozen arena and ordinary-health native campaign; no original campaign/UI equivalence.',
        'victories':2,'defeats':1,'retry_attempt':5,'final_transition':6,'command_count':len(commands),
        'content_revision':initial['content_revision'],'files':artifacts,'commands':commands}
(ROOT/'report.json').write_text(json.dumps(report,indent=2)+'\n')
print('Native campaign arena: fresh-process cast/projectile continuation, victory/defeat/retry/final completion, one-time results, wire and refusal checks passed.')
