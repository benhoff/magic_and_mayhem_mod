"""Validate native ordinal pixels outside the independently compared viewport."""
import hashlib

MARKER_ROWS = 32


def validate_capture(item, rgba, marker, width, height, ordinal):
    if (type(ordinal) is not int or not 1 <= ordinal <= 8192 or
            type(width) is not int or type(height) is not int or
            not 1 <= width <= 2048 or not 1 <= height <= 2048 or
            len(rgba) != width*height*4 or len(marker) != width*MARKER_ROWS*4):
        raise ValueError('Physical generation extent/ordinal')
    if (type(item.get('physical_ordinal')) is not int or item['physical_ordinal'] != ordinal or
            type(item.get('capture_height')) is not int or item['capture_height'] != height+MARKER_ROWS or
            item.get('marker_path') != f'frame-{ordinal-1:06}-marker.rgba'):
        raise ValueError('Physical generation identity')
    digest = lambda data: hashlib.sha256(data).hexdigest()
    if item.get('marker_sha256') != digest(marker) or item.get('capture_sha256') != digest(rgba+marker):
        raise ValueError('Physical game/marker same-image fingerprint')
    word = 0x4d4e0000 | ordinal
    for bit in range(MARKER_ROWS):
        row = marker[bit*width*4:(bit+1)*width*4]
        expected = bytes([255 if (word >> bit) & 1 else 0])*width
        if any(row[channel::4] != expected for channel in range(3)) or row[3::4] != b'\xff'*width:
            raise ValueError('Physical generation marker differs')
    return True
