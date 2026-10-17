"""Bounded Hermes readiness for physical camera tests; never changes game memory."""
import hashlib
import json
import math
import re
import subprocess
import time

from campaign_render_oracle import oracle


class CameraDialogueGuard:
    def __init__(self, output, click, record, resident_world):
        self.root, self.prefix = output.parent, 'camera-'+output.stem
        if not re.fullmatch(r'camera-(?:input|mini-input)-\d+', self.prefix):
            raise ValueError('Unsafe camera dialogue phase')
        self.click, self.record, self.resident_world = click, record, resident_world
        self.checks, self.dismissals, self.overhead_seconds = [], 0, 0.0
        self.intervals = []

    def capture(self):
        from PIL import Image
        if len(self.checks) >= 32:
            raise ValueError('Camera dialogue capture budget exhausted')
        name = self.prefix+f'-{len(self.checks):02d}'
        request = self.root/'capture-request.json'
        if request.exists():raise ValueError('Overlapping camera diagnostic request')
        temporary = request.with_suffix('.tmp')
        temporary.write_text(json.dumps(dict(name=name)));temporary.replace(request)
        metadata = self.root/(name+'.json');deadline = time.monotonic()+10
        while not metadata.exists():
            if time.monotonic()>deadline:raise ValueError('Camera dialogue capture deadline')
            time.sleep(.1)
        meta = json.loads(metadata.read_text())
        if meta.get('fallback') is not False or not all(meta.get(r+'_saved') is True for r in ('native', 'original')):
            raise ValueError('Missing current native camera dialogue capture')
        hashes, texts = {}, {}
        for route in ('native', 'original'):
            path = self.root/(name+'-'+route+'.png')
            hashes[route] = hashlib.sha256(path.read_bytes()).hexdigest()
            crop = self.root/(name+'-'+route+'-dialogue.png')
            with Image.open(path) as image:image.crop((225,90,600,270)).save(crop)
            texts[route] = subprocess.run(['tesseract',str(crop),'stdout','--psm','6'],capture_output=True,
                                         text=True,check=True,timeout=10).stdout.strip()
        return dict(name=name, metadata=meta, image_sha256=hashes, texts=texts,
                    visible={r:'hermes' in texts[r].lower() for r in texts}, dismissed=False)

    def ensure_ready(self):
        """Only click a witnessed Hermes card; two quiet captures close readiness."""
        start = time.monotonic();dismissed = False;quiet = 0
        try:
            for _ in range(16):
                item = self.capture();self.checks.append(item)
                visible = item['visible'];routes = ('native',) if self.resident_world else ('native','original')
                self.record('camera-dialogue-check',capture=item['name'],visible=visible)
                if not any(visible[r] for r in routes):
                    quiet += 1
                    if quiet == 2:return dismissed
                    time.sleep(.3);continue
                quiet = 0
                if not self.resident_world and visible['native'] != visible['original']:
                    time.sleep(.3);continue
                if self.dismissals >= 12:raise ValueError('Camera Hermes dismissal budget exhausted')
                self.click(400,300);self.dismissals += 1;dismissed = True;item['dismissed'] = True
                self.record('camera-hermes-dismiss',capture=item['name'])
                time.sleep(.6)
            raise ValueError('Camera Hermes dialogue did not clear after bounded ordinary input')
        finally:
            end = time.monotonic()
            self.overhead_seconds += end-start
            self.intervals.append(dict(begin=start,end=end))

    def report(self):
        return dict(oracle=oracle(self.resident_world),checks=self.checks,dismissals=self.dismissals,
                    overhead_seconds=self.overhead_seconds,intervals=self.intervals,
                    scope='Known Hermes heading only. Resident original CPU images are diagnostics. No unknown-modal or full dialogue semantics claim.')


def validate_camera_dialogue(inputs, root, resident_world):
    guard = inputs.get('camera_dialogue', {})
    if guard.get('oracle') != oracle(resident_world):raise ValueError('Camera dialogue oracle differs from rendering mode')
    checks = guard.get('checks', [])
    overhead = guard.get('overhead_seconds')
    if type(overhead) not in (int,float) or not math.isfinite(overhead) or overhead<0:
        raise ValueError('Invalid camera diagnostic duration')
    if not 2 <= len(checks) <= 32:raise ValueError('Missing bounded camera readiness captures')
    routes = ('native',) if resident_world else ('native','original')
    dismissals = 0
    for index, item in enumerate(checks):
        name = item['name']
        if not re.fullmatch(r'camera-(?:input|mini-input)-\d+-\d{2}', name) or int(name.rsplit('-',1)[1]) != index:
            raise ValueError('Unsafe/reordered camera dialogue capture')
        meta = item['metadata']
        if meta.get('fallback') is not False or not all(meta.get(r+'_saved') is True for r in ('native','original')):
            raise ValueError('Camera dialogue capture was not native')
        if set(item['texts']) != {'native','original'} or item['visible'] != {r:'hermes' in item['texts'][r].lower() for r in ('native','original')}:
            raise ValueError('Camera Hermes classification differs from retained OCR')
        for route in ('native','original'):
            if hashlib.sha256((root/(name+'-'+route+'.png')).read_bytes()).hexdigest() != item['image_sha256'][route]:
                raise ValueError('Camera dialogue image changed after OCR')
        if item['dismissed']:
            if not all(item['visible'][r] for r in routes):raise ValueError('Unwitnessed camera dialogue click')
            dismissals += 1
    if dismissals != guard.get('dismissals') or dismissals > 12:
        raise ValueError('Camera dialogue dismissal count differs')
    if any(check['visible'][r] or check['dismissed'] for check in checks[-2:] for r in routes):
        raise ValueError('Camera helper ended without quiet native readiness')
    return dict(verified=True,checks=len(checks),dismissals=dismissals,oracle=guard['oracle'],
                scope=guard['scope'])


def validate_camera_mini(inputs, root, resident_world, journal, expected_action):
    readiness=validate_camera_dialogue(inputs,root,resident_world)
    witness=inputs.get('mini_callback');before=inputs.get('before_sequence')
    if inputs.get('success') is not True or type(before) is not int or before<0:
        raise ValueError('Incomplete camera Mini input')
    if not isinstance(witness,list) or len(witness)!=16 or witness[0]<=before or witness[1]!=3 or witness[3]!=17 or witness[9]!=expected_action:
        raise ValueError('Missing expected original Mini callback')
    import struct
    data=journal.read_bytes()
    if data[:16]!=b'MNMMENU1'+struct.pack('<II',1,64):raise ValueError('Invalid Mini journal')
    records=list(struct.iter_unpack('<16I',data[16:]))
    if [r[0] for r in records]!=list(range(1,len(records)+1)) or witness[0]>len(records) or list(records[witness[0]-1])!=witness:
        raise ValueError('Camera Mini callback differs from original journal')
    escapes=[a for a in inputs.get('actions',[]) if a['action']=='camera-mini-escape']
    if not 1<=len(escapes)<=4 or [a.get('attempt') for a in escapes]!=list(range(len(escapes))):
        raise ValueError('Unbounded camera Mini Escape attempts')
    # A retry needs a fresh witnessed dismissal after the preceding Escape.
    dismissed=False;seen_escape=False
    for action in inputs['actions']:
        if action['action']=='camera-hermes-dismiss':dismissed=True
        if action['action']=='camera-mini-escape':
            if seen_escape and not dismissed:raise ValueError('Unwitnessed camera Mini Escape retry')
            seen_escape=True;dismissed=False
    if sum(a['action']=='camera-hermes-dismiss' for a in inputs['actions'])!=readiness['dismissals']:
        raise ValueError('Camera Mini dismissal actions differ from captures')
    return dict(verified=True,dialogue=readiness,escape_attempts=len(escapes),callback_sequence=witness[0],action=witness[9])
