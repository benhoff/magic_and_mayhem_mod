"""DRAW-01 route accounting. Recorded activity is not rendering equivalence."""
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import struct

from rendering_coverage import ROOT, evidence_record, read

CATALOG = 'research/runtime/drawing-inventory.json'
REGISTER = 'research/runtime/coverage/register.json'
CLASSES = {'producer', 'consumer', 'surface_access', 'composition', 'presentation',
           'lifecycle', 'render_state', 'unresolved'}
DECISIONS = {'original_only', 'scoped_native', 'unresolved'}
CONFIDENCE = {'unknown', 'provisional', 'high_within_scope'}


def route_digest(catalog):
    return hashlib.sha256(json.dumps(catalog['routes'], sort_keys=True, separators=(',', ':')).encode()).hexdigest()


def address(value):
    if isinstance(value, bool):
        raise ValueError('Boolean is not an address')
    result = int(value, 0) if isinstance(value, str) else value
    if not isinstance(result, int) or not 0 <= result <= 0xffffffff:
        raise ValueError('Invalid PE32 address')
    return result


def count(value):
    if type(value) is not int or value < 0:
        raise ValueError('Activity counts must be nonnegative integers')
    return value


def unique(rows, key):
    result = {}
    for row in rows:
        name = row[key]
        if not isinstance(name, str) or not name or name in result:
            raise ValueError('Missing or duplicate ' + key)
        result[name] = row
    return result


def caller_returns(static):
    """Match the exact static CALL and its byte length; never use nearest PCs."""
    result = {}
    for name, calls in static['callers'].items():
        entry = static['ranges'][name][0]
        for call in calls:
            va = address(call['call_va'])
            for line in call['context']:
                match = re.match(r'\s*([0-9a-f]+):\s+((?:[0-9a-f]{2}\s+)+)call\s+0x([0-9a-f]+)\b', line)
                if match and int(match[1], 16) == va and int(match[3], 16) == entry:
                    pc = va + len(match[2].split())
                    value = {'entry': entry, 'call_va': va, 'discovered_owners': call['discovered_owners']}
                    if pc in result and result[pc] != value:
                        raise ValueError('Ambiguous static return PC')
                    result[pc] = value
                    break
            else:
                raise ValueError('Static caller lacks exact CALL bytes: ' + hex(va))
    return result


def hook_entries(text):
    total = re.search(r'^#define HOOKS (\d+)$', text, re.M)
    rows = re.findall(r'\{(0x[0-9a-fA-F]+),\d+,\d+,\{[^}]+\},producer_enter_(\d+),-?\d+\}', text)
    if not total or len(rows) != int(total[1]) or {int(tag) for _, tag in rows} != set(range(len(rows))):
        raise ValueError('Producer hook declaration changed; review the parser and census')
    if len({address(entry) for entry, _ in rows}) != len(rows):
        raise ValueError('Duplicate producer hook entry')
    return [{'entry': address(entry), 'tag': int(tag)} for entry, tag in rows]


def project(record, definition):
    from coverage_audit import pointer
    data = pointer(record, definition['pointer'])
    kind = definition['kind']
    if kind == 'scalar':
        return [(definition['identity'], count(data))]
    if kind == 'counts':
        if not isinstance(data, dict):
            raise ValueError('Expected activity count map')
        return [(key, count(value)) for key, value in data.items()]
    if kind == 'kind_rows':
        if not isinstance(data, list):
            raise ValueError('Expected queue rows')
        counts = Counter()
        for ordinal, row in enumerate(data, 1):
            if row['queue'] != ordinal or row['status'] != 0 or row['capacity'] < count(row['draw_count']) or row['rows_captured'] != row['draw_count'] or sum(count(v) for v in row['kind_counts'].values()) != row['draw_count']:
                raise ValueError('Incomplete original draw-kind rows')
            counts.update({key: count(value) for key, value in row['kind_counts'].items()})
        return sorted(counts.items())
    raise ValueError('Unreviewed activity projection: ' + kind)


def selector_key(kind, identity):
    if kind == 'entry':
        return kind, address(identity)
    if kind == 'draw_kind':
        value = int(identity, 0) if isinstance(identity, str) else identity
        if type(value) is not int or not -0x80000000 <= value <= 0xffffffff:
            raise ValueError('Invalid signed original draw kind')
        return kind, value - 0x100000000 if value >= 0x80000000 else value
    if kind == 'operation' and isinstance(identity, str) and identity:
        return kind, identity
    raise ValueError('Invalid route selector')


def reconcile(root, catalog, register):
    if catalog['schema'] != 1:
        raise ValueError('Unsupported drawing inventory schema')
    builds = unique(register['builds'], 'id')
    build = builds[catalog['build']]
    if catalog['source_sha256'] != build['source_sha256']:
        raise ValueError('Inventory build mismatch')
    behaviors = unique(register['behaviors'], 'id')
    evidence = unique(register['evidence'], 'id')
    scenarios = unique(catalog['scenarios'], 'id')
    routes = unique(catalog['routes'], 'id')
    sources = unique(catalog['observations'], 'id')
    records = {}
    for source in sources.values():
        if not source['scope'].strip() or source['scenario'] not in scenarios:
            raise ValueError('Observation lacks scenario or bounded scope')
        ev = evidence[source['evidence']]
        if ev['kind'] not in {'live_observation', 'live_equivalence', 'live_replacement'}:
            raise ValueError('Offline fixture cannot establish observed live drawing')
        # Native movie observation still pins an original executable in its report.
        record = evidence_record(root, ev, builds)
        from coverage_audit import pointer
        if pointer(record, source['build_pointer']) != build['source_sha256']:
            raise ValueError('Observation original build mismatch')
        records[source['id']] = record
    static_source = catalog['static_source']
    static_ev = evidence[static_source['evidence']]
    if static_ev['kind'] != 'static':
        raise ValueError('Static inventory requires static evidence')
    static = evidence_record(root, static_ev, builds)
    if static['source_sha256'] != build['source_sha256']:
        raise ValueError('Static inventory build mismatch')
    returns = caller_returns(static)
    windows = {}
    selectors = {}
    binary = None
    for route in routes.values():
        if route['classification'] not in CLASSES or route['support']['decision'] not in DECISIONS:
            raise ValueError('Route needs explicit classification and support decision')
        if route['confidence'] not in CONFIDENCE:
            raise ValueError('Invalid route confidence')
        if not all(isinstance(route[k], str) and route[k].strip() for k in ('owner', 'scope', 'confidence', 'remaining')):
            raise ValueError('Route ownership, scope, confidence and gaps are required')
        if not route['behaviors'] or not set(route['behaviors']) <= behaviors.keys():
            raise ValueError('Route has unregistered behavior')
        if not route['side_effects'] or not route['support']['reason'].strip():
            raise ValueError('Route lacks side effects or support rationale')
        if not route['documents']:
            raise ValueError('Route lacks research')
        for path in route['documents']:
            target = (root / path).resolve()
            if Path(path).is_absolute() or '..' in Path(path).parts or not target.is_relative_to(root.resolve()) or not target.is_file() or path.split('/')[0] in {'original', 'working', '.git'}:
                raise ValueError('Unsafe or missing route research')
        if 'entry' in route:
            entry = address(route['entry'])
            anchors = {ref['address'] for bid in route['behaviors'] for ref in behaviors[bid]['original']
                       if ref['build'] == catalog['build']}
            if entry not in anchors and route['classification'] != 'unresolved':
                anchor = route.get('original_anchor')
                if anchor is None or address(anchor) not in anchors:
                    raise ValueError('Route entry lacks scoped original behavior anchor: ' + route['id'])
                if binary is None:
                    from coverage_audit import sha
                    binary = read(root, build['inventory'])
                    if sha(root / build['inventory']) != build['inventory_sha256'] or binary['source_sha256'] != build['source_sha256']:
                        raise ValueError('Original function inventory changed')
                owners = [f for f in binary['functions'] if f['entry'] == address(anchor)]
                if not any(start <= entry < end for f in owners for start, end in f['ranges']):
                    raise ValueError('Interior hook outside explicitly linked original function')
        for window in route.get('static_windows', []):
            if window not in static['ranges'] or window in windows:
                raise ValueError('Unknown or duplicate static window')
            windows[window] = route['id']
        for selector in route.get('selectors', []):
            key = selector_key(selector['kind'], selector['identity'])
            if selector['kind'] == 'entry' and 'entry' in route and key[1] != address(route['entry']):
                raise ValueError('Entry selector disagrees with original route')
            if key in selectors:
                raise ValueError('Ambiguous route selector')
            selectors[key] = route['id']
    if windows.keys() != static['ranges'].keys():
        raise ValueError('Static window census changed; classify every window')
    activities = []
    unaccounted = []
    for source in sources.values():
        for projection in source['projections']:
            if projection['activity'] not in {'live_original_active', 'live_takeover', 'live_observation'}:
                raise ValueError('Unreviewed activity classification')
            for identity, hits in project(records[source['id']], projection):
                key = selector_key(projection['selector'], identity)
                route = selectors.get(key)
                item = {'source': source['id'], 'scenario': source['scenario'], 'selector': projection['selector'],
                        'identity': identity, 'count': hits, 'route': route, 'activity': projection['activity']}
                activities.append(item)
                if hits and route is None:
                    unaccounted.append(item)
    hook_path = catalog['hook_source']
    if Path(hook_path).is_absolute() or '..' in Path(hook_path).parts or hook_path.startswith(('original/', 'working/', '.git/')) or not (root / hook_path).resolve().is_relative_to(root.resolve()):
        raise ValueError('Unsafe hook source')
    hooks = hook_entries((root / hook_path).read_text())
    for hook in hooks:
        key = selector_key('entry', hook['entry'])
        if key not in selectors:
            unaccounted.append({'selector': 'entry', 'identity': hex(hook['entry']), 'activity': 'instrumented_only', 'tag': hook['tag']})
    additional_hooks = []
    for source in catalog.get('additional_hook_sources', []):
        path = Path(source['path'])
        if path.is_absolute() or '..' in path.parts or path.parts[0] in {'original', 'working', '.git'} or not (root / path).resolve().is_relative_to(root.resolve()):
            raise ValueError('Unsafe additional hook source')
        if source['kind'] != 'interior_jump':
            raise ValueError('Unreviewed additional hook parser')
        entries = re.findall(r'jump\(\(u8\*\)(0x[0-9a-fA-F]+),', (root / path).read_text())
        if not entries:
            raise ValueError('Interior hook declaration changed; review the census')
        for entry in entries:
            additional_hooks.append({'entry': address(entry), 'source': str(path)})
            if selector_key('entry', entry) not in selectors:
                unaccounted.append({'selector': 'entry', 'identity': entry, 'activity': 'instrumented_only', 'source': str(path)})
    # Do not infer all indirect flows from an entry's understood outer contract.
    indirect = [{'window': name, 'route': windows[name], **item}
                for name, items in static['indirect_calls'].items() for item in items]
    jumps = [{'window': name, 'route': windows[name], **item}
             for name, items in static['computed_jumps'].items() for item in items]
    tables = [{'id': d['id'], 'scope': d['scope'], 'complete': d['complete'], 'entries': d['entries']}
              for d in register['dispatch_tables'] if d['id'] in catalog['dispatch_tables']]
    if {d['id'] for d in tables} != set(catalog['dispatch_tables']):
        raise ValueError('Missing reviewed dispatch table')
    scenario_rows = []
    for scenario in scenarios.values():
        if not scenario['scope'].strip() or not scenario['remaining'].strip():
            raise ValueError('Scenario must retain its scope and gaps')
        rows = [a for a in activities if a['scenario'] == scenario['id']]
        closed = False
        review = scenario.get('census_review')
        if review:
            from coverage_audit import pointer
            source = sources[review['observation']]
            if source['scenario'] != scenario['id'] or evidence[source['evidence']]['kind'] == 'static':
                raise ValueError('Scenario closure needs matching live observation')
            closure = pointer(records[source['id']], review['pointer'])
            fields = ('complete', 'caller_attribution_complete', 'side_effect_inventory_complete', 'owner_classification_complete')
            if not all(closure.get(k) is True for k in fields) or closure.get('lost_events') != 0 or closure.get('unknown_routes') != 0:
                raise ValueError('Incomplete route census cannot close a scenario')
            count(closure['lost_events'])
            count(closure['unknown_routes'])
            if closure.get('route_census_sha256') != route_digest(catalog) or not review['reason'].strip():
                raise ValueError('Scenario closure needs an exact reviewed route census')
            closed = True
        scenario_rows.append({**scenario, 'observations': sorted(s['id'] for s in sources.values() if s['scenario'] == scenario['id']),
                              'observed_routes': sorted({a['route'] for a in rows if a['count'] and a['route']}),
                              'closed_route_census': closed})
    # Retained aggregate counters cannot certify complete instrumentation or caller coverage.
    blockers = ([{'kind': 'unaccounted_route', 'detail': item} for item in unaccounted]
                + [{'kind': 'scenario_census_pending', 'scenario': s['id'], 'detail': s['remaining']} for s in scenario_rows if s['required'] and not s['closed_route_census']]
                + [{'kind': 'incomplete_dispatch', 'table': d['id']} for d in tables if not d['complete']]
                + [{'kind': 'support_decision_pending', 'route': r['id']} for r in routes.values() if r['support']['decision'] == 'unresolved'])
    return {'schema': 1, 'build': catalog['build'], 'source_sha256': build['source_sha256'],
            'scope': catalog['scope'], 'inventory_valid': True, 'routes': list(routes.values()),
            'image_base': static['image_base'], 'producer_operations': catalog['producer_operations'],
            'activities': activities, 'scenarios': scenario_rows, 'unaccounted': unaccounted,
            'static_callers': static['callers'], 'static_caller_returns': {hex(k): v for k, v in returns.items()},
            'static_indirect_calls': indirect, 'static_computed_jumps': jumps, 'dispatch_tables': tables,
            'hook_entries': hooks, 'additional_hook_entries': additional_hooks, 'completion_blockers': blockers, 'draw01_complete': not blockers,
            'counts': {'routes': len(routes), 'static_windows': len(windows), 'instrumented_entries': len(hooks),
                       'additional_interior_hooks': len(additional_hooks),
                       'required_scenarios': sum(s['required'] for s in scenario_rows), 'unaccounted': len(unaccounted)},
            'limitations': ['Activity counters belong to individual bounded cohorts; do not sum them into runtime frequencies.',
                            'Aggregate operations do not identify every wrapper entry or caller.',
                            'Static callers are preferred-build addresses; runtime pointers are not persistent identities.',
                            'This accounting report neither reruns original drawing nor renews historical source fingerprints.']}


def build_report(root=ROOT):
    return reconcile(root, read(root, CATALOG), read(root, REGISTER))


def producer_trace(stream, done, expected_sha256, scenario, runtime_base, report):
    """Read a closed producer journal without pixels/assets or original execution."""
    paths = [Path(stream).resolve(), Path(done).resolve()]
    if any(p.is_relative_to((ROOT / name).resolve()) for p in paths for name in ('original', '.git')):
        raise ValueError('Producer trace cannot read original/ or .git/')
    version_caps = {1: (128 * 1024 * 1024, 65536, 16), 2: (512 * 1024 * 1024, 262144, 16),
                    3: (128 * 1024 * 1024, 131072, 32)}
    if scenario not in {s['id'] for s in report['scenarios']}:
        raise ValueError('Unknown trace scenario')
    if paths[0].stat().st_size > 512 * 1024 * 1024 or paths[1].stat().st_size != 32:
        raise ValueError('Trace input size exceeds journal/marker bounds')
    def digest(path):
        with path.open('rb') as input_file:
            return hashlib.file_digest(input_file, 'sha256').hexdigest()
    before = digest(paths[0])
    if before != expected_sha256:
        raise ValueError('Producer journal hash mismatch')
    marker = paths[1].read_bytes()
    if len(marker) != 32 or marker[:8] != b'MNMPDONE':
        raise ValueError('Missing or malformed completion marker')
    finished = struct.unpack('<8I', marker)
    tally = Counter()
    checkpoints = queues = failures = sequence = 0
    queue_open = False
    operations = report['producer_operations']
    selectors = {selector_key(s['kind'], s['identity']): r['id'] for r in report['routes'] for s in r.get('selectors', [])}
    returns = report['static_caller_returns']
    preferred = address(report['image_base'])
    runtime_base = address(runtime_base)
    with paths[0].open('rb') as source:
        header = source.read(64)
        if len(header) != 64:
            raise ValueError('Short journal header')
        words = struct.unpack('<16I', header)
        version = words[2]
        if version not in version_caps or header[:8] != ('MNMPRO0' + str(version)).encode() or words[3] != 64:
            raise ValueError('Unreviewed journal version')
        byte_cap, record_cap, queue_cap = version_caps[version]
        size = paths[0].stat().st_size
        if size > byte_cap or words[4] != int(report['source_sha256'][:8], 16) or any(words[7:]):
            raise ValueError('Journal build, size or reserved fields invalid')
        if words[5] not in {32768, 65536, 131072, 262144} or words[5] > record_cap or not 1 <= words[6] <= queue_cap:
            raise ValueError('Journal record/queue limits invalid')
        while source.tell() < size:
            at = source.tell()
            raw = source.read(96)
            if len(raw) != 96:
                raise ValueError('Truncated producer record')
            row = struct.unpack('<24I', raw)
            sequence += 1
            if row[1] != sequence or sequence > words[5] or row[0] != 96 + row[18] or at + row[0] > size or row[23]:
                raise ValueError('Noncontiguous or malformed producer record')
            operation = operations.get(str(row[2]), 'unknown-op-' + str(row[2]))
            if row[2] == 10:
                checkpoints += 1
            elif row[2] == 11:
                if queue_open or row[14] != queues + 1:
                    raise ValueError('Invalid World entry/return pairing')
                queue_open = True
            elif row[2] == 12:
                if not queue_open:
                    raise ValueError('World return has no matching entry')
                queues += 1
                if row[14] != queues:
                    raise ValueError('Noncontiguous World returns')
                queue_open = False
            elif row[2] == 13:
                failures += 1
            pc = row[22] - runtime_base + preferred if row[22] >= runtime_base else None
            caller = returns.get(hex(pc)) if pc is not None else None
            entry_route = selectors.get(('entry', caller['entry'])) if caller else None
            operation_route = selectors.get(('operation', operation))
            tally[(operation, row[22], pc, entry_route, operation_route)] += 1
            source.seek(row[18], 1)
    if queue_open or finished[2] != 1 or finished[3:] != (queues, sequence, failures, checkpoints, size) or queues != words[6] or failures:
        raise ValueError('Incomplete or failed journal completion')
    if digest(paths[0]) != before or paths[1].read_bytes() != marker:
        raise ValueError('Trace inputs changed during inventory')
    rows = [{'operation': op, 'caller_va': hex(raw), 'normalized_return_pc': hex(pc) if pc is not None else None,
             'entry_route': entry, 'operation_route': operation, 'count': hits}
            for (op, raw, pc, entry, operation), hits in sorted(tally.items(), key=lambda x: str(x[0]))]
    return {'scenario': scenario, 'sha256': before, 'done_sha256': hashlib.sha256(marker).hexdigest(),
            'version': version, 'records': sequence, 'queues': queues, 'closed_journal': True,
            'runtime_image_base': hex(runtime_base), 'image_base_provenance': 'Explicit caller-supplied runtime module base; verify against the observed process before semantic attribution.',
            'rows': rows, 'unattributed_callers': [r for r in rows if r['caller_va'] != '0x0' and r['entry_route'] is None],
            'unaccounted_operations': [r for r in rows if r['operation_route'] is None],
            'scope': 'Operation/caller census of an existing closed instrumented journal only. No exhaustive writer coverage, new original execution or pixel-equivalence claim.'}


def markdown(report):
    lines = ['# DRAW-01 drawing-route inventory', '',
             ('DRAW-01 census gates pass for the declared scope.' if report['draw01_complete'] else 'DRAW-01 is incomplete.') + ' This accounting report does not establish rendering equivalence.', '',
             f"Routes: {report['counts']['routes']}; static windows: {report['counts']['static_windows']}; producer hook entries: {report['counts']['instrumented_entries']}; additional interior hooks: {report['counts']['additional_interior_hooks']}; unaccounted identities: {len(report['unaccounted'])}.", '',
             '| Required scenario | Retained observations | Remaining census |', '| --- | --- | --- |']
    for s in report['scenarios']:
        if s['required']:
            lines.append(f"| {s['id']} | {', '.join(s['observations']) or 'None'} | {s['remaining']} |")
    lines += ['', '## Unaccounted routes', '']
    lines += [str(item) for item in report['unaccounted']] or ['None in the reviewed inputs. Unknown paths outside those inputs remain possible.']
    lines += ['', '## Unresolved dispatch', '']
    lines += [f"- {d['id']}: {d['scope']}" for d in report['dispatch_tables'] if not d['complete']]
    lines += ['', '## Limitations', ''] + ['- ' + s for s in report['limitations']]
    return '\n'.join(lines) + '\n'
