"""Review inherited terminal tail edges without turning them into CALLs."""
from coverage_evidence import read_text as evidence_text
from collections import Counter
import hashlib
import json
import struct
from drawing_census_callers import review as caller_review
from drawing_census_bindings import decode
from drawing_cpu_crosswalk import _rows
from drawing_terminal_crosswalk import contracts, WRAPPERS, PROOF_SHA256

OUTER = 0x4a37d0
LEAF = 0x58b3e0
OUTER_BYTES = bytes.fromhex('a1582c640085c0741150ff15a0505c00c705582c640000000000b900206400e8ec7b0e00b9c81f6400e9e27b0e00')


def verified_contracts(proof, staged, image_offset):
    contracts(proof, staged, image_offset)
    record = json.loads(evidence_text(proof))
    raw = bytes.fromhex(record['original_entry_anchors'][hex(OUTER)])
    offset = image_offset(staged, OUTER, len(raw))
    if raw != OUTER_BYTES or staged[offset:offset+len(raw)] != raw:
        raise ValueError('Outer CloseHandle/CALL/tail original byte composition differs')
    return {**WRAPPERS, OUTER:(0x641fc8,0)}


def match(pairs, parent_ids, contract_map):
    if contract_map != {**WRAPPERS, OUTER:(0x641fc8,0)}:
        raise ValueError('Unreviewed terminal edge contracts')
    result = []
    for child_id, parent_id in sorted(parent_ids.items()):
        if child_id not in pairs or parent_id not in pairs:
            raise ValueError('Inherited edge lacks a complete parent/child pair')
        cs, ce = pairs[child_id]; ps, pe = pairs[parent_id]
        target = ps[6]
        if target not in contract_map or cs[6] != LEAF:
            raise ValueError('Inherited edge is outside terminal contract')
        if not ps[0] < cs[0] < ce[0] < pe[0] or len({ps[5],cs[5],ce[5],pe[5]}) != 1:
            raise ValueError('Inherited edge is outside active parent/thread order')
        if len({ps[4],cs[4],ce[4],pe[4]}) != 1 or len({ps[8],cs[8],ce[8],pe[8]}) != 1:
            raise ValueError('Terminal phase or RET0 stack disagreement')
        receiver, bit = contract_map[target]
        if cs[9] != receiver or cs[15:21] != ps[15:21]:
            raise ValueError('Fixed receiver or same-stack diagnostic words differ')
        if ce[9:14] != pe[9:14]:
            raise ValueError('Tail child return differs from parent return state')
        first_id = None
        if target == OUTER:
            # CloseHandle may change state before this CALL. Compare its actual
            # return, rather than assuming the outer entry state survives it.
            first = [(cid,bs,be) for cid,(bs,be) in pairs.items()
                     if bs[6] == LEAF and bs[7] == 0x4a37f4 and bs[5] == ps[5]
                     and ps[0] < bs[0] < be[0] < cs[0]
                     and bs[8] == be[8] == ps[8]-4]
            if len(first) != 1:
                raise ValueError('Outer tail lacks one exact preceding cleanup CALL')
            first_id, fs, fe = first[0]
            if fs[9] != 0x642000 or fs[4] != ps[4] or fe[10:14] != cs[10:14]:
                raise ValueError('Outer first CALL receiver/return-to-tail state differs')
        else:
            eax = ps[11] if not bit else (ps[11]&0xffffff00)|bit
            if cs[10] != ps[10] or cs[11] != eax or cs[13] != ps[13]:
                raise ValueError('Wrapper register/error effects differ')
            if not bit and cs[12] != ps[12]:
                raise ValueError('MOV/JMP wrapper changed flags')
            if bit and (cs[12]&0x841 or cs[12]&~0x8d5 != ps[12]&~0x8d5):
                raise ValueError('Conditional wrapper flags differ from TEST/OR scope')
        result.append({'parent_call_id':parent_id,'child_call_id':child_id,
            'parent_entry':target,'tail_site':target+25 if bit else 0x4a37f9 if target == OUTER else target+5,
            'tail_target':LEAF,'fixed_receiver':receiver,'one_shot_bit':bit,
            'first_cleanup_call_id':first_id,'parent_physical_caller':ps[7],
            'child_physical_caller':cs[7],'original_return_slot':cs[8]-4,
            'thread':cs[5],'phase':cs[4],
            'classification':'reviewed_original_tail_transfer',
            'original_call_edge_to_tail_child':False})
    return result


def review(path, manifest, bindings_path, callers, inventory, contract_map):
    generic = caller_review(path, manifest, bindings_path, callers, inventory)
    bindings = decode(bindings_path, manifest)
    hooks = {row['entry']:row for row in manifest['hooks']}
    for entry in [*contract_map,LEAF]:
        hook = hooks.get(entry)
        if not hook or hook['pop'] != 0 or hook['site']:
            raise ValueError('Terminal edge needs all actual RET0 entry hooks')
    active = {}; pairs = {}; parents = {}; pending = Counter()
    # Caller review already checks loss, source/build/maps, frame counts, return
    # ordering and original slots. Derive every edge; bounded samples are not
    # an exhaustive source of parent identities.
    events = generic['paired_calls']*2 + generic['site_events']
    for row in _rows(path,32,events):
        if row[1] == 1:
            if row[6] == LEAF and row[7] == bindings['core_leave']:
                candidates = [p for p in active.values() if p[5] == row[5]
                    and p[8] == row[8] and hooks[p[6]]['pop'] == 0]
                parent = max(candidates,key=lambda p:p[0]) if candidates else None
                if parent is not None and parent[6] in contract_map:
                    parents[row[2]] = parent[2]
                else:pending['inherited_leaf_parent_outside_terminal_contract'] += 1
            active[row[2]] = row
        elif row[1] == 2:
            pairs[row[2]] = (active.pop(row[2]),row)
    edges = match(pairs,parents,contract_map)
    with path.open('rb') as source:after = hashlib.file_digest(source,'sha256').hexdigest()
    with bindings_path.open('rb') as source:map_after = hashlib.file_digest(source,'sha256').hexdigest()
    if after != generic['journal_sha256'] or map_after != bindings['sha256']:
        raise ValueError('Terminal edge journals/maps changed during replay')
    return {'schema':1,'integrity':True,'reviewed_terminal_tail_edges':len(edges),'edges':edges,
        'journal_sha256':after,'bindings_sha256':map_after,
        'original_range_proof_sha256':PROOF_SHA256,
        'remaining_inherited_edges':generic['inherited_continuation_pairs']-len(edges),
        'pending':[{'reason':reason,'count':n} for reason,n in sorted(pending.items())],
        'diagnostic_stack_readability_proven':False,'initial_flag_byte_observed':False,
        'caller_attribution_complete':False,'draw01_complete':False,
        'scope':'Source-current native replay of exact original wrapper tail transfers: actual core leave, active same-thread/RET0 slot parent, fixed receiver, bounded TEST/OR register/flag effects, exact outer first CALL return-to-tail state and identical child/parent return state. Diagnostic word consistency is not a readability proof. No CALL edge is invented for a tail; unknown initial flags, handle state, callback registration, other cleanup targets, physical lifetime, full writers/callers/side effects or scenario/dispatch closure remain pending.'}
