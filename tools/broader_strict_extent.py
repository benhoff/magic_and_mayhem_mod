"""Verify strict context from existing native Mini images and staged preferences."""
import hashlib,io,json,re
from pathlib import Path
from PIL import Image
from campaign_camera_dialogue import validate_camera_mini
from rendering_pilot import require

def frame_extent(raw,sha,wanted):
 require(hashlib.sha256(raw).hexdigest()==sha,'Strict native readiness image changed')
 with Image.open(io.BytesIO(raw))as image:
  require(image.format=='PNG'and list(image.size)==wanted,'Strict native readiness frame has another actual extent');image.verify()
 return dict(sha256=sha,bytes=len(raw),size=wanted)

def observe(root,a,wanted):
 root=Path(root);directory=root/a['artifacts'];session=root/a['session'];require(directory.is_relative_to(root/'working/tests/native-output-owner')and session.is_relative_to(root/'working/experiments/menu-observer'),'Foreign strict observation roots')
 cfg=(session/'game/CFG/prefs.cfg').read_bytes();require(re.findall(rb'(?mi)^IsHighRes\s*=\s*(TRUE|FALSE)',cfg)==[b'TRUE'if wanted[0]==800 else b'FALSE'],'Strict staged preference extent differs')
 frames=[];readiness=[]
 for i in range(3):
  name='mini-input-'+str(i)+'.json';raw=(directory/name).read_bytes();inputs=json.loads(raw);readiness.append(validate_camera_mini(inputs,directory,True,session/'events.bin',4 if i==0 else 3))
  for item in inputs['camera_dialogue']['checks']:
   # The original helper has already checked every named image, metadata,
   # original callback and quiet readiness interval. Use its native image hash.
   path=directory/(item['name']+'-native.png');frames.append(dict(path=str(path.relative_to(root)),**frame_extent(path.read_bytes(),item['image_sha256']['native'],wanted)))
 require(len(frames)>=6,'Missing actual strict native frame contexts')
 return dict(verified=True,actual_native_frame_size=wanted,staged_cfg_sha256=hashlib.sha256(cfg).hexdigest(),native_readiness_images=frames,independent_original_Mini_readiness=readiness,new_game_or_pixel_capture_requests=0,scope='Read only the existing same-session native Mini PNGs, their independently validated hashes/callbacks and staged original preferences; preserve the unchanged strict actions, capture policy and zero ordinary-readback assertions.')
