"""A repository-wide bound on concurrently executing snapshot workers."""
from contextlib import contextmanager
from contextvars import ContextVar
import fcntl
import os
from pathlib import Path
import subprocess
import time

DIRECTORY = 'MM_VERIFICATION_DIRECTORY'
_current = ContextVar('verification_budget', default=None)
LIMIT = 4


def directory(root):
    if os.environ.get(DIRECTORY):
        return Path(os.environ[DIRECTORY]).resolve()
    result = subprocess.run(['git', '-C', str(root), 'rev-parse', '--git-common-dir'],
                            capture_output=True, text=True)
    if result.returncode == 0:
        common = Path(result.stdout.strip())
        return ((common if common.is_absolute() else Path(root)/common).resolve()
                / 'mm-verification-slots')
    return Path(root).resolve()/'working/coverage/verification-slots'


@contextmanager
def workers(root, requested=1):
    """Wait for one slot, then borrow available extras without holding a queue.

    A nested prefetch borrows additional slots around its thread pool. The outer
    snapshot keeps one slot for sequential audit work. Kernel locks release on
    cancellation/process exit; no stale PID records or cached decisions exist.
    """
    if type(requested) is not int or not 1 <= requested <= LIMIT:
        raise ValueError('Verification worker request must be from one to four')
    path = directory(root)
    if 'original' in path.parts:
        raise ValueError('Verification slots must not use original inputs')
    path.mkdir(parents=True, exist_ok=True)
    parent = _current.get()
    held = parent[1] if parent and parent[0] == path else 0
    if held >= requested:
        yield requested
        return
    acquired = []
    token = None
    try:
        while True:
            for index in range(LIMIT):
                fd = os.open(path/str(index), os.O_CREAT | os.O_RDWR | os.O_NOFOLLOW, 0o600)
                try:
                    fcntl.flock(fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
                except BlockingIOError:
                    os.close(fd)
                    continue
                except BaseException:
                    os.close(fd)
                    raise
                acquired.append(fd)
                if held + len(acquired) >= requested:
                    break
            if held or acquired:
                break
            time.sleep(0.05)
        count = held + len(acquired)
        token = _current.set((path, count))
        yield min(requested, count)
    finally:
        if token is not None:
            _current.reset(token)
        for fd in acquired:
            os.close(fd)
