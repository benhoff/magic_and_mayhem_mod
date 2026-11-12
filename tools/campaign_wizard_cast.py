"""Bounded ordinary Wizard-input policy; original picking/casts/hits stay separate."""
import math


def roles(hero,boss):
    return (hero.get('type')==0 and hero.get('owner')==0 and hero.get('active') and hero.get('health',0)>0
        and boss.get('type')==24 and boss.get('owner')==1 and boss.get('active') and 0<boss.get('health',0)<=100
        and all(type(v.get('slot')) is int and 0<=v['slot']<256 for v in (hero,boss)) and hero['slot']!=boss['slot']
        and all(type(v.get(k)) is int and 0<=v[k]<bound for v in (hero,boss) for k,bound in (('x',80),('y',80),('z',24))))


def probe_points(hero,boss,size,previous=None):
    if not roles(hero,boss) or tuple(size) not in ((640,480),(800,600)):return []
    w,h=size;dx,dy,dz=(boss[k]-hero[k] for k in ('x','y','z'))
    x,y=w//2+32*(dx-dy),h//2+16*(dx+dy-dz)-22
    def visible(p):
        return (isinstance(p,(tuple,list)) and len(p)==2 and all(type(v) is int for v in p)
            and 48<=p[0]<w-40 and 64<=p[1]<h-90 and not (p[0]>w-170 and (p[1]<100 or p[1]>h-180)))
    # Try the prior actually picked Boss point, then upper-body offsets before
    # spending the short ordinary pause on the lower row. These remain proposals.
    offsets=[(0,0),(0,-18),(-32,-18),(32,-18),(-64,-18),(64,-18),
        (0,-32),(-32,-32),(32,-32),(-64,-32),(64,-32),
        (-32,0),(32,0),(-64,0),(64,0),(0,22),(-32,22),(32,22),(-64,22),(64,22)]
    proposed=([tuple(previous)] if visible(previous) else [])+[(x+ox,y+oy) for ox,oy in offsets]
    return list(dict.fromkeys(p for p in proposed if visible(p)))[:20]


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
        and (tick-r[3])%2**32<=20000]
    return boss if hits else None


def retry_finishing(record,prior_count):
    """One retry of a released zero-cast window; never another accepted volley."""
    return (type(prior_count) is int and prior_count==0 and isinstance(record,dict)
        and record.get('wizard_only') is True and record.get('released') is True
        and type(record.get('accepted_in_burst')) is int and record['accepted_in_burst']==0)
