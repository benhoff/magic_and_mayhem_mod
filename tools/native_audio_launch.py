"""Authorize the fresh, session-owned native voice channel before starting Wine."""
import hashlib
import struct

SIZE = 128 + 16 * 1024 * 1024
VERSION = 5


def authorize(root, metadata, environment):
    # Inherited paths never authorize native audio in a new menu session.
    environment.pop('MNM_AUDIO_CHANNEL', None)
    enabled = metadata.get('native_voices', False)
    if type(enabled) is not bool:
        raise ValueError('Invalid native voice policy')
    if not enabled:
        return
    if not metadata.get('native_render') or metadata.get('origin') != 'menu_action_bridge':
        raise ValueError('Native voices require native menu/render staging')
    dll = root / 'game/MnmAudio.dll'
    if dll.is_symlink() or hashlib.sha256(dll.read_bytes()).hexdigest() != metadata.get('audio_dll_sha256'):
        raise ValueError('Native audio DLL hash mismatch')
    path = root / 'native-voices.bin'
    if metadata.get('audio_channel') != str(path) or path.is_symlink() or path.stat().st_size != SIZE:
        raise ValueError('Refusing redirected or invalid native voice channel')
    with path.open('rb') as source:
        header = source.read(128)
    words = struct.unpack('<32I', header)
    if header[:8] != b'MNMAUD01' or words[2:4] != (VERSION, SIZE) or words[20] != 1:
        raise ValueError('Native voice broker is not ready')
    wave_enabled = metadata.get('native_wave_loading', False)
    initial_silence = metadata.get('native_initial_silence', False)
    if type(initial_silence) is not bool or ((words[31]&16) and not initial_silence):
        raise ValueError('Unadmitted native initial-silence policy')
    if (type(wave_enabled) is not bool or metadata.get('native_extended_controls') is not True or
            metadata.get('native_output_recovery') is not True or (words[31]&~16) != (14|int(wave_enabled))):
        raise ValueError('Native audio capabilities differ from the staged session')
    if any(words[4:19]) or any(words[21:31]):
        raise ValueError('Refusing stale native voice channel')
    environment['MNM_AUDIO_CHANNEL'] = 'Z:' + str(path).replace('/', '\\')
    factory = metadata.get('native_audio_factory', False)
    if type(factory) is not bool:
        raise ValueError('Invalid native audio factory policy')
    probe=metadata.get('native_media_startup_probe',False)
    if type(probe) is not bool or (probe and metadata.get('native_media') is not True):
        raise ValueError('Invalid native media startup probe policy')
    override=environment.get('MNM_NATIVE_DISABLE_DSOUND')
    if factory and (override=='1' or (probe and override!='0')):
        overrides=environment.get('WINEDLLOVERRIDES','')
        environment['WINEDLLOVERRIDES']=(overrides+';' if overrides else '')+'dsound='


def directsound_disabled(environment):
    policy=None
    for entry in environment.get('WINEDLLOVERRIDES','').split(';'):
        names,separator,value=entry.partition('=')
        if separator and 'dsound' in [name.strip().lower() for name in names.split(',')]:
            policy=value.strip()
    return policy==''


def validate_activity(report, native_wave=False, native_factory=False, allow_initial_silence=False):
    for name in ('initial_silence','output_silent'):
        if name in report and type(report[name]) is not bool:
            raise ValueError('Invalid native silence health marker')
    for name in ('output_losses', 'output_recoveries', 'output_attempts', 'generated_frames', 'consumed_frames'):
        if type(report.get(name)) is not int or report[name] < 0:
            raise ValueError('Missing native output health: ' + name)
    quiet=allow_initial_silence and report.get('output_silent') is True and report.get('initial_silence') is True
    if ((report.get('output_available') is not True and not quiet) or report['output_attempts'] < 1 or
            not 0 < report['consumed_frames'] <= report['generated_frames'] or
            report['output_recoveries'] > report['output_losses']+int(report.get('initial_silence',False) is True)):
        raise ValueError('Native audio backend consumption/availability was not confirmed')
    for name in ('requests', 'creates', 'uploads', 'uploaded_bytes', 'plays', 'releases', 'native_selections'):
        if type(report.get(name)) is not int or report[name] < 1:
            raise ValueError('Missing native audio activity: ' + name)
    for name in ('fallbacks', 'adapter_failures', 'failed_requests'):
        if type(report.get(name)) is not int or report[name] != 0:
            raise ValueError('Native audio failed or fell back: ' + name)
    if report.get('audible_output') is not True or report.get('ready_before_stop') is not True or report.get('ready_after_stop') is not False or report.get('error') != '':
        raise ValueError('Native audio output/retirement was not confirmed')
    if native_wave and (report.get('wave_hook_installed') is not True or
                        any(type(report.get(k)) is not int or report[k]<1 for k in ('wave_loads','wave_bytes'))):
        raise ValueError('Native WAV loading was not exercised')
    if native_factory and (report.get('native_factory_only') is not True or report.get('unsupported_calls')!=0):
        raise ValueError('Native factory or supported local COM calls were not confirmed')
    return report


def validate_retirement(path, report):
    if path.is_symlink() or path.stat().st_size != SIZE:
        raise ValueError('Invalid retired native voice channel')
    with path.open('rb') as source:
        header = source.read(128)
    words = struct.unpack('<32I', header)
    if header[:8] != b'MNMAUD01' or words[2:4] != (VERSION, SIZE) or words[20] != 0 or not words[4] or words[4] != words[16]:
        raise ValueError('Native voice channel was not acknowledged and retired')
    expected = (report['native_selections'], report['fallbacks'], report['adapter_failures'])
    if words[21:24] != expected:
        raise ValueError('Native audio report differs from the final channel')
    if bool(words[31]&0x10000) != report.get('wave_hook_installed',False):
        raise ValueError('Native WAV installation differs from the final channel')
    if bool(words[31]&16) != report.get('initial_silence',False):
        raise ValueError('Native initial-silence marker differs from the final channel')
    if bool(words[31]&0x40000) != report.get('native_factory_only',False) or words[31]>>24 != report.get('unsupported_calls',0):
        raise ValueError('Native factory/local-call census differs from the final channel')
    return dict(request=words[4], acknowledgement=words[16], ready=words[20],
                native_selections=words[21], fallbacks=words[22], adapter_failures=words[23])
