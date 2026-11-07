#!/usr/bin/env python3
"""Bounded real-player enabled-intro Escape and focused Main return.

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
PREFIX='research/runtime/rendering-movie-skip-results-20261106/'
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


sys.path.insert(0,str(ROOT/'protocols/python'))
from mnm_protocols import media_v1 as wire
base=module('run-rendering-menu-acceptance.py')


def media_snapshot(path):
    if not path.exists():return None
    require(not path.is_symlink(),'Aliased media channel')
    with path.open('rb') as file:raw=file.read(wire.SIZE+1);file.seek(0);again=file.read(wire.SIZE+1)
    if len(raw)!=wire.SIZE or not wire.valid_header(raw,len(raw)):return None
    word=lambda offset:struct.unpack_from('<I',raw,offset)[0]
    request,response=word(wire.REQUEST_SEQUENCE_OFFSET),word(wire.RESPONSE_SEQUENCE_OFFSET)
    if not request or not response or request&1 or response&1:return None
    if request!=struct.unpack_from('<I',again,wire.REQUEST_SEQUENCE_OFFSET)[0] or response!=struct.unpack_from('<I',again,wire.RESPONSE_SEQUENCE_OFFSET)[0]:return None
    n=word(wire.PATH_LENGTH_OFFSET);require(n<=wire.MAX_PATH_LENGTH,'Media path budget')
    row=dict(request_sequence=request,response_sequence=response,request=word(wire.REQUEST_ID_OFFSET),operation=word(wire.OPERATION_OFFSET),
             flags=word(wire.FLAGS_OFFSET),response=word(wire.RESPONSE_ID_OFFSET),status=word(wire.RESPONSE_STATUS_OFFSET),
             accepted=word(wire.ACCEPTED_ID_OFFSET),cancelled=word(wire.CANCELLED_REQUEST_ID_OFFSET),heartbeat=word(wire.HEARTBEAT_OFFSET),
             path=raw[wire.PATH_OFFSET:wire.PATH_OFFSET+n].decode('ascii'),host_ns=time.monotonic_ns())
    return row,raw


class Journey(base.Journey):
    def __init__(self,child,out,env,skip_index):
        super().__init__(child,out,env);self.skip_index=skip_index;self.movie_skips=[]

    def save_media(self,label,value):
        row,raw=value;path=self.out/(label+'.bin');path.write_bytes(raw)
        return dict(row,path=str(path.relative_to(ROOT)),sha256=hashlib.sha256(raw).hexdigest())

    def run(self):
        self.input.activate();self.click('Launch game');deadline=time.monotonic()+400;launch_deadline=time.monotonic()+90;next_log=0
        while time.monotonic()<deadline:
            require(self.child.poll() is None,'Public host exited during enabled movies')
            value=self.read()
            if not self.session:
                require(time.monotonic()<launch_deadline,'Original session staging deadline')
            if self.session:
                media=media_snapshot(self.session/'native-media.bin')
                if media:
                    row,_=media;request=row['request'];index=request-2
                    if (row['operation']==wire.OPERATION_MOVIE and row['response']==row['accepted']==request
                            and row['status']==wire.STATUS_ACCEPTED and index in (0,1) and index>=self.skip_index
                            and not any(v['request']==request for v in self.movie_skips)):
                        time.sleep(1.0);media=media_snapshot(self.session/'native-media.bin')
                        require(media is not None and media[0]['request']==media[0]['response']==media[0]['accepted']==request
                                and media[0]['status']==wire.STATUS_ACCEPTED and media[0]['flags']==media[0]['cancelled']==0,'Movie did not remain in accepted visible playback')
                        before=self.save_media('movie-before-'+str(request),media)
                        shot=self.capture('movie-before-'+str(request));self.key('movie-escape-'+str(request),0xff1b);action=self.actions[-1]
                        end=time.monotonic()+5;after=None
                        while time.monotonic()<end:
                            now=media_snapshot(self.session/'native-media.bin')
                            if now and (now[0]['request']>request or now[0]['response']==request and now[0]['status']==wire.STATUS_CANCELLED):after=now;break
                            require(self.child.poll() is None,'Host exited before skip acknowledgement');time.sleep(.01)
                        require(after is not None,'Physical Escape did not retire accepted movie')
                        self.movie_skips.append(dict(request=request,index=index,before=before,after=self.save_media('movie-after-'+str(request),after),
                                                     physical=action,screenshot=shot['sha256']))
            if value and value[0]['ready'] and value[0]['screen']==3:break
            time.sleep(.025)
        require(any(v['index']==self.skip_index for v in self.movie_skips),'Required selected intro was not physically skipped')
        self.wait(3,0,timeout=3);time.sleep(.4);initial=self.capture('main-ready-initial','Quick Battle')
        self.click('Preferences');preferences=self.wait(10,1);self.capture('preferences-context')
        self.key('preferences-cancel',0xff1b);self.wait(3,2);self.capture('main-after-preferences','Quick Battle')
        self.key('main-focused-space',0x20);self.wait(22,3);self.capture('quick-ready')
        self.key('quick-cancel',0xff1b);self.wait(3,4);self.capture('main-ready-final','Quick Battle')
        self.click('Quit')
        return dict(preferences=preferences['preferences'],movie_skips=self.movie_skips,initial_main=initial['sha256'],selected_skip_index=self.skip_index)


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--skip-index',required=True,type=int,choices=(0,1));p.add_argument('--software',action='store_true');p.add_argument('--low',action='store_true');p.add_argument('--repair-of');p.add_argument('--repair-reason');a=p.parse_args()
    require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'Fresh owned report required')
    require(bool(a.repair_of)==bool(a.repair_reason),'Repair needs exact failed report and reason')
    require(a.skip_index==int(a.low),'Bounded High/Low selected intro context differs')
    out=ROOT/'working/movie-skip-acceptance'/Path(a.report).stem;out.mkdir(parents=True,exist_ok=False);(out/'.retain').touch()
    sources={str(x.relative_to(ROOT)):sha(x) for x in [Path(__file__),ROOT/'tools/rendering_menu_accessibility.py',ROOT/'tools/native_only_contract.py',ROOT/'tools/native_prefix_template.py',ROOT/'tools/run-native-owned.py',ROOT/'tools/run-rendering-menu-acceptance.py']}
    result=dict(schema=1,kind='native_public_movie_skip_acquisition',success=False,sources=sources,configuration=dict(software_flag=a.software,high_resolution=not a.low,skip_intros=False,terrain_lighting='native',native_ui_layout='800x600-logical',selected_skip_index=a.skip_index),original_game_runs=0,world_start_intended=False,original_pixels_used_as_native_inputs=False,scope=__doc__)
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
        budget=sqlite3.connect(ROOT/'working/movie-skip-attempts.sqlite3')
        with budget:
            budget.execute('BEGIN IMMEDIATE')
            budget.execute('CREATE TABLE IF NOT EXISTS attempts(id INTEGER PRIMARY KEY,report TEXT UNIQUE,domain TEXT,repair_of TEXT,reason TEXT,started_ns INTEGER)')
            previous=list(budget.execute('SELECT report FROM attempts'))
            require(len(previous)<6,'Bounded movie-skip attempt budget exhausted')
            with Reader(ROOT) as r:
                earlier=[(n,r.json(n)) for (n,) in previous]
                require(all(v.get('original_game_runs') in (0,1) for n,v in earlier),'Previous original launch count unresolved')
                failures=[n for n,v in earlier if v['success'] is False]
                require(not failures or a.repair_of in failures,'Stopped movie-skip batch requires an exact failed-case repair')
            budget.execute('INSERT INTO attempts(report,domain,repair_of,reason,started_ns) VALUES(?,?,?,?,?)',(a.report,str((a.software,a.low)),a.repair_of,a.repair_reason,time.monotonic_ns()));result['attempt']=budget.execute('SELECT last_insert_rowid()').fetchone()[0]
        budget.close()
        command=[str(binary),'--repo',str(ROOT),'--renderer','opengl','--live-menus','--native-commands','--resident-world','--native-only']+(['--software-rendering'] if a.software else [])
        result['command']=command
        with (out/'qt.log').open('xb') as log:
            child=subprocess.Popen(command,cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
            os.environ.update(DISPLAY=env['DISPLAY'],XAUTHORITY=env['XAUTHORITY']);journey=Journey(child,out,env,a.skip_index)
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
