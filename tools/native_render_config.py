"""Disable legacy draw skipping only in a disposable native-render installation."""
import hashlib
import re

SETTINGS = ('SkipFrameEvery', 'SkipXFrames', 'MaxSkipXFrames')


def without_draw_skipping(text):
    active = False
    counts = dict.fromkeys(SETTINGS, 0)
    names = {name.lower().encode(): name for name in SETTINGS}
    pattern = rb'([ \t]*(' + b'|'.join(name.encode() for name in SETTINGS) + rb')[ \t]*=[ \t]*)([0-9]+)([ \t]*(?:;[^\r\n]*)?)(\r?\n)?$'
    lines = []
    for line in text.splitlines(keepends=True):
        section = re.match(rb'[ \t]*\[([^]]+)\]', line)
        if section:
            active = section[1].upper() == b'DEBUG'
        match = re.match(pattern, line, re.I) if active else None
        if match:
            name = names[match[2].lower()]
            counts[name] += 1
            line = match[1] + b'0' + match[4] + (match[5] or b'')
        lines.append(line)
    if any(count != 1 for count in counts.values()):
        raise ValueError('Missing or ambiguous staged DEBUG draw-skipping settings')
    return b''.join(lines)


def stage_native_draw_cadence(game, encode, decode):
    if 'original' in game.resolve().parts:
        raise ValueError('Original input is read-only')
    plain = game / 'CFG/chaos.cfg'
    packed = game / 'CFG/Encrypted/chaos.cfg'
    paths = [path for path in (plain, packed) if path.exists()]
    if not paths:
        raise ValueError('No staged chaos configuration')
    edits = []
    # Validate both precedence variants and round trips before changing either.
    for path in paths:
        before = path.read_bytes()
        text = decode(before) if path == packed else before
        changed = without_draw_skipping(text)
        after = encode(changed) if path == packed else changed
        if path == packed and decode(after) != changed:
            raise ValueError('Native draw-cadence CFG round trip failed')
        edits.append((path, before, after))
    records = []
    for path, before, after in edits:
        path.write_bytes(after)
        records.append(dict(path=str(path.relative_to(game)),
                            before_sha256=hashlib.sha256(before).hexdigest(),
                            after_sha256=hashlib.sha256(after).hexdigest()))
    return dict(settings=dict.fromkeys(SETTINGS, 0), files=records,
                scope='Native presentation policy: disable fixed/adaptive legacy draw skipping; selected game-speed preference remains unchanged')
