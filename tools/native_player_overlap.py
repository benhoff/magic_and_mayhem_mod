"""Owned-provider alias bounds for exact retained execution phases.

This proves a finite requiredness boundary, never all-game unreachability.
"""
import hashlib
import json
import struct
import tempfile
from pathlib import Path

from native_cursor_journal import decode as cursor_decode
from native_draw_retirement import read_retirement
from retained_artifacts import read_artifact

CONTRACT = 'research/runtime/rendering-natural-overlap-results-20261109/provider-contract-02.json'
CONTRACT_SHA = '6ba7185aa391ef01774fba151e234a4be7d6ad1bef7278e941759991df4b50ac'
REVIEWED = {
    'runtime/native-draw/gpu_copy_plan.h': {
        '8ee6422cbc641cbf985688732c0e36cbc028419a03d1c4005f23da41bd88e4b9',
        'ee62e69a4b8bb8c471b57c7389eaa51fc2613303d326f6ec3bcc2670b768f941'},
    'runtime/native-draw/surfaces.h': {
        '2f3df566a6611fc24304969323c392835668cf3d0b2fbe2c92d9ce04da6cb681',
        '9bef300d2c78180c9c8b8b3ed605e46ffe94598d1676851448c3e47d37f4e3e9'},
    'runtime/native-draw/factory.c': {
        'bf48223a80d2f0081bb47e579cbff437253df1cc52447c14ec97d31a680117c0',
        '86892ea09118f1a7b4a68d74c7e5a9ed404f5b57c295e84ff6a2e98a8da6a007'},
    'runtime/native-draw/cursor_owner.h': {
        '6d8d3e608fb82fd554cb00dd573352961950c10dd29b255250e7ab0bedfbe237',
        '73ca2884db98f1ff76d519ee7d00cc1ce2bdd575916e08040c5680f7deca3890'},
    'runtime/native-draw/lifetime_dispatch.h': {'578d7cef0fb2a79d711f557e20bfc4a334d82482565535413569516f6a7eeee8'},
    'runtime/native-draw/lifetime_lock.h': {'63bdd6d9f86535e35523a4e40792d50fe55242d383a58dbcc2509d4a28c6a8c4'},
    'runtime/native-draw/native_only.h': {'0d9ac77684823dc29a6aade82d4e264f045b466a71a6324e4cec6d0a7b8a71f8'},
    'runtime/shadow/win32_min.h': {'8c2a9b52fd57459c82a4af8f2e684546fbac529a5ad831396d07cfcb2e52fcf9'},
}
LAYOUT = dict(pointer_bits=32, interface_bytes=12, interface_count=5,
              state_bytes=284, alias_span=48, minimum_distinct_interface_distance=236)


def require(value, reason):
    if not value:
        raise ValueError(reason)


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def contract(reader):
    require(reader.digest(CONTRACT) == CONTRACT_SHA, 'Provider contract identity changed')
    value = reader.json(CONTRACT)
    require(value['success'] and len(value['phases']) == 16, 'Incomplete recorded provider phases')
    expected = {(p, h) for p, hashes in REVIEWED.items() for h in hashes}
    require(len(value['source_files']) == len(expected)
            and {(s['path'], s['sha256']) for s in value['source_files']} == expected,
            'Unreviewed provider source version')
    for source in value['source_files']:
        require(reader.digest(source['record']) == source['sha256'], 'Changed provider source bytes')
    require(len(value['layouts']) == 2
            and {s['surface_source_sha256'] for s in value['layouts']} == REVIEWED['runtime/native-draw/surfaces.h'],
            'Missing PE32 source-layout compilation')
    for layout in value['layouts']:
        require(layout['surface_source_sha256'] in REVIEWED['runtime/native-draw/surfaces.h']
                and layout['exit_code'] == 0
                and (layout['interface_stride'], layout['interface_count'], layout['alias_span']) == (12, 5, 48),
                'Unreviewed interface layout')
        for key in ('input', 'output'):
            require(reader.digest(layout[key]['record']) == layout[key]['sha256'], 'Changed compiler input/output')
        require('[sizeof=284, align=4]' in reader.read_text(layout['output']['record']), 'State extent differs')
    return value


def relation(target, source, layout=LAYOUT):
    require(layout == LAYOUT, 'Only the reviewed PE32 provider layout is admitted')
    limit = 2**32 - layout['minimum_distinct_interface_distance']
    require(all(type(v) is int and 0 < v <= limit and v % 4 == 0 for v in (target, source)),
            'Invalid owned PE32 interface address')
    delta = abs(target-source)
    if delta <= layout['alias_span'] and delta % layout['interface_bytes'] == 0:
        return 'same_owned_state'
    require(delta >= layout['minimum_distinct_interface_distance'],
            'Pointer pair contradicts the reviewed owned-state layout')
    return 'distinct_owned_states'


def summarize(rows, layout=LAYOUT):
    counts = dict(calls=0, source_less=0, source_copies=0, successful_source_copies=0,
                  failed_source_copies=0, distinct_owned_state_copies=0, shared_state_calls=0,
                  overlapping_calls=0, shared_state_unknown_geometry=0)
    failures, candidates = [], []
    for row in rows:
        require(len(row) == 44, 'Version2 COPY row required')
        counts['calls'] += 1
        if not row[5]:
            counts['source_less'] += 1
            continue
        counts['source_copies'] += 1
        if row[13]:
            require(row[13] & 0x80000000, 'Unreviewed nonzero success result')
            counts['failed_source_copies'] += 1
            failures.append(list(row))
            continue
        counts['successful_source_copies'] += 1
        if relation(row[4], row[5], layout) == 'distinct_owned_states':
            counts['distinct_owned_state_copies'] += 1
            continue
        counts['shared_state_calls'] += 1
        candidate = dict(row=list(row), geometry_known=row[15] in (1, 2))
        if candidate['geometry_known']:
            signed = lambda v: v if v < 2**31 else v-2**32
            source = list(map(signed, row[24:28])); target = list(map(signed, row[28:32]))
            require(source[2] > source[0] and source[3] > source[1]
                    and target[2] > target[0] and target[3] > target[1], 'Invalid prepared geometry')
            candidate['overlapping'] = max(source[0], target[0]) < min(source[2], target[2]) and max(source[1], target[1]) < min(source[3], target[3])
            counts['overlapping_calls'] += candidate['overlapping']
        else:
            counts['shared_state_unknown_geometry'] += 1
        candidates.append(candidate)
    return dict(counts=counts, failures=failures, candidates=candidates,
                bounded_exclusion=counts['successful_source_copies'] > 0 and not candidates,
                natural_overlap_witness=counts['overlapping_calls'] > 0,
                global_unreachability_proved=False, independent_alias_output_proved=False)


def verify_phase(reader, acquired, phase):
    """The source-layout rule is not applicable to arbitrary COM providers."""
    get = lambda name: read_artifact(reader, acquired, name, max_bytes=2*1024**2)
    manifest = json.loads(get('native-draw-build.json'))
    admission = json.loads(get('native-output-admission.json'))
    outcome = json.loads(get('native-only-session.json'))
    binary = get('game/MnmDraw.dll')
    require(len(binary) >= 64 and binary[:2] == b'MZ', 'Missing native PE32 DLL')
    pe = struct.unpack_from('<I', binary, 60)[0]
    require(pe <= len(binary)-26 and binary[pe:pe+4] == b'PE\0\0'
            and struct.unpack_from('<H', binary, pe+4)[0] == 0x14c
            and struct.unpack_from('<H', binary, pe+24)[0] == 0x10b, 'Non-PE32 native provider')
    require(manifest['architecture'] == 'PE32 i386' and manifest['directdraw_imports'] is False
            and sha(binary) == manifest['sha256'] == phase['native_draw_sha256']
            == admission['native_only']['binaries']['MnmDraw.dll'], 'Executed provider DLL differs')
    require({n: manifest['sources'][n] for n in REVIEWED} == phase['provider_sources']
            and all(h in REVIEWED[n] for n, h in phase['provider_sources'].items()), 'Provider source phase differs')
    require(outcome['policy'] == admission['native_only']['policy'] == 'native-only-rendering-v1'
            and outcome['state'] == 'completed' and outcome['normal_quit'] is True
            and outcome['renderer_retired'] is True and outcome['fallback'] is False,
            'Incomplete native-only session')
    cursor = cursor_decode(get('native-cursor-lifetime.bin'))
    require(cursor['closed'] and cursor['installed_hooks'] == 18, 'Mode2 lifetime guard not witnessed')
    raw = get('native-draw-retirement.bin')
    with tempfile.TemporaryDirectory(prefix='alias-retirement-') as directory:
        path = Path(directory)/'retirement.bin'; path.write_bytes(raw)
        retired = read_retirement(path)
    require(retired['resources_released'] and not retired['ddraw_module_loaded'], 'Foreign or unretired graphics backing')
    return dict(native_draw_sha256=sha(binary), provider_sources=phase['provider_sources'],
                lifetime_mode2_installed=True, native_only_completed=True,
                retirement_sha256=sha(raw), private_backing_retired=True,
                source_phase_preserved=True, current_whole_native_execution=False)
