"""Bind fresh analysis claims to the already recorded current publisher source.

This changes a required comparer input, never a retained execution source/hash.
Every Native, runtime, model, oracle and behavior/scenario claim stays exact.
"""
import hashlib,json
from pathlib import Path
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261110/'
REVIEW=P+'nominal-claims-source-review-01.json'
PUBLISHER='tools/owned-menu-game-runner.py'
OLD='92445eea3f0d190065ffa2ba7d02b709ae7c32470b67025e5e080c5e8548aad5'
NEW='83c7aea4a230b12e2dca8d95e82b390b831d2b5259d2af81de17a1d7e1cc10c9'

def current_claims(old):
    require(old['sources'].get(PUBLISHER)==OLD,'Unreviewed nominal publisher source')
    result=json.loads(json.dumps(old));result['sources'][PUBLISHER]=NEW
    return result

def comparison_command(reader,root,phase,command):
    result=list(command)
    if phase['family']!='caller':return result,None
    require(result.count('--claims')==1,'Missing/ambiguous nominal claims selector')
    q=reader.json(REVIEW)
    require(q['success']and q['original_game_runs']==q['new_request_reservations']==0
            and q['changed_claim_source_paths']==[PUBLISHER]
            and all(reader.digest(n)==h for n,h in q['sources'].items()),'Changed analysis claims review')
    at=result.index('--claims')+1
    require(at<len(result)and result[at]==q['old_input_path'],'Foreign nominal claims input')
    before=Path(result[at]);after=Path(q['new_input_path'])
    require(before.is_absolute()and after.is_absolute()and before.is_relative_to(root/'working')and after.is_relative_to(root/'working')and before!=after,'Claims input escapes owned working area')
    require(hashlib.sha256(before.read_bytes()).hexdigest()==q['old_input']['sha256']
            and hashlib.sha256(after.read_bytes()).hexdigest()==q['new_input']['sha256'],'Claims input changed')
    require(reader.read_bytes(q['old_input']['record'])==before.read_bytes()
            and reader.read_bytes(q['new_input']['record'])==after.read_bytes(),'Claims payload custody differs')
    old,new=json.loads(before.read_bytes()),json.loads(after.read_bytes())
    require(current_claims(old)==new,'Unknown claims source or contract change')
    require(all(reader.digest(n)==h for n,h in new['sources'].items()),'Nominal analysis source not current')
    result[at]=str(after)
    return result,dict(record=REVIEW,sha256=reader.digest(REVIEW))
