"""Reference-only agreement between owned source pixels and physical copy regions."""
import numpy as np
from drawing_api_session import sha
from drawing_observed_inputs import bound_bytes, image_size, unique


def bound(directory, item, size):
    raw=bound_bytes(directory,item['path'],size)
    if len(raw)!=size or sha(raw)!=item['sha256']:raise ValueError('Binding payload/frame size or hash')
    return raw


def evaluate(calls,samples,payloads,frames,directory):
    unique(calls,'call')
    ids=[c['call'] for c in calls]
    if ids!=sorted(set(ids)):raise ValueError('Binding call order')
    sample_by=unique(samples,'submission');payload_by=unique(payloads,'ordinal');unique(frames,'entry')
    groups={}
    for c in calls:
        if c['method']==1:groups.setdefault(c['api_entry'],[]).append(c)
    comparisons=[];refusals=[]
    for frame in frames:
        entry=frame['entry']
        def refuse(reason):refusals.append(dict(frame_entry=entry,frame_ordinal=frame['ordinal'],reason=reason))
        group=groups.get(entry)
        if not group:refuse('missing_observed_source_blit');continue
        c=group[-1];sample=sample_by.get(c['call'])
        if c['hresult']&0x80000000 or c['flags'] not in (0,0x41000000):refuse('failed_or_unsupported_copy');continue
        if sample is None or sample['status']!=0:refuse('source_pixels_not_retained');continue
        if (sample['api_entry'],sample['thread'],sample['source_texture'],sample['source_generation'])!=(entry,c['thread'],c['second_object'],c['second_generation']) or c['thread']!=frame['thread'] or not c['first_generation'] or not c['second_generation'] or sample['lifetime_before']<c['lifetime_prefix']:
            refuse('source_frame_identity');continue
        source,destination=c['source_rect'],c['destination_rect'];p=payload_by.get(sample['payload_ordinal'])
        if p is None or not source or not destination or len(source)!=4 or len(destination)!=4:refuse('missing_geometry_or_payload');continue
        if any(type(v) is not int for v in source+destination):refuse('unsupported_scaled_or_clipped_geometry');continue
        image_size(p,2);image_size(frame,4)
        sx,sy,sr,sb=source;x,y,right,bottom=destination
        if not 0<=sx<sr<=p['width'] or not 0<=sy<sb<=p['height'] or not 0<=x<right<=frame['width'] or not 0<=y<bottom<=frame['height'] or (sr-sx,sb-sy)!=(right-x,bottom-y) or (p['width'],p['height'])!=tuple(sample['descriptor'][6:8]):
            refuse('unsupported_scaled_or_clipped_geometry');continue
        pixels=np.frombuffer(bound(directory,p,image_size(p,2)),dtype='<u2').reshape(p['height'],p['width'])[sy:sb,sx:sr]
        rgba=np.frombuffer(bound(directory,frame,image_size(frame,4)),dtype=np.uint8).reshape(frame['height'],frame['width'],4)[y:bottom,x:right,:3]
        red=(pixels>>11)&31;green=(pixels>>5)&63;blue=pixels&31
        expected=np.stack(((red<<3)|(red>>2),(green<<2)|(green>>4),(blue<<3)|(blue>>2)),axis=2).astype(np.int16)
        difference=np.max(np.abs(expected-rgba.astype(np.int16)),axis=2)
        over=int(np.count_nonzero(difference>1));maximum=int(difference.max());count=(right-x)*(bottom-y)
        comparisons.append(dict(frame_entry=entry,frame_ordinal=frame['ordinal'],submission=c['call'],source_texture=c['second_object'],source_generation=c['second_generation'],destination_texture=c['first_object'],destination_generation=c['first_generation'],source_rect=source,destination_rect=destination,payload_ordinal=p['ordinal'],source_sha256=p['sha256'],physical_sha256=frame['sha256'],pixels=count,max_channel_difference=maximum,pixels_over_one=over,pixels_match=over==0))
    matched=[c for c in comparisons if c['pixels_match']]
    return dict(schema=1,success=True,physical_witnesses=len(frames),compared_regions=len(comparisons),matched_regions=len(matched),mismatched_regions=len(comparisons)-len(matched),matched_pixels=sum(c['pixels'] for c in matched),comparisons=comparisons,refusals=refusals,all_witness_copy_regions_bound=bool(frames) and not refusals,all_compared_regions_match=bool(comparisons) and len(matched)==len(comparisons),channel_tolerance=1,alpha_compared=False,numpy_version=np.__version__,original_pixels_used_as_diagnostic_model_inputs=True,original_pixels_used_as_native_application_inputs=False,native_renderer_parity_performed=False,complete_presented_journey=False)
