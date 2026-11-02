"""Physical campaign-driver geometry and calibrated installed Forest waypoints.

These are test policies, not a native simulation or general map assembler.
The 20-cell unrotated translation is calibrated against three retained original
spawn anchors in validation-waypoint-calibration-01.json.
"""
import configparser
import hashlib
from pathlib import Path
import re
import struct


def frame_size(size):
    size=tuple(size)
    if size not in ((800,600),(640,480)):
        raise ValueError('Unsupported campaign frame extent')
    return size


def hud_point(point,size=(800,600)):
    width,height=frame_size(size)
    return point[0]+width-800,point[1]+height-600


def project(hero,target,size=(800,600),body=False):
    width,height=frame_size(size)
    dx,dy,dz=(target[k]-hero[k] for k in ('x','y','z'))
    return round(width/2+32*(dx-dy)),round(height/2+16*(dx+dy-dz)-(22 if body else 0))


def aim_point(hero,target,size=(800,600),body=False):
    width,height=frame_size(size);x,y=project(hero,target,size,body)
    if not (48<=x<width-40 and 64<=y<height-90):
        return None
    if x>width-170 and (y<100 or y>height-180):
        return None
    return x,y


def approach_point(hero,target,size=(800,600)):
    width,height=frame_size(size);x,y=project(hero,target,size)
    dx,dy=x-width/2,y-height/2
    scale=max(1,abs(dx)/min(230,width/2-170),abs(dy)/min(130,height/2-70))
    return round(width/2+dx/scale),round(height/2+dy/scale)


def forest_waypoints(game):
    game=Path(game);cfg=game/'Realms/Celtic/Celtic.cfg'
    config=configparser.ConfigParser(interpolation=None,strict=False)
    config.read_string(cfg.read_text(encoding='cp1252'))
    specific={n:(r,x,y) for n,r,x,y in (tuple(map(int,v)) for v in
        re.findall(r'\((-?\d+),(-?\d+),(-?\d+),(-?\d+)\)',config['REGION1']['Specific']))}
    if any(specific.get(n)!=p for n,p in {50:(0,0,3),52:(0,1,2),54:(0,2,1)}.items()):
        raise ValueError('Uncalibrated Forest placement recipe')
    areas={};inputs={str(cfg.relative_to(game)):hashlib.sha256(cfg.read_bytes()).hexdigest()}
    for section in (50,52,54):
        path=game/f'Realms/Celtic/Forest/CFsec{section}.evt';raw=path.read_bytes()
        if len(raw)<16 or raw[:4]!=b'EVT\0' or struct.unpack_from('<I',raw,8)[0]!=1:
            raise ValueError('Invalid Forest event-area header')
        count=struct.unpack_from('<I',raw,12)[0]
        if count>1024 or len(raw)!=16+72*count:
            raise ValueError('Invalid bounded Forest event areas')
        inputs[str(path.relative_to(game))]=hashlib.sha256(raw).hexdigest()
        _,x,y=specific[section]
        for row in struct.iter_unpack('<6i48s',raw[16:]):
            name=row[6].split(b'\0')[0].decode('ascii')
            if name in areas:
                raise ValueError('Ambiguous Forest event-area name')
            areas[name]=dict(x=row[0]+20*x,y=row[1]+20*y,z=row[2])
    required=('CF50 Redcap1_1','CF52 Bridge','CF52 PoP','CF54 PoP')
    if any(name not in areas for name in required):
        raise ValueError('Missing calibrated Forest waypoint')
    return dict(points={name:areas[name] for name in required},inputs=inputs,
                scope='Unrotated fixed-section physical-input candidates only; power capture, mana recovery and assembled-map identity require live evidence')


def validate_opening_anchors(route,rows):
    # Slots can differ across launches. Retain the first observed tuple for each
    # slot, then require the two named neutral Redcap spawn positions by role.
    first={}
    for row in rows:
        if row[1]==1 and row[4] not in first:first[row[4]]=row
    positions={tuple(r[9:12]) for r in first.values() if r[5]==10 and r[6]==2}
    expected={tuple(route['points'][name][k] for k in ('x','y','z'))
              for name in ('CF50 Redcap1_1','CF52 Bridge')}
    if not expected<=positions:
        raise ValueError('Original opening spawns do not support calibrated route')
    return dict(verified=True,neutral_spawn_positions=[list(p) for p in sorted(expected)])
