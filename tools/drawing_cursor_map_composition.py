"""Reference-only cursor ROI model; source pixels never enter the native app."""
import struct
from pathlib import Path
from drawing_api_session import sha


def packed_rgb(word):
    red=(word>>11)&31;green=(word>>5)&63;blue=word&31
    return ((red<<3)|(red>>2),(green<<2)|(green>>4),(blue<<3)|(blue>>2))


def read_bound(directory,name,digest,size):
    relative=Path(name)
    if relative.is_absolute() or '..' in relative.parts or len(relative.parts)!=1:
        raise ValueError('Unsafe cursor model input path')
    path=directory/relative
    if path.is_symlink():raise ValueError('Cursor model input symlink')
    raw=path.read_bytes()
    if len(raw)!=size or sha(raw)!=digest:raise ValueError('Cursor model input size/hash')
    return raw


def evaluate(calls,samples,payloads,frames,directory):
    ids=[c['call'] for c in calls]
    if ids!=sorted(set(ids)):raise ValueError('Cursor call order/identity')
    sample_by={s['submission']:s for s in samples if s['status']==0}
    payload_by={p['ordinal']:p for p in payloads}
    frame_by={f['entry']:f for f in frames}
    if len(frame_by)!=len(frames):raise ValueError('Duplicate cursor physical frame entry')
    matches=[];refusals=[];selected=0
    for restore in calls:
        background=sample_by.get(restore['call'])
        if background is None or background['shape']!=1 or restore['flags']&0x18000:continue
        selected+=1
        def refuse(reason):refusals.append(dict(restore=restore['call'],reason=reason))
        if restore['method']!=1 or restore['hresult']&0x80000000 or not restore['first_generation'] or not restore['second_generation']:
            refuse('untrusted_restore');continue
        writers=[c for c in calls if c['call']<restore['call'] and c['method']==1 and c['first_object']==restore['second_object'] and c['first_generation']==restore['second_generation'] and not c['hresult']&0x80000000]
        if not writers:refuse('missing_save');continue
        save=writers[-1]
        if save['second_object']!=restore['first_object'] or save['second_generation']!=restore['first_generation'] or save['source_rect']!=restore['destination_rect'] or save['destination_rect']!=restore['source_rect'] or save['flags']&0x18000 or save['thread']!=restore['thread']:
            refuse('latest_writer_does_not_bind_save');continue
        between=[c for c in calls if save['call']<c['call']<restore['call']]
        draws=[c for c in between if c['method']==1 and c['first_object']==restore['first_object'] and c['first_generation']==restore['first_generation'] and c['destination_rect']==restore['destination_rect'] and c['flags']&0x8000 and not c['flags']&0x10000 and not c['hresult']&0x80000000]
        if len(draws)!=1:refuse('missing_or_ambiguous_keyed_draw');continue
        draw=draws[0];overlay=sample_by.get(draw['call'])
        if overlay is None:refuse('missing_sampled_overlay');continue
        if draw['thread']!=restore['thread'] or overlay['source_texture']!=draw['second_object'] or overlay['source_generation']!=draw['second_generation'] or background['source_texture']!=restore['second_object'] or background['source_generation']!=restore['second_generation']:
            refuse('sample_generation_or_thread');continue
        presents=[c for c in between if c['method']==1 and c['call']>draw['call'] and c['second_object']==restore['first_object'] and c['second_generation']==restore['first_generation'] and not c['hresult']&0x80000000 and c['api_entry'] in frame_by]
        if not presents:refuse('missing_physical_present');continue
        present=presents[0];frame=frame_by[present['api_entry']]
        if present['thread']!=restore['thread'] or frame['thread']!=restore['thread']:
            refuse('physical_thread');continue
        rect=restore['source_rect'];destination=draw['destination_rect'];source=draw['source_rect']
        if rect!=[0,0,40,40] or not source or not destination or len(source)!=4 or len(destination)!=4:
            refuse('unsupported_geometry');continue
        x,y,right,bottom=destination;sx,sy,sright,sbottom=source
        p=payload_by[overlay['payload_ordinal']];bg=payload_by[background['payload_ordinal']]
        if (right-x,bottom-y,sright-sx,sbottom-sy)!=(40,40,40,40) or not 0<=x<right<=frame['width'] or not 0<=y<bottom<=frame['height'] or not 0<=sx<sright<=p['width'] or not 0<=sy<sbottom<=p['height'] or (bg['width'],bg['height'])!=(40,40):
            refuse('unsupported_or_clipped_geometry');continue
        front=read_bound(directory,frame['path'],frame['sha256'],frame['width']*frame['height']*4)
        image=read_bound(directory,p['path'],p['sha256'],p['width']*p['height']*2)
        backing=read_bound(directory,bg['path'],bg['sha256'],3200)
        maximum=over_one=0
        for row in range(40):
            for col in range(40):
                value=struct.unpack_from('<H',image,2*((sy+row)*p['width']+sx+col))[0]
                if value==31:value=struct.unpack_from('<H',backing,2*(row*40+col))[0]
                expected=packed_rgb(value);at=4*((y+row)*frame['width']+x+col)
                difference=max(abs(expected[k]-front[at+k]) for k in range(3))
                maximum=max(maximum,difference);over_one+=difference>1
        matches.append(dict(save=save['call'],draw=draw['call'],present=present['call'],restore=restore['call'],frame_entry=frame['entry'],frame_ordinal=frame['ordinal'],background_payload=bg['ordinal'],overlay_payload=p['ordinal'],pixels=1600,max_channel_difference=maximum,pixels_over_one=over_one,pixels_match=over_one==0,physical_frame_sha256=frame['sha256'],background_sha256=bg['sha256'],overlay_sha256=p['sha256']))
    matched=[m for m in matches if m['pixels_match']]
    return dict(schema=1,success=True,selected_restore_samples=selected,candidates=matches,refusals=refusals,matched_cycles=len(matched),mismatched_cycles=len(matches)-len(matched),matched_pixels=sum(m['pixels'] for m in matched),all_selected_cycles_bound=bool(matches) and not refusals,all_candidate_pixels_match=bool(matches) and len(matched)==len(matches),inferred_source_key=31,channel_tolerance=1,original_pixels_used_as_diagnostic_model_inputs=True,original_pixels_used_as_native_application_inputs=False,native_renderer_parity_performed=False,complete_presented_journey=False)
