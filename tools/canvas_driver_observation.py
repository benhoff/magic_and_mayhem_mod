"""Validate the original startup-clear driver result and added read lease."""
import hashlib
import struct
from pathlib import Path


def analyze(directory):
    directory = Path(directory)
    journal = directory/'canvas-driver.bin'
    raw = journal.read_bytes()
    if len(raw) < 64+4*128 or (len(raw)-64) % 128:
        raise ValueError('Incomplete original driver journal')
    h = struct.unpack_from('<16I', raw)
    if (raw[:8] != b'MNMCDB01' or h[2:6] != (1,128,0x40209ca7,4096)
            or any(h[6:])):
        raise ValueError('Original driver journal identity')
    rows = list(struct.iter_unpack('<32I', raw[64:]))
    if len(rows) > 4096 or rows[-1][1] != 99:
        raise ValueError('Original driver first-World boundary missing')
    generation = blts = leases = 0; thread = None; created = clear = None
    for seq, row in enumerate(rows, 1):
        if (row[0] != seq or row[29] or row[31] or not row[4]
                or row[1] not in {10,20,30,40,99}):
            raise ValueError('Original driver sequence/refusal/reserved words')
        if thread is None: thread = row[4]
        if row[4] != thread:
            raise ValueError('Original driver observation crossed engine threads')
        if row[1] == 10:
            generation += 1
            if (created or row[2] != 0x6a49e0 or row[3] != 0x4e41f1
                    or not all(row[5:9]) or row[7] != row[8] or any(row[9:11])):
                raise ValueError('Original startup creation/COM identity')
            created = row
        elif row[1] == 20:
            blts += 1
            if (not created or row[2] != created[2] or row[3] != 0x58bc5a
                    or row[5:7] != created[5:7] or row[11]):
                raise ValueError('Original initial Blt result/alias changed')
        elif row[1] == 30:
            leases += 1
            if (clear or blts != 1 or row[2] != created[2] or row[3] != 0x4e41fd
                    or row[5:7] != created[5:7] or any(row[11:15]) or row[26] != 7
                    or row[18:22] != (16,0xf800,0x7e0,31)
                    or not 1 <= row[15] <= 2048 or not 1 <= row[16] <= 2048
                    or not row[15]*2 <= row[17] <= 8192 or row[22] or row[23]
                    or row[25] != row[15]*row[16]*2):
                raise ValueError('Original initial clear lacks complete zero pixels under a valid lease')
            clear = row
        elif row[1] == 40:
            raise ValueError('Selected original canvas released before the first World')
        elif seq != len(rows) or row[25] != leases:
            raise ValueError('Original driver terminal denominator')
        if row[27] != generation or row[30] != blts:
            raise ValueError('Original driver creation/Blt generation mismatch')
    if not created or not clear or generation != 1 or leases != 1:
        raise ValueError('Missing exact original creation/initial-clear cohort')
    pixels = (directory/'canvas-driver-clear.565').read_bytes()
    if len(pixels) != clear[25] or any(pixels):
        raise ValueError('Original leased pixel file differs from complete initial zero fill')
    fnv = 2166136261
    for byte in pixels: fnv = ((fnv ^ byte)*16777619) & 0xffffffff
    if fnv != clear[24]:
        raise ValueError('Original leased pixel identity changed')
    return {'schema':1, 'success':True, 'records':len(rows),
            'journal_sha256':hashlib.sha256(raw).hexdigest(),
            'pixels_sha256':hashlib.sha256(pixels).hexdigest(),
            'width':clear[15], 'height':clear[16], 'pitch_bytes':clear[17],
            'pixels_compared':clear[15]*clear[16], 'pixel_mismatches':0,
            'initial_blt_hresult':clear[11], 'blt_completion_hresult':clear[12],
            'read_lock_hresult':clear[13], 'unlock_hresult':clear[14],
            'real_com_identity_matches':True, 'added_read_leases':leases,
            'initial_blt_calls':blts, 'observed_wrapper_creations':generation,
            'alternate_canvas_observed':any(row[28] for row in rows),
            'legacy_readable_pointer_snapshots_disabled':True,
            'physical_allocation_generation_proved':False,
            'all_writers_recovered':False, 'driver_loss_restore_observed':False,
            'original_pixels_used_as_native_inputs':False, 'live_replacement':False,
            'scope':'Actual original drawing-surface creation and initial zero fill; '
                    'the real Blt is forwarded and its HRESULT recorded. Additional '
                    'QI/Lock(read-only,wait)/Unlock calls are explicit intervention. '
                    'All pixels are read during that held driver lease. This is one '
                    'successful startup, not physical allocator/all-alias/all-writer '
                    'recovery, an uninstrumented timing claim or real loss/Restore.'}
