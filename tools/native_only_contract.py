"""Admission and final accounting for the native-only rendering policy.

A successful admitted session is not evidence of complete original equivalence.
"""
import hashlib
import json
import os
from pathlib import Path

POLICY = 'native-only-rendering-v1'
FLAG = 'MNM_NATIVE_ONLY'
REQUIRED_ENV = {
    FLAG: '1', 'MNM_NATIVE_DRAW': '1', 'MNM_NATIVE_CPU_GUARD': '3',
    'MNM_PUBLIC_RESIDENT_WORLD': '1', 'MNM_RENDER_CONTINUOUS': '1',
    'MNM_RENDER_COMMAND_WIRE_VERSION': '3', 'MNM_GLYPH_BACKEND': 'takeover',
    'MNM_OTHER_PIXEL_PRODUCERS': '1', 'MNM_NATIVE_PIXEL_NO_FORWARD': '1',
    **{name: '1' for name in ('MNM_PANEL_TAKEOVER', 'MNM_PIXEL_TAKEOVER',
       'MNM_JPEG_TAKEOVER', 'MNM_MINIMAP_TAKEOVER', 'MNM_COLOUR_RECT_TAKEOVER',
       'MNM_RASTER_TAKEOVER')},
}
# Diagnostic producers cannot share a native-only session's ownership contract.
FORBIDDEN_ENV = ('MNM_SCENE_PRODUCTION_PROBE', 'MNM_WINE_IMPORT_CENSUS',
                 'MNM_MENU_ORIGINAL_GAMEPLAY', 'MNM_DC_COPY_TAKEOVER')
FORBIDDEN_ARGS = ('--frame-readback', '--capture-locks', '--capture-draws',
                  '--capture-history', '--no-readback', '--legacy-media',
                  '--embedding-test', '--fixture-window', '--commands',
                  '--stream-test', '--surface-test', '--surface-demo',
                  '--opengl-test', '--smoke-test', '--repo', '--renderer')
REFUSALS = ('native-glyph-forward-refusal.bin', 'native-draw-refusal.bin', 'native-render-capability-refusal.bin',
            'native-cpu-history-refusal.bin', 'native-cpu-guard-fault.bin',
            'native-pixel-forward-refusal.bin', 'native-gpu-copy-refusal.bin')
BUILDS = {
    'native-draw': ('MnmDraw.dll', 'native_draw_sha256'),
    'render-bridge': ('MnmRender.dll', 'render_dll_sha256'),
    'scene-bridge': ('MnmScene.dll', 'resident_scene_sha256'),
    'bridge': ('MnmMenu.dll', 'dll_sha256'),
    **{name+'-bridge': (dll, name+'_dll_sha256') for name, dll in (
        ('panel','MnmPanelWriters.dll'), ('pixel','MnmPixelWriters.dll'),
        ('jpeg','MnmJpegPixels.dll'), ('minimap','MnmMinimapWriters.dll'),
        ('colour_rect','MnmColourRect.dll'), ('raster','MnmRasterWriters.dll'))},
}


def selected(environment):
    value = environment.get(FLAG)
    if value not in (None, '1'):
        raise ValueError('MNM_NATIVE_ONLY must be absent or 1')
    return value == '1'


def configure(environment, arguments):
    """Reject conflicts before staging; defaults may only strengthen admission."""
    result = dict(environment)
    for key, expected in REQUIRED_ENV.items():
        if key in result and result[key] != expected:
            raise ValueError('Native-only mode requires '+key+'='+expected)
        result[key] = expected
    for key in FORBIDDEN_ENV:
        if key in result:
            raise ValueError('Native-only mode excludes '+key)
    values = {'--live-menu-test','--music-assets'}
    switches = {'--skip-movies','--legacy-audio','--native-voices','--native-media',
                '--native-music','--no-native-music','--software-rendering','--independent-world'}
    pending = None
    for arg in arguments:
        if pending:
            if arg.startswith('--'):
                raise ValueError('Missing native-only option value: '+pending)
            pending = None
            continue
        key, equal, value = arg.partition('=')
        if key in values:
            if equal and not value:
                raise ValueError('Missing native-only option value: '+key)
            if not equal:
                pending = key
        elif key not in switches or equal:
            raise ValueError('Native-only mode excludes '+key)
    if pending:
        raise ValueError('Missing native-only option value: '+pending)
    return result


def admit(root, repository, metadata, environment):
    from native_output_owner import REQUIRED
    root, repository = Path(root).resolve(), Path(repository).resolve()
    configure(environment, ())
    if any(environment.get(key)!=value for key,value in REQUIRED_ENV.items()):
        raise ValueError('Native-only required environment was not retained')
    if metadata.get('native_only_policy') != POLICY or any(
            metadata.get(name) is not True for name in REQUIRED):
        raise ValueError('Native-only capabilities were not staged together')
    if (metadata.get('origin') != 'menu_action_bridge'
            or metadata.get('glyph_backend_mode') != 'takeover'
            or metadata.get('glyph_capture') is not False
            or not (metadata.get('native_media') is True or metadata.get('skip_movies') is True)):
        raise ValueError('Native-only mode requires owned menus, glyphs and native or disabled movies')
    sources, binaries = {}, {}
    for name, (dll, key) in BUILDS.items():
        build = json.loads((root/(name+'-build.json')).read_text())
        target = root/'game'/dll
        digest = hashlib.sha256(target.read_bytes()).hexdigest()
        if target.is_symlink() or digest != metadata.get(key) or digest != build.get('sha256'):
            raise ValueError('Native-only staged binary changed: '+dll)
        declared = {}
        for field in ('sources','compiled_sources','protocol_sources'):
            entries = build.get(field, {})
            if not isinstance(entries, dict):
                raise ValueError('Invalid native-only source receipt: '+dll)
            for path, expected in entries.items():
                # The older menu/render manifests retain basename receipts in
                # addition to their full compiler closure. Resolve only these
                # documented builder formats; never search for matching files.
                if '/' not in path and name in ('bridge','render-bridge'):
                    prefix = ('protocols/include/mnm' if name=='bridge' and path.startswith('menu_v')
                              else 'runtime/menu' if name=='bridge' else 'runtime/render')
                    path = prefix+'/'+path
                if path in declared and declared[path]!=expected:
                    raise ValueError('Conflicting native-only compiler receipts: '+path)
                declared[path] = expected
        if not declared:
            raise ValueError('Native-only build has no source closure: '+dll)
        for path, expected in declared.items():
            source = (repository/path).resolve()
            if (Path(path).is_absolute() or not source.is_relative_to(repository)
                    or Path(path).parts[0] in ('original','working','.git')
                    or hashlib.sha256(source.read_bytes()).hexdigest() != expected):
                raise ValueError('Native-only build source changed: '+path)
            sources[path] = expected
        binaries[dll] = digest
    build = json.loads((root/'native-draw-build.json').read_text())
    if (build.get('directdraw_imports') is not False
            or not {'runtime/native-draw/native_only.h','runtime/native-draw/startup_retirement.h'} <= set(build['sources'])
            or metadata.get('native_draw_startup_retirement') is not True):
        raise ValueError('Native-only facade lacks terminal capability admission')
    return dict(policy=POLICY, binaries=binaries, sources=sources,
                original_pixel_equivalence=False, complete_replacement=False)


def counter(value):
    # Qt serializes uint64 counters as canonical decimal strings to avoid
    # double precision loss. Accept that wire form and actual JSON integers.
    if type(value) is int:
        result = value
    elif isinstance(value,str) and value.isascii() and value.isdecimal() and str(int(value))==value:
        result = int(value)
    else:
        raise ValueError('Invalid native-only retirement counter')
    if not 0 <= result <= 0xffffffffffffffff:
        raise ValueError('Native-only retirement counter outside uint64')
    return result


def process_identity(pid=None):
    """Host runner identity; start ticks prevent PID reuse from changing it."""
    pid = os.getpid() if pid is None else pid
    fields = Path('/proc',str(pid),'stat').read_text().rsplit(') ',1)[1].split()
    return dict(pid=pid,start_ticks=int(fields[19]))


def restart_outcome(root, admission, shell):
    if not all(shell.get(key) is True for key in
               ('restart_requested','force_stop_verified','renderer_retired')):
        raise ValueError('Native-only restart lacks verified retirement')
    identity=admission.get('runner_identity',{})
    if (type(identity.get('pid')) is not int or identity['pid']<=0
            or type(identity.get('start_ticks')) is not int or identity['start_ticks']<=0):
        raise ValueError('Native-only restart lacks its original runner identity')
    try:
        current=process_identity(identity['pid'])
    except FileNotFoundError:
        current=None
    if current==identity:
        raise ValueError('Native-only old runner is still alive')
    # Cancellation can interrupt World work and final counter persistence. It
    # retires a generation, never grants it normal-completion/rendering credit.
    return dict(policy=POLICY,state='restarted',rendering_accepted=False,
                previous_error=shell.get('error',''),complete_replacement=False)


def outcome(root):
    """Every session must leave positive, matching shutdown records; no exit-only success."""
    from native_pixel_counters import read_counters
    root = Path(root)
    admission = json.loads((root/'native-output-admission.json').read_text())
    if admission.get('native_only', {}).get('policy') != POLICY or admission.get('session') != str(root.resolve()):
        raise ValueError('Missing native-only startup admission')
    shell = json.loads((root/'native-only-session.json').read_text())
    if (type(shell.get('schema')) is not int or shell['schema'] != 1 or
            shell.get('policy')!=POLICY or shell.get('session')!=str(root.resolve())):
        raise ValueError('Native-only shell retirement belongs to another session')
    if shell.get('state')=='restarted':
        return restart_outcome(root,admission,shell)
    if (shell.get('policy') != POLICY or shell.get('session') != str(root.resolve())
            or shell.get('state') != 'completed' or shell.get('normal_quit') is not True
            or shell.get('fallback') is not False or shell.get('error') != ''
            or type(shell.get('launcher_exit')) is not int or shell['launcher_exit'] != 0
            or shell.get('renderer_retired') is not True
            or shell.get('restart_requested') is not False
            or shell.get('force_stop_verified') is not False):
        raise ValueError('Native-only shell did not complete without fallback')
    for name in REFUSALS:
        if (root/'game'/name).exists():
            raise ValueError('Native-only rendering refused: '+name)
    original = json.loads((root/'original-process-result.json').read_text())
    if type(original.get('original_exit_code')) is not int or original['original_exit_code'] != 0:
        raise ValueError('Original simulation did not quit normally')
    counters = read_counters(root/'game')
    if counters['forwarded'] or counters['history_refused'] or counters.get('forward_guard_refusal'):
        raise ValueError('Native-only producer forwarding/history failure')
    world = json.loads((root/'resident-world-report.json').read_text())
    completed = counter(world.get('completed'))
    pending = world.get('pending')
    if (counter(world.get('imports')) != completed
            or counter(world.get('verified_returns')) != completed or not (pending is False or type(pending) is int and pending==0)
            or world.get('normal_quit_retired') is not True or world.get('error') != ''):
        raise ValueError('Native-only World work did not retire in balance')
    from native_draw_retirement import require_retired, read_startup_retirement
    graphics = require_retired(root/'native-draw-retirement.bin')
    graphics['startup_probe']=read_startup_retirement(root/'game/native-draw-startup-retirement.bin')
    return dict(policy=POLICY, state='completed', native_calls=counters['native'],
                forwarded=0, worlds=completed, graphics_retirement=graphics,
                complete_replacement=False)
