"""Correlate a stopped live-history kind refusal with diagnostic-only raw rows."""
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import sys

from world_channel import BUILD, HEADER, INPUT, ORACLE


def diagnose(root):
    """Read only after stopping the producer. Never supply rows to rendering."""
    root = Path(root)
    native_path = root/'native-world.json'
    native_bytes = native_path.read_bytes()
    native = json.loads(native_bytes)
    if native.get('producer_reason') != 8:
        return None
    channel_path = root/'world-channel.bin'
    with channel_path.open('rb') as channel:
        header = channel.read(HEADER)
        if len(header) != HEADER:
            raise ValueError('Truncated refused channel header')
        words = struct.unpack('<32I', header)
        slots, published = words[13], words[7]
        size = HEADER + slots*(32 + INPUT + ORACLE)
        if (header[:8] != b'MNMWCH02' or words[2] != 2 or
                words[3] != size or channel_path.stat().st_size != size or
                not words[4] or words[5] not in (2, 3) or not 1 <= slots <= 16 or not 0 <= published < slots or
                words[10] != 8 or words[11] not in (2, 3) or
                words[14:17] != (INPUT, ORACLE, BUILD) or words[17] != slots or
                any(words[18:])):
            raise ValueError('Invalid refused history channel identity')
        sequence = published + 1
        channel.seek(HEADER + published*(32 + INPUT + ORACLE) + 32)
        failed = channel.read(80)
    if (len(failed) != 80 or failed[:8] != b'MNMWRLD1' or
            struct.unpack_from('<4I', failed, 8) != (1, 80, 80, sequence) or
            struct.unpack_from('<3I', failed, 36) != (0, 8, BUILD)):
        raise ValueError('Kind refusal has no matching failed input header')
    spec = importlib.util.spec_from_file_location('startup_queue_diagnostics',
                                                Path(__file__).with_name('inspect-startup-queues.py'))
    inspector = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(inspector)
    paths = sorted((root/'capture').glob('startup-queue-*.bin'))
    if not sequence <= len(paths) <= slots:
        raise ValueError(f'Missing raw queue diagnostic for queue {sequence}; the kind cannot be recovered from the channel alone')
    prefix = inspector.collect(root/'capture', len(paths), startup_replay=True)
    refused = prefix['queues'][sequence - 1]
    if refused['status'] or not refused['unsupported_draws']:
        raise ValueError('Kind refusal lacks complete unsupported raw draw rows')
    return {'schema': 1, 'scope': 'Stopped live-history kind refusal; raw queue diagnostics only, no raster equivalence or rendering inputs.',
            'producer_reason': 8, 'failed_source_queue': sequence,
            'published_frames': published, 'presented_frames': native['presentations'],
            'target_frames': slots, 'captured_queues': len(paths),
            'complete_prefix': len(paths) == slots, 'refused_queue': refused,
            'native_report_sha256': hashlib.sha256(native_bytes).hexdigest(),
            'channel_header_sha256': hashlib.sha256(header).hexdigest(),
            'failed_input_header_sha256': hashlib.sha256(failed).hexdigest(),
            'original_pixels_used_as_native_inputs': False}


def summarize(report):
    draws = report['refused_queue']['unsupported_draws']
    kinds = ', '.join(str(k) for k in sorted({d['kind'] for d in draws}))
    first = draws[0]
    return (f"unsupported draw kind(s) {kinds} in source queue {report['failed_source_queue']} "
            f"({len(draws)} row(s); first row {first['ordinal']}, x={first['x']}, y={first['y']})")


def save(root):
    report = diagnose(root)
    if report is not None:
        with (Path(root)/'native-world-refusal.json').open('x') as output:
            json.dump(report, output, indent=2)
            output.write('\n')
    return report


def log_refusal(root):
    """Enrich a launch failure after cleanup without masking its original error."""
    if not (Path(root)/'native-world.json').exists():
        return
    try:
        report = save(root)
        if report is not None:
            print('Native World capture refused: '+summarize(report)+
                  '; diagnostic: '+str(Path(root)/'native-world-refusal.json'), file=sys.stderr)
    except (OSError, ValueError) as error:
        print('Native World refusal diagnostic unavailable: '+str(error), file=sys.stderr)
