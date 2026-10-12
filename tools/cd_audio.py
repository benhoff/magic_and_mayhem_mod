"""Strict single-image CUE BINARY reader and read-only PCM track extraction."""
from dataclasses import dataclass
import hashlib
from pathlib import Path
import re
import wave

SECTOR_BYTES=2352
SAMPLE_RATE=44100

@dataclass(frozen=True)
class Track:
    number:int
    mode:str
    start:int
    pregap:int|None

def timecode(value):
    match=re.fullmatch(r'(\d+):(\d{2}):(\d{2})',value)
    if not match:raise ValueError('Invalid CUE timecode')
    minutes,seconds,frames=map(int,match.groups())
    if seconds>=60 or frames>=75:raise ValueError('Invalid CUE seconds/frames')
    return (minutes*60+seconds)*75+frames

def parse_cue(text,image_bytes):
    if image_bytes<=0 or image_bytes%SECTOR_BYTES:raise ValueError('Image must contain complete CD sectors')
    image=None;entries=[];current=None
    for raw in text.splitlines():
        line=raw.strip()
        if not line or line.startswith('REM '):continue
        match=re.fullmatch(r'FILE "([^"\r\n]+)" BINARY',line)
        if match:
            if image is not None or entries:raise ValueError('Only one BINARY image is supported')
            image=match[1]
            if '/' in image or '\\' in image or image in ('.','..'):raise ValueError('Image must be a local leaf filename')
            continue
        match=re.fullmatch(r'TRACK (\d{1,2}) (MODE1/2352|AUDIO)',line)
        if match:
            if image is None:raise ValueError('TRACK before FILE')
            number=int(match[1]);mode=match[2]
            if number!=len(entries)+1 or (number==1 and mode!='MODE1/2352') or (number>1 and mode!='AUDIO'):
                raise ValueError('Expected track1 MODE1/2352 followed by contiguous AUDIO tracks')
            current={'number':number,'mode':mode,'start':None,'pregap':None};entries.append(current);continue
        match=re.fullmatch(r'INDEX (0?[01]) (\d+:\d{2}:\d{2})',line)
        if match:
            if current is None:raise ValueError('INDEX before TRACK')
            key='start' if int(match[1])==1 else 'pregap'
            if current[key] is not None or (key=='pregap' and current['start'] is not None):raise ValueError('Duplicate or unordered INDEX')
            current[key]=timecode(match[2]);continue
        raise ValueError('Unsupported CUE statement: '+line)
    if image is None or len(entries)<2:raise ValueError('CUE has no audio tracks')
    previous=-1;leadout=image_bytes//SECTOR_BYTES
    for entry in entries:
        start=entry['start'];pregap=entry['pregap']
        if start is None or not previous<start<leadout or (pregap is not None and not previous<pregap<=start):
            raise ValueError('Missing, unordered or out-of-image track index')
        previous=start
    if entries[0]['start']!=0:raise ValueError('Data track must start at image sector0')
    return image,[Track(**entry) for entry in entries]

def sha(path):
    digest=hashlib.sha256()
    with path.open('rb') as stream:
        for data in iter(lambda:stream.read(1024*1024),b''):digest.update(data)
    return digest.hexdigest()

def extract(cue,output):
    cue=Path(cue).resolve();output=Path(output)
    text=cue.read_text(encoding='ascii')
    match=re.search(r'^\s*FILE "([^"\r\n]+)" BINARY\s*$',text,re.MULTILINE)
    if not match or '/' in match[1] or '\\' in match[1]:raise ValueError('Expected local BINARY image')
    image=cue.parent/match[1]
    if image.is_symlink() or not image.is_file():raise ValueError('Image must be a regular local file')
    name,tracks=parse_cue(text,image.stat().st_size)
    if name!=image.name:raise ValueError('Image name changed during admission')
    before={str(cue):sha(cue),str(image):sha(image)}
    output.mkdir(parents=True,exist_ok=False)
    rows=[]
    with image.open('rb') as source:
        for index,track in enumerate(tracks[1:],1):
            end=tracks[index+1].start if index+1<len(tracks) else image.stat().st_size//SECTOR_BYTES
            size=(end-track.start)*SECTOR_BYTES;path=output/f'Track{track.number:02}.wav'
            digest=hashlib.sha256();source.seek(track.start*SECTOR_BYTES)
            with path.open('xb') as destination,wave.open(destination,'wb') as target:
                target.setparams((2,2,SAMPLE_RATE,0,'NONE','not compressed'))
                remaining=size
                while remaining:
                    data=source.read(min(remaining,1024*1024))
                    if not data:raise ValueError('Truncated CD image')
                    target.writeframesraw(data);digest.update(data);remaining-=len(data)
            rows.append(dict(track=track.number,file=path.name,start_sector=track.start,end_sector=end,
                             pcm_bytes=size,frames=size//4,pcm_sha256=digest.hexdigest(),wav_sha256=sha(path)))
    if any(sha(Path(path))!=fingerprint for path,fingerprint in before.items()):raise ValueError('CD inputs changed during extraction')
    return dict(schema=1,success=True,inputs=before,inputs_unchanged=True,
                format=dict(channels=2,bits=16,rate=SAMPLE_RATE,byte_order='little'),
                span_policy='INDEX1 to next INDEX1 or image leadout; trailing pregap retained',tracks=rows)
