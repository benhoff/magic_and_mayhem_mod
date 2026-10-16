"""Comparator-only default framebuffer witnesses, captured before the real swap."""
from hashlib import sha256
from pathlib import Path
import struct
import numpy as np


def decode(raw):
    if len(raw)!=128 or raw[:8]!=b'MNMEDB01':raise ValueError('EGL destination extent/magic')
    r=struct.unpack('<32I',raw)
    begin,end=r[22]|r[23]<<32,r[24]|r[25]<<32
    if (r[2:4]!=(1,128) or not all(r[4:9]) or max(r[7:9])>2048 or r[9] or
        r[13] not in (0x405,0x402,0x404,0x400) or r[14]!=r[7]*r[8]*2 or r[15]!=1 or
        r[16] not in (1,2,4,8) or (r[7]*2)%r[16] or any(r[17:22]) or
        not begin or end<begin or r[26:28]!=(1,1) or any(r[28:])):
        raise ValueError('EGL destination identity/state')
    return dict(call=r[4],pid=r[5],thread=r[6],width=r[7],height=r[8],draw_fbo=r[9],
                prior_read_fbo=r[10],prior_read_buffer=r[11],default_read_buffer=r[12],
                captured_buffer=r[13],bytes=r[14],restored=True,pack_alignment=r[16],
                begin_ns=begin,end_ns=end,context_unchanged=True,before_real_swap=True,
                origin='gl_lower_left',original_oracle_only=True)


def attach(directory,row):
    directory=Path(directory);name=f"egl-destination-before-swap-{row['call']:06d}"
    rebind=directory/f"egl-drawable-rebind-{row['call']:06d}.bin"
    if rebind.exists():
        raw=rebind.read_bytes();value=decode_rebind(raw)
        if any(value[k]!=row[k] for k in ('call','pid','thread')):raise ValueError('EGL rebind identity differs')
        row['drawable_rebind']=dict(**value,path=rebind.name,sha256=sha256(raw).hexdigest())
    metadata=directory/(name+'.bin')
    packets=[]
    for packet in sorted(directory.glob(f"egl-xcb-image-{row['call']:06d}-*.bin")):
        raw=packet.read_bytes();v=decode_xcb(raw)
        if packet.name!=f"egl-xcb-image-{row['call']:06d}-{v['packet']:04d}.bin" or v['packet']!=len(packets)+1 or any(v[k]!=row[k] for k in ('call','pid','thread','width','height')) or v['end_ns']>row['begin_ns']:raise ValueError('EGL XCB packet/frame differs')
        packets.append(dict(**v,path=packet.name,sha256=sha256(raw).hexdigest()))
    row['xcb_images']=packets
    if not metadata.exists():return
    if metadata.is_symlink():raise ValueError('EGL destination metadata symlink')
    raw=metadata.read_bytes();value=decode(raw)
    if any(value[k]!=row[k] for k in ('call','pid','thread','width','height')) or value['end_ns']>row['begin_ns']:
        raise ValueError('EGL destination frame/completion differs')
    path=directory/(name+'.565')
    if path.is_symlink():raise ValueError('EGL destination pixel symlink')
    data=path.read_bytes()
    if len(data)!=value['bytes']:raise ValueError('EGL destination pixel extent')
    if any(p['begin_ns']<value['end_ns'] for p in packets):raise ValueError('XCB packet precedes destination witness')
    row['gpu_destination']=dict(**value,path=metadata.name,sha256=sha256(raw).hexdigest(),
                                pixels_path=path.name,pixels_sha256=sha256(data).hexdigest())


def plane(directory,row):
    value=row['gpu_destination'];name=f"egl-destination-before-swap-{row['call']:06d}"
    for key,expected in [('path',name+'.bin'),('pixels_path',name+'.565')]:
        if value[key]!=expected or (Path(directory)/value[key]).is_symlink():raise ValueError('Unsafe EGL destination oracle path')
    raw=(Path(directory)/value['path']).read_bytes()
    decoded=decode(raw)
    if sha256(raw).hexdigest()!=value['sha256'] or any(value[k]!=v for k,v in decoded.items()):raise ValueError('EGL destination metadata changed')
    if any(decoded[k]!=row[k] for k in ('call','pid','thread','width','height')):raise ValueError('EGL destination identity changed')
    data=(Path(directory)/value['pixels_path']).read_bytes()
    if len(data)!=value['bytes'] or sha256(data).hexdigest()!=value['pixels_sha256']:raise ValueError('EGL destination oracle changed')
    return np.frombuffer(data,dtype='<u2').reshape(value['height'],value['width'])[::-1].copy()


def compare_frame(original,directory,ordinal,native,physical,width,height):
    rows=original.get('egl_swap_completion_observation',{}).get('frames',[])
    if not rows or 'gpu_destination' not in rows[ordinal-1]:return None
    row=rows[ordinal-1]
    if row['ordinal']!=ordinal or (row['width'],row['height'])!=(width,height) or len(native)!=width*height*2 or len(physical)!=width*height*4:
        raise ValueError('EGL destination comparison identity/extent')
    destination=plane(directory,row);native_words=np.frombuffer(native,dtype='<u2').reshape(height,width)
    rgba=np.frombuffer(physical,dtype=np.uint8).reshape(height,width,4).astype(np.int16)
    rgb=np.stack([((destination>>11)*255//31),(((destination>>5)&63)*255//63),((destination&31)*255//31)],axis=2).astype(np.int16)
    source_bad=None
    if 'gpu_source' in row:
        from drawing_egl_gpu_reference import compare
        compare(original,directory,ordinal,native,width,height)  # Validate independent source projection/hash.
        source=np.frombuffer((Path(directory)/row['gpu_source']['path']).read_bytes(),dtype='<u2').reshape(height,width)
        source_bad=int(np.count_nonzero(source!=destination))
    return dict(pixels=width*height,native_destination_mismatching_words=int(np.count_nonzero(native_words!=destination)),
                source_destination_mismatching_words=source_bad,destination_physical_mismatching_pixels=int(np.count_nonzero(np.max(abs(rgb-rgba[:,:,:3]),axis=2)>1)),
                destination_top_down_sha256=sha256(destination.astype('<u2').tobytes()).hexdigest(),
                channel_tolerance=1,original_pixels_used_as_native_inputs=False,
                scope='Nominal EGL rectangle from default framebuffer before real swap; binding/readback is a diagnostic perturbation. No underlying allocation extent or error neutrality claim.')


def decode_rebind(raw):
    if len(raw)!=96 or raw[:8]!=b'MNMERB01':raise ValueError('EGL rebind extent/magic')
    r=struct.unpack('<24I',raw)
    version=r[2]
    mode=1 if version==1 else r[15]
    if (version not in (1,2) or r[3]!=96 or not all(r[4:7]) or r[7]>16384 or
        mode not in (1,2) or (version==1 and any(r[15:])) or (version==2 and (r[16]!=int(bool(r[7]) and mode==2) or any(r[17:]))) or
        (not r[7] and any(r[8:15])) or (r[7] and (r[8] not in (1,2) or not all(r[11:13]) or max(r[9:13])>2048 or r[13:15]!=(1,1) or (r[8]==2 and (not all(r[9:11]) or r[9:11]==r[11:13]))))):
        raise ValueError('EGL rebind state/result')
    return dict(call=r[4],pid=r[5],thread=r[6],count=r[7],reason=r[8],old_width=r[9],old_height=r[10],width=r[11],height=r[12],applied=bool(r[7]),state_preserved=bool(r[13]) if r[7] else None,success=bool(r[14]) if r[7] else None,policy_mode=mode,context_released=bool(r[7]) and mode==2)


def decode_xcb(raw):
    if len(raw) not in (128,224,240) or raw[:8]!=b'MNMEXI01':raise ValueError('EGL XCB extent/magic')
    r=struct.unpack('<'+str(len(raw)//4)+'I',raw);begin,end=r[28]|r[29]<<32,r[30]|r[31]<<32
    dx,dy=struct.unpack_from('<2i',raw,72)
    if (r[2:4] not in ((1,128),(2,224),(3,240)) or r[3]!=len(raw) or not all(r[4:8]) or r[7]>1024 or r[8] not in (1,2,3,4) or not all(r[9:11]) or max(r[9:11])>2048 or not r[11] or max(r[12:18])>65535 or not -32768<=dx<=32767 or not -32768<=dy<=32767 or max(r[20:24])>255 or not begin or end<begin):raise ValueError('EGL XCB identity/packet')
    if len(raw)==224 and (max(r[32:34])>2048 or any(r[36:40]) or r[40:56]!=r[11:27]):raise ValueError('EGL XCB original/forwarded metadata differs')
    if len(raw)==240:
        original=list(r[40:56]);effective=list(r[11:27]);expected=list(original)
        if r[36]>1 or r[58]>1 or r[59]>1 or any(r[38:40]) or max(r[32:34])>2048 or max(r[56:58])>65535:raise ValueError('EGL XCB crop metadata')
        if r[36]:
            if (not r[59] or not r[58] or not r[34] or not r[35] or original[0]!=r[34] or not r[9] or not r[10] or r[9]>r[32] or r[10]>=r[33] or original[1:3]!=list(r[56:58]) or original[1]<r[32] or original[2]<r[33] or any(original[3:5]) or original[5:7]!=original[1:3] or any(original[7:9]) or original[9:14]!=[24,2,0,0,0] or not original[14] or r[37]!=r[33]-r[10]):raise ValueError('Unsupported EGL crop admission')
            expected[4:7]=[r[33]-r[10],r[9],r[10]]
        elif r[37]:raise ValueError('Unapplied EGL crop delta')
        if effective!=expected:raise ValueError('EGL crop forwarded packet differs')
    return dict(resize_crop_applied=bool(r[36]) if len(raw)==240 else False,resize_crop_enabled=bool(r[59]) if len(raw)==240 else False,previous_image_width=r[56] if len(raw)==240 else None,previous_image_height=r[57] if len(raw)==240 else None,original_packet=list(r[40:56]) if len(raw)>=224 else None,previous_width=r[32] if len(raw)>=224 else None,previous_height=r[33] if len(raw)>=224 else None,window=r[34] if len(raw)>=224 else None,window_generation=r[35] if len(raw)>=224 else None,call=r[4],pid=r[5],thread=r[6],packet=r[7],method=r[8],width=r[9],height=r[10],drawable=r[11],total_width=r[12],total_height=r[13],source_x=r[14],source_y=r[15],source_width=r[16],source_height=r[17],destination_x=dx,destination_y=dy,depth=r[20],format=r[21],send_event=r[22],left_pad=r[23],data_bytes=r[24],shm_segment=r[25],shm_offset=r[26],cookie_sequence=r[27],begin_ns=begin,end_ns=end)
