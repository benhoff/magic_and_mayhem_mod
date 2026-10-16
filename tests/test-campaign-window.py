#!/usr/bin/env python3
"""Real Qt campaign journey: dialogs, process restart, results, retry and completion."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

WINDOW,CLI,MUTATOR=map(lambda p:Path(p).resolve(),sys.argv[1:4])
retained=os.environ.get('MNM_CAMPAIGN_WINDOW_OUTPUT')
temporary=None if retained else tempfile.TemporaryDirectory(prefix='mnm-campaign-window-')
ROOT=Path(retained or temporary.name)
ROOT.mkdir(parents=True,exist_ok=True)
assert not any(ROOT.iterdir()),'Refusing occupied campaign window output'
commands=[]
def cli(*args):
    result=subprocess.run([str(CLI),*map(str,args)],capture_output=True,text=True,timeout=30)
    commands.append({'argv':list(map(str,args)),'returncode':result.returncode,'stdout':result.stdout,'stderr':result.stderr})
    assert not result.returncode,(args,result.stdout,result.stderr)
    return json.loads(result.stdout)
def window(name,actions,load=None):
    script=ROOT/(name+'.json');script.write_text(json.dumps(actions,indent=2)+'\n')
    output=ROOT/name
    argv=['xvfb-run','-a','-s','-screen 0 1280x1200x24',str(WINDOW),'--test-script',str(script),'--test-output',str(output)]
    if load:argv+=['--load-campaign',str(load)]
    result=subprocess.run(argv,capture_output=True,text=True,timeout=70,
        env=dict(os.environ,QT_SCALE_FACTOR='1',QT_AUTO_SCREEN_SCALE_FACTOR='0'))
    (ROOT/(name+'.stdout')).write_text(result.stdout);(ROOT/(name+'.stderr')).write_text(result.stderr)
    commands.append({'argv':argv,'returncode':result.returncode})
    assert not result.returncode,(name,result.stderr,(output/'failure.txt').read_text() if (output/'failure.txt').exists() else '')
    assert json.loads((output/'report.json').read_text())['success']
    return output/'final.mnmc'
def button(name):return {'kind':'button','name':name+'Button'}
def steps(count):return {'kind':'steps','count':count}
def save(name):return {'kind':'save','name':name}
def expect(**args):return dict(kind='expect',**args)
def cast(spell):
    return [button(spell),dict(kind='cell',**({'x':2,'y':5} if spell=='summon' else {'wizard':1}))]
def win_actions(first=3):
    result=[]
    for _ in range(first,15,3):result+=cast('bolt')+[steps(3)]
    return result+[steps(100)]
# Reference CLI and window share unchanged resource-bound arena policy.
seed=ROOT/'seed.mnmc';cli('create',seed)
active=ROOT/'active.mnmc';cli('start',seed,active)
queued=ROOT/'queued.mnmc';cli('cast',active,queued,'summon',2,5)
summoned=ROOT/'summoned.mnmc';cli('resume',queued,summoned,3)
bolt=ROOT/'bolt.mnmc';cli('cast',summoned,bolt,'bolt')
flight=ROOT/'flight.mnmc';cli('resume',bolt,flight,3)
assert cli('inspect',flight)['battle']['projectiles']
cut=window('mid-flight',[button('newCampaign')]+cast('summon')+[save('pending.mnmc'),steps(3)]+cast('bolt')+[steps(3),save('flight.mnmc'),dict(kind='pause')])
assert (ROOT/'mid-flight/pending.mnmc').read_bytes()==queued.read_bytes()
assert cut.read_bytes()==flight.read_bytes()
# Checksummed bad resources and content fail through the actual load dialog.
invalid=[]
for kind in ['navigation','health','revision','participants']:
    path=ROOT/('bad-'+kind+'.mnmc')
    result=subprocess.run([str(MUTATOR),'rewrite',str(flight),str(path),kind],capture_output=True,text=True,timeout=30)
    assert not result.returncode,result.stderr
    invalid.append(dict(kind='load',path=str(path),reject=True))
corrupt=ROOT/'corrupt.mnmc';raw=bytearray(flight.read_bytes());raw[-1]^=1;corrupt.write_bytes(raw)
invalid.append(dict(kind='load',path=str(corrupt),reject=True))
# Fresh process loads exact pending projectile state, finishes, then freezes result.
won=window('resume-victory',invalid+win_actions(6)+[expect(transition=2,battle=False,completed=False,label='Continue',continue_enabled=True),save('victory.mnmc'),dict(kind='pause')],cut)
path=flight
for tick in [6,9,12]:
    next_path=ROOT/f'cli-bolt-{tick}.mnmc';cli('cast',path,next_path,'bolt');path=next_path
    next_path=ROOT/f'cli-step-{tick}.mnmc';cli('resume',path,next_path,3);path=next_path
reference=ROOT/'reference-victory.mnmc';cli('resume',path,reference,100)
assert won.read_bytes()==reference.read_bytes()
reopened=window('result-reopened',[expect(transition=2,battle=False,label='Continue'),dict(kind='pause'),save('result.mnmc')],won)
assert reopened.read_bytes()==won.read_bytes(),'Reopened result applied twice'
# Continue starts second duel. Real timer is exercised independently below.
lost=window('continue-defeat',[button('continueCampaign'),expect(transition=3,battle=True),steps(200),expect(transition=4,battle=False,label='Retry',continue_enabled=True),save('defeat.mnmc')],won)
second=ROOT/'second.mnmc';cli('start',reference,second)
defeat=ROOT/'reference-defeat.mnmc';cli('resume',second,defeat,200)
assert lost.read_bytes()==defeat.read_bytes()
complete=window('retry-complete',[expect(label='Retry'),button('continueCampaign'),expect(transition=5,battle=True)]+cast('summon')+[steps(3)]+win_actions()+[expect(transition=6,battle=False,completed=True,continue_enabled=False),save('completed.mnmc')],lost)
final=cli('inspect',complete)
assert final['completed'] and final['last_result']['attempt']==5 and final['last_result']['outcome']==1
reopen_complete=window('complete-reopened',[expect(transition=6,completed=True,continue_enabled=False),dict(kind='pause')],complete)
assert reopen_complete.read_bytes()==complete.read_bytes()
# Open/save dialogs also handle pre-battle v1 checkpoints without auto-start.
loaded=window('idle-load-dialog',[button('newCampaign'),dict(kind='load',path=str(seed)),expect(transition=0,battle=False,label='Continue'),save('idle.mnmc'),button('continueCampaign'),save('started.mnmc')])
assert (ROOT/'idle-load-dialog/idle.mnmc').read_bytes()==seed.read_bytes()
assert loaded.read_bytes()==active.read_bytes()
# Actual Play timer progresses; pause retains exact bytes across an event-loop wait.
timed=window('timer',[button('newCampaign'),dict(kind='play'),dict(kind='pause'),save('timer.mnmc')])
meta=cli('inspect',timed);tick=meta['battle']['tick']
assert tick>0
timed_reference=ROOT/'timer-reference.mnmc';cli('resume',active,timed_reference,tick)
assert timed.read_bytes()==timed_reference.read_bytes()
# Existing standalone window regression separately covers its full save/load flow.
files={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in ROOT.rglob('*') if p.is_file()}
report={'success':True,'fresh_process_continuation':True,'actual_dialogs':True,'refused_loads':len(invalid),
        'result_reopen_unchanged':True,'retry_attempt':5,'final_transition':6,'timer_tick':tick,
        'window_processes':8,'files':files,'commands':commands}
(ROOT/'report.json').write_text(json.dumps(report,indent=2)+'\n')
print('Native campaign window: mid-flight process restart, dialogs/rollback, timer/pause, Continue/Retry and final completion passed.')
