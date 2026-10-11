"""Portable references at observed original opaque primary publication sites.

The broader presented-frame and complete-action journey boundary stays separate.
"""
import json
import os
from pathlib import Path
import subprocess
import struct
import tempfile
from drawing_corpus import ROOT,BUILD,artifact,local,sha,compare,load_tool
from drawing_scene_corpus import asset_name,raster_inputs
from drawing_session import same_json

CATALOG='tests/fixtures/drawing-publication/publication.json'
RETENTION='NR.drawing-publication.retention-20261011'



def validate_trace(data,observed,records):
    version=observed.get('wire_version',1)
    if (version not in (1,2) or len(data)<128 or len(data)>32+65536*96 or (len(data)-32)%96 or
        data[:8]!=(b'MNMDP001' if version==1 else b'MNMDP002') or
        struct.unpack_from('<6I',data,8)!=(version,96,0x40209ca7,65536,1024,0)):
        raise ValueError('Invalid original portable publication envelope')
    calls=[];frames=[];prior=0
    for i,r in enumerate(struct.iter_unpack('<24I',data[32:]),1):
        status=struct.unpack('<i',struct.pack('<I',r[2]))[0]
        if (r[0]!=i or r[1] not in (1,2) or r[8] not in (0,1) or r[9]!=(0x1000000 if r[1]==1 else 16) or
            not prior<=r[4]<len(records) or r[5]>observed['producers']['queues'] or
            (any(r[22:]) if version==1 else not r[22] or r[23]>8192)):
            raise ValueError('Invalid portable COM attempt identity')
        prior=r[4]
        if r[8] and status>=0:
            if (r[19]!=len(frames)+1 or r[10] or r[11]&0x80000000 or r[14:18]!=(16,0xf800,0x7e0,31) or
                not 1<=r[12]<=2048 or not 1<=r[13]<=2048 or not r[12]*2<=r[18]<=8192):
                raise ValueError('Incomplete portable primary readback')
            f=observed['frames'][len(frames)]
            expected={'ordinal':r[19],'attempt':r[0],'source_sequence':r[4],'queue':r[5],'canvas':r[6],'destination':r[7],'width':r[12],'height':r[13]}
            if not same_json({k:f[k] for k in expected},expected):raise ValueError('Portable original frame identity changed')
            frames.append(f)
        elif r[19]:raise ValueError('Oracle belongs to a failed/offscreen attempt')
        if r[20] or r[21]!=len(frames):raise ValueError('Portable primary loss/count mismatch')
        calls.append({'attempt':r[0],'route':'Blt' if r[1]==1 else 'BltFast','hresult':status,'last_error':r[3],
                      'source_sequence':r[4],'queue':r[5],'source_canvas':r[6],'destination_canvas':r[7],'primary':bool(r[8]),
                      'flags':r[9],'observer_lock_hresult':r[10],'observer_unlock_hresult':r[11],'readback_ordinal':r[19],'thread':r[22],'action_record':r[23]})
    if version==1 and observed['calls'] and 'thread' not in observed['calls'][0]:
        calls=[{k:v for k,v in c.items() if k not in ('thread','action_record')} for c in calls]
    if not same_json(calls,observed['calls']) or len(frames)!=len(observed['frames']) or not calls[-1]['primary'] or calls[-1]['hresult']<0 or calls[-1]['queue']!=observed['producers']['queues']:
        raise ValueError('Portable publication history differs from original acquisition')
    return calls,frames


def admit(root,catalog,register):
    from rendering_coverage import evidence_record
    evidence={e['id']:e for e in register['evidence']};builds={b['id']:b for b in register['builds']}
    retained=evidence_record(root,evidence[RETENTION],builds)
    if not same_json(catalog,retained) or type(catalog['schema']) is not int or catalog['schema']!=1 or catalog['source_sha256']!=BUILD:
        raise ValueError('Publication retention schema/identity changed')
    ev=evidence[catalog['evidence']];comp_ev=evidence[catalog['asset_binding_evidence']]
    if ev['kind']!='isolated_original' or ev['build']!=catalog['build'] or ev['sha256']!=catalog['historical_report_sha256'] or comp_ev['kind']!='native_integration':
        raise ValueError('Publication requires independent original acquisition and owned asset bindings')
    original=evidence_record(root,ev,builds);comparison=evidence_record(root,comp_ev,builds)
    observed=catalog['observation']
    if (not same_json(observed,original['drawing_publication']) or original['original_manifest_verified_before_after'] is not True or
        original['original_pixels_used_as_native_inputs'] is not False or comparison['capture_report_sha256']!=ev['sha256'] or
        comparison['original_pixels_used_as_native_inputs'] is not False or comparison['original_oracles_read_by_native'] is not False or
        observed['boundary']!='observed_primary_blt' or observed['readback_drops'] or observed['all_primary_routes_observed'] is not False or
        observed['actions_bound'] is not False or observed['producers']['failures'] or not catalog['scope'].strip()):
        raise ValueError('Publication original provenance or declared boundary changed')
    chunks=catalog['input_chunks'];raw=bytearray()
    if not 1<=len(chunks)<=16:raise ValueError('Publication chunk budget exceeded')
    for entry in chunks:
        data=artifact(root,entry)
        if len(raw)+len(data)>512*1024*1024:raise ValueError('Publication stream budget exceeded')
        raw.extend(data)
    raw=bytes(raw)
    if sha(raw)!=observed['producers']['sha256']:raise ValueError('Original producer stream changed')
    _,records=load_tool('inspect-canvas-producers').decode(raw,complete=False)
    if len(records)!=observed['producers']['records']:raise ValueError('Publication record count changed')
    trace=artifact(root,catalog['trace'])
    if sha(trace)!=observed['trace_sha256']:raise ValueError('Original COM trace changed')
    calls,frames=validate_trace(trace,observed,records)
    if observed.get('wire_version',1)==2:
        actions=artifact(root,catalog['actions'])
        if sha(actions)!=observed['applied_actions']['sha256']:raise ValueError('Original applied action trace changed')
        from drawing_publication import applied_actions
        with tempfile.TemporaryDirectory(prefix='portable-applied-actions-') as tmp:
            directory=Path(tmp);(directory/'drawing-actions-0000.bin').write_bytes(actions)
            if not same_json(applied_actions(directory,calls,frames,len(records)),observed['applied_actions']):raise ValueError('Portable applied actions differ from original')
    assets=[];names=set();owned_hashes={sha(raw),sha(trace)}
    for entry in catalog['assets']:
        requested=entry['name'];name=asset_name(requested);data=artifact(root,entry)
        if (name.casefold() in names or comparison['asset_sources'].get(requested)!=entry['historical_path'] or
            comparison['inputs'].get(entry['historical_path'])!=sha(data)):
            raise ValueError('Publication source asset identity changed')
        names.add(name.casefold());owned_hashes.add(sha(data));assets.append((requested,name,data))
    if set(comparison['asset_sources'])!={a[0] for a in assets} or not assets:raise ValueError('Incomplete publication source asset bindings')
    expected=[];boundaries=[];prior=0
    if len(catalog['frames'])!=len(observed['frames']) or not 1<=len(catalog['frames'])<=1024:raise ValueError('Partial publication frame history')
    for retained_frame,frame in zip(catalog['frames'],observed['frames']):
        if not same_json(retained_frame['identity'],frame) or not prior<frame['source_sequence']<=len(records):raise ValueError('Publication frame identity/order changed')
        prior=frame['source_sequence'];data=artifact(root,retained_frame['oracle'])
        if sha(data)!=frame['sha256'] or sha(data) in owned_hashes or len(data)!=frame['width']*frame['height']*2:
            raise ValueError('Incomplete/oracle-dependent primary reference')
        if not comparison['frames'][len(expected)]['matching'] or comparison['frames'][len(expected)]['original_sha256']!=sha(data):raise ValueError('Original publication acquisition identity mismatch')
        queues=sum(r[2]==12 for r,_ in records[:prior])
        boundaries.append(dict(sequence=prior,queues=queues,canvas=frame['canvas'],width=frame['width'],height=frame['height']))
        expected.append((frame,data))
    prefix=raw[:64+sum(r[0] for r,_ in records[:prior])]
    return prefix,assets,boundaries,expected


def check(*,root,catalog_path=CATALOG,executable,headless=False):
    path=local(root,catalog_path);catalog_bytes=path.read_bytes();catalog=json.loads(catalog_bytes)
    register_bytes=(root/'research/runtime/coverage/register.json').read_bytes();register=json.loads(register_bytes)
    raw,assets,boundaries,planes=admit(root,catalog,register)
    parent=root/'working/tests/drawing-publication-corpus';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent))
    owned=run/'owned';owned.mkdir();asset_root=owned/'assets';asset_root.mkdir()
    for _,name,data in assets:
        target=asset_root/name.casefold();target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data)
    (owned/'inputs.bin').write_bytes(raster_inputs(raw))
    config={'schema':2,'boundary':boundaries[-1],'frames':boundaries,'assets':{name:sha(data) for name,_,data in assets},'retain_checkpoints':[]}
    (owned/'scene.json').write_text(json.dumps(config)+'\n')
    binary=Path(executable).resolve();binary_hash=sha(binary.read_bytes());command=[str(binary),str(owned/'inputs.bin'),str(asset_root),str(owned/'scene.json'),str(run/'native')];env=dict(os.environ)
    if headless:command=['xvfb-run','-a',*command];env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')
    completed=subprocess.run(command,capture_output=True,env=env,timeout=600);(run/'native.log').write_bytes(completed.stdout+completed.stderr)
    if completed.returncode:raise ValueError('Native publication replay refused; inspect '+str(run/'native.log'))
    native=json.loads((run/'native/scene-report.json').read_text())
    if (native['success'] is not True or type(native['remaining_surfaces']) not in (int,float) or native['remaining_surfaces']!=0 or
        native['original_pixels_used_as_native_inputs'] is not False or native['original_oracles_read'] is not False or
        native['frames_published']!=len(planes) or len(native['frames'])!=len(planes)):
        raise ValueError('Native publication output is partial or oracle-dependent')
    checks=[]
    for i,((identity,expected),boundary,frame) in enumerate(zip(planes,boundaries,native['frames']),1):
        if not same_json({k:frame[k] for k in boundary},boundary) or type(frame['ordinal']) is not int or frame['ordinal']!=i:
            raise ValueError('Native publication frame order/identity changed')
        actual=(run/f'native/frame-{i:04}.565').read_bytes()
        if sha(actual)!=frame['sha256']:raise ValueError('Native publication output hash changed')
        checks.append({'identity':identity,'comparison':compare(actual,expected,identity['width'],identity['height'])})
    if sha(binary.read_bytes())!=binary_hash or path.read_bytes()!=catalog_bytes or (root/'research/runtime/coverage/register.json').read_bytes()!=register_bytes:
        raise ValueError('Publication corpus/register/backend changed')
    admit(root,catalog,register)
    return {'success':all(c['comparison']['matching'] for c in checks),'frames':checks,
            'pixels_compared':sum(c['comparison']['pixels'] for c in checks),'mismatches':sum(c['comparison']['mismatching_pixels'] for c in checks),
            'native_binary_sha256':binary_hash,'remaining_surfaces':0,'boundary':'observed_primary_blt',
            'all_primary_routes_observed':False,'actions_bound':False,'original_pixels_used_as_native_inputs':False,
            'original_work_bypassed':False,'new_original_execution':False}
