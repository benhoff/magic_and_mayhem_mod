"""Request a bounded EGL interval end only after an exact complete source marker."""
import time
from drawing_api_session import sha
from drawing_source_completion import wait_for_source


def poll_source_close(capture, expected, process, heartbeat):
    marker = capture / 'canvas-producers.done'
    if not marker.exists() or marker.stat().st_size < 32:
        return None
    # Reuse the existing exact marker admission; partial/corrupt/budget results
    # never create a physical-close request.
    completion = wait_for_source(marker, expected, process, heartbeat)
    raw = marker.read_bytes()
    close = capture / 'egl-physical-close.flag'
    with close.open('xb') as out:
        out.write(b'producer-completion-observed\n')
    return dict(source_completion=completion, source_marker_sha256=sha(raw),
                requested_boundary='producer-completion-observed',
                request_time_ns=time.monotonic_ns(), clock_domain='linux_CLOCK_MONOTONIC',
                full_source_or_physical_ownership_proved=False)
