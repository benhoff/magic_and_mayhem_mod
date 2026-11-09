"""Bounded ordinary Wizard-input policy; original picking/casts/hits stay separate."""
import math


def roles(hero,boss):
    return (hero.get('type')==0 and hero.get('owner')==0 and hero.get('active') and hero.get('health',0)>0
        and boss.get('type')==24 and boss.get('owner')==1 and boss.get('active') and 0<boss.get('health',0)<=100
        and all(type(v.get('slot')) is int and 0<=v['slot']<256 for v in (hero,boss)) and hero['slot']!=boss['slot']
        and all(type(v.get(k)) is int and 0<=v[k]<bound for v in (hero,boss) for k,bound in (('x',80),('y',80),('z',24))))


def probe_points(hero,boss,size):
    if not roles(hero,boss) or tuple(size) not in ((640,480),(800,600)):return []
    w,h=size;dx,dy,dz=(boss[k]-hero[k] for k in ('x','y','z'))
    x,y=w//2+32*(dx-dy),h//2+16*(dx+dy-dz)-22
    return [(x+ox,y+oy) for oy in (0,22,-18,-32) for ox in (0,-32,32,-64,64)
        if 48<=x+ox<w-40 and 64<=y+oy<h-90 and not (x+ox>w-170 and (y+oy<100 or y+oy>h-180))]


def finishing_target(hero,actors,first,damage,tick,thread,already=False):
    if (already or not isinstance(first,dict) or first.get('wizard_only') is not True or first.get('released') is not True
            or type(first.get('accepted_in_burst')) is not int or not 1<=first['accepted_in_burst']<=4
            or hero.get('health',0)<40 or hero.get('mana',0)<12*256):return None
    bosses=[v for v in actors.values() if v.get('type')==24 and v.get('owner')==1 and v.get('active') and v.get('health',0)>0]
    if len(bosses)!=1:return None
    boss=bosses[0]
    if (not roles(hero,boss) or boss['slot']!=first.get('target',{}).get('slot') or boss['health']>60
            or math.hypot(boss['x']-hero['x'],boss['y']-hero['y'])>7):return None
    hits=[r for r in damage if len(r)==16 and r[1]==3 and r[2]==thread and r[4:7]==(boss['slot'],24,1)
        and 0<=r[8]<r[7]<=100 and r[9] and r[10]>0 and r[11:13]==(0,71) and r[14:16]==(hero['slot'],0)
        and (tick-r[3])%2**32<=10000]
    return boss if hits else None
