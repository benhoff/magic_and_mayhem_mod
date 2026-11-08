"""Validate actual640frame title geometry before a legacy compatibility view."""
import hashlib,json
from pathlib import Path
from PIL import Image,ImageOps
from rendering_pilot import require
RECT=(395,110,485,140)
PROCESSING=dict(mode='L',scale=6,border=20,fill=255)

def fitted_title(image):
    cropped=image.crop(RECT).convert('L')
    return ImageOps.expand(cropped.resize((cropped.width*6,cropped.height*6)),border=20,fill=255)

def validate_inputs(directory,legacy_validator):
    directory=Path(directory);raw_records=[];qualified=0
    for path in sorted(directory.glob('storage-input-*.json')):
        raw=path.read_bytes();record=json.loads(raw)
        require(record.get('source_frame_pixels')==[640,480] and record.get('input_active_checked') is True,'Missing actual640source/input admission')
        w,h=record['viewport_pixels'];require(w>=320 and h>=240 and w*3==h*4,'Fitted title viewport aspect differs')
        for o in record['tutorial_observations']:
            if o.get('title_rule')!='hermes-low-fit-title-v1':continue
            require(o['rect']==list(RECT) and o.get('ocr_preprocessing')==PROCESSING and o.get('ocr_mode')==7,'Unknown fitted title geometry/processing')
            require(all(Path(o[k]).name==o[k] for k in ('image','crop')),'Unsafe fitted title image')
            image=directory/o['image'];crop=directory/o['crop']
            require(hashlib.sha256(image.read_bytes()).hexdigest()==o['image_sha256'] and hashlib.sha256(crop.read_bytes()).hexdigest()==o['crop_sha256'],'Changed fitted title input')
            with Image.open(image) as original,Image.open(crop) as actual:
                require(original.size==(800,600),'Unnormalized title observation')
                expected=fitted_title(original)
                require(actual.mode==expected.mode and actual.size==expected.size and actual.tobytes()==expected.tobytes(),'Title crop differs from exact source-derived transform')
            # The existing generic input guard knows title text and ordered
            # physical edges. Geometry for this new fitted rule is checked above.
            # Only this temporary consumer view loses the unknown rule label;
            # original immutable acquisition bytes and display dimensions remain.
            o.pop('title_rule');qualified+=1
        raw_records.append(dict(path=path.name,sha256=hashlib.sha256(raw).hexdigest()))
        path.write_bytes((json.dumps(record,sort_keys=True,separators=(',',':'))+'\n').encode())
    result=legacy_validator(directory)
    result.update(source_frame_pixels=[640,480],fitted_title_observations=qualified,raw_records=raw_records,
        compatibility_view='Only temporary derived validator input; fitted geometry/pixels independently checked, raw evidence and actual display dimensions preserved')
    return result
