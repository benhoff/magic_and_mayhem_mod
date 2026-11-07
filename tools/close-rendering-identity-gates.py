#!/usr/bin/env python3
"""Bind the two identity gates to retained bytes for the accepted primary cohort."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path, PurePosixPath
import struct

from coverage_claims import fingerprint
from coverage_evidence import Reader, write_report
from evidence_objects import configured
from rendering_acceptance import requirements, validate
from rendering_menu_acceptance import artifact
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/rendering-identity-results-20261107/'
PARENT = 'research/runtime/rendering-retained-results-20261107/closeout-analysis-04.json'
BID = 'NP.accepted-identity-gate-closeout'
LEAF = 'gate.identity'
POLICY = 'native-only-rendering-v1'
spec = importlib.util.spec_from_file_location('identity_retained', ROOT/'tools/close-rendering-retained-rows.py')
retained = importlib.util.module_from_spec(spec)
spec.loader.exec_module(retained)
sha = lambda raw: hashlib.sha256(raw).hexdigest()


def hashes(values):
    require(isinstance(values, dict) and values and all(isinstance(n, str) and n
            and isinstance(h, str) and len(h) == 64 and all(c in '0123456789abcdef' for c in h)
            for n, h in values.items()), 'Missing/malformed source or binary closure')


def installation(raw, inputs, original, preference=None):
    lines = raw.decode().splitlines()
    require(lines and lines[0] == 'sha256\tsize\tpath', 'Unknown installation manifest')
    observed = {}
    for line in lines[1:]:
        h, size, name = line.split('\t')
        path = PurePosixPath(name)
        require(name not in observed and name and not path.is_absolute() and '..' not in path.parts
                and '\\' not in name and size.isdecimal(), 'Unsafe/duplicate installation entry')
        hashes({name: h})
        observed[name] = dict(sha256=h, bytes=int(size))
    if preference is not None:
        require(preference['path'] == 'CFG/prefs.cfg' and preference['byte_delta_verified'] is True
                and preference['before'] == observed['CFG/prefs.cfg']
                and preference['after'] == inputs['CFG/prefs.cfg']
                and preference['high_resolution'] is False, 'Unreviewed effective preference override')
        observed['CFG/prefs.cfg'] = preference['after']
    require(observed == inputs and inputs.get('Chaos.exe', {}).get('sha256') == original,
            'Original executable or complete installation identity differs')
    require(all(type(v['bytes']) is int and v['bytes'] > 0 for v in inputs.values()), 'Invalid input extent')
    return dict(files=len(inputs), manifest_sha256=sha(raw), inventory_sha256=fingerprint(inputs),
                original_executable_sha256=original, effective_preference_override=preference)


def pe32(raw, expected):
    require(sha(raw) == expected and len(raw) >= 64 and raw[:2] == b'MZ', 'Changed/non-PE original or staged binary')
    offset = struct.unpack_from('<I', raw, 60)[0]
    require(offset + 26 <= len(raw) and raw[offset:offset+4] == b'PE\0\0'
            and struct.unpack_from('<H', raw, offset+4)[0] == 0x14c
            and struct.unpack_from('<H', raw, offset+24)[0] == 0x10b, 'Wrong PE32 i386 image')


def elf_compiler(raw, expected):
    require(sha(raw) == expected and raw[:6] == b'\x7fELF\x02\x01' and len(raw) >= 64, 'Changed/non-ELF64 player')
    h = struct.unpack_from('<16sHHIQQQIHHHHHH', raw)
    require(h[2] == 62 and h[11] == 64 and h[12] > 0 and h[13] < h[12]
            and h[6] + h[11]*h[12] <= len(raw), 'Invalid player section table')
    sections = [struct.unpack_from('<IIQQQQIIQQ', raw, h[6]+i*h[11]) for i in range(h[12])]
    names = sections[h[13]]
    strings = raw[names[4]:names[4]+names[5]]
    comments = []
    for s in sections:
        name = strings[s[0]:].split(b'\0', 1)[0]
        if name == b'.comment':
            require(s[4]+s[5] <= len(raw), 'Truncated compiler identity')
            comments += [v.decode('utf-8') for v in raw[s[4]:s[4]+s[5]].split(b'\0') if v]
    require(comments and all(v.strip() for v in comments), 'Player compiler identity missing')
    return sorted(set(comments))


def configuration(a, admission, stage, shell, process, expected_profile):
    cfg = a['configuration']
    require(a['success'] and a['sources_stable'] and a['original_manifest_verified_before_after']
            and a['exit_code'] == 0 and a['private_wine_stopped']
            and not a.get('forced_private_prefix_stop', False), 'Unaccepted primary execution')
    require(type(cfg['software_flag']) is bool and cfg['software_flag'] is (expected_profile == 'player-software')
            and ('--software-rendering' in a['command']) is cfg['software_flag']
            and type(cfg['skip_intros']) is bool and cfg['terrain_lighting'] == 'native'
            and not cfg.get('diagnostic_only', False) and not cfg.get('compiled_test_driver_invoked', False),
            'Launch/profile policy differs')
    require(admission['session'] == a['session'] == shell['session']
            and admission['prefix'] == process['prefix'] and stage['game_copy'] == a['session']+'/game'
            and process['original_exit_code'] == 0 and process['cleanup_after_original_exit'], 'Mixed process/session/prefix')
    require(admission['native_only']['policy'] == stage['native_only_policy'] == shell['policy'] == POLICY
            and admission['required_backing_mode'] == 3 and admission['legacy_embedding'] is False
            and admission['host_graphics_devices'] is False
            and all(admission[k] is True for k in ('namespace_network','namespace_pid','namespace_ipc'))
            and admission['complete_replacement'] is False
            and admission['native_only']['complete_replacement'] is False
            and stage['native_pixel_no_forward'] is True and stage['native_draw_startup_retirement'] is True
            and shell['normal_quit'] and shell['renderer_retired'] and shell['fallback'] is False,
            'Native-only isolation/policy boundary differs')
    renderer = a['resident']['renderer']
    require(isinstance(renderer, str) and renderer.startswith('llvmpipe (LLVM ') and renderer.endswith(' bits)'),
            'Observed provider is missing/outside the retained Mesa software domain')
    return dict(configuration=cfg, configuration_sha256=fingerprint(cfg), command_sha256=fingerprint(a['command']),
                observed_renderer=renderer, provider_scope='Observed llvmpipe renderer string in the admitted isolated Xvfb/Mesa run; no physical hardware, arbitrary driver, GL ABI or full-provider equivalence.',
                policy=POLICY, backing_mode=3, host_graphics_devices=False, legacy_embedding=False)


def member(reader, a, suffix):
    matches = [v for v in a['artifacts'] if v['path'].endswith(suffix)]
    require(len(matches) == 1, 'Missing/ambiguous exact identity artifact: '+suffix)
    ref = matches[0]
    raw = reader.read_bytes(ref['record'])
    require(sha(raw) == ref['sha256'] and len(raw) == ref['bytes'], 'Changed retained identity artifact')
    return raw, dict(record=ref['record'], sha256=ref['sha256'])


def identify(reader, pair, item, original, verified_objects):
    a = retained.checked(reader, item['reference'])
    manifest_ref = a['launch_manifest']
    raw = reader.read_bytes(manifest_ref['record'])
    require(sha(raw) == manifest_ref['sha256'], 'Changed launch manifest')
    inputs = a['installation_inputs']
    baseline = [line.split('\t') for line in raw.decode().splitlines()[1:]
                if line.endswith('\tCFG/prefs.cfg')]
    require(len(baseline) == 1, 'Missing/duplicate baseline preferences')
    h, size, name = baseline[0]
    before = dict(sha256=h, bytes=int(size))
    effective = inputs[name]
    preference = None
    if before != effective:
        high = pair.read(before['sha256'], before['bytes'])
        low = pair.read(effective['sha256'], effective['bytes'])
        marker = b'\r\nIsHighRes=TRUE\r\n'
        require(high.count(marker) == 1 and low == high.replace(marker, b'\r\nIsHighRes=FALSE\r\n')
                and a['configuration'].get('high_resolution', False) is False,
                'Preference override changes more than the exact High-to-Low input')
        preference = dict(path=name, before=before, after=effective, byte_delta_verified=True, high_resolution=False)
    installed = installation(raw, inputs, original, preference)
    for v in inputs.values():
        key = v['sha256'], v['bytes']
        if key not in verified_objects:
            body = pair.read(*key)
            require(len(body) == key[1] and sha(body) == key[0], 'Changed retained installation input')
            verified_objects.add(key)
    pe32(pair.read(original, inputs['Chaos.exe']['bytes']), original)
    admission_raw, admission_ref = member(reader, a, '/native-output-admission.json')
    admission = json.loads(admission_raw)
    stage_raw, stage_ref = member(reader, a, '/'+Path(a['session']).name+'/manifest.json')
    stage = json.loads(stage_raw)
    shell = json.loads(artifact(reader, a, 'native-only-session.json'))
    process = json.loads(artifact(reader, a, 'original-process-result.json'))
    policy = configuration(a, admission, stage, shell, process, item['profile'])
    require(stage['source_sha256'] == original and stage['native_only_policy'] == POLICY, 'Wrong staged original input')
    staged_hash = stage['staged_sha256']
    size = pair.stores[0].db.execute('SELECT size FROM objects WHERE sha256=?', (staged_hash,)).fetchone()
    require(size is not None, 'Retained staged executable bytes missing')
    pe32(pair.read(staged_hash, size[0]), staged_hash)
    build = a.get('player_build', {})
    compiler = a.get('compiler_sources', build.get('compiler_sources'))
    hashes(compiler)
    binary = a.get('player_binary', build.get('binary'))
    require(reader.digest(binary['record']) == binary['sha256'], 'Wrong player reference')
    banners = elf_compiler(reader.read_bytes(binary['record']), binary['sha256'])
    hashes(admission['native_only']['sources'])
    hashes(admission['native_only']['binaries'])
    for phase in (compiler, admission['native_only']['sources']):
        require(all(a['sources'][n] == h for n,h in phase.items() if n in a['sources']),
                'Recorded source phases disagree')
    dlls = []
    fields = {'MnmDraw.dll':'native_draw_sha256', 'MnmRender.dll':'render_dll_sha256',
              'MnmScene.dll':'resident_scene_sha256', 'MnmMenu.dll':'dll_sha256',
              'MnmPanelWriters.dll':'panel_dll_sha256', 'MnmPixelWriters.dll':'pixel_dll_sha256',
              'MnmJpegPixels.dll':'jpeg_dll_sha256', 'MnmMinimapWriters.dll':'minimap_dll_sha256',
              'MnmColourRect.dll':'colour_rect_dll_sha256', 'MnmRasterWriters.dll':'raster_dll_sha256'}
    require(set(admission['native_only']['binaries']) == set(fields), 'Native DLL closure changed')
    for name, h in admission['native_only']['binaries'].items():
        body, ref = member(reader, a, '/game/'+name)
        require(stage[fields[name]] == h == ref['sha256'], 'Staged DLL differs from admitted binary')
        pe32(body, h)
        dlls.append(dict(name=name, reference=ref, bytes=len(body)))
    result = dict(item, kind=a['kind'], session=a['session'], installation=installed,
                  installation_payloads_verified=True, original_payload_verified=True,
                  launch_manifest=manifest_ref, staged_manifest=stage_ref, admission=admission_ref,
                  staged_executable=dict(sha256=staged_hash, bytes=size[0], payload_verified=True),
                  player=dict(reference=binary, compiler_source_sha256=fingerprint(compiler),
                              compiler_sources=len(compiler), compiler_banners=banners, payload_verified=True),
                  native_source_sha256=fingerprint(admission['native_only']['sources']),
                  native_sources=len(admission['native_only']['sources']), dlls=dlls, policy=policy,
                  source_phase='Exact recorded historical compiler/native/player execution; current source hashes are not substituted.')
    check_case(result)
    return result


def check_case(case):
    require(case['installation_payloads_verified'] and case['original_payload_verified']
            and case['staged_executable']['payload_verified'] and case['player']['payload_verified']
            and case['installation']['files'] > 0 and case['player']['compiler_sources'] > 0
            and case['player']['compiler_banners'] and case['native_sources'] > 0
            and len(case['dlls']) == 10 and len({v['name'] for v in case['dlls']}) == 10,
            'Incomplete retained payload/compiler identity')
    p = case['policy']
    require(p['policy'] == POLICY and p['backing_mode'] == 3 and not p['host_graphics_devices']
            and not p['legacy_embedding'] and p['observed_renderer'].startswith('llvmpipe (LLVM ')
            and p['configuration_sha256'] == fingerprint(p['configuration'])
            and p['configuration']['software_flag'] is (case['profile'] == 'player-software'),
            'Unbound provider/configuration identity')


def close_rows(plan, parent, cohort, cases):
    validate(plan)
    require(parent['success'] and parent['dossier_identity'] == fingerprint(requirements(plan))
            and len(parent['requirements']) == 156, 'Changed parent acceptance inventory')
    require(len(cases) == len(cohort) and len({v['session'] for v in cases}) == len(cases)
            and {v['reference']['record'] for v in cases} == {v['reference']['record'] for v in cohort},
            'Primary identity cohort missing/duplicated')
    binary_sources = {}
    for c in cases:
        item = next(v for v in cohort if v['reference'] == c['reference'])
        require(all(c[k] == item[k] for k in ('reference','profile','rows','comparisons')), 'Cohort identity/profile/attachments changed')
        check_case(c)
        h = c['player']['reference']['sha256']
        previous = binary_sources.setdefault(h, c['player']['compiler_source_sha256'])
        require(previous == c['player']['compiler_source_sha256'], 'One player binary has conflicting compiler closures')
    rows = []
    for old in parent['requirements']:
        row = dict(old)
        if row['id'] == LEAF:
            require(row['state'] == 'pending', 'Identity gate already closed')
            proof = [dict(execution=c['reference'], reason='Retained installation, original/staged executable, actual native DLL/player payloads, source/compiler closures, observed renderer, policy/isolation and exact launch configuration bind this accepted primary execution. Historical native source phase; future primary cases require this identity admission.') for c in cases if c['profile'] == row['configuration']]
            require(proof, 'Missing profile identity proof')
            row.update(state='passed', admitted_execution_evidence=proof)
        rows.append(row)
    require(sum(v['state']=='passed' for v in rows) == parent['passed_requirements']+2, 'Closeout exceeds two identity rows')
    return rows


def sources(reader):
    b = next(b for b in reader.json('research/runtime/coverage/register.json')['behaviors'] if b['id'] == BID)
    return {n:reader.digest(n) for role in ('implementation','tests','validation_dependencies') for n in b[role]}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report', required=True)
    args = p.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts and not (ROOT/args.report).exists(), 'Fresh owned result required')
    result = dict(schema=1, success=False)
    try:
        with Reader(ROOT) as r:
            before = sources(r)
            parent = r.json(PARENT)
            cohort = retained.primary_cohort(r, parent)
            original = next(v['source_sha256'] for v in r.json('research/runtime/coverage/register.json')['builds'] if v['id']=='nocd-40209ca7')
            cases, verified = [], set()
            with configured(ROOT) as pair:
                for item in cohort:
                    cases.append(identify(r, pair, item, original, verified))
                    print('Verified retained identity', item['profile'], item['reference']['record'], flush=True)
            rows = close_rows(r.json('docs/native-rendering-closure.json'), parent, cohort, cases)
            require(before == sources(r), 'Analysis source changed during execution')
            result = dict(schema=1,kind='retained_accepted_cohort_identity_gates',success=True,
                scope='Close exactly both identity rows for this frozen accepted primary cohort through fresh retained-byte provenance analysis. Preserve all parent rows, both decisions and historical native/compiler/viewport source phases. Future primary executions require this identity admission. No new gameplay, provider equivalence, current native execution, driver ABI or final decision claim.',
                sources=dict(parent['sources'], **{n:h for n,h in before.items() if n not in parent['sources']}),
                analysis_sources=before,analysis_sources_stable=True,parent=dict(record=PARENT,sha256=r.digest(PARENT)),
                cohort=cohort,cases=cases,unique_verified_installation_objects=len(verified),
                unique_verified_installation_bytes=sum(v[1] for v in verified),
                player_binary_versions=len({c['player']['reference']['sha256'] for c in cases}),
                actual_primary_executions=len(cases),requirements=rows,dossier_identity=parent['dossier_identity'],
                decisions=parent['decisions'],closed_leaf_ids=sorted(set(parent['closed_leaf_ids'])|{LEAF}),
                passed_requirements=parent['passed_requirements']+2,pending_requirements=154-parent['passed_requirements'],
                newly_passed_requirements=2,original_game_runs=0,new_fixture=False,new_native_code=False,
                current_native_engine_execution=False,historical_failures_retired=False)
    except Exception as error:
        result.update(success=False,error=str(error))
    h = write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'],error=result.get('error'),record=args.report,sha256=h)),flush=True)
    return int(not result['success'])


if __name__ == '__main__':
    raise SystemExit(main())
