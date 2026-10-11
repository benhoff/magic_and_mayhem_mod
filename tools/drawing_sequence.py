"""Portable full producer-history comparison, with independently retained outputs."""
from collections import Counter
import json
import os
from pathlib import Path
import subprocess
import tempfile
from drawing_corpus import ROOT, BUILD, artifact, compare, local, load_tool, sha
from drawing_scene_corpus import asset_name

CATALOG = 'tests/fixtures/drawing-sequence/sequence.json'
RETENTION = 'NR.drawing-sequence.retention-20261010'
MAX_SEQUENCE_BYTES = 128 * 1024 * 1024


def oracle_planes(root,checkpoints):
    """Decode lossless deltas only against the immediately prior same canvas.

    Dependencies are implicit in ordered checkpoint identities, not arbitrary
    paths or recursive graph references. Each decoded plane is independently
    hashed against its original capture; original planes never reach rendering.
    """
    previous={}
    for c in checkpoints:
        entry=c['oracle'];key=(c['canvas'],c['width'],c['height'])
        if entry['encoding']=='xor-gzip':
            prior=previous.get(key)
            if not prior or entry['base_checkpoint']!=prior[0]:
                raise ValueError('Sequence delta base identity mismatch')
            delta=artifact(root,dict(entry,encoding='gzip',raw_sha256=entry['delta_sha256']))
            if len(delta)!=len(prior[1]):raise ValueError('Sequence delta extent mismatch')
            raw=(int.from_bytes(delta,'little')^int.from_bytes(prior[1],'little')).to_bytes(len(delta),'little')
            if sha(raw)!=entry['raw_sha256']:raise ValueError('Sequence decoded oracle hash mismatch')
        else:
            raw=artifact(root,entry)
        if len(raw)!=c['width']*c['height']*2:raise ValueError('Partial sequence oracle')
        previous[key]=(c['ordinal'],raw)
        yield c,raw


def admit(root, catalog, register):
    from rendering_coverage import evidence_record
    evidence={e['id']:e for e in register['evidence']}
    builds={b['id']:b for b in register['builds']}
    retained=evidence_record(root,evidence[RETENTION],builds)
    if catalog!=retained or catalog['schema']!=1 or catalog['source_sha256']!=BUILD:
        raise ValueError('Sequence retention identity changed')
    ev=evidence[catalog['evidence']]
    if (ev['kind'] not in ('isolated_original','live_equivalence','live_replacement') or
        ev['build']!=catalog['build'] or ev['sha256']!=catalog['historical_report_sha256'] or
        catalog['original_inputs_verified'] is not True):
        raise ValueError('Sequence requires independent original evidence')
    original=evidence_record(root,ev,builds)
    if original['original_pixels_used_as_native_inputs'] or original['original_oracles_read_by_native']:
        raise ValueError('Oracle-dependent original sequence')
    stream=catalog['historical_stream']
    if original['inputs'].get(stream['path'])!=stream['sha256']:
        raise ValueError('Original stream provenance mismatch')
    chunks=catalog['input_chunks']
    if not 1<=len(chunks)<=8 or not 0<stream['bytes']<=MAX_SEQUENCE_BYTES:
        raise ValueError('Sequence extent/chunk budget exceeded')
    raw=bytearray()
    for chunk in chunks:
        data=artifact(root,chunk)
        if len(raw)+len(data)>MAX_SEQUENCE_BYTES:
            raise ValueError('Sequence decode budget exceeded')
        raw.extend(data)
    raw=bytes(raw)
    if len(raw)!=stream['bytes'] or sha(raw)!=stream['sha256']:
        raise ValueError('Sequence chunks do not reconstruct original input')
    _,records=load_tool('inspect-canvas-producers').decode(raw)
    boundary=catalog['boundary']
    if (len(records)!=boundary['sequence'] or records[-1][0][2]!=12 or
        (records[-1][0][14],records[-1][0][3],records[-1][0][5],records[-1][0][6])!=
        (boundary['queues'],boundary['canvas'],boundary['width'],boundary['height']) or
        any(r[23] or r[2]==13 for r,_ in records) or
        dict(sorted(Counter(str(r[2]) for r,_ in records).items()))!=catalog['producer_counts']):
        raise ValueError('Sequence boundary/count/failure identity mismatch')
    entries=[r[14] for r,_ in records if r[2]==11]
    returns=[r[14] for r,_ in records if r[2]==12]
    if entries!=list(range(1,boundary['queues']+1)) or returns!=entries:
        raise ValueError('Sequence World history is not contiguous')
    assets,names=[],set()
    for a in catalog['assets']:
        name=asset_name(a['name'])
        if name.casefold() in names:raise ValueError('Duplicate sequence asset binding')
        names.add(name.casefold());data=artifact(root,a)
        if original['inputs'].get(a['historical_path'])!=sha(data):
            raise ValueError('Sequence source asset provenance mismatch')
        assets.append((a['name'],name,data))
    native={c['oracle']:c for c in original['native']['checkpoints']}
    comparisons={c['oracle']:c for c in original['checkpoints']}
    captured={r[14]:r for r,_ in records if r[2]==10}
    checkpoints=catalog['checkpoints']
    if [c['ordinal'] for c in checkpoints]!=list(range(1,len(captured)+1)):
        raise ValueError('Sequence omits or duplicates original checkpoints')
    if catalog['comparison_pixels']!=sum(c['width']*c['height'] for c in checkpoints):
        raise ValueError('Sequence comparison pixel denominator changed')
    for c,expected in oracle_planes(root,checkpoints):
        r=captured[c['ordinal']];n=native[c['ordinal']]
        if ((c['sequence'],c['canvas'],c['width'],c['height'],c['reason'],c['queue'])!=
            (r[1],r[3],r[5],r[6],r[15],r[16]) or
            (n['sequence'],n['canvas'])!=(r[1],r[3]) or not comparisons[c['ordinal']]['equal']):
            raise ValueError('Sequence checkpoint identity mismatch')
        if (len(expected)!=c['width']*c['height']*2 or n['sha256']!=sha(expected) or
            original['inputs'].get(c['oracle']['historical_path'])!=sha(expected)):
            raise ValueError('Sequence oracle provenance/extent mismatch')
    observations=catalog['observations']
    if (observations['boundary']!='composed_canvas' or observations['actions_bound'] is not False or
        observations['contiguous_presented_frames'] is not False or
        observations['contiguous_producer_records'] is not True or not catalog['remaining']):
        raise ValueError('Historical sequence cannot claim presented action/frame coverage')
    return raw,assets,checkpoints


def check(*,root=ROOT,catalog_path=CATALOG,executable,headless=False):
    root=Path(root).resolve();path=local(root,catalog_path);catalog_bytes=path.read_bytes()
    catalog=json.loads(catalog_bytes)
    register_path=root/'research/runtime/coverage/register.json'
    register_bytes=register_path.read_bytes();register=json.loads(register_bytes)
    raw,assets,checkpoints=admit(root,catalog,register)
    executable=executable.resolve();binary_hash=sha(executable.read_bytes())
    parent=root/'working/tests/drawing-sequence';parent.mkdir(parents=True,exist_ok=True)
    output=Path(tempfile.mkdtemp(prefix='run-',dir=parent));native_input=output/'inputs'
    asset_root=native_input/'assets';asset_root.mkdir(parents=True)
    (native_input/'producers.bin').write_bytes(raw)
    for _,name,data in assets:
        target=asset_root/name.casefold();target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data)
    declaration={'schema':1,'boundary':catalog['boundary'],'assets':{name:sha(data) for name,_,data in assets},
                 'retain_checkpoints':[c['ordinal'] for c in checkpoints]}
    (native_input/'scene.json').write_text(json.dumps(declaration,indent=2)+'\n')
    command=[str(executable),str(native_input/'producers.bin'),str(asset_root),str(native_input/'scene.json'),str(output/'native')]
    environment=dict(os.environ)
    if headless:
        command=['xvfb-run','-a',*command];environment.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')
    execution=subprocess.run(command,capture_output=True,timeout=600,env=environment)
    (output/'native.log').write_bytes(execution.stdout+execution.stderr)
    if execution.returncode:
        raise ValueError('Native sequence refused: '+str(output/'native.log'))
    native=json.loads((output/'native/scene-report.json').read_text())
    if (native['success'] is not True or native['boundary']!=catalog['boundary'] or native['remaining_surfaces'] or
        native['original_pixels_used_as_native_inputs'] or native['original_oracles_read'] or
        native['native_gpu_presentation_ready'] is not True):
        raise ValueError('Native sequence completion/ownership mismatch')
    completed=native['checkpoints']
    if [n['ordinal'] for n in completed]!=[c['ordinal'] for c in checkpoints]:
        raise ValueError('Native sequence omits/reorders checkpoints')
    rows=[]
    for (c,expected),n in zip(oracle_planes(root,checkpoints),completed):
        actual=(output/f"native/checkpoint-{c['ordinal']:04}.565").read_bytes()
        if any(c[k]!=n[k] for k in ('sequence','canvas','width','height')) or sha(actual)!=n['sha256']:
            raise ValueError('Native sequence checkpoint identity mismatch')
        # Byte equality checks the complete little-endian plane. Detailed word
        # diagnostics are needed only for mismatches; never sample or crop.
        if len(actual)!=c['width']*c['height']*2:raise ValueError('Partial native sequence plane')
        result=({'matching':True,'pixels':c['width']*c['height'],'mismatching_pixels':0,'bounds':None,
                 'examples':[],'native_sha256':sha(actual),'original_sha256':sha(expected)} if actual==expected
                else compare(actual,expected,c['width'],c['height']))
        rows.append({'ordinal':c['ordinal'],'sequence':c['sequence'],'canvas':c['canvas'],
                     'queue':c['queue'],'reason':c['reason'],**result})
    final=next(c for c in reversed(checkpoints) if c['canvas']==catalog['boundary']['canvas'])
    scene=(output/'native/scene.565').read_bytes()
    if (sha(scene)!=native['scene_sha256'] or
        scene!=(output/f"native/checkpoint-{final['ordinal']:04}.565").read_bytes()):
        raise ValueError('Final sequence canvas differs from final native checkpoint')
    if path.read_bytes()!=catalog_bytes or register_path.read_bytes()!=register_bytes or sha(executable.read_bytes())!=binary_hash:
        raise ValueError('Sequence catalog/register/binary changed during replay')
    admit(root,catalog,register)
    return {'schema':1,'success':all(c['matching'] for c in rows),'scope':catalog['scope'],
            'boundary':catalog['boundary'],'comparisons':rows,'native':native,
            'pixels_compared':sum(c['pixels'] for c in rows),'mismatches':sum(c['mismatching_pixels'] for c in rows),
            'original_pixels_used_as_native_inputs':False,'original_oracles_read_by_native':False,
            'new_original_execution':False,'original_work_bypassed_by_replay':False,
            'historical_capture_original_work_bypassed':catalog['observations']['original_work_bypassed'],
            'binary_sha256':binary_hash,'inputs_stable':True,'draw02_complete':False,
            'contiguous_producer_records':True,'contiguous_presented_frames':False,'actions_bound':False,
            'remaining':catalog['remaining'],'artifacts':str(output.relative_to(root))}
