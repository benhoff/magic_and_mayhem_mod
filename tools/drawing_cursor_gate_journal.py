"""Prospective strict MNMDGT01diagnostic journal, no successful-prefix admission."""
import struct

def decode(data):
    if len(data)<64 or data[:8]!=b'MNMDGT01':raise ValueError('Gate journal magic/header')
    header=struct.unpack('<16I',data[:64])
    if header[2:6]!=(1,192,0x40209ca7,8192) or any(header[12:]):raise ValueError('Gate journal header fields')
    count,drops,reason,active,fault,orphans=header[6:12]
    if count>8192 or len(data)!=64+count*192:raise ValueError('Gate journal extent')
    rows=[]
    for ordinal in range(count):
        row=struct.unpack('<48I',data[64+ordinal*192:64+(ordinal+1)*192])
        if row[0]!=ordinal+1 or row[1] not in range(1,12) or not row[2] or any(row[32:]):raise ValueError('Gate journal row fields')
        point=row[16:32]
        if any(point):
            if point[0:3]!=(1,128,0x40209ca7) or point[5]!=row[5] or point[7]!=row[6] or point[8]!=row[7]:raise ValueError('Gate point binding')
        rows.append(dict(ordinal=row[0],phase=row[1],owner=row[2],tick=row[3],reason=row[4],token=row[5],outer_identity=row[6],inner_identity=row[7],original_result=row[8],diagnostic_result=row[9],last_error=row[10],role_mask=row[11],call_identity=row[12],mutex_identity=row[13],pending_wrappers=row[14],flags=row[15],point=list(point)))
    return dict(rows=rows,dropped=drops,terminal_reason=reason,active_leases=active,fault=fault,orphans=orphans,complete=False,clean_header=(not drops and not active and not fault and not orphans and reason==1),original_behavior_equivalence=False)

def account(observation):
    """Prospective scoped lifecycle pairing; no complete capture promotion."""
    creations={};identities={};retirements={};tokens={};drained={};gaps=[]
    for row in observation['rows']:
        phase=row['phase'];call=row['call_identity'];identity=row['mutex_identity'];role=row['role_mask'];token=row['token']
        if phase==1:
            if not call or call in creations:gaps.append('duplicate/zero creation call')
            else:creations[call]=row
        elif phase==2 and row['diagnostic_result']:
            if call not in creations or not creations[call]['original_result'] or not identity or identity in identities or role not in (1,2):gaps.append('unbound publication')
            else:identities[identity]=dict(role=role,retired=False,live=True)
        elif phase==3:
            state=identities.get(identity)
            if not call or call in retirements or not state or not state['live'] or state['role']!=role:gaps.append('unbound retirement')
            else:state['retired']=True;retirements[call]=identity
        elif phase==4 and role:
            if call not in retirements:gaps.append('close return without retirement')
            else:retirements.pop(call)
        elif phase==11:
            state=identities.get(identity)
            if call or row['reason']!=3 or row['diagnostic_result']!=1 or not state or not state['live'] or state['retired'] or state['role']!=role:gaps.append('unbound diagnostic snapshot retirement')
            else:state['retired']=True
        elif phase==5:
            state=identities.get(identity)
            if not state or not state['live'] or not state['retired'] or state['role']!=role:gaps.append('private close without retired identity')
            elif any(identity in pair[1:] for pair in tokens.values()):gaps.append('private close before explicit drain')
            elif row['diagnostic_result']:state['live']=False
        elif phase==6 and row['diagnostic_result']:
            if not token or token in tokens or row['point'][13]!=3:gaps.append('invalid begin token')
            else:tokens[token]=(row['owner'],row['outer_identity'],row['inner_identity'])
        elif phase==10 and row['diagnostic_result']:
            expected=(row['owner'],row['outer_identity'],row['inner_identity'])
            if tokens.get(token)!=expected or row['point'][13]:gaps.append('unbound drain token')
            else:drained[token]=tokens.pop(token)
        elif phase==7 and row['diagnostic_result']:
            if drained.get(token)!=(row['owner'],row['outer_identity'],row['inner_identity']) or row['point'][13]:gaps.append('end without drain')
            else:drained.pop(token)
    if not observation['rows'] or observation['rows'][-1]['phase']!=9:gaps.append('missing final terminal row')
    if sum(row['phase']==9 for row in observation['rows'])!=1:gaps.append('terminal count')
    if observation['rows'] and observation['rows'][-1]['reason']!=observation['terminal_reason']:gaps.append('terminal reason mismatch')
    if retirements:gaps.append('pending original close returns')
    if tokens:gaps.append('outstanding gate tokens')
    if drained:gaps.append('drain without end result')
    return dict(gaps=gaps,remaining_live_identities=[i for i,s in identities.items() if s['live']],complete_journal_admitted=False)
