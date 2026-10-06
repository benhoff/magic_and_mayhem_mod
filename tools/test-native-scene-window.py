#!/usr/bin/env python3
"""Installed Forest crop and Redcap ANI: same native entry point, actual Qt events and modal Save."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
ROOT=Path(__file__).resolve().parents[1]
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
    if '--inner' not in sys.argv:
        subprocess.run(['xvfb-run','-a','--server-num',str(200+os.getpid()%10000),sys.executable,__file__,*sys.argv[1:],'--inner'],check=True,timeout=360);return
    args=[a for a in sys.argv[1:] if a!='--inner']
    if len(args)!=2:raise ValueError('Supply world-scene build directory and new output directory')
    build=Path(args[0]).resolve();out=Path(args[1]).resolve();out.mkdir(parents=True,exist_ok=False)
    os.environ.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')
    def run(command,name,timeout=90):
        p=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=timeout)
        (out/(name+'.stdout')).write_text(p.stdout);(out/(name+'.stderr')).write_text(p.stderr)
        if p.returncode or 'runtime error:' in p.stderr or 'AddressSanitizer' in p.stderr:raise RuntimeError((command,p.returncode,p.stdout,p.stderr))
        return p.stdout
    def verify(phase):run([ROOT/'tools/original-manifest.sh','verify'],'manifest-'+phase,120)
    verify('before')
    try:
        assets=ROOT/'working/game-clean';realm='Realms/Celtic/Forest'
        inputs=[assets/realm/'CFsec01.map',assets/realm/'Terrain.spr',assets/realm/'Terrain.ttd',assets/'Creatures/RedCap.spr',assets/'Creatures/redcap.ani']
        recorded_inputs={str(p.relative_to(ROOT)):sha(p) for p in inputs}
        scene=build/'mnm-world-scene-preview';window=build/'world-scene-window-test';sandbox=build/'world/mnm-world-sandbox';export=build/'mnm-map-navigation-export'
        crop=out/'forest';navigation=json.loads(run([export,assets,realm+'/CFsec01.map',realm,0,0,8,8,crop],'crop'))
        frozen=Path(str(crop)+'.frozen');geometry=Path(str(crop)+'.geometry');standing=set(map(tuple,navigation['standing']))
        start=(3,4,4);target=(6,4,4);assert start in standing and target in standing
        initial=out/'initial.mnms';run([sandbox,'move-terrain-ani',frozen,inputs[-1],0,initial,*start,4,4,4,3],'initial')
        pixels=module('window_pixels',ROOT/'tools/test-terrain-preview.py')
        terrain={'sprite':inputs[1].read_bytes(),'frames':{}};creature={'sprite':inputs[3].read_bytes(),'frames':{}}
        checks=0;continuations=0;cases=[]
        def oracle(frame):
            image=[0x2124]*(512*256);owners={};cells={}
            for d in frame['queue']:
                ox,oy,opaque=pixels.frame(creature if d['creature'] else terrain,d['frame'])
                for x,y,value in opaque:
                    x+=d['x']-ox;y+=d['y']-oy
                    if 0<=x<512 and 0<=y<256:
                        image[y*512+x]=value;owners[x,y]=d.get('slot') if d['creature'] else None
                        if not d['creature']:cells[x,y]=tuple(d['cell']) if 'cell' in d else None
            return struct.pack('<'+'H'*len(image),*image),owners,cells
        def point(mapping,value):
            found=[p for p,v in mapping.items() if v==value];assert found,('No visible native click target',value)
            return found[len(found)//2]
        def mouse(p,button):return {'op':'mouse','x':p[0],'y':p[1],'button':button}
        def button(name):return {'op':'button','button':name}
        def capture(name):return {'op':'capture','name':name}
        for view in range(4):
            options=['--root',assets,'--realm',realm,'--terrain-map',geometry,'--view',view]
            baseline=out/f'v{view}-baseline';run([scene,'--checkpoint',initial,*options,'--output',baseline,'--frames',1],f'baseline-{view}')
            frame=json.loads(Path(str(baseline)+'.json').read_text())['frames'][0];expected,owners,cells=oracle(frame)
            assert Path(str(baseline)+'-000.565').read_bytes()==expected
            select=point(owners,0);destination=point(cells,target)
            script=[capture('initial'),mouse(select,'left'),mouse(destination,'right'),capture('pending-move'),{'op':'ticks','count':2},capture('mid'),{'op':'wait','ms':220},capture('paused'),{'op':'save','name':'saved-mid','play':True},capture('after-save'),{'op':'ticks','count':2},capture('continued'),{'op':'run','count':1,'before':[button('queueStop'),capture('pending-stop')]},capture('stopped'),{'op':'wait','ms':220},capture('idle-paused'),{'op':'save','name':'saved-stop'},capture('idle-after-save'),{'op':'ticks','count':2},capture('idle-advanced'),mouse(destination,'right'),{'op':'ticks','count':2},capture('restarted')]
            path=out/f'v{view}-whole-script.json';path.write_text(json.dumps(script,indent=2)+'\n');whole=out/f'v{view}-whole'
            run([window,'--checkpoint',initial,*options,'--window-script',path,'--window-output',whole],f'window-whole-{view}')
            report=json.loads((whole/'window-report.json').read_text());assert report['all_match'];captures={r['name']:r for r in report['captures']}
            assert not captures['initial']['playing'] and captures['initial']['selected_index']==0
            pending=captures['pending-move']['pending'];assert pending==[{'operation':3,'slot':0,'generation':1,'destination':list(target)}]
            assert captures['pending-move']['tick']==captures['initial']['tick'] and captures['pending-move']['selected_index']==1
            assert not captures['mid']['playing'] and captures['mid']['tick']==captures['initial']['tick']+2
            state=json.loads(run([sandbox,'inspect-json',whole/'mid.mnw'],f'mid-state-{view}'));actor=state['entities'][0]
            assert actor['action']==2 and 0<actor['progress']<192 and 'animation' in actor and actor['destination']==list(target)
            assert (whole/'mid.mnw').read_bytes()==(whole/'paused.mnw').read_bytes()==(whole/'after-save.mnw').read_bytes()==(whole/'saved-mid.mnms').read_bytes()
            assert (whole/'mid.canvas.png').read_bytes()==(whole/'paused.canvas.png').read_bytes()==(whole/'after-save.canvas.png').read_bytes()
            assert not captures['after-save']['playing'] and captures['after-save']['selected_index']==1
            assert captures['pending-stop']['playing'] and captures['pending-stop']['pending']==[{'operation':5,'slot':0,'generation':1}]
            stopped=json.loads(run([sandbox,'inspect-json',whole/'stopped.mnw'],f'stopped-state-{view}'));assert stopped['pending']==0 and stopped['entities'][0]['action']==6 and stopped['entities'][0]['route']==[]
            stopped_frame=json.loads((whole/'stopped.json').read_text());bodies=[d for d in stopped_frame['queue'] if d['creature']]
            assert len(bodies)==1 and bodies[0]['slot']==0, 'Stopped actor must retain its body and identity'
            assert (whole/'stopped.mnw').read_bytes()[8]==8
            assert (whole/'stopped.mnw').read_bytes()==(whole/'idle-paused.mnw').read_bytes()==(whole/'idle-after-save.mnw').read_bytes()==(whole/'saved-stop.mnms').read_bytes()
            for name in ['idle-paused','idle-after-save','idle-advanced']:
                assert (whole/'stopped.565').read_bytes()==(whole/(name+'.565')).read_bytes(), 'Static pose advanced'
            assert captures['idle-advanced']['tick']==captures['stopped']['tick']+2
            restarted=json.loads(run([sandbox,'inspect-json',whole/'restarted.mnw'],f'restarted-state-{view}'))
            assert restarted['entities'][0]['action']==2 and 'animation' in restarted['entities'][0]

            # Default admission and commands are identical; a test-only after-tick observer chooses exact pause boundaries.
            for name in captures:
                f=json.loads((whole/(name+'.json')).read_text());expected,_,_=oracle(f);assert (whole/(name+'.565')).read_bytes()==expected;checks+=1
            middle=json.loads((whole/'mid.json').read_text());_,midowners,_=oracle(middle)
            resumed_script=[capture('loaded'),mouse(point(midowners,0),'left'),{'op':'ticks','count':2},capture('continued')]
            path=out/f'v{view}-resumed-script.json';path.write_text(json.dumps(resumed_script,indent=2)+'\n');resumed=out/f'v{view}-resumed'
            run([window,'--checkpoint',whole/'saved-mid.mnms',*options,'--window-script',path,'--window-output',resumed],f'window-resumed-{view}')
            rr=json.loads((resumed/'window-report.json').read_text());assert rr['all_match'] and not rr['captures'][0]['playing'] and rr['captures'][0]['selected_index']==0
            for suffix in ['mnw','json','565','png']:
                assert (resumed/('loaded.'+suffix)).read_bytes()==(whole/('mid.'+suffix)).read_bytes(),(view,suffix,'loaded')
                assert (resumed/('continued.'+suffix)).read_bytes()==(whole/('continued.'+suffix)).read_bytes(),(view,suffix,'continued')
            for name in ['loaded','continued']:
                f=json.loads((resumed/(name+'.json')).read_text());expected,_,_=oracle(f);assert (resumed/(name+'.565')).read_bytes()==expected;checks+=1
            # Independent explicit-step process checks the real-timer whole-window branch and stop admission.
            for source,name,ticks in [(whole/'mid.mnw','continued',2),(whole/'pending-stop.mnw','stopped',1)]:
                reference=out/f'v{view}-reference-{name}';run([scene,'--checkpoint',source,*options,'--ticks',ticks,'--output',reference,'--frames',1],f'reference-{view}-{name}')
                assert Path(str(reference)+'.mnms').read_bytes()==(whole/(name+'.mnw')).read_bytes()
                assert json.loads(Path(str(reference)+'.json').read_text())['frames'][0]==json.loads((whole/(name+'.json')).read_text())
                for suffix in ['565','png']:assert Path(str(reference)+'-000.'+suffix).read_bytes()==(whole/(name+'.'+suffix)).read_bytes()
            _,stopowners,_=oracle(stopped_frame)
            stop_script=[capture('loaded'),mouse(point(stopowners,0),'left'),{'op':'ticks','count':2},capture('idle-advanced'),mouse(destination,'right'),{'op':'ticks','count':2},capture('restarted')]
            path=out/f'v{view}-idle-resumed-script.json';path.write_text(json.dumps(stop_script,indent=2)+'\n');idle=out/f'v{view}-idle-resumed'
            run([window,'--checkpoint',whole/'saved-stop.mnms',*options,'--window-script',path,'--window-output',idle],f'window-idle-resumed-{view}')
            ir=json.loads((idle/'window-report.json').read_text());assert ir['all_match'] and not ir['captures'][0]['playing'] and ir['captures'][0]['selected_index']==0
            for current,reference in [('loaded','stopped'),('idle-advanced','idle-advanced'),('restarted','restarted')]:
                for suffix in ['mnw','json','565','png']:
                    assert (idle/(current+'.'+suffix)).read_bytes()==(whole/(reference+'.'+suffix)).read_bytes(),(view,current,suffix)
                f=json.loads((idle/(current+'.json')).read_text());expected,_,_=oracle(f);assert (idle/(current+'.565')).read_bytes()==expected;checks+=1
            continuations+=2;cases.append({'view':view,'mid_tick':captures['mid']['tick'],'progress':actor['progress'],'whole':str(whole.relative_to(ROOT)),'resumed':str(resumed.relative_to(ROOT))})
        sources=sorted(set([*ROOT.glob('apps/world-scene/*.cpp'),*ROOT.glob('apps/world-scene/*.hpp'),*ROOT.glob('apps/world-sandbox/*.cpp'),*ROOT.glob('apps/world-sandbox/*.hpp'),*ROOT.glob('game/**/*.cpp'),*ROOT.glob('game/**/*.hpp'),ROOT/'apps/world-scene/CMakeLists.txt',ROOT/'game/CMakeLists.txt',ROOT/'tests/scene-window-driver.cpp',ROOT/'tests/scene-window-driver.hpp',ROOT/'tools/test-terrain-preview.py',Path(__file__).resolve()]))
        assert all(sha(ROOT/p)==h for p,h in recorded_inputs.items()),'Installed inputs changed during experiment'
        artifacts={str(p.relative_to(out)):sha(p) for p in sorted(out.rglob('*')) if p.is_file()}
        report={'all_match':True,'live_validated':False,'scope':'Same native window entry point, production QTimer/elapsed clock and actual Qt mouse/button/modal Save events: installed Forest 8x8 crop and Redcap movement ANI base 0/SPR, four diagnostic views, corrected same-layer native terrain click mapping, mid-segment pause, pending move/stop admission and byte-exact fresh-process native state/frame continuation. Test-only observer pauses exact committed tick boundaries; original input/rays/cadence and live equivalence excluded. Static last-displayed ANI pose survives Stop, paused/modal Save, admitted idle ticks and fresh-window v8 restoration; movement restarts an active cursor. Original idle/action sequence selection is not claimed.',
                'window_runs':12,'pixel_comparisons':checks,'fresh_process_continuations':continuations,'same_layer_target':True,'real_timer_native_window':True,'actual_modal_save':True,'pause_preserves_motion':True,'stop_while_playing':True,'stopped_body_display':'retained-static-pose','stopped_checkpoint_restart':True,'cases':cases,'source_sha256':{str(p.relative_to(ROOT)):sha(p) for p in sources},'input_sha256':recorded_inputs,'binary_sha256':{str(p.relative_to(ROOT)):sha(p) for p in [window,scene,sandbox,export]},'artifact_sha256':artifacts,'fixture_sha256':{str(p.relative_to(ROOT)):sha(p) for p in [frozen,geometry,initial]},'remaining':['Other installed crops/ANI profiles and world sizes','Original input/ray/pause/cadence equivalence','Original idle/action ANI selection and initially spawned idle body mapping','Live replacement']}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'all_match':True,'window_runs':12,'pixel_comparisons':checks,'fresh_process_continuations':continuations}))
    finally:verify('after')
if __name__=='__main__':main()
