"""Strict, file-based RTS shortcut policy; no UI or original media dependency."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT = ROOT/'patches/rts-controls.json'
ACTIONS = {'wizard':1, 'focus':2, 'idle':3, 'all_idle':3,
           'ungrouped':4, 'all_ungrouped':4,
           'same_type':5, 'cycle_type':6, 'cancel':11, 'stop':13}
KEYS = {'Tab':9, 'Escape':27, 'Space':32, 'Home':36, 'End':35,
        **{str(i):48+i for i in range(10)},
        **{chr(i):i for i in range(65,91)},
        **{f'F{i}':111+i for i in range(1,13)}}


def chord(text):
    if not isinstance(text, str):
        raise ValueError('Shortcut must be a string')
    parts = text.split('+')
    if not parts or parts[-1] not in KEYS:
        raise ValueError('Unsupported shortcut: '+text)
    mask = 0
    for p in parts[:-1]:
        bit = {'Shift':1, 'Ctrl':2, 'Alt':4}.get(p)
        if bit is None or mask & bit:
            raise ValueError('Invalid shortcut modifiers: '+text)
        mask |= bit
    return KEYS[parts[-1]], mask


def load(path=DEFAULT):
    raw = Path(path).read_bytes()
    if len(raw) > 16384:
        raise ValueError('Controls configuration exceeds 16 KiB')
    def unique(pairs):
        result = {}
        for k, v in pairs:
            if k in result:
                raise ValueError('Duplicate configuration key: '+k)
            result[k] = v
        return result
    value = json.loads(raw, object_pairs_hook=unique)
    if not isinstance(value, dict) or set(value) != {
        'schema','ctrl_additive_selection','double_tap_ms','mouse_scroll_speed','bindings','spell_slots'}:
        raise ValueError('Invalid controls configuration fields')
    if type(value['schema']) is not int or value['schema'] != 1:
        raise ValueError('Unsupported controls schema')
    if type(value['ctrl_additive_selection']) is not bool:
        raise ValueError('Additive selection must be boolean')
    for key, low, high in [('double_tap_ms',0,1000), ('mouse_scroll_speed',1,128)]:
        if type(value[key]) is not int or not low <= value[key] <= high:
            raise ValueError('Invalid '+key)
    if not isinstance(value['bindings'], dict) or set(value['bindings']) != set(ACTIONS):
        raise ValueError('Configure every named action exactly once')
    if not isinstance(value['spell_slots'], dict) or len(value['spell_slots']) > 26:
        raise ValueError('At most 26 spell shortcuts')
    entries = []
    for action, key in value['bindings'].items():
        vk, mask = chord(key)
        entries.append((vk, mask, ACTIONS[action], int(action.startswith('all_'))))
    for key, slot in value['spell_slots'].items():
        vk, mask = chord(key)
        if type(slot) is not int or not 0 <= slot < 26:
            raise ValueError('Spell slot must be 0..25')
        entries.append((vk, mask, 12, slot))
    used = {(vk, mask) for vk, mask, _, _ in entries}
    if len(used) != len(entries):
        raise ValueError('Conflicting shortcut bindings')
    # Number chords and F5..F8 bookmarks are reserved; preserve Escape as cancel.
    for vk, mask in used:
        if 48 <= vk <= 57 or (116 <= vk <= 119 and mask in (2,4)):
            raise ValueError('Shortcut conflicts with groups or camera bookmarks')
    return value, sorted(entries)
