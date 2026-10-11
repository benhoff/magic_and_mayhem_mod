"""Exact CPU CALL/core entry pairing across independent closed journals."""
from collections import Counter, defaultdict
import hashlib
import struct

from drawing_api_bindings import decode as api_bindings
from drawing_api_details import trace as api_trace
from drawing_api_cohorts import call_role, require_bindings, cpu_journal_words
from drawing_api_journal import trace as compact_trace
from drawing_census import BUILD_SHA256, trace as core_trace
from drawing_census_bindings import decode as core_bindings
from drawing_census_callers import review as core_callers
from drawing_cpu_sites import SLOTS


def _rows(path, words, events):
    with path.open('rb') as source:
        source.seek(64)
        for _ in range(events):
            yield struct.unpack('<%dI' % words, source.read(words * 4))


def review(core_path, core_manifest, core_binding_path, cpu_path, cpu_manifest,
           cpu_binding_path, cohort_manifest, caller_census, inventory):
    """No target guesses, nearest owner, shared sequence or subset matching."""
    paths = (core_path, core_binding_path, cpu_path, cpu_binding_path)
    before = [path.stat() for path in paths]
    if call_role(cohort_manifest) != 'cpu_backend_dispatch':
        raise ValueError('Crosswalk requires an explicit CPU/core cohort')
    if cpu_manifest.get('call_boundary_role') != 'cpu_backend_dispatch' or cpu_manifest.get('cohort') != 'core_api':
        raise ValueError('CPU build role/cohort mismatch')
    for hook in cpu_manifest['hooks']:
        mode = hook['target_mode']; raw = bytes.fromhex(hook['call']['bytes'])
        absolute = mode.get('kind') == 'absolute_memory' and mode.get('address') in SLOTS
        callback = hook['site'] == 0x59d98a and mode == {'kind':'register', 'register':'eax'}
        if not (absolute and raw == b'\xff\x15' + struct.pack('<I', mode['address']) or callback and raw == b'\xff\xd0'):
            raise ValueError('Crosswalk CALL is outside the reviewed CPU namespace')
        if hook['details'].get('formal_words') != 0xffffffff or hook['details'].get('snapshots') != []:
            raise ValueError('CPU diagnostic words cannot acquire typed API arguments')
    core = core_trace(core_path, core_manifest, caller_census, inventory)
    core_callers(core_path, core_manifest, core_binding_path, caller_census, inventory)
    cpu_words = cpu_journal_words(cpu_manifest)
    cpu = (compact_trace if cpu_words == 32 else api_trace)(cpu_path, cpu_manifest)
    cb = core_bindings(core_binding_path, core_manifest)
    ab = api_bindings(cpu_binding_path, cpu_manifest)
    require_bindings(ab, 'core_api')
    if cb['core_module'] != ab['modules']['core'] or cb['core_leave'] != ab['core_leave']:
        raise ValueError('Independent maps disagree about the actual core module/leave')
    # Prepared maps cannot alias allocations, stubs or original core entries.
    for c in cb['hooks']:
        for a in ab['hooks']:
            if c['trampoline'] < a['trampoline'] + a['trampoline_size'] and a['trampoline'] < c['trampoline'] + c['trampoline_size']:
                raise ValueError('Independent core/CPU allocations overlap')
    exact = {row['physical_call_return']:row for row in ab['hooks']}
    hooks = {row['tag']:row for row in core_manifest['hooks']}
    cpu_groups = defaultdict(list); core_groups = defaultdict(list)
    frames = {}; counts = Counter(); gaps = Counter(); samples = defaultdict(list)
    for row in _rows(cpu_path, cpu_words, cpu['events']):
        cid = row[2]
        if row[1] == 1:
            frames[cid] = row
        else:
            start = frames.pop(cid)
            key = (start[5], start[3], start[7], start[16])
            cpu_groups[key].append((start, row))
    frames = {}
    for row in _rows(core_path, 32, core['events']):
        cid = row[2]
        if row[1] == 1:
            frames[cid] = row
        elif row[1] == 2:
            start = frames.pop(cid); binding = exact.get(start[7])
            if binding is None:
                gaps[('core_caller_outside_cpu_return_map', start[6], start[7])] += 1
                continue
            pop = hooks[start[3]]['pop']
            if type(pop) is not int or not 0 <= pop <= 0xffff or start[8] <= pop + 4:
                raise ValueError('Invalid core cleanup/original return slot')
            key = (start[5], binding['tag'], start[8] - pop, start[6])
            core_groups[key].append((start, row))
    matched_cpu = matched_core = 0; matched_groups = set()
    for key, core_calls in sorted(core_groups.items()):
        cpu_calls = cpu_groups.get(key, [])
        core_calls.sort(key=lambda pair:pair[0][0])
        cpu_calls.sort(key=lambda pair:pair[0][0])
        if len(cpu_calls) != len(core_calls):
            for start, _ in core_calls:
                gaps[('direct_target_stack_group_count_mismatch', start[6], start[7])] += 1
            continue
        matched_groups.add(key)
        for (cs, ce), (ps, pe) in zip(core_calls, cpu_calls):
            pop = hooks[cs[3]]['pop']; binding = exact[cs[7]]
            if not ps[17] or cpu_words == 200 and not ps[34]:
                raise ValueError('Direct crosswalk needs readable diagnostic stack words')
            if cs[4] != ps[4] or ce[8] != pe[7] or pe[7] - ps[7] != pop:
                raise ValueError('Direct CPU/core phase or cleanup disagreement')
            if tuple(ps[8:13]) != tuple(cs[9:14]) or ps[18:24] != cs[15:21]:
                raise ValueError('Direct CPU/core BEFORE register/stack disagreement')
            # These are actual return snapshots, not a typed HRESULT contract.
            if tuple(pe[8:13]) != tuple(ce[9:14]):
                raise ValueError('Direct CPU/core AFTER register disagreement')
            if not ps[0] < pe[0] or not cs[0] < ce[0]:
                raise ValueError('Crosswalk ordering invalid')
            group = (ps[4], binding['site'], binding['original_call_return'], cs[7], cs[6], ps[14], ps[15])
            counts[group] += 1; matched_cpu += 1; matched_core += 1
            if len(samples[group]) < 4:
                samples[group].append({'thread':cs[5], 'cpu_call_id':ps[2], 'core_call_id':cs[2],
                    'cpu_before_sequence':ps[0], 'core_entry_sequence':cs[0],
                    'core_return_sequence':ce[0], 'cpu_after_sequence':pe[0],
                    'cpu_before_sp':ps[7], 'core_normalized_return_sp':cs[8],
                    'original_return_slot':ps[7] - 4, 'cleanup_bytes':pop})
    expected = (core['sha256'], cb['sha256'], cpu['journal_sha256'], ab['sha256'])
    for path, initial, digest in zip(paths, before, expected):
        with path.open('rb') as source:
            actual = hashlib.file_digest(source, 'sha256').hexdigest()
        final = path.stat()
        if actual != digest or (initial.st_ino, initial.st_size, initial.st_mtime_ns) != (final.st_ino, final.st_size, final.st_mtime_ns):
            raise ValueError('Crosswalk journal/bindings changed during review')
    return {'schema':1, 'integrity':True, 'original_sha256':BUILD_SHA256,
        'cpu_journal_version':cpu_manifest['journal_version'],
        'core_journal_sha256':expected[0], 'core_bindings_sha256':expected[1],
        'cpu_journal_sha256':expected[2], 'cpu_bindings_sha256':expected[3],
        'core_paired_calls':core['paired_calls'], 'cpu_paired_calls':cpu['paired_calls'],
        'matched_direct_cpu_pairs':matched_cpu, 'matched_direct_core_pairs':matched_core,
        'pending_cpu_pairs':cpu['paired_calls'] - matched_cpu,
        'core_pairs_outside_direct_cpu_crosswalk':core['paired_calls'] - matched_core,
        'pending_cpu_groups':[{'thread':thread, 'tag':tag,
            'site':cpu_manifest['hooks'][tag]['site'], 'cpu_before_sp':sp,
            'observed_target':target, 'count':len(calls),
            'reason':'no_complete_direct_core_group; wrapper/non-core flow remains unreviewed'}
            for key,calls in sorted(cpu_groups.items()) if key not in matched_groups
            for thread,tag,sp,target in [key]],
        'counts':[{'phase':p, 'original_call_site':site, 'original_call_return':ret,
            'physical_call_return':pc, 'observed_core_entry':entry, 'target_state':state,
            'target_slot':slot, 'count':n, 'proof_samples':samples[key]}
            for key,n in sorted(counts.items()) for p,site,ret,pc,entry,state,slot in [key]],
        'gaps':[{'reason':reason, 'entry':entry, 'physical_caller':pc, 'count':n}
            for (reason,entry,pc),n in sorted(gaps.items())],
        'caller_attribution_complete':False, 'draw01_complete':False,
        'scope':'Closed independent CPU/core journals and matching actual allocation maps. '
            'Direct pairs require exact physical CALL return, thread, target, original return slot, '
            'cleanup, readable six diagnostic words, preserved registers/flags/LastError and full '
            'group cardinality/order. No cross-journal shared sequence or atomic target read is asserted. '
            'Wrapper/tail targets, unmatched groups and non-core callbacks remain pending; '
            'no typed CPU arguments, pixels, lifetime, all writers or scenario/dispatch closure.'}
