#!/usr/bin/env python3
"""Source640 regression: real clipped-title OCR and no offscreen focus attempt."""
import argparse
import importlib.util
import io
import json
from pathlib import Path
import subprocess
import struct
import sys
import tempfile
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from coverage_claims import behavior_contract,scenario_contract,required_sources
from coverage_evidence import Reader,write_report
from rendering_menu_acceptance import artifact
from rendering_pilot import require
from rendering_ui_acceptance import PREFIX
from rendering_ui_low import caption_image,make_journey,input_hint
ROOT=Path(__file__).resolve().parents[1]
BID='NP.source640-ui-controls';SID='UI07.source640-controls'


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);args=p.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(),'Fresh control fixture result required')
    result=dict(schema=1,kind='source640_ui_control_fixture',success=False,original_game_runs=0)
    try:
        from PIL import Image
        with Reader(ROOT) as r:
            reg=r.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors'] if v['id']==BID);s=next(v for v in reg['scenarios'] if v['id']==SID)
            sources={n:r.digest(n) for n in required_sources(b)}
            parent=PREFIX+'default640-09-acquisition.json';a=r.json(parent)
            require(not a['success'] and a['configuration']['actual_size']==[640,480] and a['error']=='Source wizard anchor outside frame','Wrong retained regression')
            spec=importlib.util.spec_from_file_location('low_fixture_camera',ROOT/'tools/run-rendering-camera-acceptance.py');camera=importlib.util.module_from_spec(spec);spec.loader.exec_module(camera)
            paint=camera.LiveViewport(Path(a['temporary_root'])/'capture').poll()
            shot=next(v for v in a['screenshots'] if v['label']=='failure-last-observation')
            rect=next(v['rect'] for v in shot['nodes'] if v.get('accessible_id','').endswith('.nativeGameViewport'))
            image=Image.open(io.BytesIO(artifact(r,a,shot['path'])));out=Path(tempfile.mkdtemp(prefix='mnm-low-caption-'));crop=out/'caption.png'
            caption_image(image,rect,paint,[640,480]).save(crop)
            text=subprocess.run(['tesseract',str(crop),'stdout','--psm','6'],capture_output=True,text=True,check=True,timeout=10).stdout
            require('hermes' in text.lower() and any('fermes' in v.get('text','').lower() for v in a['actions']),'Real clipped-title regression not reproduced/corrected')
            main=next(v for v in a['screenshots'] if v['label']=='main-ready-initial')
            negative=Image.open(io.BytesIO(artifact(r,a,main['path'])));caption_image(negative,rect,paint,[640,480]).save(out/'main.png')
            no_text=subprocess.run(['tesseract',str(out/'main.png'),'stdout','--psm','6'],capture_output=True,text=True,check=True,timeout=10).stdout
            require('hermes' not in no_text.lower(),'No-dialogue original Main misclassified')
            for size,picture in [([800,600],paint),([640,480],dict(paint,image_width=1060)),([640,480],dict(paint,scaling=0))]:
                try:caption_image(image,rect,picture,size)
                except ValueError:pass
                else:raise ValueError('Unsupported source/fit geometry accepted')
            class Viewport:
                def poll(self):return dict(scaling=2)
            class Base:
                def __init__(self):self.size=[640,480];self.viewport=Viewport();self.actions=[];self.focused=0;self.delegated=[]
                def focus_viewport(self):self.focused+=1
                def set_scaling(self,mode):self.delegated.append(mode)
            low=make_journey(Base)();low.set_scaling('integer');require(low.focused==1 and not low.delegated,'Redundant scaling still sought an offscreen sprite')
            low.set_scaling('sharp');require(low.delegated==['sharp'],'Real scaling no longer delegates')
            base=Base();base.set_scaling('integer');require(base.delegated==['integer'] and base.focused==0,'Source800 base class was modified')
            failed=PREFIX+'default640-10-acquisition.json';quit_case=r.json(failed)
            require(not quit_case['success'] and quit_case['error']=='Original ready menu deadline: (6, None)','Wrong confirmation regression')
            quit_paint=camera.LiveViewport(Path(quit_case['temporary_root'])/'capture');quit_paint.poll();gui=quit_paint.paint
            raw=artifact(r,quit_case,'render-input.bin')
            require(input_hint(raw,gui) and not input_hint(raw,gui,[212,316]),'Retained failed cursor no longer explains the missed answer')
            target=bytearray(raw);struct.pack_into('<II',target,24,212,316)
            require(input_hint(target,gui,[212,316]),'Ready target input refused')
            for offset,value in [(8,2),(12,1000),(16,1),(20,0),(32,800)]:
                bad=bytearray(target);struct.pack_into('<I',bad,offset,value)
                require(not input_hint(bad,gui,[212,316]),'Invalid input hint accepted')
            require(not input_hint(target,dict(gui,presentation_owner='world'),[212,316]),'World ownership accepted as confirmation')
            class QuitViewport:
                paint=dict(gui,presentation_owner='world')
                def poll(self):self.paint=gui;return gui
            class QuitBase(Base):
                def __init__(self):
                    super().__init__();self.viewport=QuitViewport();self.session=out;self.motions=[]
                    (out/'render-input.bin').write_bytes(raw)
                def click_id(self,name):self.clicked=name
                def pointer(self,point):
                    self.motions.append(list(point));value=bytearray(raw);struct.pack_into('<II',value,24,*point)
                    (out/'render-input.bin').write_bytes(value)
            qlow=make_journey(QuitBase)();qlow.click_id('miniMenuButton4');qlow.pointer([212,316])
            require(qlow.focused and qlow.motions==[[40,100],[212,316]] and not qlow.confirmation_pending
                    and qlow.actions[-1]['action']=='source640-confirmation-input-ready','Confirmation handoff does not focus, generate and observe fresh motion')
            qlow.pointer([50,100]);require(qlow.motions[-1]==[50,100],'Ordinary pointer path changed')
            require(r.digest('tools/run-rendering-ui.py')==r.json(PREFIX+'input-plan-07.json')['sources']['tools/run-rendering-ui.py'],'Preserved800 controller changed')
            require(sources=={n:r.digest(n) for n in sources},'Control fixture source drift')
            raw=crop.read_bytes();name=args.report.removesuffix('.json')+'-caption.bin'
            result.update(success=True,sources=sources,claims=[dict(behavior=BID,contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={SID:scenario_contract(s)})],
                parent=dict(record=parent,sha256=r.digest(parent)),confirmation_parent=dict(record=failed,sha256=r.digest(failed)),caption=dict(record=name,sha256=write_report(ROOT,name,raw)),text=text,no_dialogue_text=no_text,checks=18,
                source800_controller_unchanged=True,native_sources_changed=False,
                scope='Retained clipped-title and missed-confirmation regressions: complete Hermes OCR, no-dialogue negative, geometry refusals, redundant focus, GUI/active/sealed input hints, fresh-motion admission and ordinary pointer delegation. Preserved800 controller. Headless control policy only; no new game, original message-handler or rendering credit.')
    except Exception as e:result.update(success=False,error=str(e))
    print(write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode()))
    return int(not result['success'])


if __name__=='__main__':raise SystemExit(main())
