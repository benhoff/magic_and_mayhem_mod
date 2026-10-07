"""Offline traceability checks. Unknowns are findings; invalid claims are errors."""
import hashlib
import json
from pathlib import Path
import re
from bisect import bisect_right

STAGES = {
    'understanding': {'unknown', 'partial', 'scoped'},
    'implementation': {'none', 'partial', 'scoped'},
    'comparison': {'none', 'recorded'},
    'integration': {'none', 'headless', 'preview', 'live_observation', 'live_equivalence'},
    'replacement': {'none', 'scoped_live'},
}
KINDS = {'static', 'isolated_original', 'native_integration', 'live_observation', 'live_equivalence', 'live_replacement'}


def range_complement(sections, functions):
    gaps = []
    for section in sections:
        if not section['executable']:
            continue
        cursor = section['start']
        spans = sorted((max(a, section['start']), min(b, section['end']))
                       for f in functions for a, b in f['ranges']
                       if a < section['end'] and b > section['start'])
        for start, end in spans:
            if start > cursor:
                gaps.append([cursor, start])
            cursor = max(cursor, end)
        if cursor < section['end']:
            gaps.append([cursor, section['end']])
    return gaps


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def pointer(value, path):
    if path == '':
        return value
    if not path.startswith('/'):
        raise ValueError('JSON pointer must start with /')
    for key in path[1:].split('/'):
        key = key.replace('~1', '/').replace('~0', '~')
        value = value[int(key)] if isinstance(value, list) else value[key]
    return value


def source_hashes(value):
    if isinstance(value, dict):
        return value
    if isinstance(value, list):
        return {item['path']: item['sha256'] for item in value}
    raise ValueError('Expected source hash map/list')


def audit(root, register):
    root = Path(root).resolve()
    errors, warnings = [], []
    findings = {'unclassified_functions': [], 'unassigned_executable_ranges': [],
                'unresolved_indirect_flows': [], 'unmapped_imports': [],
                'computed_flows_with_inferred_targets': [], 'unmapped_callback_candidates': [],
                'unmapped_dispatch_candidates': [],
                'calls_to_uninventoried_targets': [],
                'unmapped_dispatch_entries': [], 'behaviors_without_scenarios': [],
                'behaviors_without_implementation': [], 'behaviors_without_comparison': [], 'behaviors_without_tests': [],
                'stale_evidence': []}
    findings['behavior_anchors_outside_discovered_functions'] = []

    def require(condition, message):
        if not condition:
            errors.append(message)

    def file(path):
        if not isinstance(path, str) or Path(path).is_absolute():
            raise ValueError(f'Expected repository-relative file: {path}')
        result = (root/path).resolve()
        if not result.is_relative_to(root) or not result.is_file():
            raise ValueError(f'Missing or escaping repository file: {path}')
        return result

    def unique(items, label):
        result = {}
        for item in items:
            name = item['id']
            require(bool(re.fullmatch(r'[A-Za-z0-9][A-Za-z0-9_.-]*', name)), f'Invalid {label} ID: {name}')
            require(name not in result, f'Duplicate {label} ID: {name}')
            result[name] = item
        return result

    try:
        require(register['schema'] == 1, 'Unsupported register schema')
        builds = unique(register['builds'], 'build')
        behaviors = unique(register['behaviors'], 'behavior')
        evidence = unique(register['evidence'], 'evidence')
        scenarios = unique(register['scenarios'], 'scenario')
        inventories, entries = {}, {}
        for build_id, build in builds.items():
            inventory_file = file(build['inventory'])
            require(sha(inventory_file) == build['inventory_sha256'], f'{build_id}: inventory hash mismatch')
            inventory = json.loads(inventory_file.read_text())
            require(inventory['schema'] == 1 and inventory['build'] == build_id, f'{build_id}: inventory identity mismatch')
            require(inventory['source_sha256'] == build['source_sha256'], f'{build_id}: original binary hash mismatch')
            require(bool(re.fullmatch(r'[0-9a-f]{64}', build['source_sha256'])), f'{build_id}: malformed original digest')
            fs = {f['entry']: f for f in inventory['functions']}
            require(len(fs) == len(inventory['functions']), f'{build_id}: duplicate function entry')
            executable = [s for s in inventory['sections'] if s['executable']]
            for f in fs.values():
                require(any(a <= f['entry'] < b for a, b in f['ranges']), f'{build_id}: entry outside body')
                require(all(a < b and any(s['start'] <= a < b <= s['end'] for s in executable)
                            for a, b in f['ranges']), f'{build_id}: invalid executable function ranges')
                require(bool(re.fullmatch(r'[0-9a-f]{64}', f['sha256'])), f'{build_id}: malformed function byte digest')
            require(inventory['unassigned_executable_ranges'] == range_complement(inventory['sections'], inventory['functions']),
                    f'{build_id}: unassigned executable accounting does not equal function-body complement')
            require(len({i['iat_va'] for i in inventory['imports']}) == len(inventory['imports']), f'{build_id}: duplicate import slot')
            inventories[build_id], entries[build_id] = inventory, fs
            findings['unassigned_executable_ranges'].extend(
                {'build': build_id, 'start': a, 'end': b} for a, b in inventory['unassigned_executable_ranges'])
            findings['unresolved_indirect_flows'].extend(
                {'build': build_id, **flow} for flow in inventory['flows'] if flow['computed'] and not flow['targets'])
            findings['computed_flows_with_inferred_targets'].extend(
                {'build': build_id, **flow} for flow in inventory['flows'] if flow['computed'] and flow['targets'])
            spans = sorted((a, b) for f in fs.values() for a, b in f['ranges'])
            merged = []
            for a, b in spans:
                if merged and a <= merged[-1][1]:
                    merged[-1][1] = max(b, merged[-1][1])
                else:
                    merged.append([a, b])
            starts = [a for a, _ in merged]
            for flow in inventory['flows']:
                if flow['kind'] != 'call':
                    continue
                for target in flow['targets']:
                    if not isinstance(target, int) or not any(s['start'] <= target < s['end'] for s in executable):
                        continue
                    index = bisect_right(starts, target)-1
                    if index < 0 or target >= merged[index][1]:
                        findings['calls_to_uninventoried_targets'].append(
                            {'build': build_id, 'site': flow['site'], 'owner': flow['owner'], 'target': target})

        recovered_ranges = unique(register['recovered_ranges'], 'recovered range')
        for rid, recovered in recovered_ranges.items():
            build_id = recovered['build']
            require(build_id in inventories, f'{rid}: recovered range references unknown build')
            require(recovered['start'] < recovered['end'] and any(
                    s['executable'] and s['start'] <= recovered['start'] < recovered['end'] <= s['end']
                    for s in inventories.get(build_id, {}).get('sections', [])), f'{rid}: invalid recovered code range')
            file(recovered['document'])
            require(recovered['evidence'] in evidence, f'{rid}: missing range evidence')
            ev = evidence.get(recovered['evidence'])
            if ev:
                require(ev.get('build') == build_id, f'{rid}: recovered range/evidence build mismatch')
                require(ev['kind'] in {'isolated_original', 'live_equivalence', 'live_replacement'},
                        f'{rid}: range exception requires pinned original execution evidence')
                record = json.loads(file(ev['record']).read_text())
                anchors = pointer(record, recovered['anchors_pointer'])
                require(any(int(a, 16) == recovered['start'] for a in anchors), f'{rid}: range start lacks pinned original entry bytes')
                require(all(isinstance(value, str) and bool(re.fullmatch(r'(?:[0-9a-fA-F]{2})+', value)) for value in anchors.values()),
                        f'{rid}: malformed original entry-byte anchors')

        def anchor(ref, label):
            build_id = ref['build']
            require(build_id in builds, f'{label}: unknown build {build_id}')
            address = ref['address']
            require(isinstance(address, int) and not isinstance(address, bool), f'{label}: address must be integer VA')
            if build_id not in entries:
                return None
            relation = ref.get('relation', 'code')
            require(relation in {'code', 'data'}, f'{label}: invalid address relation')
            if relation == 'data':
                require(any(s['start'] <= address < s['end'] for s in inventories[build_id]['sections']),
                        f'{label}: data address outside file-backed image')
                require(bool(ref['scope'].strip()), f'{label}: original scope missing')
                return None
            owners = [f['entry'] for f in entries[build_id].values()
                      if any(a <= address < b for a, b in f['ranges'])]
            if not owners and ref.get('recovered_range') in recovered_ranges:
                recovered = recovered_ranges[ref['recovered_range']]
                require(recovered['build'] == build_id and recovered['start'] <= address < recovered['end'],
                        f'{label}: address outside linked recovered range')
                findings['behavior_anchors_outside_discovered_functions'].append(
                    {'behavior': label, 'build': build_id, 'address': address, 'recovered_range': recovered['id']})
            else:
                require(bool(owners), f'{label}: address {address:#x} not in discovered function; resolve inventory gap explicitly')
            require(bool(ref['scope'].strip()), f'{label}: original scope missing')
            return (build_id, owners[0]) if owners else None

        evidence_sources = {}
        for eid, ev in evidence.items():
            require(ev['kind'] in KINDS, f'{eid}: invalid evidence kind')
            require(bool(ev['scope'].strip()), f'{eid}: evidence scope missing')
            require(bool(ev['assertions']), f'{eid}: evidence requires explicit success assertions')
            record_file = file(ev['record'])
            require(sha(record_file) == ev['sha256'], f'{eid}: evidence record hash mismatch')
            record = json.loads(record_file.read_text())
            for assertion in ev['assertions']:
                require(pointer(record, assertion['pointer']) == assertion['equals'], f'{eid}: failed evidence assertion {assertion["pointer"]}')
            if ev['kind'] in {'isolated_original', 'live_equivalence', 'live_replacement'}:
                build_id = ev['build']
                require(build_id in builds, f'{eid}: unknown original build')
                require(pointer(record, ev['original_hash_pointer']) == builds[build_id]['source_sha256'],
                        f'{eid}: evidence belongs to different original binary')
            if ev['kind'] in {'live_equivalence', 'live_replacement'}:
                require(pointer(record, ev['equivalence_pointer']) is True, f'{eid}: live equivalence outcome not established')
            if ev['kind'] == 'live_replacement':
                require(pointer(record, ev['bypass_pointer']) is True, f'{eid}: original work bypass not established')
            sources = source_hashes(pointer(record, ev['sources_pointer']))
            recorded_inputs = ev.get('recorded_inputs', [])
            require(isinstance(recorded_inputs, list) and len(recorded_inputs) == len(set(recorded_inputs)),
                    f'{eid}: malformed/duplicate recorded input keys')
            code_suffixes = {'.cpp', '.hpp', '.c', '.h', '.inc', '.S', '.py', '.java', '.sh', '.json', '.html', '.css', '.js'}
            for key in recorded_inputs:
                require(key in sources, f'{eid}: recorded input lacks fingerprint')
                require(Path(key).suffix not in code_suffixes and Path(key).name != 'CMakeLists.txt',
                        f'{eid}: code provenance cannot be declared an input')
                target = ev.get('source_bindings', {}).get(key, key)
                require(Path(target).suffix not in code_suffixes and Path(target).name != 'CMakeLists.txt',
                        f'{eid}: bound code provenance cannot be declared an input')
                require(bool(re.fullmatch(r'[0-9a-f]{64}', sources[key])), f'{eid}: malformed input digest')
            if 'source_bindings' in ev:
                bindings = ev['source_bindings']
                if not isinstance(bindings, dict):
                    raise ValueError(f'{eid}: source bindings must be a map')
                require(bool(bindings), f'{eid}: empty source bindings')
                require(len(set(bindings.values())) == len(bindings), f'{eid}: duplicate bound source path')
                if any(key not in sources for key in bindings):
                    raise ValueError(f'{eid}: binding lacks recorded source fingerprint')
                sources = {path: sources[record_path] for record_path, path in bindings.items()
                           if record_path not in recorded_inputs}
            else:
                sources = {path: digest for path, digest in sources.items() if path not in recorded_inputs}
            require(bool(sources), f'{eid}: empty source provenance')
            evidence_sources[eid] = sources
            changed = []
            for path, digest in sources.items():
                require(bool(re.fullmatch(r'[0-9a-f]{64}', digest)), f'{eid}: malformed source digest')
                if sha(file(path)) != digest:
                    changed.append(path)
            if changed:
                findings['stale_evidence'].append({'id': eid, 'changed_sources': sorted(changed)})

        def links(ids, available, label):
            require(len(ids) == len(set(ids)), f'{label}: duplicate links')
            for identifier in ids:
                require(identifier in available, f'{label}: dangling link {identifier}')

        mapped = set()
        for bid, behavior in behaviors.items():
            require(bool(behavior['title'].strip()) and bool(behavior['scope'].strip()), f'{bid}: title/scope missing')
            require(behavior['kind'] in {'recovered', 'native_policy'}, f'{bid}: invalid behavior kind')
            require(behavior['confidence'] in {'unknown', 'provisional', 'high_within_scope'}, f'{bid}: invalid confidence')
            require(set(behavior['status']) == set(STAGES), f'{bid}: missing/extra status dimensions')
            for stage, values in STAGES.items():
                require(behavior['status'].get(stage) in values, f'{bid}: invalid {stage} status')
            for ref in behavior['original']:
                owner = anchor(ref, bid)
                if owner:
                    mapped.add(owner)
            for path in behavior['documents']:
                file(path)
            for path in behavior['tests']:
                file(path)
            for path in behavior['implementation']:
                file(path)
            links(behavior['evidence'], evidence, bid)
            kinds = {evidence[x]['kind'] for x in behavior['evidence'] if x in evidence}
            status = behavior['status']
            if status['understanding'] != 'unknown' and behavior['kind'] == 'recovered':
                require(bool(behavior['original']) and bool(behavior['documents']), f'{bid}: recovered claim missing original/docs')
            if status['implementation'] != 'none':
                require(bool(behavior['implementation']), f'{bid}: implementation missing code/tests')
                if status['implementation'] == 'scoped':
                    require(bool(behavior['tests']), f'{bid}: implementation missing code/tests')
                if not behavior['tests']:
                    findings['behaviors_without_tests'].append(bid)
            else:
                findings['behaviors_without_implementation'].append(bid)
            if status['comparison'] == 'recorded':
                require(behavior['kind'] == 'recovered', f'{bid}: native policy must not claim original equivalence')
                comparisons = [x for x in behavior['evidence'] if x in evidence and evidence[x]['kind'] in {'isolated_original', 'live_equivalence', 'live_replacement'}]
                require(bool(comparisons), f'{bid}: comparison without original comparison evidence')
                require(any(set(behavior['implementation']) & set(evidence_sources.get(x, {})) for x in comparisons),
                        f'{bid}: comparison does not fingerprint linked implementation')
                original_builds = {r['build'] for r in behavior['original']}
                require(all(evidence[x]['build'] in original_builds for x in comparisons), f'{bid}: comparison and original mapping builds differ')
            else:
                findings['behaviors_without_comparison'].append(bid)
            if status['integration'] in {'headless', 'preview'}:
                require('native_integration' in kinds, f'{bid}: headless claim without integration evidence')
            if status['integration'] in {'live_observation', 'live_equivalence'}:
                require(status['integration'] in kinds or 'live_replacement' in kinds, f'{bid}: live integration without matching evidence')
            if status['replacement'] == 'scoped_live':
                require('live_replacement' in kinds and status['integration'] == 'live_equivalence'
                        and status['implementation'] == 'scoped' and status['comparison'] == 'recorded',
                        f'{bid}: replacement requires live equivalence and bypass evidence')

        for classification in register['classifications']:
            build_id, entry = classification['build'], classification['entry']
            require(entry in entries.get(build_id, {}), 'Classification references unknown function')
            require(classification['category'] in {'runtime_support', 'legacy_only', 'unused_candidate'}, 'Invalid classification category')
            require(bool(classification['reason'].strip()) and bool(classification['documents']), 'Classification requires reason/evidence')
            for path in classification['documents']:
                file(path)
            mapped.add((build_id, entry))
        scenario_behaviors = set()
        for sid, scenario in scenarios.items():
            require(scenario['level'] in {'headless', 'preview', 'live_observation', 'live_equivalence'}, f'{sid}: invalid scenario level')
            require(bool(scenario['scope'].strip()), f'{sid}: scenario scope missing')
            links(scenario['behaviors'], behaviors, sid)
            links(scenario['evidence'], evidence, sid)
            require(bool(scenario['evidence']) and bool(scenario['tests']), f'{sid}: scenario missing evidence/tests')
            for path in scenario['tests']:
                file(path)
            required_kind = 'native_integration' if scenario['level'] in {'headless', 'preview'} else scenario['level']
            accepted_kinds = {required_kind} | ({'live_replacement'} if required_kind == 'live_equivalence' else set())
            require(any(evidence[e]['kind'] in accepted_kinds for e in scenario['evidence'] if e in evidence), f'{sid}: scenario evidence level mismatch')
            scenario_behaviors.update(scenario['behaviors'])
        for bid, behavior in behaviors.items():
            if bid not in scenario_behaviors:
                findings['behaviors_without_scenarios'].append(bid)
            if behavior['status']['integration'] != 'none':
                require(any(bid in s['behaviors'] and s['level'] == behavior['status']['integration'] for s in scenarios.values()),
                        f'{bid}: integration status lacks matching scenario')
        for table in register['dispatch_tables']:
            anchor(table['original'], table['id'])
            file(table['document'])
            require(isinstance(table['complete'], bool) and bool(table['scope'].strip()), 'Dispatch table scope/completeness missing')
            values = set()
            for item in table['entries']:
                require(item['value'] not in values, f'{table["id"]}: duplicate dispatch value')
                values.add(item['value'])
                links(item['behaviors'], behaviors, table['id'])
                if not item['behaviors']:
                    findings['unmapped_dispatch_entries'].append({'table': table['id'], 'value': item['value']})
        bound_imports = set()
        for binding in register['api_bindings']:
            build_id = binding['build']
            require(build_id in inventories, 'API binding references unknown build')
            links(binding['behaviors'], behaviors, 'API binding')
            require(bool(binding['behaviors']) and bool(binding['scope'].strip()), 'API binding requires behaviors/scope')
            slot = binding['iat_va']
            require(any(i['iat_va'] == slot for i in inventories.get(build_id, {}).get('imports', [])), 'API binding references unknown import slot')
            bound_imports.add((build_id, slot))
        for build_id, inventory in inventories.items():
            findings['unclassified_functions'].extend({'build': build_id, 'entry': f['entry'], 'name': f['name']}
                for f in inventory['functions'] if (build_id, f['entry']) not in mapped)
            # Imports remain external boundaries until an explicit API behavior mapping exists.
            findings['unmapped_imports'].extend({'build': build_id, **i} for i in inventory['imports']
                                               if (build_id, i['iat_va']) not in bound_imports)
            findings['unmapped_callback_candidates'].extend(
                {'build': build_id, 'entry': f['entry'],
                 'data_references': [r for r in f['data_references'] if isinstance(r, int)]}
                for f in inventory['functions'] if (build_id, f['entry']) not in mapped
                and any(isinstance(r, int) for r in f['data_references']))
            dispatch_addresses = {t['original']['address'] for t in register['dispatch_tables']
                                  if t['original']['build'] == build_id}
            findings['unmapped_dispatch_candidates'].extend(
                {'build': build_id, **flow} for flow in inventory['flows'] if flow['kind'] == 'jump' and flow['computed']
                and flow['site'] not in dispatch_addresses
                and not any(r['target'] in dispatch_addresses for r in flow.get('references', [])))
        summaries = {stage: {value: sum(b['status'][stage] == value for b in behaviors.values())
                             for value in sorted(values)} for stage, values in STAGES.items()}
        if findings['stale_evidence']:
            warnings.append('Recorded evidence fingerprints older source; historical claims are retained, current validation is not implied.')
        if findings['behavior_anchors_outside_discovered_functions']:
            warnings.append('Pinned recovered code exists outside Ghidra function bodies; explicit range links preserve this discovery gap.')
        stale_ids = {item['id'] for item in findings['stale_evidence']}
        rows = [{**behavior, 'evidence_freshness': 'stale' if set(behavior['evidence']) & stale_ids else
                 ('current_fingerprints' if behavior['evidence'] else 'no_evidence'),
                 'scenarios': [sid for sid, s in scenarios.items() if bid in s['behaviors']]}
                for bid, behavior in behaviors.items()]
        if 'register_imports' in register:
            findings['changed_focused_registers'] = []
            for imported in register['register_imports']:
                require(bool(imported['scope'].strip()), 'Focused-register import scope missing')
                require(bool(re.fullmatch(r'[0-9a-f]{64}', imported['sha256'])), 'Malformed focused-register import digest')
                if sha(file(imported['path'])) != imported['sha256']:
                    findings['changed_focused_registers'].append(imported['path'])
            if findings['changed_focused_registers']:
                warnings.append('Focused register changed since consolidation; reconcile its entries into the central register.')
        source_counts = {}
        if 'code_index' in register:
            from importlib.util import spec_from_file_location, module_from_spec
            spec = spec_from_file_location('coverage_source_index', Path(__file__).with_name('index-coverage-sources.py'))
            source_tool = module_from_spec(spec)
            spec.loader.exec_module(source_tool)
            index_file = file(register['code_index']['path'])
            require(sha(index_file) == register['code_index']['sha256'], 'Source index hash mismatch')
            catalog = json.loads(index_file.read_text())
            require(catalog['schema'] == 1, 'Unsupported source index schema')
            # Retained Git bases predate the host adapter root. Accept their
            # exact declaration; current_paths still includes new compat code,
            # so an old census cannot hide adapter additions from the gate.
            legacy_roots = (catalog['roots'] == [name for name in source_tool.SOURCE_ROOTS if name != 'compat']
                            and not any(Path(row['path']).parts[0] == 'compat' for row in catalog['files']))
            require(catalog['roots'] == list(source_tool.SOURCE_ROOTS) or legacy_roots,
                    'Source index roots differ from census scope')
            # Retained Git bases predate the web-source extension. Accept that
            # exact older declaration only for catalogs without web rows. New
            # web files still enter current_paths and fail the gate as drift;
            # this does not permit an old census to hide frontend changes.
            web_suffixes = {'.html', '.css', '.js'}
            legacy_catalog = (catalog['suffixes'] == sorted(source_tool.SUFFIXES - web_suffixes)
                              and not any(Path(row['path']).suffix in web_suffixes for row in catalog['files']))
            require(catalog['suffixes'] == sorted(source_tool.SUFFIXES) or legacy_catalog,
                    'Source index suffixes differ from census scope')
            current_paths = set(source_tool.source_paths(root))
            indexed_paths = {item['path'] for item in catalog['files']}
            require(len(indexed_paths) == len(catalog['files']), 'Duplicate source index path')
            findings['new_sources_without_index'] = sorted(current_paths-indexed_paths)
            findings['removed_indexed_sources'] = sorted(indexed_paths-current_paths)
            findings['changed_indexed_sources'] = []
            findings['sources_without_behavior_links'] = []
            for item in catalog['files']:
                path = item['path']
                parts = Path(path).parts
                require(bool(parts) and not Path(path).is_absolute() and '..' not in parts
                        and parts[0] in source_tool.SOURCE_ROOTS,
                        f'{path}: source index path outside census scope')
                require(bool(re.fullmatch(r'[0-9a-f]{64}', item['sha256'])), f'{path}: malformed source index digest')
                expected = [{'behavior': b['id'], 'role': role} for b in behaviors.values()
                            for role in ('implementation', 'tests') if path in b[role]]
                require(item['references'] == expected, f'{path}: source index/register link drift')
                if not expected:
                    findings['sources_without_behavior_links'].append(path)
                if path in current_paths and sha(file(path)) != item['sha256']:
                    findings['changed_indexed_sources'].append(path)
            if findings['new_sources_without_index'] or findings['removed_indexed_sources'] or findings['changed_indexed_sources']:
                warnings.append('Source census differs from current tree; review and refresh affected index entries.')
            source_counts['indexed_sources'] = len(indexed_paths)
        return {'schema': 1, 'scope': 'Registered checklist only; no whole-engine completion percentage',
                'errors': errors, 'warnings': warnings, 'status_counts': summaries,
                'behaviors': rows,
                'counts': {'builds': len(builds), 'behaviors': len(behaviors), 'evidence': len(evidence),
                           'scenarios': len(scenarios), 'functions': sum(len(i['functions']) for i in inventories.values()),
                           'registered_recovered_ranges': len(recovered_ranges),
                           'incomplete_dispatch_tables': sum(not t['complete'] for t in register['dispatch_tables']),
                           **source_counts, **{name: len(items) for name, items in findings.items()}}, 'findings': findings}
    except (KeyError, ValueError, TypeError, OSError, IndexError) as exc:
        errors.append(f'Invalid register/inventory/evidence: {exc}')
        return {'schema': 1, 'errors': errors, 'warnings': warnings, 'findings': findings}


def markdown(report):
    lines = ['# Reverse-engineering traceability audit', '',
             'Generated by `tools/audit-re-coverage.py`. Counts describe the registered checklist,',
             'not whole-engine completeness. Historical comparisons do not validate changed source.', '',
             '| Accounting boundary | Count |', '| --- | ---: |']
    lines += [f'| {key.replace("_", " ")} | {value} |' for key, value in report.get('counts', {}).items()]
    lines += ['', '| Independent status | Counts |', '| --- | --- |']
    for stage, counts in report.get('status_counts', {}).items():
        lines.append(f'| {stage} | ' + '; '.join(f'{v}: {n}' for v, n in counts.items()) + ' |')
    subsystems = sorted({b['subsystem'] for b in report.get('behaviors', [])})
    for subsystem in subsystems:
        lines += ['', f'## {subsystem.replace("_", " ").title()} checklist', '',
                  '| ID | Behavior | Original addresses | Understanding | Implementation | Comparison | Integration | Replacement | Evidence freshness |',
                  '| --- | --- | --- | --- | --- | --- | --- | --- | --- |']
        for b in report.get('behaviors', []):
            if b['subsystem'] != subsystem:
                continue
            addresses = ', '.join(f'`{r["address"]:#x}`' for r in b['original']) or ('native policy' if b['kind'] == 'native_policy' else 'unmapped')
            title = b['title'].replace('|', '\\|').replace('\n', ' ')
            lines.append(f'| {b["id"]} | {title} | {addresses} | ' + ' | '.join(b['status'][s] for s in STAGES) + f' | {b["evidence_freshness"]} |')
    lines += ['', '## Evidence freshness', '']
    for item in report['findings']['stale_evidence']:
        lines.append(f'- **{item["id"]}**: ' + ', '.join(f'`{p}`' for p in item['changed_sources']))
    if not report['findings']['stale_evidence']:
        lines.append('No registered source fingerprint differs.')
    lines += ['', '## Checklist gaps', '']
    for key in ('behaviors_without_implementation', 'behaviors_without_tests', 'behaviors_without_comparison', 'behaviors_without_scenarios'):
        lines.append(f'- {key.replace("_", " ")}: ' + ', '.join(report['findings'][key]))
    lines += ['', 'Full addresses, indirect flows, imports and unassigned ranges are in the JSON report.',
              'Computed flows with inferred targets still need dispatch validation; unresolved is a narrower subset.',
              'Unassigned bytes may be padding/data. External DLL and dynamically resolved behavior remains separate.', '']
    if report['errors']:
        lines += ['## Invalid claims', ''] + [f'- {error}' for error in report['errors']]
    return '\n'.join(lines)
