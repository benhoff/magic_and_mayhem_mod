"""Admit a fresh session-owned native movie/file-sound channel before Wine."""
from pathlib import Path
import struct
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'protocols/python'))
from mnm_protocols import media_v1

def authorize(root,metadata,environment):
    environment.pop('MNM_RENDER_MEDIA',None)
    enabled=metadata.get('native_media',False)
    if type(enabled) is not bool:raise ValueError('Invalid native media policy')
    if not enabled:return
    if not metadata.get('native_render') or metadata.get('origin')!='menu_action_bridge':raise ValueError('Native media requires native menu/render staging')
    channel=root/'native-media.bin'
    if metadata.get('media_channel')!=str(channel) or channel.is_symlink() or channel.stat().st_size!=media_v1.SIZE:
        raise ValueError('Refusing redirected/invalid native media channel')
    data=channel.read_bytes()
    if not media_v1.valid_header(data,len(data)) or any(data[16:512]) or any(data[512:520]) or any(data[524:]):
        raise ValueError('Refusing stale native media channel')
    if struct.unpack_from('<I',data,520)[0]==0:raise ValueError('Native media broker has not started')
    environment['MNM_RENDER_MEDIA']='Z:'+str(channel).replace('/','\\')
