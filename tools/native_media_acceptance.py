"""Independent native movie completion and startup admission checks."""
import json
from pathlib import Path
import struct
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'protocols/python'))
from mnm_protocols import media_v1

def validate_media(session, *, native_factory=False, allow_initial_silence=False, require_intro_completion=False):
    report = {}
    media=json.loads((session/'native-media-report.json').read_text())
    availability=[entry for entry in media if entry.get('operation')==4]
    if not availability or any(entry.get('status')!=3 or entry.get('native_pipeline_available') is not True for entry in availability):
        raise ValueError('Native startup media availability acknowledgement missing')
    policy=json.loads((session/'native-audio-launch-policy.json').read_text())
    if policy.get('native_media_startup_probe') is not True or (native_factory and policy.get('wine_directsound_disabled') is not True):
        raise ValueError('Native media startup/module policy was not confirmed')
    report['native_audio_launch_policy']=policy
    movies=[entry for entry in media if entry.get('operation')==1]
    if not movies or any(entry.get('status') not in (3,4) or not entry.get('accepted') or entry.get('error') or
                         entry.get('video_frames',0)<1 or entry.get('audio_frames',0)<1 or
                         entry.get('audible_output_requested') is not True or
                         (entry.get('audio_device_available') is not True and not (allow_initial_silence and entry.get('initial_silent_output') is True)) for entry in movies):
        raise ValueError('Native movie decoding/output/completion history missing or failed')
    if any(entry.get('status') in (5,6) for entry in media):raise ValueError('Native media rejected or failed a submitted request')
    report['native_media']=media
    if require_intro_completion:
        intros=[row for row in media if row.get('operation')==1 and Path(row.get('asset','')).name.lower() in ('intro0.avi','intro1.avi')]
        if {Path(row['asset']).name.lower() for row in intros}!={'intro0.avi','intro1.avi'} or any(row.get('status')!=3 or not row.get('accepted') or min(row.get('video_frames',0),row.get('presented_video_frames',0))<=0 for row in intros):
            raise ValueError('Enabled intros did not naturally complete decoded and presented video')
        channel=session/'native-media.bin'
        if channel.is_symlink():raise ValueError('Redirected native media channel')
        with channel.open('rb') as wire:media_header=wire.read(media_v1.HEADER_SIZE)
        if not media_v1.valid_header(media_header,channel.stat().st_size):raise ValueError('Invalid native media retirement channel')
        if struct.unpack_from('<I',media_header,media_v1.CANCELLED_REQUEST_ID_OFFSET)[0]:raise ValueError('Media producer cancelled after a host/acceptance/playback timeout')
        report['intro_completion']={'verified':True,'movies':[{k:row[k] for k in ('request','status','video_frames','presented_video_frames','asset')} for row in intros],'producer_cancelled_request':0,'scope':'Natural completion of both encoded intro clips; no original decoder pixel equivalence claim'}
    return report
