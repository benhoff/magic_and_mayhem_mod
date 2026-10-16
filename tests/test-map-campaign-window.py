#!/usr/bin/env python3
"""Real terrain-campaign clicks/dialogs and fresh-process checkpoint journeys."""
import hashlib,json,os,subprocess,sys,tempfile
from pathlib import Path
WINDOW,CLI,FORGE,ASSETS=map(lambda s:Path(s).resolve(),sys.argv[1:5])
ROOT=Path(os.environ.get('MNM_MAP_CAMPAIGN_WINDOW_OUTPUT',tempfile.mkdtemp(prefix='mnm-map-window-')));ROOT.mkdir(parents=True,exist_ok=True)
commands=[]
def run(argv):
    r=subprocess.run(list(map(str,argv)),capture_output=True,text=True,timeout=75)
    commands.append({'argv':list(map(str,argv)),'exit_code':r.returncode,'stdout':r.stdout,'stderr':r.stderr})
    assert not r.returncode,(argv,r.stdout,r.stderr)
    return r
index=0
def cli(command,source=None,*args):
    global index
    index+=1;target=ROOT/f'cli-{index}.mnmc'
    argv=[CLI,ASSETS,command]
    if source:argv.append(source)
    argv+=[target,*args];run(argv);return target

def button(name):return dict(kind='button',name=name+'Button')
def cell(x,y,z=3,right=False):return dict(kind='cell',x=x,y=y,z=z,right=right)
def steps(count):return dict(kind='steps',count=count)
def save(name):return dict(kind='save',name=name)
def expect(**args):return dict(kind='expect',**args)
def window(name,actions,load=None):
    script=ROOT/(name+'.json');script.write_text(json.dumps(actions,indent=2)+'\n');out=ROOT/name
    argv=['xvfb-run','-a','-s','-screen 0 1280x1200x24',WINDOW,'--asset-root',ASSETS,'--test-script',script,'--test-output',out]
    if load:argv+=['--load-campaign',load]
    r=subprocess.run(list(map(str,argv)),capture_output=True,text=True,timeout=75,env=dict(os.environ,QT_SCALE_FACTOR='1',QT_AUTO_SCREEN_SCALE_FACTOR='0'))
    (ROOT/(name+'.stdout')).write_text(r.stdout);(ROOT/(name+'.stderr')).write_text(r.stderr)
    commands.append({'argv':list(map(str,argv)),'exit_code':r.returncode})
    assert not r.returncode,(name,r.stderr,(out/'failure.txt').read_text() if (out/'failure.txt').exists() else '')
    return out/'final.mnmc'
seed=cli('create-sphere');active=cli('start',seed);pending=cli('cast',active,'summon',2,5,3)
summoned=cli('resume',pending,3);moving=cli('move',summoned,3,5,3);queued=cli('cast',moving,'bolt');flight=cli('resume',queued,3)
cut=window('move-cast-save',[button('newMapCampaign'),button('summon'),cell(2,5),steps(3),cell(2,5),cell(3,5,right=True),dict(kind='cell',wizard=0),button('bolt'),dict(kind='cell',wizard=1),save('queued.mnmc'),steps(3),save('flight.mnmc'),dict(kind='pause')])
assert (ROOT/'move-cast-save/queued.mnmc').read_bytes()==queued.read_bytes(),'Terrain clicks changed semantic order bytes'
assert cut.read_bytes()==flight.read_bytes(),'Window mid-flight differs from headless map campaign'
# Save/reopen the in-flight world and continue across a fresh window process.
resumed=window('reload-continue',[steps(2),save('split.mnmc'),steps(4),dict(kind='pause')],cut)
reference=cli('resume',flight,6);assert resumed.read_bytes()==reference.read_bytes()
forged=ROOT/'forged-policy.mnmc';run([FORGE,ASSETS,flight,forged])
corrupt=ROOT/'corrupt.mnmc';raw=bytearray(flight.read_bytes());raw[-1]^=1;corrupt.write_bytes(raw)
won=window('finish-after-reopen',[dict(kind='load',path=str(forged),reject=True),dict(kind='load',path=str(corrupt),reject=True),dict(kind='finish'),expect(transition=2,battle=False,label='Continue',continue_enabled=True),save('victory.mnmc'),dict(kind='pause')],resumed)
info=json.loads(run([CLI,ASSETS,'inspect',won]).stdout);assert info['last_result']['outcome']==1 and info['region']=='arena-finale'
reopen=window('result-reopened',[dict(kind='pause'),save('unchanged.mnmc')],won);assert reopen.read_bytes()==won.read_bytes()
nextmission=window('advance',[button('continueCampaign'),expect(transition=3,battle=True),save('next.mnmc'),dict(kind='pause')],won)
reference=cli('start',won);assert reference.read_bytes()==nextmission.read_bytes()
info=json.loads(run([CLI,ASSETS,'inspect',nextmission]).stdout);assert info['battle']['tick']==0 and info['attempt']==3
report={'success':True,'scope':'Diagnostic terrain/native glyph window, real map click movement/casting and dialogs, independent-process resource-bound restart/finish/advance. No original pixels/UI/rules equivalence.','processes':5,'refused_loads':2,'final_transition':3,'queued_sha256':hashlib.sha256(queued.read_bytes()).hexdigest(),'flight_sha256':hashlib.sha256(flight.read_bytes()).hexdigest(),'commands':commands}
(ROOT/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'success':True,'report':str(ROOT/'report.json')}))
