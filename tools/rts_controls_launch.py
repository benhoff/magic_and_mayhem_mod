"""Stage and admit an isolated controls adapter in a disposable installation."""
import hashlib
import importlib.util
import json
import re
from pathlib import Path
import shutil
from launch_build_cache import build_adapter
from rts_controls_config import ROOT, DEFAULT, load

HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
GUARDS = {0x46c070:'83ec145355568b74', 0x4fdad0:'81ec300100005355',
          0x5073d0:'578bf98b8fa80000', 0x5074f0:'53578bf933db395f',
          0x4f7ae0:'578b7c240885ff74', 0x4f7a20:'558b442408568b74',
          0x4d4d60:'83ec1455568bf157', 0x5060a0:'6aff681b245c00',
          0x537bd0:'8b413485c0750433',
          0x5c5dec:'70c04600', 0x5c6a54:'d0da4f00'}


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT/path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def enabled_from_environment(environment):
    value = environment.get('MNM_RTS_CONTROLS')
    if value not in (None,'0','1'):
        raise ValueError('MNM_RTS_CONTROLS must be 0 or 1')
    return value == '1'


def scroll_preferences(game, speed):
    encoder = module('rts_cfg_encoder','tools/encode-cfg.py')
    decoder = encoder.load_decoder(ROOT)
    edits = []
    for name, packed in [('CFG/prefs.cfg',False),('CFG/Encrypted/prefs.cfg',True)]:
        path = Path(game)/name
        if not path.exists():
            continue
        before = path.read_bytes()
        text = decoder.decode(before)[1] if packed else before
        active = False
        count = 0
        lines = []
        for line in text.splitlines(keepends=True):
            section = re.match(rb'\s*\[([^]]+)\]',line)
            if section:
                active = section[1].upper() == b'MOUSE'
            match = re.fullmatch(rb'(\s*MouseScrollSpeed\s*=\s*)[0-9]+([ \t]*(?:;[^\r\n]*)?)(\r?\n)?',line,re.I) if active else None
            if match:
                count += 1
                line = match[1]+str(speed).encode('ascii')+match[2]+(match[3] or b'')
            lines.append(line)
        if count != 1:
            raise ValueError('Missing/ambiguous mouse scroll preference: '+name)
        changed = b''.join(lines)
        after = encoder.encode(changed,decoder)[0] if packed else changed
        if packed and decoder.decode(after)[1] != changed:
            raise ValueError('Mouse preference round trip failed')
        edits.append({'path':name,'before_sha256':hashlib.sha256(before).hexdigest(),
                      'after_sha256':hashlib.sha256(after).hexdigest()})
        # Preflight all preference files before writing either representation.
        edits[-1]['_path'] = path
        edits[-1]['_bytes'] = after
    if not edits:
        raise ValueError('Missing staged mouse preferences')
    for edit in edits:
        edit.pop('_path').write_bytes(edit.pop('_bytes'))
    return edits


def stage(root, original, patched, enabled, config=DEFAULT):
    if type(enabled) is not bool:
        raise ValueError('Invalid controls activation policy')
    if not enabled:
        return patched, {}
    if hashlib.sha256(original).hexdigest() != HASH:
        raise ValueError('Unsupported controls executable')
    root = Path(root).resolve()
    game = root/'game'
    if game.is_symlink() or not game.resolve().is_relative_to(root):
        raise ValueError('Controls game directory escapes disposable staging')
    offsets = module('rts_offsets','tools/export-menu-support.py')
    for va, hexbytes in GUARDS.items():
        expected = bytes.fromhex(hexbytes)
        at = offsets.image_offset(original,va,len(expected))
        if original[at:at+len(expected)] != expected or patched[at:at+len(expected)] != expected:
            raise ValueError('Controls code/vtable changed before staging')
    settings, _ = load(config)
    # Validate every destination before producing or copying adapter output.
    for name in ('MnmRtsControls.dll','CFG/prefs.cfg','CFG/Encrypted/prefs.cfg'):
        target = root/'game'/name
        if target.is_symlink() or not target.resolve().is_relative_to((root/'game').resolve()):
            raise ValueError('Controls staging target escapes game copy')
    dll = build_adapter('tools/build-rts-controls.py',
                        lambda: module('rts_build','tools/build-rts-controls.py').build(config=config),
                        options={'settings':settings})
    manifest = json.loads((dll.parent/'manifest.json').read_text())
    if manifest['settings'] != settings:
        raise ValueError('Controls build configuration mismatch')
    updated = module('rts_import','tools/prepare-shadow-experiment.py').add_import(
        patched,'MnmRtsControls.dll','RtsControlsAnchor',b'.mnrts')
    shutil.copy2(dll,root/'game/MnmRtsControls.dll')
    shutil.copy2(dll.parent/'manifest.json',root/'rts-controls-build.json')
    edits = scroll_preferences(root/'game',settings['mouse_scroll_speed'])
    (root/'rts-controls.json').write_text(json.dumps(settings,indent=2)+'\n')
    return updated, {'rts_controls':True,
        'rts_controls_dll_sha256':hashlib.sha256(dll.read_bytes()).hexdigest(),
        'rts_controls_config_sha256':hashlib.sha256((root/'rts-controls.json').read_bytes()).hexdigest(),
        'rts_controls_preferences':edits,
        'rts_controls_scope':'Campaign-only shortcut policy. Synthetic validation and live playtest are separate.'}


def authorize(root, metadata, environment):
    environment.pop('MNM_RTS_CONTROLS_ACTIVE',None)
    enabled = metadata.get('rts_controls',False)
    if type(enabled) is not bool:
        raise ValueError('Invalid staged controls policy')
    if not enabled:
        return
    root = Path(root).resolve()
    game = root/'game'
    if game.is_symlink() or not game.resolve().is_relative_to(root):
        raise ValueError('Controls game directory is redirected')
    target = root/'game/MnmRtsControls.dll'
    config = root/'rts-controls.json'
    if (target.is_symlink() or not target.resolve().is_relative_to((root/'game').resolve())
        or config.is_symlink() or
        hashlib.sha256(target.read_bytes()).hexdigest() != metadata.get('rts_controls_dll_sha256') or
        hashlib.sha256(config.read_bytes()).hexdigest() != metadata.get('rts_controls_config_sha256')):
        raise ValueError('Controls DLL/configuration changed or redirected')
    environment['MNM_RTS_CONTROLS_ACTIVE'] = '1'
