"""Stage and admit owned prepared CD tracks and their fresh native music channel."""
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys
import time
sys.path.insert(0, str(Path(__file__).resolve().parent))
from launch_asset_checks import full_asset_verification, verify_staged_assets
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'protocols/python'))
from mnm_protocols import music_v1 as wire

def sha(path):
    with path.open('rb') as source:return hashlib.file_digest(source,'sha256').hexdigest()

def startup_snapshot(path):
    deadline=time.monotonic()+.25
    while True:
        data=path.read_bytes()
        if len(data)!=wire.SIZE or not wire.valid_header(data,wire.SIZE):raise ValueError('Invalid native music channel')
        sequence=struct.unpack_from('<I',data,wire.RESPONSE_SEQUENCE_OFFSET)[0]
        with path.open('rb') as source:
            source.seek(wire.RESPONSE_SEQUENCE_OFFSET);after=source.read(4)
        if not(sequence&1) and after==struct.pack('<I',sequence):return data
        if time.monotonic()>=deadline:raise ValueError('Native music startup snapshot did not settle')
        time.sleep(.001)

def catalog_inputs(root, verify=True):
    if root.is_symlink() or (root/'manifest.json').is_symlink():raise ValueError('Redirected native music catalog')
    manifest=json.loads((root/'manifest.json').read_text())
    if manifest.get('success') is not True or manifest.get('schema')!=1:raise ValueError('Unprepared native CD tracks')
    inputs={'manifest.json':sha(root/'manifest.json')};toc=[];previous=0
    for expected,row in enumerate(manifest['tracks'],2):
        name=f'Track{expected:02d}.wav';path=root/name
        start,end=row['start_sector'],row['end_sector']
        if row['track']!=expected or row['file']!=name or type(start) is not int or type(end) is not int or not 0<start<end<=0xffffffff or (previous and start!=previous):raise ValueError('Invalid native CD track table')
        if path.is_symlink() or not path.is_file():raise ValueError('Native CD asset is missing or redirected')
        if verify and sha(path)!=row['wav_sha256']:raise ValueError('Native CD asset hash mismatch')
        if not toc:toc.append((0,start))
        toc.append((start,end));inputs[name]=row['wav_sha256'];previous=end
    if not 2<=len(toc)<=99:raise ValueError('Invalid native music track count')
    return inputs,toc

def stage(root,source):
    verify=full_asset_verification()
    before,toc=catalog_inputs(source,verify);target=root/'native-music'
    if target.exists():raise ValueError('Native music staging already exists')
    subprocess.run(['cp','-a','--reflink=auto','--',str(source),str(target)],check=True)
    after,copied=catalog_inputs(target,verify)
    if before!=after or toc!=copied or catalog_inputs(source,verify)[0]!=before:raise ValueError('Native music inputs changed during staging')
    return dict(native_music=True,full_asset_verification=verify,music_channel=str(root/'native-music.bin'),music_catalog=str(target),music_inputs=before,music_toc=[list(x) for x in toc])

def authorize(root,metadata,environment):
    environment.pop('MNM_MUSIC_CHANNEL',None)
    enabled=metadata.get('native_music',False)
    if type(enabled) is not bool:raise ValueError('Invalid native music policy')
    if not enabled:return
    if not metadata.get('native_voices') or not metadata.get('native_render') or metadata.get('origin')!='menu_action_bridge':raise ValueError('Native music requires native voice/menu/render staging')
    dll=root/'game/MnmAudio.dll'
    if dll.is_symlink() or sha(dll)!=metadata.get('audio_dll_sha256'):raise ValueError('Native music adapter hash mismatch')
    catalog=root/'native-music'
    if metadata.get('music_catalog')!=str(catalog):raise ValueError('Redirected native music catalog')
    inputs,toc=catalog_inputs(catalog,verify_staged_assets(metadata))
    if inputs!=metadata.get('music_inputs') or [list(x) for x in toc]!=metadata.get('music_toc'):raise ValueError('Staged native music catalog drift')
    path=root/'native-music.bin'
    if metadata.get('music_channel')!=str(path) or path.is_symlink():raise ValueError('Redirected native music channel')
    data=startup_snapshot(path)
    def word(offset):return struct.unpack_from('<I',data,offset)[0]
    if word(wire.READY_OFFSET)!=1 or not word(wire.HEARTBEAT_OFFSET):raise ValueError('Native music worker is not ready')
    # Only the fresh consumer may have published sequence, phase, volume and TOC.
    permitted={wire.RESPONSE_SEQUENCE_OFFSET,wire.HEARTBEAT_OFFSET,wire.READY_OFFSET,wire.PHASE_OFFSET,wire.VOLUME_OFFSET,wire.TRACK_COUNT_OFFSET,wire.LEADOUT_SECTOR_OFFSET}
    if any(word(at) for at in range(16,wire.TOC_OFFSET,4) if at not in permitted):raise ValueError('Stale native music channel')
    if word(wire.RESPONSE_SEQUENCE_OFFSET)&1 or word(wire.PHASE_OFFSET)!=wire.PHASE_STOPPED or word(wire.VOLUME_OFFSET)!=65535:raise ValueError('Native music worker startup snapshot differs')
    expected=b''.join(struct.pack('<II',*x) for x in toc)
    if word(wire.TRACK_COUNT_OFFSET)!=len(toc) or word(wire.LEADOUT_SECTOR_OFFSET)!=toc[-1][1] or data[wire.TOC_OFFSET:wire.TOC_OFFSET+len(expected)]!=expected or any(data[wire.TOC_OFFSET+len(expected):]):raise ValueError('Native music channel TOC differs')
    environment['MNM_MUSIC_CHANNEL']='Z:'+str(path).replace('/','\\')

def validate_activity(report,allow_initial_silence=False):
    if report.get('producer_features')!=63 or report.get('unsupported_cd_calls')!=0 or report.get('failed_requests')!=0 or report.get('error')!='':raise ValueError('Native CD hooks/requests failed or unsupported')
    if report.get('plays',0)<1 or report.get('requests',0)<1 or report.get('ready_after_stop') is not False or report.get('audible_output_requested') is not True:raise ValueError('Native music activity/retirement missing')
    output=report.get('output',{})
    quiet=allow_initial_silence and output.get('initial_silent_output') is True
    if output.get('audio_frames',0)<1 or (output.get('audio_device_available') is not True and not quiet) or output.get('error'):raise ValueError('Native music PCM/output missing')
    return report
