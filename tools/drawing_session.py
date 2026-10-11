"""Independent presented-session admission and owned-input backend ABI.

Original capture evidence is required before a session can contribute coverage.
No original pixels, expected API results or oracle paths enter the backend input.
"""
import json
import os
from pathlib import Path
import subprocess
import tempfile
from drawing_corpus import ROOT, BUILD, artifact, local, sha
from drawing_journeys import REQUIREMENTS, OPERATIONS, token

FORMATS = {'rgb565': 2, 'rgba8': 4}
MAX_FRAMES = 8192
MAX_SESSION_BYTES = 2 * 1024 * 1024 * 1024


def plane_compare(actual, expected, width, height, pixel_format):
    channels = FORMATS[pixel_format]
    if len(actual) != width * height * channels or len(actual) != len(expected):
        raise ValueError('Session comparison requires the entire presented plane')
    count = 0; left=width;top=height;right=-1;bottom=-1;examples=[]
    if actual != expected:
        for at in range(0,len(actual),channels):
            a,b=actual[at:at+channels],expected[at:at+channels]
            if a==b:continue
            i=at//channels;x=i%width;y=i//width;count+=1
            left=min(left,x);top=min(top,y);right=max(right,x);bottom=max(bottom,y)
            if len(examples)<8:examples.append({'x':x,'y':y,'native':a.hex(),'original':b.hex()})
    return {'matching':not count,'pixels':width*height,'mismatching_pixels':count,
            'bounds':[left,top,right,bottom] if count else None,'examples':examples,
            'native_sha256':sha(actual),'original_sha256':sha(expected)}


def same_json(left,right):
    # Python considers True==1 and False==0. Wire identities and caller-visible
    # results must retain their JSON types as well as their values.
    return json.dumps(left,sort_keys=True,separators=(',',':'),allow_nan=False)==json.dumps(right,sort_keys=True,separators=(',',':'),allow_nan=False)


def acquisition_contract(catalog):
    """Portable paths/codecs may change; original identity and raw bytes cannot."""
    import copy
    contract={k:copy.deepcopy(catalog[k]) for k in ('id','session_identity','scenario','requirements','operations',
                                                  'presentation','actions','frames','effects','inputs','scope','remaining')}
    for entry in [i['artifact'] for i in contract['inputs']]+[f['oracle'] for f in contract['frames']]:
        identity={k:entry[k] for k in ('raw_sha256','historical_path')}
        entry.clear();entry.update(identity)
    return contract


def admit(root,catalog,register):
    from rendering_coverage import evidence_record
    if type(catalog['schema']) is not int or catalog['schema']!=1 or catalog['source_sha256']!=BUILD:
        raise ValueError('Session schema/build mismatch')
    token(catalog['id']);token(catalog['session_identity'])
    scenario=catalog['scenario']
    if scenario not in REQUIREMENTS:raise ValueError('Unknown session journey')
    gates=catalog['requirements'];ops=catalog['operations']
    if (len(gates)!=len(set(gates)) or any(g not in REQUIREMENTS[scenario] for g in gates) or
        len(ops)!=len(set(ops)) or any(o not in OPERATIONS for o in ops)):
        raise ValueError('Unknown or duplicate session requirement')
    evidence={e['id']:e for e in register['evidence']};builds={b['id']:b for b in register['builds']}
    ev=evidence[catalog['evidence']]
    if (ev['kind'] not in ('isolated_original','live_equivalence','live_replacement') or
        ev['build']!=catalog['build'] or ev['sha256']!=catalog['historical_report_sha256']):
        raise ValueError('Session needs independently registered original execution')
    original=evidence_record(root,ev,builds)
    if (original['original_manifest_verified_before_after'] is not True or
        original['original_pixels_used_as_native_inputs'] is not False):
        raise ValueError('Invalid original manifest/oracle separation provenance')
    # This exact metadata record must have been made by the original collector;
    # changing requirements or calling a canvas presented needs new evidence.
    contract=acquisition_contract(catalog)
    if not same_json(acquisition_contract(original['drawing_session']),contract):
        raise ValueError('Session contract differs from original acquisition')
    presentation=catalog['presentation']
    frames=catalog['frames']
    if (presentation['boundary']!='presented_frame' or type(presentation['first_index']) is not int or
        not 0<=presentation['first_index'] or type(presentation['dropped_frames']) is not int or presentation['dropped_frames']!=0 or
        type(presentation['sampling_interval']) is not int or presentation['sampling_interval']!=1 or
        type(presentation['last_index']) is not int or not 1<=len(frames)<=MAX_FRAMES or
        presentation['last_index']!=presentation['first_index']+len(frames)-1):
        raise ValueError('Session presented history is partial or discontinuous')
    if not isinstance(catalog['scope'],str) or not catalog['scope'].strip():raise ValueError('Session scope is empty')
    actions=catalog['actions']
    if any(set(a)!={'index','action','before_presentation'} for a in actions):
        raise ValueError('Session action fields are not admitted')
    if not 1<=len(actions)<=65536 or any(type(a['index']) is not int for a in actions) or [a['index'] for a in actions]!=list(range(len(actions))):
        raise ValueError('Session action identities are absent or unordered')
    if any(not isinstance(a['action'],str) or not a['action'].strip() or len(a['action'])>128 or type(a['before_presentation']) is not int or
           not presentation['first_index']<=a['before_presentation']<=presentation['last_index'] for a in actions):
        raise ValueError('Action boundary lies outside presented history')
    if [a['before_presentation'] for a in actions]!=sorted(a['before_presentation'] for a in actions):
        raise ValueError('Session action boundaries are reordered')
    total=0
    # Every artifact must be in the immutable original collector input map.
    # Owned-input roles and output roles are disjoint, regardless of filenames.
    inputs,names,input_hashes=[],set(),set()
    if not 1<=len(catalog['inputs'])<=4096:raise ValueError('Session input binding budget exceeded')
    for item in catalog['inputs']:
        name=token(item['id'])
        if name in names:raise ValueError('Duplicate session input binding')
        names.add(name);data=artifact(root,item['artifact']);total+=len(data)
        if original['inputs'].get(item['artifact']['historical_path'])!=sha(data):
            raise ValueError('Session owned-input provenance mismatch')
        if total>MAX_SESSION_BYTES:raise ValueError('Session artifact budget exceeded')
        input_hashes.add(sha(data));inputs.append((name,data,item['role']))
        if item['role'] not in ('owned-state','encoded-asset','operation-input'):
            raise ValueError('Unsupported owned session input role')
    if not inputs:raise ValueError('Session lacks owned drawing inputs')
    for i,frame in enumerate(frames):
        identity=frame['identity']
        if set(identity)!={'session','presentation_index','width','height','pixel_format','actions_completed'}:
            raise ValueError('Session frame identity fields are not admitted')
        if (identity['session']!=catalog['session_identity'] or
            type(identity['presentation_index']) is not int or identity['presentation_index']!=presentation['first_index']+i or
            identity['pixel_format'] not in FORMATS or type(identity['width']) is not int or
            type(identity['height']) is not int or not 0<identity['width']<=2048 or not 0<identity['height']<=2048):
            raise ValueError('Session frame/extent/format identity mismatch')
        action_count=sum(a['before_presentation']<=identity['presentation_index'] for a in actions)
        if type(identity['actions_completed']) is not int or identity['actions_completed']!=action_count or not frame['input_ids'] or any(n not in names for n in frame['input_ids']):
            raise ValueError('Session frame/action/input binding mismatch')
        data=artifact(root,frame['oracle']);total+=len(data)
        if (len(data)!=identity['width']*identity['height']*FORMATS[identity['pixel_format']] or
            original['inputs'].get(frame['oracle']['historical_path'])!=sha(data) or sha(data) in input_hashes):
            raise ValueError('Session oracle provenance/extent/input-role mismatch')
        if total>MAX_SESSION_BYTES:raise ValueError('Session artifact budget exceeded')
    effects=catalog['effects']
    if len(effects)>65536 or len({e['id'] for e in effects})!=len(effects):
        raise ValueError('Duplicate/oversized session effects')
    for effect in effects:
        token(effect['id'])
        if (effect['operation'] not in ops or not isinstance(effect['result'],dict) or not effect['result'] or
            type(effect['presentation_index']) is not int or
            not presentation['first_index']<=effect['presentation_index']<=presentation['last_index']):
            raise ValueError('Session effect/result identity mismatch')
    if any(o not in {e['operation'] for e in effects} for o in ops):
        raise ValueError('Operation claim lacks independent caller-visible result')
    return inputs,frames,effects


def check(*,root=ROOT,catalog_path,executable,headless=False):
    root=Path(root).resolve();path=local(root,catalog_path);catalog_bytes=path.read_bytes();catalog=json.loads(catalog_bytes)
    register_path=local(root,'research/runtime/coverage/register.json');register_bytes=register_path.read_bytes()
    register=json.loads(register_bytes);inputs,frames,effects=admit(root,catalog,register)
    executable=executable.resolve();binary_hash=sha(executable.read_bytes())
    parent=root/'working/tests/drawing-sessions';parent.mkdir(parents=True,exist_ok=True)
    output=Path(tempfile.mkdtemp(prefix='run-',dir=parent));owned=output/'inputs';owned.mkdir()
    bindings=[]
    for name,data,role in inputs:
        (owned/(name+'.bin')).write_bytes(data);bindings.append({'id':name,'path':name+'.bin','role':role,'sha256':sha(data)})
    declaration={'schema':1,'session_identity':catalog['session_identity'],'inputs':bindings,
                 'actions':catalog['actions'],'frames':[{'identity':f['identity'],'input_ids':f['input_ids']} for f in frames],
                 'effects':[{'id':e['id'],'operation':e['operation'],'presentation_index':e['presentation_index']} for e in effects]}
    # No scope/remaining labels, original results, output paths/hashes or oracle
    # payloads are passed to the native process.
    (owned/'session.json').write_text(json.dumps(declaration,indent=2)+'\n')
    command=[str(executable),str(owned),str(output/'native')];env=dict(os.environ)
    if headless:
        command=['xvfb-run','-a',*command];env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1')
    execution=subprocess.run(command,capture_output=True,timeout=600,env=env)
    (output/'native.log').write_bytes(execution.stdout+execution.stderr)
    if execution.returncode==64:
        if path.read_bytes()!=catalog_bytes or register_path.read_bytes()!=register_bytes or sha(executable.read_bytes())!=binary_hash:
            raise ValueError('Session identities changed during unsupported replay')
        admit(root,catalog,register)
        return {'success':False,'outcome':'unsupported','scope':catalog['scope'],'pixels_compared':0,'mismatches':0,
                'artifacts':str(output.relative_to(root)),'original_reference_admitted':True}
    if execution.returncode:raise ValueError('Session backend failed: '+str(output/'native.log'))
    native=json.loads((output/'native/session-report.json').read_text())
    if (native['success'] is not True or native['session_identity']!=catalog['session_identity'] or
        type(native['remaining_surfaces']) is not int or native['remaining_surfaces']!=0 or native['original_pixels_used_as_native_inputs'] is not False or
        native['original_oracles_read'] is not False or len(native['frames'])!=len(frames)):
        raise ValueError('Session backend completion/ownership mismatch')
    comparisons=[]
    for i,(frame,n) in enumerate(zip(frames,native['frames'])):
        if not same_json(n['identity'],frame['identity']):raise ValueError('Session backend dropped/reordered frame')
        # Fixed output names prevent backend-supplied path traversal or reading
        # the expected plane via an attacker-selected output path.
        actual=(output/f'native/frame-{i:06}.bin').read_bytes()
        if sha(actual)!=n['sha256']:raise ValueError('Session native output hash mismatch')
        ident=frame['identity'];expected=artifact(root,frame['oracle'])
        comparisons.append({'identity':ident,**plane_compare(actual,expected,ident['width'],ident['height'],ident['pixel_format'])})
    observed=native['effects']
    if [e['id'] for e in observed]!=[e['id'] for e in effects]:
        raise ValueError('Session backend omitted/reordered caller-visible effects')
    effects_match=same_json(observed,effects)
    if path.read_bytes()!=catalog_bytes or register_path.read_bytes()!=register_bytes or sha(executable.read_bytes())!=binary_hash:
        raise ValueError('Session catalog/register/binary changed during replay')
    admit(root,catalog,register)
    success=all(c['matching'] for c in comparisons) and effects_match
    return {'success':success,'outcome':'matching' if success else 'mismatch','scope':catalog['scope'],
            'comparisons':comparisons,'effects_matching':effects_match,
            'expected_effects':effects,'native_effects':observed,
            'pixels_compared':sum(c['pixels'] for c in comparisons),
            'mismatches':sum(c['mismatching_pixels'] for c in comparisons),
            'original_reference_admitted':True,'inputs_stable':True,'binary_sha256':binary_hash,
            'original_pixels_used_as_native_inputs':False,'original_oracles_read_by_native':False,
            'artifacts':str(output.relative_to(root)),'remaining':catalog['remaining']}
