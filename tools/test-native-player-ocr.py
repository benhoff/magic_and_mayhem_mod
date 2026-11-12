#!/usr/bin/env python3
"""Recheck the retained 640x480 Hermes crop without running the original game."""
import argparse
import hashlib
import io
import json
from pathlib import Path
import subprocess
import tempfile
from PIL import Image
from campaign_camera_dialogue import dialogue_crop
from campaign_storage import hermes_title_line
from coverage_evidence import read_bytes,write_report
ROOT=Path(__file__).resolve().parents[1]

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--image',type=Path,required=True)
    p.add_argument('--report',type=Path,required=True)
    a=p.parse_args()
    if a.report.exists():p.error('Fresh report required')
    names=['tools/test-native-player-ocr.py','tools/campaign_camera_dialogue.py','tools/campaign_storage.py','tools/coverage_evidence.py']
    sources={n:hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in names}
    result=dict(success=False,sources=sources,original_game_executed=False,image=str(a.image),scope=__doc__)
    try:
        raw=read_bytes(a.image);result['image_sha256']=hashlib.sha256(raw).hexdigest()
        with Image.open(io.BytesIO(raw)) as image, tempfile.TemporaryDirectory() as directory:
            if image.size!=(640,480):raise ValueError('Require retained native 640x480 image')
            result['ocr']={}
            for name,rect in [('previous',(180,72,480,216)),('current',dialogue_crop(*image.size))]:
                crop=Path(directory)/(name+'.png');image.crop(rect).save(crop)
                command=['tesseract',str(crop),'stdout','--psm','6']
                text=subprocess.run(command,capture_output=True,text=True,check=True,timeout=20).stdout.strip()
                result['ocr'][name]=dict(rect=list(rect),text=text,exact_hermes_heading=hermes_title_line(text))
            if result['ocr']['previous']['exact_hermes_heading'] or not result['ocr']['current']['exact_hermes_heading']:
                raise ValueError('Retained image did not reproduce the previous refusal and corrected exact heading')
        result['success']=True
    except (ValueError,OSError,subprocess.SubprocessError) as error:result['error']=str(error)
    result['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items())
    result['success'] &= result['sources_stable']
    write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode());print(json.dumps(result))
    return int(not result['success'])
if __name__=='__main__':raise SystemExit(main())
