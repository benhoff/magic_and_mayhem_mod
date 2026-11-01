"""Offline residual review. Retained execution and fresh reader execution stay separate."""
import hashlib
import io
from pathlib import PurePosixPath
import struct
import tarfile


def sha(data):
    return hashlib.sha256(data).hexdigest()


def selected_members(raw, select, *, max_bytes=256*1024**2, max_members=4096):
    """Read bounded regular members; refuse aliases, collisions and unsafe paths.

    The enclosing archive must be pinned by the caller. A selector maps an
    archive name to a relative output name, or returns None. Never extract tar
    permissions, symlinks or paths through tarfile.extract.
    """
    result = {}
    total = 0
    seen = set()
    with tarfile.open(fileobj=io.BytesIO(raw), mode='r:gz') as archive:
        for count, member in enumerate(archive, 1):
            name = PurePosixPath(member.name)
            if count > max_members or name.is_absolute() or '..' in name.parts or '\\' in member.name:
                raise ValueError('Unsafe or excessive archive members')
            if member.name in seen:
                raise ValueError('Duplicate archive member')
            seen.add(member.name)
            target = select(member.name)
            if target is None:
                continue
            target = PurePosixPath(target)
            if target.is_absolute() or '..' in target.parts or not target.parts or '\\' in str(target):
                raise ValueError('Unsafe selected output path')
            if not member.isfile() or str(target) in result:
                raise ValueError('Selected member is not a unique regular file')
            total += member.size
            if member.size < 0 or total > max_bytes:
                raise ValueError('Selected archive bytes exceed budget')
            data = archive.extractfile(member).read(member.size+1)
            if len(data) != member.size:
                raise ValueError('Truncated selected member')
            result[str(target)] = data
    return result


def pinned_member(data, row):
    if len(data) != row['bytes'] or sha(data) != row['sha256']:
        raise ValueError('Retained member identity differs')
    return data


def fnv(text):
    value = 2166136261
    for byte in text.encode('ascii'):
        value = ((value ^ byte)*16777619) & 0xffffffff
    return value


def signed(value):
    return value if value < 0x80000000 else value-0x100000000


def inside(rect, width, height):
    x0, y0, x1, y1 = rect
    return 0 <= x0 < x1 <= width and 0 <= y0 < y1 <= height


def refused_copies(rows):
    """Classify only captured invalid-rectangle fast-copy refusals.

    Requires strict closed census decoding/bijection by the caller. Geometry,
    bridge reason, HRESULT and absent command interval must all agree. This
    does not establish what the original DirectDraw driver would return.
    """
    result = []
    reasons = {fnv(name): name for name in ('blit_reject_source_rect', 'blit_reject_target_rect')}
    for row in rows:
        if row[15] != 0 and not row[13] & 0x80000000:
            continue
        sr, dr = list(map(signed, row[24:28])), list(map(signed, row[28:32]))
        bad_source = not inside(sr, row[16], row[17])
        bad_target = not inside(dr, row[19], row[20])
        reason = reasons.get(row[32])
        expected = 'blit_reject_source_rect' if bad_source else 'blit_reject_target_rect' if bad_target else None
        established = (row[11] == 1 and row[15] == 0 and row[13] == 0x88760096
                       and not any(row[40:44]) and reason == expected and expected is not None)
        result.append(dict(call=row[0], caller=hex(row[1]), caller_rva=hex(row[2]),
                           flags=hex(row[12]), result=hex(row[13]), reason=reason,
                           source_extent=list(row[16:19]), target_extent=list(row[19:22]),
                           source_rectangle=sr, target_rectangle=dr,
                           source_outside=bad_source, target_outside=bad_target,
                           invalid_fast_rectangle_established=established,
                           emitted_commands=list(row[40:44])))
    return result


def plane_differences(left, right, width, height):
    size = width*height*2
    if not 0 < width <= 2048 or not 0 < height <= 2048 or len(left) != size or len(right) != size:
        raise ValueError('Pixel plane extent differs')
    count, samples = 0, []
    for i, (a, b) in enumerate(zip(struct.iter_unpack('<H', left), struct.iter_unpack('<H', right))):
        if a != b:
            count += 1
            if len(samples) < 32:
                samples.append(dict(x=i % width, y=i // width, left=a[0], right=b[0]))
    return dict(mismatches=count, samples=samples)


def locate_wave(data, ordinal, x, y):
    """Locate a selected direct-distortion dependency, not a raster executor.

    The payload's owned effective row offsets are indexed by absolute canvas y,
    as in world_frame.cpp and the production compositing shader. This identifies
    the required source pixel; it does not prove its prior writer or coverage.
    """
    if len(data) < 80 or len(data) > 32*1024**2 or data[:8] != b'MNMWRLD1':
        raise ValueError('World envelope differs')
    h = struct.unpack_from('<18I', data, 8)
    if h[0] != 1 or h[1] != 80 or h[2] != len(data) or not h[3] or h[8] or h[9] != 0x40209ca7 or h[16] != 0:
        raise ValueError('Refused or unsupported World envelope')
    width, height, stride, count = h[4:8]
    if not 0 < width <= stride <= 4096 or not 0 < height <= 2048 or not 0 <= x < width or not 0 <= y < height or not 1 <= ordinal <= count <= 32768:
        raise ValueError('World locator bounds differ')
    selected = None
    at = 80
    for number in range(1, count+1):
        if at+64 > len(data):
            raise ValueError('Truncated World record')
        r = struct.unpack_from('<16I', data, at)
        size, op, encoded, payload = r[0], r[1], r[8], r[10]
        if size != 64+encoded+payload or size > len(data)-at or encoded > 1024*1024:
            raise ValueError('World record extent differs')
        if number == ordinal:
            if op != 3 or r[13] != 9 or payload != 64 or encoded < 40 or r[15] != 16:
                raise ValueError('Selected record is not direct distortion')
            clip = list(map(signed, r[4:8]))
            if not (0 <= clip[0] <= x < clip[2] <= width and 0 <= clip[1] <= y < clip[3] <= height):
                raise ValueError('Target outside selected clip')
            offsets = struct.unpack_from('<16I', data, at+64+encoded)
            if any(n > 16 for n in offsets):
                raise ValueError('Displacement outside supported range')
            sample = y*stride+x+offsets[y % 16]
            if sample >= stride*height:
                raise ValueError('Sample outside canvas')
            selected = dict(record=ordinal, operation=op, backend=r[13], target=[x,y],
                            row_offset=offsets[y % 16], source=[sample % stride, sample // stride],
                            clip=clip, width=width, height=height, stride=stride,
                            footprint_membership_proved=False, prior_writer_proved=False)
        at += size
    if at != len(data):
        raise ValueError('Trailing World records')
    return selected
