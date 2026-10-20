"""Strict private-source admission profile; join physical proof separately."""
import struct
from drawing_api_session import sha

REQUIRED_OPTIONS = ('wine_submission', 'wine_texture_entry', 'wine_resource_map',
    'wine_resource_copy', 'wine_source_descriptor', 'wine_source_map',
    'wine_source_map_bounded', 'wine_blit_worker', 'wine_blit_packet',
    'wine_source_version', 'wine_gpu_completion', 'wine_source_reservations',
    'wine_physical_owners', 'wine_source_handoff')


def configure(options):
    if not getattr(options, 'wine_source_ownership', False):
        return
    if options.observer_mode != 'physical' or options.control or options.wine_client_memory:
        raise ValueError('Source ownership requires physical API observation without client-point mode')
    for name in REQUIRED_OPTIONS:
        setattr(options, name, True)


def validate_manifest(metadata):
    if (metadata.get('observer_mode') != 'physical'
            or metadata.get('api_observer_enabled') is not True
            or metadata.get('wine_client_memory', False) is not False
            or any(metadata.get(name) is not True for name in REQUIRED_OPTIONS)):
        raise ValueError('Incomplete strict source ownership manifest')


def decode(data):
    if len(data) != 192:
        raise ValueError('Source ownership envelope or incomplete/refused session')
    header = struct.unpack_from('<16I', data)
    if data[:8] != b'MNMSOW01' or header[2:7] != (1, 64, 0x40209ca7, 1023, 2048) or any(header[7:]):
        raise ValueError('Source ownership header')
    ready, closed = struct.iter_unpack('<16I', data[64:])
    if ready[0:2] != (1, 1) or closed[0:2] != (2, 2) or not ready[2] or not closed[2]:
        raise ValueError('Source ownership phase/thread')
    if ready[3] != 1023 or closed[3] != 1023 or any(ready[4:]) or any(closed[6:]):
        raise ValueError('Source ownership capabilities/reserved')
    if not closed[4] or closed[4] != closed[5]:
        raise ValueError('Source ownership COPY/handoff denominator')
    return dict(schema=1, success=True, copies=closed[4], handoffs=closed[5],
                journal_sha256=sha(data), prerequisite_admission=True,
                global_host_driver_alias_ownership=False, native_replacement=False)


def analyze(directory, report):
    from drawing_source_handoff import analyze as handoff_analyze
    result = decode((directory/'wine-source-ownership.bin').read_bytes())
    handoff = handoff_analyze(directory, report)
    if result['copies'] != handoff['handoffs']:
        raise ValueError('Source ownership private binding denominator')
    result.update(handoff_journal_sha256=handoff['journal_sha256'],
                  private_copy_value_ownership_closed=True,
                  source_binding_restored_before_api_return=True)
    return result
