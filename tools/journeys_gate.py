"""Conjoin admitted journey leaves with their exact output and normal lifetime.

Historical semantic admissions and independently compared output keep their
original source phases. This tool executes only the retained-data join.
"""
import ast
import copy
import json
from pathlib import Path

from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader
from pixels_history_gate import Join, module
from rendering_acceptance import requirements, validate
from rendering_menu_acceptance import artifact
from rendering_pilot import require
from retained_artifacts import scoped_artifacts

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/journeys-gate-results-20261110/'
BID = 'NP.journeys-gate-closeout'
SID = 'native.journeys-gate-closeout'
GATE = 'gate.journeys'
PROFILES = ('player-default', 'player-software')
PH = 'research/runtime/pixels-history-gate-results-20261109/admission-01.json'
PH_SHA = '100abb6f59fa972cbeea661d33796151375d4440776016118f50113c3f0a4c10'
SCOPE = ('Conjunction of the exact 47 admitted journey leaves in each default/software profile, '
         'covering all fourteen named journeys. Bind each unchanged semantic admission and every '
         'primary attachment to its accepted complete output/history proof and freshly checked raw '
         'normal process/input/cursor/graphics/channel retirement in the same session. Native menu '
         'semantics and independent World/movie pixels retain distinct roles and original domain '
         'limits. Historical original/native/compiler/oracle phases and failures remain unchanged. '
         'No new game, native execution, raster comparison, combined-current player, physical '
         'scanout, transition-gate, required-boundary or final-decision promotion.')


def local_python_sources():
    """Conservative local imports, including the named dynamically loaded readers."""
    pending = [Path(__file__), Path(module('close-rendering-retained-rows').__file__)]
    seen = set()
    while pending:
        path = pending.pop().resolve()
        if not path.is_relative_to(ROOT) or not path.is_file(): continue
        name = path.relative_to(ROOT).as_posix()
        if name in seen: continue
        seen.add(name)
        for node in ast.walk(ast.parse(path.read_bytes())):
            names = ([v.name for v in node.names] if isinstance(node, ast.Import) else
                     [node.module] if isinstance(node, ast.ImportFrom) and node.module else [])
            for value in names:
                for base in (ROOT/'tools', ROOT/'protocols/python'):
                    candidate = base/Path(*value.split('.')).with_suffix('.py')
                    if candidate.is_file(): pending.append(candidate)
            if isinstance(node, ast.Call) and node.args and isinstance(node.args[0], ast.Constant):
                fn = node.func
                if ((isinstance(fn, ast.Name) and fn.id == 'module') or
                    (isinstance(fn, ast.Attribute) and fn.attr == 'module')) and isinstance(node.args[0].value, str):
                    pending.append(ROOT/'tools'/(node.args[0].value+'.py'))
                    if len(node.args) > 1 and isinstance(node.args[1],ast.Constant) and isinstance(node.args[1].value,str):
                        pending.append(ROOT/node.args[1].value)
    return seen


def basis(plan, parent, bindings):
    validate(plan)
    definitions = {(v['configuration'], v['id']): v for v in requirements(plan)}
    rows = parent['requirements']; keys = [(v['configuration'], v['id']) for v in rows]
    leaves = {v['id'] for v in definitions.values() if v['kind'] == 'journey'}
    journeys = {v['journey'] for v in definitions.values() if v['kind'] == 'journey'}
    require(parent['success'] is True and len(rows) == len(set(keys)) == len(definitions) == 156
            and set(keys) == set(definitions), 'Changed or duplicate acceptance inventory')
    require(len(leaves) == 47 and len(journeys) == 14, 'Changed journey denominator')
    require(parent['passed_requirements'] == sum(v['state'] == 'passed' for v in rows)
            and parent['pending_requirements'] == sum(v['state'] == 'pending' for v in rows)
            and parent['passed_requirements']+parent['pending_requirements'] == 156, 'False parent counts')
    require(parent['decisions'] == {'supported-player':'pending','original-recovery':'pending'}
            and parent['scope_amendments'] == plan['acceptance_scope_amendments'], 'Changed decisions/scope')
    require(bindings['success'] is True and len(bindings['rows']) == 94, 'Incomplete frozen semantic bindings')
    bound = {(v['row']['configuration'],v['row']['id']):v for v in bindings['rows']}
    require(len(bound) == 94 and set(bound) == {(p,n) for p in PROFILES for n in leaves}, 'Mixed semantic binding domain')
    for row in rows:
        key = row['configuration'], row['id']
        if row['kind'] == 'journey' or row['id'] == GATE:
            require(all(row[k] == v for k,v in definitions[key].items()), 'Changed journey condition/owner')
        if row['kind'] == 'journey':
            require(row['state'] == 'passed' and row == bound[key]['row'] and row['admitted_execution_evidence'],
                    'Journey leaf lacks its exact accepted semantic admission')
        elif row['id'] == GATE:
            require(row['state'] == 'pending' and row['admitted_execution_evidence'] == [], 'Journey gate already changed')
    return bound


def accepted_pixels(reader):
    """Consume an unchanged accepted parent; no old source hash is refreshed."""
    require(reader.digest(PH) == PH_SHA, 'Unreviewed pixel/history admission')
    q = reader.json(PH); reg = reader.json('research/runtime/coverage/register.json')
    b = next(v for v in reg['behaviors'] if v['id'] == 'NP.pixels-history-gate-closeout')
    s = next(v for v in reg['scenarios'] if v['id'] == 'native.pixels-history-gate-closeout')
    e = next(v for v in reg['evidence'] if v['id'] == 'PH09.pixels-history01')
    require(e['record'] == PH and e['sha256'] == PH_SHA and e['id'] in b['evidence'], 'Pixel evidence authority differs')
    require(q['success'] is True and q['sources_stable'] is True and required_sources(b) <= q['sources'].keys()
            and all(reader.digest(n) == h for n,h in q['sources'].items()), 'Accepted pixel analysis requires targeted revalidation')
    require(q['claims'] == [dict(behavior=b['id'],contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),
                                scenarios={s['id']:scenario_contract(s)})], 'Pixel contract/scenario changed')
    return q, Join(reader).checked(q['proof'])


def expand(join, row):
    values = []
    for item in row['admitted_execution_evidence']:
        if 'matrix' in item:
            matrix = join.checked(item['matrix'])
            require(matrix['mismatching_pixels'] == 0, 'Failed matrix')
            selected = []
            for ref in matrix['comparisons']:
                c = join.checked(ref); a = join.checked(c['acquisition'])
                flag = a['configuration']['software_flag']
                require(type(flag) is bool, 'Untyped matrix profile')
                if flag is (row['configuration'] == PROFILES[1]) and a['configuration']['actual_size'] in item['actual_world_extents']:
                    selected.append(dict(acquisition=c['acquisition'],comparison=ref))
            require(len(selected) == 1, 'Missing or duplicate matrix domain')
            values.extend(selected)
        else:
            value = item.get('execution',item)
            values.append({k:value[k] for k in ('acquisition','comparison')} if 'acquisition' in value else value)
    identities = [json.dumps(v,sort_keys=True) for v in values]
    require(values and len(identities) == len(set(identities)), 'Missing or duplicate journey primary')
    return values


def completed_output(proof, primary, profile):
    if 'acquisition' in primary:
        cases = [v for v in proof['cases']+proof['additional_presented_cases'] if v['pair'] == primary]
        require(len(cases) == 1 and cases[0]['configuration'] == profile, 'Journey lacks same-profile complete viewport/history proof')
        case = cases[0]
        require(case['completed_output']['swaps'] > 0 and case['completed_output']['pixels'] > 0
                and case['command_history']['imports'] == case['source_generations'] > 0
                and case['command_history']['command_end'] > 0, 'Incomplete accepted output/history')
        return dict(role='complete-independent-World-viewport-and-contiguous-history',case=case)
    cases = [v for v in proof['semantic_primary_cases'] if v['execution'] == primary]
    require(len(cases) == 1 and cases[0]['configuration'] == profile
            and cases[0]['whole_viewport_pixel_credit'] is False, 'Unknown semantic primary or invented widget pixels')
    movie = [v for v in proof['movie_cases'] if v['execution'] == primary]
    require(len(movie) <= 1, 'Duplicate movie output')
    if movie:
        p = movie[0]['output']
        require(p['verified'] is True and p['mismatching_pixels'] == 0 and p['movie_swaps'] > 0
                and p['compared_pixels'] > 0, 'Missing independent movie output')
    return dict(role='independent-enabled-movie-plus-semantic-menu' if movie else 'semantic-native-widget-policy',
                case=cases[0],movie=movie)


def lifetime(reader, ref, profile, output):
    old = module('close-rendering-retained-rows')
    a = Join(reader).checked(ref); a, excluded = scoped_artifacts(a,label='journeys')
    require(type(a['configuration']['software_flag']) is bool and
            a['configuration']['software_flag'] is (profile == PROFILES[1]), 'Lifetime profile differs')
    names = ('native-only-session.json','native-output-admission.json','original-process-result.json',
             'render-input.bin','render-commands.bin','render-commands.bin.control',
             'native-cursor-lifetime.bin','native-draw-retirement.bin')
    raw = {n:artifact(reader,a,n) for n in names}
    events_raw = artifact(reader,a,'events.bin')
    callbacks = [v for v in old.menu_events.decode(events_raw) if v['event'] == 3]
    threads = {v['thread'] for v in callbacks}
    require(len(threads) == 1 and next(iter(threads)) > 0, 'Ambiguous raw original callback thread')
    states = a.get('states',a.get('flow',{}).get('states',[]))
    state_threads = {v['thread'] for v in states if 'thread' in v and 'event' not in v}
    require(not state_threads or state_threads == threads, 'State and raw callback owner threads differ')
    # Some collectors keep flow steps rather than state samples. The shared
    # terminal parser's owner-thread input comes from real callback bytes here.
    normalized = dict(a,states=[dict(thread=next(iter(threads)))])
    calls = [[v['menu_id'],v['argument']] for v in callbacks]
    p = old.terminal_state(normalized,*(json.loads(raw[n]) for n in names[:3]),*(raw[n] for n in names[3:]),calls)
    identity = a.get('session_identity',a['session'])
    if 'acquisition' in output['case'].get('pair',{}):
        c = output['case']
        require(c['identity'] == identity and p['source_imports'] == c['source_generations']
                and old.integer(a['resident']['nonce']) == c['command_history']['nonce'], 'Output/lifetime session or source nonce differs')
        raw['world-continuous.bin'] = artifact(reader,a,'world-continuous.bin')
        raw['world-terminal-0001.bin'] = artifact(reader,a,'world-terminal-0001.bin')
        p['source_terminal'] = old.world_exit.terminal_source(raw['world-continuous.bin'],raw['world-terminal-0001.bin'],
            dict(nonce=c['command_history']['nonce'],imports=range(p['source_imports'])),p['original_thread'])
    elif p['source_imports']:
        require(a['kind'] == 'public_results_return_acquisition', 'Unreviewed semantic primary with World work')
        raw['world-continuous.bin'] = artifact(reader,a,'world-continuous.bin')
        p['source_terminal'] = old.source_header_boundary(raw['world-continuous.bin'],
            p['source_imports'],old.integer(a['resident']['nonce']))
        # A semantic Results/Main receipt supplies no independently compared
        # World pixels. Its real preceding World must still retire correctly.
    return dict(acquisition=ref,configuration=profile,identity=identity,proof=p,
                raw_terminal_sha256={n:old.sha(v) for n,v in dict(raw,**{'events.bin':events_raw}).items()},
                owner_thread_source='raw-original-menu-callbacks-cross-checked-against-available-states',
                excluded_other_session_artifacts=excluded)


def analyze(reader, parent):
    bindings = reader.json(PREFIX+'bindings-01.json')
    bound = basis(reader.json('docs/native-rendering-closure.json'),parent,bindings)
    admission, pixels = accepted_pixels(reader); join = Join(reader)
    pixel_rows = {(v['configuration'],v['id']):v for v in admission['requirements'] if v['kind'] == 'journey'}
    links = []; retired = {}; identities = {}
    for row in parent['requirements']:
        if row['kind'] != 'journey': continue
        key = row['configuration'],row['id']; binding = bound[key]
        origin = join.checked(binding['inventory'])
        require(row in origin['requirements'] and origin['dossier_identity'] == parent['dossier_identity']
                and row == pixel_rows[key], 'Semantic/output admission belongs to another leaf or source phase')
        executions = []
        for primary in expand(join,row):
            output = completed_output(pixels,primary,row['configuration'])
            ref = primary.get('acquisition',primary); name = ref['record']
            if name not in retired:
                print('Rechecking journey terminal owners:',name,flush=True)
                retired[name] = lifetime(reader,ref,row['configuration'],output)
                identity = retired[name]['identity']
                require(identity not in identities,'Duplicate original session identity')
                identities[identity] = ref
            require(retired[name]['acquisition'] == ref and retired[name]['configuration'] == row['configuration'], 'Mixed retirement domain')
            executions.append(dict(primary=primary,output=output,lifetime=ref))
        links.append(dict(configuration=row['configuration'],requirement=row['id'],journey=row['journey'],
                          semantic_admission=binding['inventory'],executions=executions,
                          retained_scope=copy.deepcopy(row['admitted_execution_evidence'])))
    expected = {(p,v['row']['id']) for v in bindings['rows'] for p in PROFILES}
    require(len(links) == 94 and {(v['configuration'],v['requirement']) for v in links} == expected, 'Incomplete journey conjunction')
    return dict(schema=1,kind='retained_journeys_gate_proof',success=True,journeys=14,journey_cells=94,
        linked_requirements=links,normal_lifetimes=list(retired.values()),distinct_primary_sessions=len(retired),
        accepted_pixels=dict(record=PH,sha256=PH_SHA),accepted_pixel_proof=admission['proof'],
        semantic_admissions=list(join.refs.values()),historical_semantic_source_phases=join.phases,
        historical_output_source_phases=pixels['retained_execution_source_phases'],
        original_game_runs=0,new_native_executions=0,new_pixel_comparisons=0,current_whole_player_proved=False,scope=SCOPE)


def close_rows(parent, proof_ref, proof):
    require(proof['success'] is True and proof['journey_cells'] == 94 and proof['journeys'] == 14
            and len(proof['linked_requirements']) == 94, 'Incomplete journey proof')
    rows = copy.deepcopy(parent['requirements'])
    for row in rows:
        if row['id'] != GATE: continue
        selected = [v for v in proof['linked_requirements'] if v['configuration'] == row['configuration']]
        expected = {v['id'] for v in rows if v['kind'] == 'journey' and v['configuration'] == row['configuration']}
        require(row['state'] == 'pending' and len(selected) == 47 and {v['requirement'] for v in selected} == expected
                and len({v['journey'] for v in selected}) == 14, 'Incomplete profile journey gate')
        row.update(state='passed',admitted_execution_evidence=[dict(gate_proof=proof_ref,
            journey_requirements=sorted(expected),joined_journey_cells=47,journeys=14,reason=SCOPE)])
    require([v for v in rows if v['id'] != GATE] == [v for v in parent['requirements'] if v['id'] != GATE]
            and sum(v['state'] == 'passed' for v in rows) == parent['passed_requirements']+2, 'Unrelated acceptance changed')
    return rows
