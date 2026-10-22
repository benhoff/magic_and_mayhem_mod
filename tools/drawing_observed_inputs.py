"""Bounded diagnostic inputs and exact identities; no ownership inference."""
import os
from pathlib import Path
import stat


def unique(items, key, *, allow_zero=False):
    result = {}
    for item in items:
        value = item[key]
        if type(value) is not int or value < (0 if allow_zero else 1) or value in result:
            raise ValueError('Ambiguous or invalid observed identity: ' + key)
        result[value] = item
    return result


def image_size(item, bytes_per_pixel):
    width, height = item['width'], item['height']
    if any(type(v) is not int or not 1 <= v <= 4096 for v in (width, height)):
        raise ValueError('Diagnostic image extent outside 1..4096')
    return width * height * bytes_per_pixel


def regular_bytes(path, maximum):
    """Refuse symlinks/devices/FIFOs and bound reads before allocating payloads."""
    fd = os.open(path, os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK)
    try:
        info = os.fstat(fd)
        if not stat.S_ISREG(info.st_mode) or info.st_size > maximum:
            raise ValueError('Expected bounded regular diagnostic input')
        with os.fdopen(fd, 'rb', closefd=False) as stream:
            raw = stream.read(maximum + 1)
        after = os.fstat(fd)
        if len(raw) > maximum or (info.st_size, info.st_mtime_ns, info.st_ctime_ns) != (after.st_size, after.st_mtime_ns, after.st_ctime_ns):
            raise ValueError('Diagnostic input changed during read')
        return raw
    finally:
        os.close(fd)


def bound_bytes(directory, name, maximum):
    relative = Path(name)
    if relative.is_absolute() or len(relative.parts) != 1 or relative.name in ('.', '..'):
        raise ValueError('Unsafe diagnostic input path')
    path = directory / relative
    if path.is_symlink():
        raise ValueError('Diagnostic input symlink')
    return regular_bytes(path, maximum)
