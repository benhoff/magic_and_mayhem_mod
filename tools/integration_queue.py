"""Cooperative integration admission; never holds Git's index.lock."""
from contextlib import contextmanager
import contextvars
import fcntl
import os
from pathlib import Path
import stat
import subprocess
import time

_current = contextvars.ContextVar('integration_lease', default=None)
FD_VARIABLE = 'MM_INTEGRATION_LOCK_FD'


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args], stderr=subprocess.PIPE)


def lock_path(root):
    root = Path(root).resolve()
    common = Path(os.fsdecode(git(root, 'rev-parse', '--git-common-dir')).strip())
    common = (common if common.is_absolute() else root/common).resolve()
    if 'original' in root.parts or 'original' in common.parts:
        raise ValueError('Integration admission must not write original inputs')
    return common/'mm-integration.lock'


def status(root):
    path = lock_path(root)
    result = dict(lock=str(path), dry_run=True, busy=False,
                  master=git(root, 'rev-parse', '--verify', 'master^{commit}').decode().strip())
    if not path.exists() and not path.is_symlink():
        return result
    fd = os.open(path, os.O_RDONLY | os.O_NOFOLLOW)
    try:
        try:
            fcntl.flock(fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
            fcntl.flock(fd, fcntl.LOCK_UN)
        except BlockingIOError:
            result['busy'] = True
    finally:
        os.close(fd)
    return result


class Lease:
    def __init__(self, root, fd, wait_seconds):
        self.root, self.fd, self.wait_seconds = Path(root).resolve(), fd, wait_seconds

    def run(self, command, **kwargs):
        """Propagate admission to child Python operators/finish without deadlock."""
        env = dict(kwargs.pop('env', os.environ), **{FD_VARIABLE: str(self.fd)})
        descriptors = tuple(dict.fromkeys((*kwargs.pop('pass_fds', ()), self.fd)))
        return subprocess.run(command, env=env, pass_fds=descriptors, **kwargs)


def require_lease(root):
    """Check admission without acquiring it, including from Git reference hooks."""
    path = lock_path(root)
    try:
        lease = _current.get()
        fd = lease.fd if lease else int(os.environ[FD_VARIABLE])
        value, expected = os.fstat(fd), path.stat()
        if (not stat.S_ISREG(value.st_mode) or value.st_nlink != 1
                or (value.st_dev, value.st_ino) != (expected.st_dev, expected.st_ino)):
            raise ValueError('Integration descriptor names a different lock')
        # A fresh open-file-description must be blocked by the inherited lease.
        probe = os.open(path, os.O_RDONLY | os.O_NOFOLLOW)
        try:
            try:
                fcntl.flock(probe, fcntl.LOCK_EX | fcntl.LOCK_NB)
            except BlockingIOError:
                pass
            else:
                raise ValueError('Integration descriptor has no active admission')
        finally:
            os.close(probe)
        # This must be the description holding that admission, not an arbitrary
        # descriptor opened while another integrator owns the lock.
        fcntl.flock(fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
        return fd
    except (KeyError, OSError, ValueError) as error:
        raise ValueError('Master update requires integration admission. Use tools/integrate-work.py '
                         'or tools/with-integration-lock.py --execute -- <whole integration command>. '
                         'Preserve the current work; do not disable hooks.') from error


def run_with_lease(command, **kwargs):
    lease = _current.get()
    if lease is not None:
        return lease.run(command, **kwargs)
    if os.environ.get(FD_VARIABLE):
        root = subprocess.check_output(['git', 'rev-parse', '--show-toplevel'], text=True).strip()
        fd = require_lease(root)
        return Lease(root, fd, 0).run(command, **kwargs)
    return subprocess.run(command, **kwargs)


@contextmanager
def integration_lock(root, *, expected_master=None):
    path = lock_path(root)
    existing = _current.get()
    inherited = os.environ.get(FD_VARIABLE)
    own = existing is None and inherited is None
    if existing is not None:
        fd = existing.fd
    elif inherited is not None:
        try:
            fd = int(inherited)
            os.fstat(fd)
        except (ValueError, OSError) as error:
            raise ValueError('Integration lock was not inherited; use Lease.run/pass_fds') from error
    else:
        fd = os.open(path, os.O_CREAT | os.O_RDWR | os.O_NOFOLLOW, 0o600)
    token = None
    try:
        value, expected = os.fstat(fd), path.stat()
        if (not stat.S_ISREG(value.st_mode) or value.st_nlink != 1
                or (value.st_dev, value.st_ino) != (expected.st_dev, expected.st_ino)):
            raise ValueError('Integration descriptor does not name this repository lock')
        started = time.monotonic()
        if own:
            fcntl.flock(fd, fcntl.LOCK_EX)
        else:
            # A descriptor passed by the wrapper owns this open-file-description
            # lock. Re-locking it is nonblocking and also detects an unrelated lock.
            fcntl.flock(fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
        actual = git(root, 'rev-parse', '--verify', 'master^{commit}').decode().strip()
        if expected_master is not None:
            wanted = git(root, 'rev-parse', '--verify', '--end-of-options', expected_master+'^{commit}').decode().strip()
            if wanted != actual:
                raise ValueError('Master changed before integration admission; prepare against '+actual)
        lease = Lease(root, fd, time.monotonic()-started)
        token = _current.set(lease)
        yield lease
    finally:
        if token is not None:
            _current.reset(token)
        if own:
            os.close(fd)


@contextmanager
def finish_lock(root):
    value = subprocess.run(['git', '-C', str(root), 'symbolic-ref', '--quiet', '--short', 'HEAD'], capture_output=True)
    if value.returncode not in (0, 1):
        raise ValueError('Cannot determine finish branch')
    branch = value.stdout.decode().strip()
    if branch == 'master' or _current.get() is not None or os.environ.get(FD_VARIABLE):
        with integration_lock(root) as lease:
            yield lease
    else:
        yield None
