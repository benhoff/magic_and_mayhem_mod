"""Exact observed command association and reference-only post-EGL copy agreement."""
import numpy as np
from drawing_source_frame_binding import evaluate, unique, bound
from drawing_delivery_timing import capture_span


def whole_image_agreement(payload, frame, directory):
    if (payload['width'], payload['height']) != (frame['width'], frame['height']):
        return dict(compared=False, reason='source_and_physical_extents_differ')
    width, height = payload['width'], payload['height']
    pixels = np.frombuffer(bound(directory, payload, width*height*2), dtype='<u2').reshape(height, width)
    actual = np.frombuffer(bound(directory, frame, width*height*4), dtype=np.uint8).reshape(height, width, 4)[:, :, :3]
    red, green, blue = (pixels>>11)&31, (pixels>>5)&63, pixels&31
    expected = np.stack(((red<<3)|(red>>2), (green<<2)|(green>>4), (blue<<3)|(blue>>2)), axis=2).astype(np.int16)
    diff = np.max(np.abs(expected-actual.astype(np.int16)), axis=2)
    over = int(np.count_nonzero(diff>1))
    return dict(compared=True, pixels=width*height, pixels_over_one=over, max_channel_difference=int(diff.max()), pixels_match=over==0)


def delivery_boundary(api, egl, timing, context):
    keys = ('frame_entry', 'submission', 'source_texture', 'source_generation', 'destination_texture', 'destination_generation', 'source_rect', 'destination_rect', 'payload_ordinal', 'source_sha256')
    if any(api[k] != egl[k] for k in keys) or (timing['entry'], timing['ordinal'], timing['sha256']) != (api['frame_entry'], api['frame_ordinal'], api['physical_sha256']):
        raise ValueError('API/EGL source and physical identity differ')
    if (context['pid'], context['call']) != (egl['egl_pid'], egl['egl_call']) or not context['stable_active_context']:
        raise ValueError('Boundary context identity')
    if timing['clock_domain'] != 'linux_CLOCK_MONOTONIC':
        raise ValueError('Boundary clock domain')
    first, second = capture_span(timing['first_capture']), capture_span(timing['second_capture'])
    start, end = [p['time_ns'] for p in context['points']]
    if first[1] > second[0] or start > end:
        raise ValueError('Boundary capture order')
    before = second[1] < start
    return dict(submission=egl['submission'], api_entry=api['frame_entry'], api_physical_ordinal=api['frame_ordinal'], egl_pid=egl['egl_pid'], egl_call=egl['egl_call'], egl_physical_ordinal=egl['physical_ordinal'],
                source_sha256=egl['source_sha256'], api_physical_sha256=api['physical_sha256'], egl_physical_sha256=egl['physical_sha256'],
                api_region_matches=api['pixels_match'], egl_region_matches=egl['pixels_match'], api_second_capture_end_ns=second[1], egl_entry_ns=start, egl_return_ns=end,
                api_capture_before_egl_entry=before, api_mismatch_before_matching_egl=before and not api['pixels_match'] and egl['pixels_match'], clock_domain='linux_CLOCK_MONOTONIC')


def bind(calls, samples, payloads, packets, workers, contexts, frames, directory, api_calls):
    by_call = unique(calls, 'call')
    by_sample = unique(samples, 'submission')
    by_payload = unique(payloads, 'ordinal')
    by_worker = unique(workers, 'call')
    unique(packets, 'command')
    unique(packets, 'worker')
    by_submission = unique(packets, 'submission')
    by_api = unique(api_calls, 'entry')
    by_context = {(c['pid'], c['call']): c for c in contexts}
    if len(by_context) != len(contexts):
        raise ValueError('Ambiguous EGL context identity')
    if len({(f['pid'], f['entry']) for f in frames}) != len(frames):
        raise ValueError('Ambiguous EGL physical identity')
    source_links = []
    source_refusals = []
    for sample in samples:
        sid = sample['submission']
        call, packet = by_call.get(sid), by_submission.get(sid)
        worker = by_worker.get(packet['worker']) if packet else None
        api = by_api.get(sample['api_entry'])
        reason = None
        if call is None or packet is None or worker is None:
            reason = 'missing_submission_packet_or_worker'
        elif sample['status'] != 0:
            reason = 'source_pixels_not_retained'
        elif api is None or api['thread'] != call['thread']:
            reason = 'missing_or_changed_enclosing_api'
        elif (sample['api_entry'], sample['thread'], sample['source_texture'], sample['source_generation']) != (call['api_entry'], call['thread'], call['second_object'], call['second_generation']):
            reason = 'source_submission_identity'
        elif (packet['emit_thread'], packet['worker_thread']) != (call['thread'], worker['thread']) or not packet['emit_record'] < packet['worker_entry_record'] < packet['worker_return_record']:
            reason = 'packet_worker_identity_or_order'
        elif not call['lifetime_prefix'] <= sample['lifetime_before'] <= sample['lifetime_after'] <= packet['lifetime_prefix'] <= worker['lifetime_prefix']:
            reason = 'source_packet_worker_lifetime_order'
        elif not call['first_generation'] or not call['second_generation'] or (worker['source'], worker['source_generation'], worker['destination'], worker['destination_generation']) != (call['second_object'], call['second_generation'], call['first_object'], call['first_generation']):
            reason = 'worker_texture_generation'
        elif (sample['subresource'], worker['source_subresource'], worker['destination_subresource']) != (call['source_subresource'], call['source_subresource'], call['destination_subresource']):
            reason = 'worker_subresource'
        elif worker['source_box'] != call['source_rect'] + [0, 1] or worker['destination_box'] != call['destination_rect'] + [0, 1] or worker['flags'] != call['flags']:
            reason = 'worker_geometry_or_flags'
        if reason:
            source_refusals.append(dict(submission=sid, reason=reason))
        else:
            source_links.append(dict(submission=sid, worker=worker['call'], command=packet['command'], payload_ordinal=sample['payload_ordinal'], api_entry=api['entry'], api_caller=api['caller'], api_method=api['method'], api_hresult=api['hresult'], api_last_error=api['last_error']))
    linked = unique(source_links, 'submission')
    comparisons = []
    refusals = []
    for frame in frames:
        identity = dict(egl_pid=frame['pid'], egl_call=frame['entry'], physical_ordinal=frame['ordinal'])
        context = by_context.get((frame['pid'], frame['entry']))
        reason = None
        if context is None or not context['stable_active_context']:
            reason = 'missing_unstable_or_inactive_worker_context'
        else:
            before, after = context['points']
            value = before['context']
            link = linked.get(value['submission'])
            worker = by_worker.get(value['worker'])
            if before['status'] or after['status'] or value != after['context'] or not value['active'] or value['flags']:
                reason = 'unavailable_changed_or_unknown_context'
            elif before['pid'] != frame['pid'] or after['pid'] != frame['pid'] or before['unix_thread'] != frame['thread'] or after['unix_thread'] != frame['thread']:
                reason = 'physical_context_process_or_thread'
            elif link is None:
                reason = 'source_worker_binding_unavailable'
            elif (link['worker'], link['command']) != (value['worker'], value['command']) or worker is None or (worker['thread'], worker['lifetime_prefix']) != (value['win_thread'], value['lifetime_prefix']):
                reason = 'active_context_packet_worker_identity'
            elif worker['filter'] != 1 or any(worker['effects']):
                reason = 'unsupported_worker_filter_or_effects'
        if reason:
            refusals.append(dict(**identity, reason=reason))
            continue
        call = by_call[value['submission']]
        # The command identity selects the submission. Only its observed source
        # copy region is compared; the physical Win/Unix thread IDs stay separate.
        region_frame = dict(frame, entry=call['api_entry'], thread=call['thread'])
        result = evaluate([call], [by_sample[call['call']]], payloads, [region_frame], directory)
        if result['refusals']:
            refusals.append(dict(**identity, reason=result['refusals'][0]['reason']))
            continue
        agreement = whole_image_agreement(by_payload[link['payload_ordinal']], frame, directory)
        comparisons.append(dict(**result['comparisons'][0], **identity, worker=value['worker'], command=value['command'], win_thread=value['win_thread'], unix_thread=frame['thread'], api_caller=link['api_caller'], api_method=link['api_method'], api_hresult=link['api_hresult'], api_last_error=link['api_last_error'], whole_source_image_agreement=agreement))
    witnessed = {(f['pid'], f['entry']) for f in frames}
    unwitnessed = [dict(egl_pid=c['pid'], egl_call=c['call'], stable_active_context=c['stable_active_context']) for c in contexts if (c['pid'], c['call']) not in witnessed]
    matched = [c for c in comparisons if c['pixels_match']]
    whole = [c['whole_source_image_agreement'] for c in comparisons]
    return dict(schema=1, success=True, source_samples=len(samples), source_worker_bindings=source_links, source_refusals=source_refusals,
                physical_witnesses=len(frames), compared_regions=len(comparisons), matched_regions=len(matched), mismatched_regions=len(comparisons)-len(matched), matched_pixels=sum(c['pixels'] for c in matched),
                comparisons=comparisons, refusals=refusals, contexts_without_physical_witness=unwitnessed,
                whole_source_images_compared=sum(c['compared'] for c in whole),
                whole_source_images_matched=sum(c.get('pixels_match', False) for c in whole),
                whole_source_image_pixels_matched=sum(c['pixels'] for c in whole if c.get('pixels_match')),
                all_physical_whole_images_agree=bool(frames) and not refusals and all(c.get('pixels_match') for c in whole),
                all_observed_sources_worker_bound=bool(samples) and not source_refusals,
                all_physical_copy_regions_bound=bool(frames) and not refusals,
                all_compared_regions_match=bool(comparisons) and len(matched)==len(comparisons),
                channel_tolerance=1, alpha_compared=False, numpy_version=np.__version__, original_pixels_used_as_native_application_inputs=False,
                original_source_atomicity_proved=False, all_alias_writers_accounted=False, scanout_proved=False,
                native_renderer_parity_performed=False, complete_presented_journey=False)
