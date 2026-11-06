#!/usr/bin/env python3
"""Bounded external public-UI proof for Main focus, setup changes and Cancel.

Uses the accepted unchanged binary, physical XTest input, read-only AT-SPI and
original menu seqlocks. It never writes an original channel or process memory.
"""
import argparse,hashlib,importlib.util,io,json,os,signal,sqlite3,struct,subprocess,sys,time
from pathlib import Path
from coverage_evidence import Reader,write_report
from native_only_contract import configure,outcome
from native_prefix_template import borrow
from rendering_menu_accessibility import HostInput,AccessibleMenus
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-main-setup-results-20261106/'
PLAN=PREFIX+'input-plan-01.json'


def sha(path):
    with Path(path).open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()


def module(name):
    s=importlib.util.spec_from_file_location('menu_'+name.replace('-','_'),ROOT/'tools'/name)
    m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m


def state(path):
    """Read an unchanged stable V2..V22 engine lane and its authored payload."""
    if not path.exists():return None
    require(not path.is_symlink(),'Aliased menu channel')
    with path.open('rb') as f:
        raw=f.read(524289);f.seek(0);again=f.read(160)
    if len(raw)<540 or not any(raw[:16]):return None
    w=struct.unpack_from('<40I',raw)
    require(2<=w[2]<=22 and raw[:8]==(('MNMMCMD' if w[2]<10 else 'MNMMCM')+str(w[2])).encode() and len(raw)==w[3],'Unexpected public menu wire')
    if raw[:160]!=again or not w[32] or w[32]&1:return None
    require(w[35] in (0,1) and w[37]<=6,'Invalid original menu state')
    def text(offset,count):
        s=raw[offset:offset+count];require(b'\0' in s,'Unterminated original text');return s.split(b'\0',1)[0].decode('cp1252')
    row=dict(sequence=w[32],generation=w[33],screen=w[34],ready=bool(w[35]),ack=w[36],status=w[37],thread=w[38],handoff=w[39],host_ns=time.monotonic_ns())
    if row['ready'] and row['screen'] in (14,25):
        n=struct.unpack_from('<I',raw,164)[0];require(n<=128,'Map inventory overflow')
        row['setup']=dict(map=struct.unpack_from('<I',raw,160)[0],map_name=text(412,128),rules=list(struct.unpack_from('<13I',raw,168)),players=[dict(zip(('active','portrait','colour','handicap'),struct.unpack_from('<4I',raw,220+48*i)))|dict(name=text(236+48*i,32)) for i in range(4)])
        row['maps']=[text(540+128*i,128) for i in range(n)]
    if row['ready'] and row['screen']==10:row['preferences']=list(struct.unpack_from('<7i',raw,44708+16))
    return row,raw


def preserved_setup(setup):
    # Fresh original Create Single Player reinitializes AI cosmetics. Persisted
    # rules/map, human identity and every AI active/colour/handicap remain exact.
    return dict(setup,players=[dict(v) if i==0 else {k:x for k,x in v.items() if k not in ('name','portrait')} for i,v in enumerate(setup['players'])])


class Journey:
    def __init__(self,child,out,env):
        self.child=child;self.out=out;self.env=env;self.input=HostInput();self.access=AccessibleMenus(child.pid)
        self.states=[];self.actions=[];self.screenshots=[];self.session=None;self.last=None

    def discover(self):
        sessions=list((self.out/'owner/sessions').glob('*.json'))
        if len(sessions)==1:self.session=Path(json.loads(sessions[0].read_text())['session'])
        elif len(sessions)>1:raise ValueError('More than one native session')
        return self.session

    def read(self):
        if not self.discover():return None
        value=state(self.session/'channel.bin')
        return value

    def wait(self,screen,ack=None,timeout=30):
        deadline=time.monotonic()+timeout
        while time.monotonic()<deadline:
            require(self.child.poll() is None,'Public Qt host exited early')
            value=self.read()
            if value:
                row,raw=value
                if row['ready'] and row['screen']==screen and (ack is None or row['ack']==ack):
                    require(row['status'] in (0,1) and row['thread']>0,'Original menu refused request')
                    if self.last:
                        require(row['thread']==self.last['thread'] and row['generation']>self.last['generation'],'Original owner/generation changed')
                    self.last=row;self.states.append(row)
                    path=self.out/f'wire-{len(self.states):02}.bin';path.write_bytes(raw);row['wire_path']=str(path.relative_to(ROOT))
                    time.sleep(.25);return row
            time.sleep(.05)
        raise ValueError('Original ready menu deadline: '+str((screen,ack)))

    def capture(self,label,focus_name=None):
        from PIL import ImageGrab
        nodes=self.access.nodes();items=[v for n,v in nodes];focused=[v for v in items if v['focused']]
        if focus_name:
            n,v=self.access.find(focus_name,'button');require(v['focused'] and v['enabled'],'Expected ready control does not own focus: '+focus_name)
        name=self.out/(label+'.png');ImageGrab.grab(xdisplay=self.env['DISPLAY']).save(name)
        row=dict(label=label,host_ns=time.monotonic_ns(),state_index=len(self.states)-1,host_focus=self.input.focus(),focused=focused,nodes=items,path=str(name.relative_to(ROOT)),sha256=sha(name))
        require(row['host_focus']['host_focused'],'Native host does not own X input')
        self.screenshots.append(row);return row

    def click(self,name):
        n,v=self.access.find(name,'button');require(v['enabled'],'Disabled public button: '+name)
        self.actions.append(dict(action='click',name=name,host_ns=time.monotonic_ns(),physical=self.input.click(v['rect'])))

    def key(self,name,symbol):
        self.actions.append(dict(action=name,host_ns=time.monotonic_ns(),physical=self.input.key(symbol)))

    def change_rules(self):
        sliders=[(n,v) for n,v in self.access.nodes() if v['role']=='slider']
        require(len(sliders)==17,'Expected17native setup sliders; found '+str(len(sliders)))
        changed=[]
        for index in (0,8,13):
            identified=[(n,v) for n,v in sliders if v.get('accessible_id','').endswith('.singlePlayerSlider'+str(index+1))]
            n,v=identified[0] if len(identified)==1 else sliders[index];value=v['value'];symbol=0xff50 if value['current']!=value['minimum'] else 0xff57
            self.input.click(v['rect']);self.key('setup-slider-'+str(index+1),symbol);time.sleep(.1)
            current=n.get_value_iface().get_current_value();require(current!=value['current'],'Slider did not change')
            changed.append(dict(index=index,before=value['current'],after=current,rect=v['rect']))
        self.actions.append(dict(action='changed-setup-draft',host_ns=time.monotonic_ns(),values=changed));return changed

    def select_other_map(self):
        lists=[n for n,v in self.access.nodes() if v['role']=='list' and n.get_child_count()>2]
        require(len(lists)==1,'Ambiguous visible map list')
        listing=lists[0];current=self.last['setup']['map'];target=2 if current!=2 else 1
        rect=listing.get_component_iface().get_extents(self.access.a.CoordType.SCREEN)
        self.input.click([rect.x,rect.y,rect.width,rect.height]);self.key('map-first',0xff50)
        if target==2:self.key('map-next',0xff54)
        time.sleep(.1)
        selected=listing.get_selection_iface().get_selected_child(0);require(selected is not None,'No selected map')
        name=selected.get_name();require(name==self.last['maps'][target-1],'Actual map selection differs')
        row=dict(action='map-draft',host_ns=time.monotonic_ns(),map=target,name=name,original_map=current)
        self.actions.append(row);return row

    def run(self):
        self.input.activate();self.click('Launch game')
        # Native movies remain enabled and finish naturally; no movie-output
        # acceptance credit is inferred from this menu-only observer.
        deadline=time.monotonic()+420
        while time.monotonic()<deadline:
            require(self.child.poll() is None,'Public host exited during startup')
            value=self.read()
            if value and value[0]['ready'] and value[0]['screen']==3:break
            time.sleep(.1)
        self.wait(3,0,timeout=2);self.capture('main-ready-initial','Quick Battle')
        self.click('Preferences');preferences=self.wait(10,1);self.capture('preferences-context')
        self.key('preferences-cancel',0xff1b);self.wait(3,2);self.capture('main-ready-after-preferences','Quick Battle')
        self.key('main-focused-space',0x20);self.wait(22,3);self.capture('quick-ready')
        self.click('Create Single Player Game');initial=self.wait(14,4);self.capture('setup-initial')
        draft_cancel=self.change_rules();self.key('setup-draft-cancel',0xff1b);self.wait(22,5);self.capture('quick-after-cancel')
        self.click('Create Single Player Game');reopened=self.wait(14,6);self.capture('setup-cancel-preserved')
        require(preserved_setup(reopened['setup'])==preserved_setup(initial['setup']),'Setup Cancel applied an uncommitted draft')
        changed=self.change_rules();self.click('Map');opened=self.wait(25,7);self.capture('map-with-committed-rules')
        desired=list(reopened['setup']['rules']);desired[0]=int(changed[0]['after']);desired[8]=int(changed[1]['after'])
        require(opened['setup']['rules']==desired and opened['setup']['players'][0]['handicap']==int(changed[2]['after']),'Original setup rule changes differ')
        cancelled_map=self.select_other_map();self.capture('map-changed-draft');self.key('map-cancel',0xff1b);cancelled=self.wait(14,8);self.capture('map-cancel-preserved')
        require(cancelled['setup']==opened['setup'],'Map Cancel applied a selection or lost setup')
        self.click('Map');self.wait(25,9);selected=self.select_other_map();self.capture('map-selected');self.key('map-accept',0xff0d);confirmed=self.wait(14,10);self.capture('setup-map-confirmed')
        require(confirmed['setup']['map']==selected['map'] and confirmed['setup']['rules']==opened['setup']['rules'],'Original map/rules confirmation differs')
        self.key('setup-return',0xff1b);self.wait(22,11)
        self.click('Create Single Player Game');returned=self.wait(14,12);self.capture('setup-reentered')
        require(preserved_setup(returned['setup'])==preserved_setup(initial['setup']),'Outer Setup Cancel did not restore entry defaults')
        self.key('setup-cancel-again',0xff1b);self.wait(22,13)
        self.key('quick-cancel',0xff1b);self.wait(3,14);self.capture('main-ready-returned','Quick Battle')
        self.key('returned-main-focused-space',0x20);self.wait(22,15)
        self.key('quick-return-again',0xff1b);self.wait(3,16);self.capture('main-ready-final','Quick Battle')
        self.click('Quit')
        return dict(preferences=preferences['preferences'],initial_setup=initial['setup'],cancelled_draft=draft_cancel,applied_changes=changed,map_cancel=cancelled_map,selected_map=selected,final_setup=returned['setup'])


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--software',action='store_true');p.add_argument('--low',action='store_true');p.add_argument('--repair-of');p.add_argument('--repair-reason');a=p.parse_args()
    require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'Fresh owned report required')
    require(bool(a.repair_of)==bool(a.repair_reason),'Repair needs exact failed report and reason')
    out=ROOT/'working/menu-acceptance'/Path(a.report).stem;out.mkdir(parents=True,exist_ok=False);(out/'.retain').touch()
    sources={str(x.relative_to(ROOT)):sha(x) for x in [Path(__file__),ROOT/'tools/rendering_menu_accessibility.py',ROOT/'tools/native_only_contract.py',ROOT/'tools/native_prefix_template.py',ROOT/'tools/run-native-owned.py']}
    result=dict(schema=1,kind='native_public_main_setup_menu_acquisition',success=False,sources=sources,configuration=dict(software_flag=a.software,high_resolution=not a.low,skip_intros=False,terrain_lighting='native',native_ui_layout='800x600-logical'),original_game_runs=0,world_start_intended=False,original_pixels_used_as_native_inputs=False,scope=__doc__)
    hs=ls=child=journey=lease=None;before=False
    try:
        with Reader(ROOT) as reader:
            plan=reader.json(PLAN);require(plan['success'] is True,'Input plan incomplete');result['input_plan']=dict(record=PLAN,sha256=reader.digest(PLAN))
            require(all(reader.digest(n)==h for n,h in plan['compiler_sources'].items()),'Public binary source drift')
            binary=ROOT/plan['player_binary']['path'];require(sha(binary)==plan['player_binary']['sha256'],'Public binary changed');result['player_binary']=plan['player_binary'];result['compiler_sources']=plan['compiler_sources']
            if a.repair_of:require(reader.json(a.repair_of)['success'] is False,'Repair target is not failed')
        subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True);before=True
        # Change only this owned source's original resolution selector. Preserve
        # the exact initial bytes and restore after all cases using the input plan.
        patcher=module('run-opengl-game.py');patcher.patch_staged_preferences(ROOT/'working/game-nocd','VIDEO',{'IsHighRes':'FALSE' if a.low else 'TRUE'})
        result['installation_inputs']={str(f.relative_to(ROOT/'working/game-nocd')):dict(sha256=sha(f),bytes=f.stat().st_size) for f in sorted((ROOT/'working/game-nocd').rglob('*')) if f.is_file()}
        from rendering_progression import prepare_launch_manifest
        raw=prepare_launch_manifest(ROOT/'working/game-nocd',ROOT/'working/manifests/game-nocd.tsv');name=a.report.removesuffix('.json')+'-launch-manifest.bin';result['launch_manifest']=dict(record=name,sha256=write_report(ROOT,name,raw))
        host=out/'host';owner=out/'owner';host.mkdir();owner.mkdir();launcher=module('run-native-owned.py')
        hs,hr,_=launcher.start_display(host,allow_glx=True);ls,lr,cookie=launcher.start_display(owner)
        env=configure({k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))},['--software-rendering'] if a.software else [])
        env.update(DISPLAY=hr['display'],XAUTHORITY=hr['authority'],MNM_NATIVE_OUTPUT_OWNER=str(owner/'owner.json'),MNM_LEGACY_DISPLAY=lr['display'],MNM_LEGACY_XCOOKIE=cookie.hex(),MNM_MENU_GAME_SOURCE=str(ROOT/'working/game-nocd'),MNM_MENU_DIRECTPLAY_DX6='1',MNM_CACHE_ADAPTERS='1',MNM_VERIFY_ORIGINALS='0',MNM_VERIFY_ASSETS='0',MNM_TERRAIN_LIGHTING='native',QT_QPA_PLATFORM='xcb',QT_LINUX_ACCESSIBILITY_ALWAYS_ON='1',QT_ACCESSIBILITY='1',LP_NUM_THREADS='4',QT_SCALE_FACTOR='1',WINEDEBUG='-all',XDG_CONFIG_HOME=str(out/'config'))
        lease,result['prefix_pool']=borrow(ROOT/'working/fixtures/prefix-seed',out/'wineprefix',env);env['MNM_OWNED_WINE_PREFIX']=str(out/'wineprefix')
        budget=sqlite3.connect(ROOT/'working/menu-attempts.sqlite3')
        with budget:
            budget.execute('BEGIN IMMEDIATE')
            budget.execute('CREATE TABLE IF NOT EXISTS attempts(id INTEGER PRIMARY KEY,report TEXT UNIQUE,domain TEXT,repair_of TEXT,reason TEXT,started_ns INTEGER)')
            # Retain both original rows. Their verified zero-game preparation
            # outcomes are excluded only from the six-original-game bound.
            with Reader(ROOT) as r:
                repair=r.json(PREFIX+'staging-repair-02.json');excluded={v['record'] for v in repair['preparation_refusals']}
                previous=list(budget.execute('SELECT report FROM attempts'));charged=0;failures=0
                for (name,) in previous:
                    data=r.json(name)
                    if name in excluded:
                        expected=next(v['sha256'] for v in repair['preparation_refusals'] if v['record']==name)
                        require(r.digest(name)==expected and data['success'] is False and data['session'] is None,'Preparation exclusion differs')
                        continue
                    observed=data['original_game_runs']
                    if observed is None:
                        correction=r.json(PREFIX+'setup-assertion-repair-01.json')
                        require(correction['failed_execution']==dict(record=name,sha256=r.digest(name)) and correction['actual_original_game_runs']==1,'Prior original launch unresolved')
                        observed=1
                    require(observed in (0,1),'Prior original launch unresolved')
                    charged+=observed;failures+=int(data['success'] is False and observed==1)
                require(len(previous)<9 and charged<7 and failures<=3,'Bounded public/original attempt budget exhausted')
            budget.execute('INSERT INTO attempts(report,domain,repair_of,reason,started_ns) VALUES(?,?,?,?,?)',(a.report,str((a.software,a.low)),a.repair_of,a.repair_reason,time.monotonic_ns()));result['attempt']=budget.execute('SELECT last_insert_rowid()').fetchone()[0]
        budget.close()
        command=[str(binary),'--repo',str(ROOT),'--renderer','opengl','--live-menus','--native-commands','--resident-world','--native-only']+(['--software-rendering'] if a.software else [])
        result['command']=command
        with (out/'qt.log').open('xb') as log:
            child=subprocess.Popen(command,cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
            os.environ.update(DISPLAY=env['DISPLAY'],XAUTHORITY=env['XAUTHORITY']);journey=Journey(child,out,env)
            deadline=time.monotonic()+20
            while time.monotonic()<deadline:
                if any(v['name']=='Launch game' for n,v in journey.access.nodes()) and journey.input.find_window():break
                require(child.poll() is None,'Qt host failed to open');time.sleep(.1)
            result['game_launch_requested']=True;result['proof']=journey.run()
            require(result['proof']['preferences'][2]==int(a.low),'Original resolution context differs')
            deadline=time.monotonic()+20
            while time.monotonic()<deadline:
                f=journey.session/'original-process-result.json'
                if f.exists() and json.loads(f.read_text()).get('original_exit_code')==0 and (journey.session/'native-only-session.json').exists() and json.loads((journey.session/'native-only-session.json').read_text()).get('state')=='completed':break
                time.sleep(.1)
            result['retirement']=outcome(journey.session);require(result['retirement']['state']=='completed','Original Quit/resource retirement incomplete')
            resident=json.loads((journey.session/'resident-world-report.json').read_text());result['resident']=resident
            require(int(resident['completed'])==0 and int(resident['imports'])==0 and not resident['pending'],'Cancel started an unintended World')
            child.wait(timeout=10);result['exit_code']=child.returncode;require(child.returncode==0,'Qt host did not close normally')
        subprocess.run(['wineserver','-w'],env=dict(env,WINEPREFIX=str(out/'wineprefix')),check=True,timeout=10);result['private_wine_stopped']=True
        result['success']=True
    except Exception as e:result['error']=str(e)
    finally:
        if journey:
            result.update(states=journey.states,actions=journey.actions,screenshots=journey.screenshots,session=str(journey.session) if journey.session else None)
            try:
                result['launch_log']=journey.access.launch_log()
                result['final_accessibility']=[v for n,v in journey.access.nodes()]
            except Exception:pass
            journey.input.close()
        if child and child.poll() is None:
            os.killpg(child.pid,signal.SIGTERM)
            try:child.wait(timeout=12)
            except subprocess.TimeoutExpired:os.killpg(child.pid,signal.SIGKILL);child.wait(timeout=10)
        if lease and not result['success']:
            subprocess.run(['wineserver','-k'],env=dict(env,WINEPREFIX=str(out/'wineprefix')),timeout=10)
            subprocess.run(['wineserver','-w'],env=dict(env,WINEPREFIX=str(out/'wineprefix')),timeout=10)
            result['forced_private_prefix_stop']=True
        for server in (hs,ls):
            if server and server.poll() is None:server.terminate();server.wait(timeout=10)
        if lease:
            try:
                from experiment_environment import _idle
                deadline=time.monotonic()+20
                while True:
                    try:_idle(out/'wineprefix','wineprefix');break
                    except (ValueError,subprocess.TimeoutExpired):
                        if time.monotonic()>=deadline:raise
                        time.sleep(.2)
                lease.close();result['prefix_pool']['returned']=True
            except Exception as e:result.update(success=False,prefix_return_error=str(e))
        if before:
            try:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True);result['original_manifest_verified_before_after']=True
            except Exception as e:result.update(success=False,original_verification_error=str(e))
        result['original_game_runs']=0
        if journey and journey.session:
            original=journey.session/'original-process-result.json'
            result['original_game_runs']=int(type(json.loads(original.read_text()).get('original_exit_code')) is int) if original.exists() else (1 if journey.states else None)
        result['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items());result['success'] &= result['sources_stable']
        result['artifacts']=[];candidates=[f for f in out.rglob('*') if f.is_file() and 'wineprefix' not in f.relative_to(out).parts and f.name!='Xauthority']
        if journey and journey.session:
            candidates += [f for f in journey.session.rglob('*') if f.is_file() and ('game' not in f.relative_to(journey.session).parts or f.suffix.lower() in ('.dll','.cfg','.sav','.ini') or (f.name.startswith('native-') and f.suffix=='.bin'))]
        for f in sorted(set(candidates)):
            if f.is_symlink() or f.stat().st_size>512*1024**2:continue
            path=str(f.relative_to(ROOT));name=a.report.removesuffix('.json')+'-raw-'+hashlib.sha256(path.encode()).hexdigest()[:20]+'.bin';raw=f.read_bytes();h=write_report(ROOT,name,raw);result['artifacts'].append(dict(path=path,record=name,sha256=h,bytes=len(raw)))
        h=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(dict(success=result['success'],error=result.get('error'),record=a.report,sha256=h,attempt=result.get('attempt'),states=len(result.get('states',[])))),flush=True)
    return 0 if result['success'] else 1

if __name__=='__main__':raise SystemExit(main())
