#!/usr/bin/env python3
"""Replay bounded native heading OCR at both admitted World canvas sizes."""
import argparse,hashlib,json,shutil,subprocess
from pathlib import Path
from unittest.mock import patch
from PIL import Image,ImageDraw,ImageFont,__version__ as PIL_VERSION
from campaign_camera_dialogue import CameraDialogueGuard,dialogue_crop
from coverage_claims import behavior_contract,required_sources,scenario_contract
ROOT=Path(__file__).resolve().parents[1];BID='NP.camera-hermes-crop-extents'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def declaration():
    r=json.load(open(ROOT/'research/runtime/coverage/register.json'));b=next(b for b in r['behaviors'] if b['id']==BID);s=next(s for s in r['scenarios'] if s['id']=='camera-hermes-crop-extents')
    return [dict(behavior=BID,contract_sha256=behavior_contract(b,{x['id']:x for x in r['builds']}),scenarios={s['id']:scenario_contract(s)})],{p:sha(ROOT/p) for p in sorted(required_sources(b)|set(s['tests']))}
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--failed-run',type=Path,default=ROOT/'research/runtime/fixtures/camera-hermes-crop-20261020');a=p.parse_args();out=a.output.resolve();source=a.failed_run.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working'):p.error('Fresh output below working required')
    out.mkdir(parents=True);claims,sources=declaration();inputs={};report=dict(schema=1,success=False,claims=claims,sources=sources,scope='Headless production capture/OCR against authored response files: four retained failed640image pairs, old crop misses, authored800heading and two-size unknown-speaker/stale-CPU controls, unsupported extents. Ordinary click callbacks are recorded only; no fresh game, physical input, camera movement, renderer or live acceptance.',original_renderer_executed=False,live_replacement=False)
    (out/'prospective.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(label,cmd):
        with (out/(label+'.log')).open('x') as f:subprocess.run([str(x) for x in cmd],cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,check=True,timeout=180)
    def old_text(path,target):
        with Image.open(path) as image:image.crop((225,90,600,270)).save(target)
        return subprocess.check_output(['tesseract',str(target),'stdout','--psm','6'],stderr=subprocess.DEVNULL,text=True).strip()
    def pair(root,index,native,original):
        stem='camera-input-0-'+str(index).zfill(2)
        for route,image in [('native',native),('original',original)]:
            if isinstance(image,Path):shutil.copyfile(image,root/(stem+'-'+route+'.png'))
            else:image.save(root/(stem+'-'+route+'.png'))
        (root/(stem+'.json')).write_text(json.dumps(dict(fallback=False,native_saved=True,original_saved=True)))
    def fixture_guard(root,clicks):
        guard=CameraDialogueGuard(root/'input-0.json',lambda *xy:clicks.append(xy),lambda *args,**kw:None,True)
        capture=guard.capture
        def acknowledged_capture():
            # Model consumption of this authored request by the response broker.
            # All parsing, image extent/crop and OCR remain the production method.
            try:return capture()
            finally:(root/'capture-request.json').unlink(missing_ok=True)
        guard.capture=acknowledged_capture
        return guard
    run('original-before' ,[ROOT/'tools/original-manifest.sh','verify'])
    try:
        original=json.load(open(source/'report.json'))
        if original['success'] or 'No original Mini callback' not in original.get('error',''):raise ValueError('Expected preserved failed hardware witness')
        inputs[str(source/'report.json')]=sha(source/'report.json');inputs[str(source/'mini-input-0.json')]=sha(source/'mini-input-0.json');witness=json.load(open(source/'mini-input-0.json'));cases=[]
        for i in range(4):
            root=out/('retained640-'+str(i));root.mkdir();native=source/f'camera-mini-input-0-{i:02d}-native.png';diagnostic=source/f'camera-mini-input-0-{i:02d}-original.png'
            for route,path in [('native',native),('original',diagnostic)]:
                inputs[str(path)]=sha(path)
                if witness['camera_dialogue']['checks'][i]['image_sha256'][route]!=sha(path):raise ValueError('Retained witness hash differs from image')
            with Image.open(native) as image:
                if image.size!=(640,480):raise ValueError('Wrong retained native extent')
            previous=old_text(native,root/'old-crop.png')
            if 'hermes' in previous.lower():raise ValueError('Old regression no longer reproduces')
            pair(root,0,native,diagnostic);quiet=Image.new('RGB',(640,480))
            pair(root,1,quiet,quiet);pair(root,2,quiet,quiet);clicks=[]
            guard=fixture_guard(root,clicks)
            with patch('campaign_camera_dialogue.time.sleep'):
                if not guard.ensure_ready() or clicks!=[(400,300)] or len(guard.checks)!=3:raise ValueError('Production known-heading replay failed')
            cases.append(dict(input_sha256=sha(native),old_text=previous,old_visible=False,new_text=guard.checks[0]['texts']['native'],new_visible=True,checks=guard.checks,dismissals=guard.dismissals))
        face=ImageFont.truetype('DejaVuSerif.ttf',24);font=Path(face.path);inputs[str(font)]=sha(font);authored=[]
        for size in [(640,480),(800,600)]:
            # Independent authored typography, not a claim about original800pixels.
            x,y=dialogue_crop(*size)[:2]
            def heading(text):
                image=Image.new('RGB',size,'white');ImageDraw.Draw(image).text((x+40,y+20),text,font=face,fill='black');return image
            for text,expected in [('Hermes',True),('Merlin',False)]:
                root=out/(str(size[0])+'-'+text);root.mkdir();pair(root,0,heading(text),heading('Hermes'));pair(root,1,heading('Forest'),heading('Hermes'))
                if expected:pair(root,2,heading('Forest'),heading('Hermes'))
                clicks=[];guard=fixture_guard(root,clicks)
                with patch('campaign_camera_dialogue.time.sleep'):dismissed=guard.ensure_ready()
                if dismissed!=expected or len(clicks)!=int(expected):raise ValueError('Authored speaker/stale CPU classification failed')
                authored.append(dict(extent=size,speaker=text,visible=expected,checks=guard.checks,dismissals=len(clicks)))
        refused=[]
        for size in [(640,600),(800,480),(641,480),(1280,960)]:
            root=out/('refused-'+str(size[0])+'x'+str(size[1]));root.mkdir();image=Image.new('RGB',size);pair(root,0,image,image);clicks=[];guard=fixture_guard(root,clicks)
            try:guard.ensure_ready()
            except ValueError as error:
                if 'extent' not in str(error) or clicks or guard.checks:raise
                refused.append(size)
            else:raise ValueError('Unsupported image classified as quiet')
        run('unit',['python3',ROOT/'tests/test-camera-dialogue.py'])
        if declaration()!=(claims,sources) or any(sha(Path(path))!=h for path,h in inputs.items()):raise ValueError('Source/input changed')
        report.update(success=True,sources_stable=True,inputs=inputs,retained640_cases=cases,retained640_count=4,authored_controls=authored,authored_count=4,unsupported_refusals=refused,unsupported_count=4,font_sha256=sha(font),PIL_version=PIL_VERSION,tesseract_version=subprocess.check_output(['tesseract','--version'],text=True).splitlines()[0],unit_methods=11,physical_input_executed=False)
    finally:
        run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True;report['artifacts']={p.name:sha(p) for p in out.iterdir() if p.is_file()};(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
if __name__=='__main__':main()
