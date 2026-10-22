"""Exact original cursor journal-prefix joins; no interpolation or click inference."""
import struct
from drawing_api_session import decode as decode_api


def bind(hud,data):
    observed=decode_api(data)
    rows=list(struct.iter_unpack('<32I',data[64:]));last_cursor={};latest_by_prefix=[{}]
    max_entry=0;entry_by_prefix=[0]
    for row in rows:
        max_entry=max(max_entry,row[29]);entry_by_prefix.append(max_entry)
        if row[1]==2 and row[2]==3:
            last_cursor[row[6]]=row
            if len(last_cursor)>16:raise ValueError('Cursor point owner capacity exceeded')
        latest_by_prefix.append(dict(last_cursor))
    links=[];refused=missing=0
    for sample in hud['samples']:
        point=sample.get('input_point')
        if point is None:refused+=1;continue
        prefix=point['record_prefix']
        if type(prefix) is not int or not 0<=prefix<len(latest_by_prefix) or type(point['entry_prefix']) is not int or not entry_by_prefix[prefix]<=point['entry_prefix']<=max_entry:
            raise ValueError('Cursor point API prefix differs')
        if point['owner_thread']!=sample['thread'] or point['flags']&~3 or point.get('drops') or point.get('gaps'):
            raise ValueError('Cursor point owner/loss identity differs')
        active=bool(point['flags']&1)
        if active==(prefix==len(rows)) or not active and point.get('pending'):
            raise ValueError('Cursor point active/terminal identity differs')
        planes=rows[prefix-1][30] if prefix and rows[prefix-1][1]!=4 else rows[prefix-1][22] if prefix else 0
        if point.get('after_frame',planes)!=planes:raise ValueError('Cursor point frame prefix differs')
        admissible=point['flags']==3
        if type(point['current_cursor_admissible']) is not bool or point['current_cursor_admissible']!=admissible:
            raise ValueError('Cursor point invented cursor admission')
        expected=latest_by_prefix[prefix].get(point['owner_thread'])
        if expected is None:
            if point['cursor_record'] or point['cursor_entry'] or point['cursor_tick'] or point['cursor'] is not None or point['flags']&2:raise ValueError('Cursor point invents an owner record')
            missing+=1;continue
        if point['owner_thread']!=sample['thread'] or (point['cursor_record'],point['cursor_entry'],point['cursor_tick'])!=(expected[0],expected[29],expected[11]):
            raise ValueError('Cursor point is not latest exact completed owner call')
        cursor=list(expected[27:29]) if expected[31] else None
        if bool(point['flags']&2)!=(cursor is not None) or point['cursor']!=cursor:raise ValueError('Cursor point result/coordinate words differ')
        age=(sample['tick']-expected[11])&0xffffffff
        if point['cursor_age_ms']!=age or point['timely_cursor']!=(point['current_cursor_admissible'] and age<=100):raise ValueError('Cursor point age classification differs')
        links.append(dict(hud_ordinal=sample['ordinal'],api_record=expected[0],api_entry=expected[29],record_prefix=prefix,cursor=cursor,cursor_age_ms=age,timely_cursor=point['timely_cursor'],journal_active=bool(point['flags']&1),candidate_state=sample['candidate_state'],actor_slot=sample['actor_slot'],actor_type=sample['actor_type'],actor_owner=sample['actor_owner']))
    return dict(success=True,linked_samples=len(links),refused_points=refused,missing_owner_points=missing,stopped_linked_samples=sum(not x['journal_active'] for x in links),timely_samples=sum(x['timely_cursor'] for x in links),timely_validated_hits=sum(x['timely_cursor'] and x['actor_slot'] is not None for x in links),links=links,api_journal_records=observed['records'],selection_verified=False,tooltip_verified=False,click_dispatch_proved=False,input_to_state_causality_proved=False,complete_presented_journey=False)
