"""Small synthetic protocol fixtures; these bytes never certify game execution.

Keep layouts independent of production decoders. Builders emit only requested
operations/publications; malformed-input mutations belong in each owning test.
"""
import json
from pathlib import Path
import struct


def u32(*values):
    return struct.pack('<' + 'I' * len(values), *values)


def command_stream(operations):
    """Emit MNMCMD05 records with consecutive sequences, without adding END."""
    return b'MNMCMD05' + u32(5, 16) + b''.join(
        u32(op, sequence, len(body)) + body
        for sequence, (op, body) in enumerate(operations, 1))


def menu_publication(*, version=22, size=540, sequence=2, ready=1, screen=3,
                     generation=1, ack=0, status=0, thread=99, handoff=0):
    """Mutable menu header/engine lane; leave screen-specific payload to tests."""
    raw = bytearray(size)
    raw[:8] = (('MNMMCMD' if version < 10 else 'MNMMCM') + str(version)).encode()
    struct.pack_into('<II', raw, 8, version, size)
    struct.pack_into('<8I', raw, 128, sequence, ready, screen, generation,
                     ack, status, thread, handoff)
    return raw


def owned_menu_session(root):
    """Create a fresh synthetic owner admission and its private original session."""
    root = Path(root)
    session = root / 'working/experiments/menu-observer/run-test'
    session.mkdir(parents=True)
    admissions = root / 'owner/sessions'
    admissions.mkdir(parents=True)
    (admissions / 'one.json').write_text(json.dumps({'session': str(session)}))
    return session
