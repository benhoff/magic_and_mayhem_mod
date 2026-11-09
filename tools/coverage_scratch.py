"""Capacity-checked temporary Git copies; never durable evidence storage."""
from contextlib import contextmanager
import os
from pathlib import Path
import subprocess
import tempfile
import time


def tree_requirements(root, revision=None):
    if revision is not None:
        rows = subprocess.check_output(['git', '-C', str(root), 'ls-tree', '-rlz', revision]).split(b'\0')
        entries = []
        for row in filter(None, rows):
            fields, name = row.split(b'\t', 1)
            mode, kind, _, size = fields.split()
            if kind == b'blob':
                entries.append((name, int(size)))
    else:
        rows = subprocess.check_output(['git', '-C', str(root), 'ls-files', '--stage', '-z']).split(b'\0')
        blobs = []
        for row in filter(None, rows):
            fields, name = row.split(b'\t', 1)
            mode, oid, stage = fields.split()
            if stage != b'0':
                raise ValueError('Resolve index conflicts before scratch preparation')
            blobs.append((name, oid))
        result = subprocess.run(['git', '-C', str(root), 'cat-file', '--batch-check=%(objectsize)'],
                                input=b''.join(oid+b'\n' for _, oid in blobs), capture_output=True, check=True)
        sizes = result.stdout.splitlines()
        if len(sizes) != len(blobs):
            raise ValueError('Incomplete Git scratch size inventory')
        entries = [(name, int(size)) for (name, _), size in zip(blobs, sizes)]
    entries = [(name, size) for name, size in entries
               if name.split(b'/', 1)[0] not in (b'original', b'working', b'.git')]
    # Include filesystem allocation rounding and directory/inode headroom.
    return sum(max(4096, (size+4095)//4096*4096) for _, size in entries)*12//10 + 16*1024**2, len(entries)*2+128


def check_capacity(path, size, inodes):
    value = os.statvfs(path)
    if value.f_bavail*value.f_frsize < size:
        raise ValueError('insufficient free bytes')
    if value.f_files and value.f_favail < inodes:
        raise ValueError('insufficient free inodes')
    # Also catches permissions and several per-user quota failures which a
    # filesystem-wide statvfs result alone cannot detect.
    with tempfile.TemporaryFile(dir=path) as probe:
        probe.write(b'scratch write probe\n')
        probe.flush()
        os.fsync(probe.fileno())


@contextmanager
def scratch(root, prefix, *, revision=None):
    started = time.perf_counter()
    size, inodes = tree_requirements(root, revision)
    candidates = [Path(tempfile.gettempdir()), Path(root)/'working/coverage/scratch']
    errors = []
    for path in dict.fromkeys(candidates):
        try:
            if 'original' in path.resolve().parts:
                raise ValueError('original inputs cannot hold scratch')
            path.mkdir(parents=True, exist_ok=True)
            check_capacity(path, size, inodes)
        except (OSError, ValueError) as error:
            errors.append(str(path)+': '+str(error))
            continue
        with tempfile.TemporaryDirectory(prefix=prefix, dir=path) as temporary:
            if os.environ.get('MM_COVERAGE_TIMINGS') == '1':
                import sys
                print(f'Coverage scratch: {temporary}; bytes={size}; inodes={inodes}; '
                      f'preflight_seconds={time.perf_counter()-started:.3f}', file=sys.stderr, flush=True)
            yield Path(temporary)
        return
    raise ValueError('No usable coverage scratch: '+'; '.join(errors))
