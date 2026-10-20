"""Validate native COM ownership before process reclamation, independently of exit."""
import hashlib
from pathlib import Path
import struct


def read_retirement(path):
    path = Path(path)
    if path.is_symlink() or not path.is_file():
        raise ValueError('Missing owned native graphics retirement receipt')
    raw = path.read_bytes()
    if len(raw) < 64 or len(raw) % 64 or len(raw) > (1+64+512)*64:
        raise ValueError('Invalid native graphics retirement extent')
    words = struct.unpack('<'+'I'*(len(raw)//4), raw)
    header = words[:16]
    if (header[:4] != (0x444d4e4d, 0x31544552, 1, 64)
            or header[4] not in (0, 1) or any(header[14:16])):
        raise ValueError('Invalid native graphics retirement header')
    names = ('devices', 'surfaces', 'dcs', 'clippers', 'palettes', 'pixel_bytes')
    counts = dict(zip(names, header[5:11]))
    if (any(counts[key] > limit for key, limit in zip(names, (64,512,128,64,256,256*1024*1024)))
            or header[11] < counts['devices'] or header[12] < counts['surfaces']
            or header[13] not in (0, 1)):
        raise ValueError('Invalid native graphics retirement counters')
    rows = [list(words[i:i+16]) for i in range(16, len(words), 16)]
    if (sum(row[0] == 1 for row in rows) != counts['devices']
            or sum(row[0] == 2 for row in rows) != counts['surfaces']
            or any(row[0] not in (1, 2) for row in rows)
            or len({row[1] for row in rows}) != len(rows)):
        raise ValueError('Native graphics retirement owner inventory differs from counters')
    return dict(schema=1, path=str(path), sha256=hashlib.sha256(raw).hexdigest(),
                process_exit=bool(header[4]), counts=counts, surviving_owners=rows,
                factory_creations=header[11], surface_creations=header[12],
                ddraw_module_loaded=bool(header[13]),
                resources_released=not any(counts.values()))


def require_retired(path):
    report = read_retirement(path)
    if report['ddraw_module_loaded']:
        raise ValueError('Real DirectDraw loaded during native graphics retirement')
    if not report['resources_released']:
        raise ValueError('Native graphics references survived normal Quit: '+str(report['counts']))
    if not report['factory_creations']:
        raise ValueError('Native graphics retirement lacks startup activity')
    return report


def read_startup_retirement(path):
    """Positive execution of the guarded checker cleanup, not normal Release."""
    path = Path(path)
    if path.is_symlink() or not path.is_file():
        raise ValueError('Missing owned native startup probe retirement receipt')
    raw = path.read_bytes()
    if len(raw) != 64:raise ValueError('Invalid startup probe retirement extent')
    r = struct.unpack('<16I', raw)
    if (r[:4] != (0x444d4e4d,0x31425250,1,64) or r[4] not in (0xbdb2a,0xbdb6f)
            or not 1 <= r[5] <= 3 or r[5] != sum(r[6:9]) or any(v not in (0,1) for v in r[6:9])
            or not 1 <= r[9] <= 64 or r[10]+1 != r[9]
            or not 1 <= r[11] <= 512 or r[12]+1 != r[11] or r[13:] != (1,0,0)):
        raise ValueError('Invalid native startup probe reference retirement')
    return dict(path=str(path),sha256=hashlib.sha256(raw).hexdigest(),caller_rva=r[4],
                surface_references_released=r[5],surface1=r[6],surface4=r[7],surface3=r[8],
                devices_released=1,surfaces_released=1,original_release_result_preserved=True)
