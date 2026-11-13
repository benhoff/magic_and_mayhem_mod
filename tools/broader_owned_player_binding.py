"""Reuse the verified player with the production shell script's exact defaults."""
from pathlib import Path
from broader_native_host import gpu_environment
from rendering_pilot import require

def player_command(root,binary,command,environment,software):
    root=Path(root);require(command and Path(str(command[0]))==root/'tools/run-qt-shell.sh','Only the owned Qt shell launch may bind the preserved player')
    require(type(software)is bool,'Typed original player profile required')
    env=dict(environment)
    for name,value in [('QT_QPA_PLATFORM','xcb'),('MNM_VERIFY_ORIGINALS','0'),('MNM_VERIFY_ASSETS','0'),('MNM_CACHE_ADAPTERS','1'),('MNM_GLYPH_BACKEND','takeover')]:env.setdefault(name,value)
    if software:
        env.update(LIBGL_ALWAYS_SOFTWARE='1');env.pop('GALLIUM_DRIVER',None);env.pop('MESA_LOADER_DRIVER_OVERRIDE',None)
    else:env=gpu_environment(env)
    actual=[str(binary),'--repo',str(root),*map(str,command[1:])]
    require('--software-rendering'not in actual,'Source launcher already selected a profile')
    if software:actual.append('--software-rendering')
    return actual,env
