#!/usr/bin/env python3
"""Source-bound actual-widget Crossroads journeys using Qt's offscreen platform."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[1]
def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    for key in ('assets','build','sanitized','claims','output'):p.add_argument('--'+key,type=Path,required=True)
    a=p.parse_args()
    for key in ('assets','build','sanitized','output'):setattr(a,key,getattr(a,key).resolve())
    a.output.mkdir(parents=True,exist_ok=False);(a.output/'.retain').write_text('Source-bound Crossroads actual-widget journeys, checkpoints, captures and logs.\n')
    declaration=json.loads(a.claims.read_text());sources=declaration['sources'];assert all(sha(ROOT/k)==v for k,v in sources.items())
    env=dict(os.environ,QT_QPA_PLATFORM='offscreen',QT_FORCE_STDERR_LOGGING='1',QT_SCALE_FACTOR='1',QT_AUTO_SCREEN_SCALE_FACTOR='0',ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    report=dict(success=False,sources=sources,claims=declaration['claims'],commands=[],windows=[],comparisons=[],platform='Qt offscreen actual QWidget mouse/button/dialog/paint execution; no X11 or human visual acceptance.',scope='Native Crossroads window: both full-health routes through actual widgets, exact queued orders and headless checkpoint equality, fresh-process reopening, explicit defeat/retry fixtures, earned spell controls/delivery, rejected loads, legacy windows and bounded sanitizer smoke. No original equivalence, gameplay changes or live replacement.')
    def run(name,args):
        args=list(map(str,args));r=subprocess.run(args,cwd=ROOT,env=env,capture_output=True,text=True,timeout=1200)
        report['commands'].append(dict(argv=args,exit_code=r.returncode));(a.output/(name+'.stdout')).write_text(r.stdout);(a.output/(name+'.stderr')).write_text(r.stderr)
        assert r.returncode==0,(name,r.stdout[-1200:],r.stderr[-2500:])
        assert 'runtime error:' not in r.stderr and 'ERROR: AddressSanitizer' not in r.stderr
        return r.stdout
    def equal(label,left,right):
        assert left.read_bytes()==right.read_bytes(),label
        report['comparisons'].append(dict(label=label,left=str(left.relative_to(ROOT)),right=str(right.relative_to(ROOT)),sha256=sha(left)))
    def expect(**kw):return dict(kind='expect',**kw)
    def button(name):return dict(kind='button',name=name+'Button')
    def save(name):return dict(kind='save',name=name+'.mnmc')
    def route(region,reject=False):return dict(kind='route',region=region,reject=reject)
    def pilot(ticks=1024,win=True):return dict(kind='pilot',ticks=ticks,win=win)
    def window(name,actions,load=None,sanitized=False,start=False):
        script=a.output/(name+'.json');script.write_text(json.dumps(actions,indent=2)+'\n');out=a.output/name
        args=[(a.sanitized if sanitized else a.build)/'campaign-graph-window-test','--asset-root',a.assets,'--test-script',script,'--test-output',out]
        if load:args+=['--load-campaign',load]
        if start:args+=['--crossroads-campaign']
        run(name,args);result=json.loads((out/'report.json').read_text());assert result['success'];report['windows'].append(dict(name=name,sanitized=sanitized,**result));return out
    def cli(name,command,source,*args):
        dest=a.output/(name+'.mnmc');run(name,[a.build/'mnm-map-campaign-cli',a.assets,command,source,dest,*args]);return dest
    run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
    installed=['Realms/Celtic/Plains/CPsec01.map','Realms/Celtic/Plains/Terrain.ttd','Realms/Celtic/Plains/Terrain.spr','CFG/Encrypted/creature.cfg','CFG/Encrypted/HTH.cfg','Creatures/crocodile.ani','CFG/Encrypted/effectani.cfg']
    installed += [q.relative_to(a.assets).as_posix() for q in sorted((a.assets/'Sprites').glob('*')) if q.name.lower().startswith('effects') and q.suffix.lower()=='.ani']
    report['installed']={q:sha(a.assets/q) for q in installed}
    try:
        run('configure',['cmake','-S',ROOT/'apps/skirmish','-B',a.build,'-DCMAKE_BUILD_TYPE=Debug','-DCMAKE_CXX_FLAGS_DEBUG=-g0'])
        run('build',['cmake','--build',a.build,'-j4'])
        run('sanitized-configure',['cmake','-S',ROOT/'apps/skirmish','-B',a.sanitized,'-DCMAKE_BUILD_TYPE=Debug','-DCMAKE_CXX_FLAGS_DEBUG=-g0','-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie','-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])
        run('sanitized-build',['cmake','--build',a.sanitized,'--target','campaign-graph-window-test','-j4'])
        refs=a.output/'reference'
        report['headless']=json.loads(run('headless-reference',[a.build/'campaign-graph-test',a.assets,'emit',refs]));assert report['headless']['chapter_completed']
        opening=[button('newCrossroadsCampaign'),expect(battle=True,terrain=True,cure=False,sphere=False,choices=0,text='Keep alive throughout'),dict(kind='disabled',name='cureButton'),dict(kind='disabled',name='sphereButton'),route('arena-finale',True),save('start'),pilot(1,False),expect(feedback_not='complete'),pilot(23,False),save('mid'),dict(kind='pause')]
        out=window('opening',opening);equal('new action/start',out/'start.mnmc',refs/'opening-start.mnmc');equal('opening mid',out/'mid.mnmc',refs/'opening-mid.mnmc')
        san=window('sanitized-opening',opening,sanitized=True);equal('sanitized widget opening',san/'mid.mnmc',out/'mid.mnmc')
        fork=window('opening-reopened',[pilot(),expect(battle=False,choices=0,route_text='Last Stand — Available'),expect(route_text='Watchpost — Available'),save('fork'),dict(kind='capture',name='fork.png')],out/'mid.mnmc')
        equal('opening production resume',fork/'fork.mnmc',refs/'fork.mnmc')
        bad=a.output/'corrupt.mnmc';bad.write_bytes(b'MNMS truncated native checkpoint')
        report['routes']=[]
        for index,name,other,reward in [(1,'last-stand','watchpost','Cure'),(2,'watchpost','last-stand','Magic Sphere')]:
            prefix='route-'+str(index)
            active=window(prefix+'-selected',[dict(kind='load',path=str(bad),reject=True),expect(region='last-stand',choices=0),route(name),save('selected'),expect(choices=0),button('continueCampaign'),expect(choices=1,battle=True,terrain=True),route(other,True),save('start'),pilot(1,False),expect(feedback_not='complete'),pilot(23,False),save('mid'),dict(kind='pause')],fork/'fork.mnmc')
            equal(prefix+' selection',active/'selected.mnmc',refs/(prefix+'-selected.mnmc'));equal(prefix+' start',active/'start.mnmc',refs/(prefix+'-start.mnmc'));equal(prefix+' mid',active/'mid.mnmc',refs/(prefix+'-mid.mnmc'))
            won=window(prefix+'-reopened',[dict(kind='load',path=str(active/'mid.mnmc')),pilot(),expect(region='arena-finale',battle=False,text='Earned spells: '+reward),route(other,True),save('win'),dict(kind='capture',name='reward.png')],active/'mid.mnmc')
            equal(prefix+' production resume',won/'win.mnmc',refs/(prefix+'-win.mnmc'))
            finale=window(prefix+'-finale',[save('idle'),button('continueCampaign'),expect(terrain=False,cure=index==1,sphere=index==2),save('start'),pilot(),expect(completed=True,continue_enabled=False,battle=False),save('complete'),dict(kind='disabled',name='continueCampaignButton'),route(other,True)],won/'win.mnmc')
            equal(prefix+' reward reopen',finale/'idle.mnmc',won/'win.mnmc');equal(prefix+' finale start',finale/'start.mnmc',refs/(prefix+'-finale-start.mnmc'));equal(prefix+' complete',finale/'complete.mnmc',refs/(prefix+'-finale-win.mnmc'))
            complete=window(prefix+'-complete-reopened',[expect(completed=True,continue_enabled=False,battle=False),dict(kind='pause')],finale/'complete.mnmc');equal(prefix+' terminal reopen',complete/'final.mnmc',finale/'complete.mnmc')
            # Explicit one-health/projectile fixture; complete journeys above retain full health.
            fixture=a.output/(prefix+'-defeat-input.mnmc');run(prefix+'-defeat-fixture',[a.build/'campaign-graph-window-reference',a.assets,active/'start.mnmc',fixture])
            loss=cli(prefix+'-headless-loss','resume',fixture,1)
            lost=window(prefix+'-loss',[dict(kind='steps',count=1),expect(label='Retry',battle=False,choices=1),route(other,True),save('loss')],fixture);equal(prefix+' defeat',lost/'loss.mnmc',loss)
            retry=cli(prefix+'-headless-retry','start',loss)
            retried=window(prefix+'-retry',[expect(label='Retry'),button('continueCampaign'),expect(region=name,choices=1,battle=True),save('retry')],lost/'loss.mnmc');equal(prefix+' retry',retried/'retry.mnmc',retry)
            # Existing earned-spell fixture supplies a valid target before its order is queued.
            reward_input=a.output/(prefix+'-reward-input.mnmc');run(prefix+'-reward-fixture',[a.build/'campaign-graph-window-reference',a.assets,refs/(prefix+'-earned-pending.mnmc'),reward_input,'reward'])
            cast=window(prefix+'-earned-spell',[expect(cure=index==1,sphere=index==2),dict(kind='spell',spell='cure' if index==1 else 'sphere',target=2),save('pending'),dict(kind='steps',count=48),save('delivered')],reward_input)
            equal(prefix+' earned control queue',cast/'pending.mnmc',refs/(prefix+'-earned-pending.mnmc'))
            delivered=cli(prefix+'-headless-earned','resume',refs/(prefix+'-earned-pending.mnmc'),48);equal(prefix+' earned delivery',cast/'delivered.mnmc',delivered)
            report['routes'].append(dict(route=index,completed=True,restarted=True,retried=True,reward=reward))
        # Typed version-4 objectives and legacy profiles remain loadable in the same application.
        old=ROOT/'working/tests/mission-objectives-bound-20261017-v3/boundaries/boundary-0.mnmc'
        legacy=window('legacy-objectives',[expect(battle=True,terrain=True,text='Keep alive throughout'),save('old')],old);equal('version-4 typed window',legacy/'old.mnmc',old)
        report['ctest']=run('ctest',['ctest','--test-dir',a.build,'--output-on-failure'])
        # Header/source/build closure of the actual configured application and test binaries.
        dependencies=set()
        for dep in a.build.rglob('*.o.d'):
            for token in dep.read_text().replace('\\\n',' ').split()[1:]:
                path=Path(token);path=(path if path.is_absolute() else a.build/path).resolve()
                if path.is_file() and path.is_relative_to(ROOT):dependencies.add(path.relative_to(ROOT).as_posix())
        report['compiler_dependencies']=sorted(dependencies);report['undeclared_dependencies']=sorted(dependencies-set(sources));assert not report['undeclared_dependencies'],report['undeclared_dependencies']
        report['sources_unchanged']=all(sha(ROOT/k)==v for k,v in sources.items());report['inputs_unchanged']=all(sha(a.assets/k)==v for k,v in report['installed'].items());assert report['sources_unchanged'] and report['inputs_unchanged']
        report['success']=True
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify']);(a.output/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(dict(success=report['success'],windows=len(report['windows']),comparisons=len(report['comparisons']),report=str(a.output/'report.json'))))
if __name__=='__main__':main()
