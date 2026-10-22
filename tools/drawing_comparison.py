"""Strict identity and complete-plane admission shared by rendering diagnostics."""
import hashlib


def index(rows, key, label):
    result = {}
    for row in rows:
        value = row[key]
        if type(value) is not int or not 1 <= value <= 0xffffffff or value in result:
            raise ValueError('Invalid or duplicate '+label+' identity')
        result[value] = row
    return result


def plane(data, width, height, digest=None):
    if (type(width) is not int or type(height) is not int or
            not 1 <= width <= 2048 or not 1 <= height <= 2048 or
            len(data) != width*height*2):
        raise ValueError('Invalid complete RGB565 plane extent')
    actual = hashlib.sha256(data).hexdigest()
    if digest is not None and (not isinstance(digest, str) or digest != actual):
        raise ValueError('Complete RGB565 plane hash differs')
    return actual


def same_held_observation(actual, recorded):
    """Admit only the two explicit zero defaults missing from legacy v1 reports."""
    import copy
    if 'wire_version' in recorded:
        return actual == recorded
    if actual.get('wire_version') != 1:
        return False
    legacy = copy.deepcopy(actual)
    del legacy['wire_version']
    if len(legacy.get('frames', [])) != len(recorded.get('frames', [])):
        return False
    for frame, old in zip(legacy['frames'], recorded['frames']):
        if frame.get('pending_admission') != 0:
            return False
        if 'pending_admission' not in old:
            del frame['pending_admission']
    return legacy == recorded
