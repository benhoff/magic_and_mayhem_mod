#!/usr/bin/env python3
"""Synthetic ACK peer for the PE32 fixture; never original acquisition evidence."""
import json
import struct
import sys
import time
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_api_session import sha
from drawing_presented_session import rgba565

root, report_path = map(Path, sys.argv[1:3])
frames = []
deadline = time.monotonic() + 90
for ordinal in range(1, 9):
    request_path = root / f'api-physical-request-{ordinal:06d}.bin'
    while not request_path.exists() or request_path.stat().st_size != 128:
        if time.monotonic() > deadline:
            raise ValueError('Synthetic physical ACK fixture timed out')
        time.sleep(.005)
    request = request_path.read_bytes()
    row = struct.unpack('<32I', request)
    if row[12] != ordinal or row[14:17] != (8, 6, 16):
        raise ValueError('Wrong synthetic fixture identity')
    physical = rgba565(b''.join(((i*997)&65535).to_bytes(2, 'little') for i in range(48)))
    name = f'api-physical-oracle-{ordinal:06d}.rgba'
    with (root / name).open('xb') as out:
        out.write(physical)
    frames.append(dict(ordinal=ordinal, record=row[0], entry=row[29], method=row[2],
                       object=row[3], caller=row[5], thread=row[6], width=8, height=6,
                       path=name, request_sha256=sha(request), sha256=sha(physical),
                       stable_double_capture=True))
    with (root / f'api-physical-ack-{ordinal:06d}.bin').open('xb') as out:
        out.write(b'MNMAPI01' + struct.pack('<II', ordinal, row[0]))
with report_path.open('x') as out:
    json.dump(dict(success=True, native_rendering=False, all_global_presentation_routes=False,
                   original_draw_thread_fenced=True, source_inputs_bound=False,
                   boundary='observed_primary_api_physical', synthetic_fixture=True, frames=frames), out)
