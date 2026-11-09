"""Exact original checkpoint input; later natural progression remains mandatory."""
import hashlib
from pathlib import Path
from campaign_checkpoint import save_name,decode,compare
from rendering_progression import require


def input_reference(reader,record,name):
    save_name(name);q=reader.json(record);checkpoint=q.get('checkpoint',{})
    mode=q.get('configuration',{}).get('checkpoint_mode')
    require(q.get('scenario')=='checkpoint-capture' and mode=='capture'
        and q.get('kind') in ('public_native_checkpoint_diagnostic_acquisition','public_native_campaign_lightweight_diagnostic'),'Initial input requires a retained original capture, not a reload/progression claim')
    require(q.get('sources_stable') is True and q.get('flow',{}).get('saved_name')==name,'Original captured input source/name differs')
    require(q.get('success') is True and checkpoint.get('verified') is True
        and q.get('private_wine_stopped') is True and q.get('artifact_retention_complete') is True
        and q.get('original_manifest_verified_before_after') is True,'Original checkpoint input lacks complete retained provenance')
    matches=[v for v in q['artifacts'] if v['path'].endswith('/game/Save/'+name+'.sav') and v['sha256']==checkpoint['save_sha256']]
    require(len(matches)==1,'Missing or ambiguous original checkpoint bytes')
    source=matches[0];raw=reader.read_bytes(source['record'])
    require(0<len(raw)<=16*1024**2 and hashlib.sha256(raw).hexdigest()==source['sha256']==checkpoint['save_sha256'] and len(raw)==checkpoint.get('save_bytes'),'Original checkpoint bytes differ')
    domain=checkpoint.get('domain','bridge');require(domain in ('bridge','transition-ready'),'Unknown original checkpoint input domain')
    goal=None
    if domain=='transition-ready':
        from campaign_transition_checkpoint import retired_goal
        goal=retired_goal(checkpoint.get('portal_goal'))
        require(q.get('configuration',{}).get('checkpoint_route')=='transition-ready','Transition input capture domain differs')
    return raw,dict(reference=dict(record=record,sha256=reader.digest(record)),name=name,sha256=checkpoint['save_sha256'],bytes=len(raw),original_save=source,saved_snapshot=checkpoint['saved_snapshot'],domain=domain,portal_goal=goal,scope='Unchanged original serialization as an initial input only; no historical outcome, whole-state equivalence or progression acceptance is reused.')


def validate_entry_flow(flow):
    require(flow.get('entry_mode')=='original-checkpoint' and flow.get('entry_load_verified') is True,'Initial original Load is not verified')
    name=save_name(flow.get('entry_checkpoint_name',''));sha=flow.get('entry_checkpoint_sha256','')
    require(len(sha)==64 and all(c in '0123456789abcdef' for c in sha),'Invalid original entry input hash')
    entries=[v for v in flow['states'] if v['event']=='checkpoint-entry']
    require(len(entries)==1 and flow['states'][3] is entries[0],'Initial original Load entry must be unique and precede the natural outcome')
    entry=entries[0]
    require(entry['screen']==12 and entry.get('save_name')==name and entry.get('save_count')==1
        and entry.get('parent')==3 and entry.get('depth')==3,'Initial original Load name/list/parent differs')
    require(flow['worlds'][0].get('entry_mode')=='original-checkpoint','First World was not bound to the initial original Load')
    return dict(verified=True,name=name,sha256=sha,scope='Initial original Load only; the later World must still follow natural report, Realm travel and region2 admission.')


def validate_loaded_snapshot(data,raw,reference):
    groups=decode(data);require(len(groups)==1 and groups[0]['screen']==12,'Exactly one initial original Load snapshot required')
    result=compare(reference['saved_snapshot'],groups[0]);require(result['verified'],'Original loaded actor fields differ: '+str(result['differences'][:5]))
    require(hashlib.sha256(raw).hexdigest()==reference['sha256'],'Original entry input changed')
    if reference.get('domain')=='transition-ready':
        from campaign_transition_checkpoint import transition_state
        transition_state(reference['saved_snapshot'],groups[0],reference['portal_goal'])
    return dict(verified=True,sha256=reference['sha256'],bytes=len(raw),storage=result,scope='Synchronous original active actor fields only; complete world/RNG/AI equivalence is not inferred. Later progression and rendering remain independent.')
