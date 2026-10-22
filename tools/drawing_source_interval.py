"""Request a bounded EGL interval end only after an exact complete source marker."""
import time
from drawing_api_session import sha
from drawing_source_completion import decode_completion
from drawing_observed_inputs import regular_bytes


def poll_source_close(capture, expected, process, heartbeat):
    marker = capture / 'canvas-producers.done'
    try:
        raw = regular_bytes(marker, 32)
    except FileNotFoundError:
        return None
    if len(raw) < 32:
        return None
    # Validate and hash the same bounded snapshot. A second read could bind
    # the close request to bytes different from those actually admitted.
    completion = decode_completion(raw, expected)
    close = capture / 'egl-physical-close.flag'
    with close.open('xb') as out:
        out.write(b'producer-completion-observed\n')
    return dict(source_completion=completion, source_marker_sha256=sha(raw),
                requested_boundary='producer-completion-observed',
                request_time_ns=time.monotonic_ns(), clock_domain='linux_CLOCK_MONOTONIC',
                full_source_or_physical_ownership_proved=False)
