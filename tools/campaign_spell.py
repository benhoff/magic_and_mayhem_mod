"""Read and validate passive cast/impact diagnostics; EAX is not an outcome."""
import struct
from campaign_gameplay import signed, creatures

HEADER = b'MNMCA001' + struct.pack('<II', 1, 64)
HEADER_V2 = b'MNMCA002' + struct.pack('<II', 2, 64)


def read_spell_rows(path, complete=False):
    data = path.read_bytes()
    if data[:16] not in (HEADER,HEADER_V2) or complete and (len(data)-16) % 64:
        raise ValueError('Invalid or truncated spell observation')
    end = 16 + (len(data)-16)//64*64
    rows = list(struct.iter_unpack('<16I', data[16:end]))
    kinds=(1,2,3) if data[:16]==HEADER_V2 else (1,2)
    if len(rows) >= 65536 or [r[0] for r in rows] != list(range(1, len(rows)+1)) or any(r[1] not in kinds for r in rows):
        raise ValueError('Invalid or exhausted spell observation sequence')
    return rows


def validate_spell_observation(rows, gameplay, inputs):
    if not rows or len({r[2] for r in rows+gameplay}) != 1:
        raise ValueError('Missing same-thread original spell observations')
    summon = inputs['summon']
    begin = gameplay[summon['before_sequence']-1][3]
    end = gameplay[summon['after_sequence']-1][3]
    casts = [r for r in rows if r[1] == 1]
    player = [r for r in casts if r[4] == 14 and r[5] == inputs['wizard_slot'] and r[6] == 0
              and r[7] == inputs['player_owner'] and r[15] == 1
              and ((r[3]-begin) & 0xffffffff) <= ((end-begin) & 0xffffffff)
              and signed(r[8]) > signed(r[9]) >= 0]
    if not player:
        raise ValueError('Successful native summon lacks original caster mana debit')
    impacts = [r for r in rows if r[1] == 2 and r[14] and signed(r[10]) > 0 and signed(r[11]) < signed(r[10])]
    return dict(cast_returns=len(casts), impact_returns=sum(r[1] == 2 for r in rows),
                first_player_summon_cast=list(player[0]), observed_mana_debit=signed(player[0][8])-signed(player[0][9]),
                damaging_impacts=[list(r) for r in impacts],
                scope='Original cast/impact returns and successful summon mana debit only; EAX acceptance, refunds, ranged source lineage and formulas excluded')


def validate_spell_cases(rows, gameplay, inputs):
    observation = validate_spell_observation(rows, gameplay, inputs)
    owner, slot = inputs['player_owner'], inputs['wizard_slot']
    refused = inputs.get('blocked_casts', [])
    if sorted(p.get('kind', '') for p in refused) != ['insufficient-mana', 'invalid-target']:
        raise ValueError('Missing distinct blocked native cast cases')

    def interval(item):
        begin, end = item['before_sequence'], item['after_sequence']
        if not 0 < begin < end <= gameplay[-1][0]:
            raise ValueError('Invalid spell-case observation interval')
        first = creatures([r for r in gameplay if r[0] <= begin])
        samples = [r for r in gameplay if begin < r[0] <= end and r[1] == 1]
        last = creatures([r for r in gameplay if r[0] <= end])
        actors = [first.get(slot)] + [creatures([r])[slot] for r in samples if r[4] == slot]
        if len(actors) < 2 or any(not a or a['type'] != 0 or a['owner'] != owner or not a['active'] or a['health'] <= 0 for a in actors):
            raise ValueError('Spell-case caster identity changed or died')
        t0, t1 = gameplay[begin-1][3], gameplay[end-1][3]
        elapsed = (t1-t0) & 0xffffffff
        casts = [r for r in rows if r[1] == 1 and r[5] == slot and r[6] == 0 and r[7] == owner
                 and ((r[3]-t0) & 0xffffffff) <= elapsed]
        return first, last, samples, casts, elapsed

    refusals = []
    for probe in refused:
        first, last, samples, casts, elapsed = interval(probe)
        if elapsed < 1500 or probe['created_slots'] or probe['mana_before'] != first[slot]['mana'] or probe['mana_after'] != last[slot]['mana']:
            raise ValueError('Blocked cast evidence is incomplete or altered')
        if last[slot]['mana'] < first[slot]['mana'] or any(r[4] == 14 and (not r[15] or signed(r[9]) < signed(r[8])) for r in casts):
            raise ValueError('Blocked cast spent mana or changed caster identity')
        for r in samples:
            old = first.get(r[4])
            if r[5] == 14 and r[6] == owner and r[7]:
                if signed(r[8]) > 0 and (not old or not old['active'] or old['health'] <= 0 or old['type'] != 14 or old['owner'] != owner):
                    raise ValueError('Blocked cast created a living player Zombie')
        if probe['kind'] == 'insufficient-mana' and first[slot]['mana'] >= observation['observed_mana_debit']:
            raise ValueError('Caster had enough mana for the observed summon cost')
        refusals.append(dict(kind=probe['kind'], original_cast_returns=sum(r[4] == 14 for r in casts), elapsed_ms=elapsed))

    ranged = player_fireball_damage(rows,owner,slot)
    if not ranged:raise ValueError('Missing original player Fireball defended damage')
    return dict(verified=True, refusals=refusals, ranged_damage_events=len(ranged),first_ranged_damage=list(ranged[0]),
                scope='Bounded invalid-target/insufficient-mana native input with no new Zombie or mana loss, and original defended enemy damage inside a player-sourced Fireball effect. Pre-ingress UI refusal is distinguished from cast returns; admission/refund/damage formulas, Cure and full animation equivalence remain pending')


def player_fireball_damage(rows,owner,wizard_slot):
    return [r for r in rows if r[1]==3 and r[12]==71 and r[14]==wizard_slot and r[15]==owner and r[11]==owner
            and r[6] not in (owner,0xffffffff) and r[9] and signed(r[7])>0 and signed(r[8])<signed(r[7])
            and signed(r[10])>0 and r[13] in (0x48ed24,0x48b4e2)
            and any(c[1]==1 and c[2]==r[2] and c[4]==71 and c[5]==wizard_slot and c[6]==0 and c[7]==owner
                    and c[15]==1 and signed(c[8])>signed(c[9])>=0 and ((r[3]-c[3])&0xffffffff)<=10000 for c in rows)]
