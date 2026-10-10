"""Read and validate passive cast/impact diagnostics; EAX is not an outcome."""
import struct
from campaign_gameplay import signed

HEADER = b'MNMCA001' + struct.pack('<II', 1, 64)


def read_spell_rows(path, complete=False):
    data = path.read_bytes()
    if data[:16] != HEADER or complete and (len(data)-16) % 64:
        raise ValueError('Invalid or truncated spell observation')
    end = 16 + (len(data)-16)//64*64
    rows = list(struct.iter_unpack('<16I', data[16:end]))
    if len(rows) >= 65536 or [r[0] for r in rows] != list(range(1, len(rows)+1)) or any(r[1] not in (1, 2) for r in rows):
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
