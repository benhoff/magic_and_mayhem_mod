"""Rendering-mode-specific HUD observations, separate from original game events."""
import hashlib
import re


def oracle(resident_world):
    return 'resident-native' if resident_world else 'dual-draw'


def validate_summon_capture(capture, root, count, resident_world=False):
    """Retain both diagnostics; only current rendered images can prove a count.

    Resident CPU World images can contain older HUD pixels. The resident native
    image proves the displayed count; the caller must independently validate
    newly living original creatures and original cast/combat events.
    """
    name = capture['name']
    if not re.fullmatch(r'combat-\d{2}-[a-z-]+', name):
        raise ValueError('Unsafe summon capture name')
    if (capture['metadata']['fallback'] or
            not all(capture['metadata'][r+'_saved'] for r in ('native', 'original')) or
            set(capture['ocr']) != {'native', 'original'}):
        raise ValueError('Missing summon image diagnostics')
    for route in ('native', 'original'):
        path = root / (name+'-'+route+'.png')
        if hashlib.sha256(path.read_bytes()).hexdigest() != capture['image_sha256'][route]:
            raise ValueError('Summon capture changed after OCR')
    routes = ('native',) if resident_world else ('native', 'original')
    if any(capture['ocr'][r] != count for r in routes):
        raise ValueError('Missing current '+oracle(resident_world)+' summon count: '+str(capture['ocr']))
