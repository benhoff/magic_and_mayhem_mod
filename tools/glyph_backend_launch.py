"""Stage and authorize the fixed-build live glyph producer hook."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent))
from launch_build_cache import build_adapter
ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def load(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def mode_from_environment():
    mode=os.environ.get('MNM_GLYPH_BACKEND','')
    if mode not in ('','off','shadow','takeover'):raise ValueError('MNM_GLYPH_BACKEND must be off, shadow or takeover')
    return None if mode in ('','off') else mode
def stage(root,original,patched,mode,scene_imported=False):
    if mode not in (None,'shadow','takeover'):raise ValueError('Unsupported glyph backend mode')
    if not mode:return patched,{}
    if hashlib.sha256(original).hexdigest()!=HASH:raise ValueError('Unsupported glyph source executable')
    exporter=load('glyph_export','tools/export-menu-support.py');at=exporter.image_offset(original,0x581ec0,7)
    if original[at:at+7]!=bytes.fromhex('83ec288b442430'):raise ValueError('Unsupported glyph entry')
    if not (root/'game/MnmScene.dll').resolve().is_relative_to((root/'game').resolve()):raise ValueError('Glyph DLL target escapes its disposable game copy')
    if not scene_imported:
        dll=build_adapter('tools/build-scene-observer.py',lambda: load('glyph_build','tools/build-scene-observer.py').build())
        patched=load('glyph_import','tools/prepare-shadow-experiment.py').add_import(patched,'MnmScene.dll','SceneAnchor',b'.mnglyph')
        shutil.copy2(dll,root/'game'/dll.name)
        shutil.copy2(dll.parent/'manifest.json',root/'glyph-bridge-build.json')
    directory=root/'glyph-backend';directory.mkdir()
    return patched,dict(glyph_backend_mode=mode,glyph_directory=str(directory),glyph_dll_sha256=sha(root/'game/MnmScene.dll'))
def authorize(root,metadata,env):
    for name in ('MNM_GLYPH_BACKEND','MNM_GLYPH_DIR'):env.pop(name,None)
    mode=metadata.get('glyph_backend_mode')
    if mode is None:return
    if mode not in ('shadow','takeover'):raise ValueError('Invalid staged glyph mode')
    directory=Path(metadata['glyph_directory']).resolve()
    if directory!=root.resolve()/'glyph-backend' or directory.is_symlink() or not directory.is_dir() or any(directory.iterdir()):raise ValueError('Glyph capture directory must be fresh and owned')
    if sha(root/'game/MnmScene.dll')!=metadata['glyph_dll_sha256']:raise ValueError('Glyph DLL hash mismatch')
    for name in ('MNM_CANVAS_PRODUCERS','MNM_CANVAS_STARTUP','MNM_SCENE_WORLD','MNM_WORLD_RASTER_BATCH','MNM_WORD_SPRITES'):
        if env.get(name):raise ValueError('Glyph mode conflicts with '+name)
    env['MNM_GLYPH_BACKEND']=mode;env['MNM_GLYPH_DIR']='Z:'+str(directory).replace('/','\\')
