"""Reviewed rendering denominators; scoped support never implies completion."""
from collections import Counter
import json
from pathlib import Path

from coverage_audit import audit, pointer, sha

ROOT = Path(__file__).resolve().parents[1]
REGISTER = 'research/runtime/coverage/register.json'
MATRIX = 'research/runtime/native-surface-operation-matrix.json'
SCOPE = 'research/runtime/rendering-coverage-scope.json'
ORIGINAL = {'isolated_original', 'live_equivalence', 'live_replacement'}


def read(root, path):
    relative = Path(path)
    target = (root / relative).resolve()
    if relative.is_absolute() or '..' in relative.parts or not relative.parts or relative.parts[0] in {'original', 'working', '.git'} or not target.is_relative_to(root.resolve()):
        raise ValueError('Only repository accounting inputs may be read: ' + path)
    return json.loads(target.read_text())


def measure(count, total):
    return {'count': count, 'total': total, 'percent': round(100 * count / total, 2) if total else None}


def evidence_record(root, evidence, builds):
    record = read(root, evidence['record'])
    if sha(root / evidence['record']) != evidence['sha256']:
        raise ValueError(evidence['id'] + ': changed evidence record')
    if not evidence['assertions'] or any(pointer(record, a['pointer']) != a['equals'] for a in evidence['assertions']):
        raise ValueError(evidence['id'] + ': failed evidence assertion')
    if evidence['kind'] in ORIGINAL:
        if pointer(record, evidence['original_hash_pointer']) != builds[evidence['build']]['source_sha256']:
            raise ValueError(evidence['id'] + ': wrong original build')
    if evidence['kind'] in {'live_equivalence', 'live_replacement'} and pointer(record, evidence['equivalence_pointer']) is not True:
        raise ValueError(evidence['id'] + ': no equivalence proof')
    if evidence['kind'] == 'live_replacement' and pointer(record, evidence['bypass_pointer']) is not True:
        raise ValueError(evidence['id'] + ': no bypass proof')
    return record


def reconcile(root, register, matrix, definition, audit_report):
    if definition['schema'] != 1:
        raise ValueError('Unsupported rendering scope schema')
    behaviors = {b['id']: b for b in register['behaviors']}
    evidence = {e['id']: e for e in register['evidence']}
    builds = {b['id']: b for b in register['builds']}
    if matrix['build'] != definition['build'] or matrix['source_sha256'] != builds[definition['build']]['source_sha256']:
        raise ValueError('Rendering inputs disagree on original build')
    required = {o['id']: o for o in matrix['operations'] if o['requirement'] == 'required'}
    apis = definition['surface_operations']
    routes = definition['drawing_routes']
    if len(apis) != len(required) or {r['id'] for r in apis} != required.keys():
        raise ValueError('Required surface matrix changed; review the denominator')
    inventory = {bid for bid in behaviors if bid.startswith('RI.')}
    if len(routes) != len(inventory) or {r['id'] for r in routes} != inventory:
        raise ValueError('Drawing inventory changed; review the denominator')
    audited = {b['id']: b for b in audit_report.get('behaviors', [])}
    stale = {e['id'] for e in audit_report.get('findings', {}).get('stale_evidence', [])}
    trusted = not audit_report['errors']
    records = {}

    def proof(item, domain):
        if domain not in {'original_executable', 'standalone_driver', 'replacement'}:
            raise ValueError('Unreviewed validation domain: ' + domain)
        b, e = behaviors[item['behavior']], evidence[item['evidence']]
        if e['id'] not in b['evidence']:
            raise ValueError(b['id'] + ': evidence not linked')
        if domain == 'original_executable' and (b['kind'] != 'recovered' or b['status']['comparison'] != 'recorded' or e['kind'] not in ORIGINAL):
            raise ValueError(b['id'] + ': no registered original comparison')
        if domain == 'standalone_driver' and e['kind'] != 'native_integration':
            raise ValueError(b['id'] + ': invalid driver comparison kind')
        if domain == 'replacement' and (b['kind'] != 'recovered' or b['status']['replacement'] != 'scoped_live' or e['kind'] != 'live_replacement'):
            raise ValueError(b['id'] + ': observation/policy cannot count as replacement')
        if e['id'] not in records:
            records[e['id']] = evidence_record(root, e, builds)
        stage = 'replacement' if domain == 'replacement' else 'comparison' if domain == 'original_executable' else 'implementation'
        current = trusted and e['id'] in audited.get(b['id'], {}).get('current_validation', {}).get(stage, {}).get('evidence', [])
        return {**item, 'domain': domain, 'record': e['record'], 'scope': e['scope'],
                'source_freshness': 'stale' if e['id'] in stale else 'unchanged' if trusted else 'unknown',
                'current': current}

    def row_view(row):
        if not row['scope'].strip() or not row['remaining'].strip():
            raise ValueError(row['id'] + ': scope and remaining boundary are required')
        code = [behaviors[bid] for bid in row['implementation']]
        for b in code:
            if b['status']['implementation'] != 'none' and not b['implementation']:
                raise ValueError(b['id'] + ': implemented status has no source links')
        statuses = {b['status']['implementation'] for b in code}
        implementation = 'scoped_support' if 'scoped' in statuses else 'partial_support' if 'partial' in statuses else 'none'
        validations = [proof(p, p['domain']) for p in row['validation']]
        replacements = [proof(p, 'replacement') for p in row['replacement']]
        return {**row, 'implementation_state': implementation,
                'validation': validations, 'replacement': replacements,
                'current_implementation': trusted and any(audited.get(b['id'], {}).get('current_validation', {}).get('implementation', {}).get('state') == 'current' for b in code),
                'remaining_matrix_gap': required[row['id']]['remaining_gap'] if row['id'] in required else behaviors[row['id']]['scope']}

    def group(rows):
        result = [row_view(r) for r in rows]
        return {'rows': result, 'implementation_states': dict(Counter(r['implementation_state'] for r in result)),
                'scoped_code_support': measure(sum(r['implementation_state'] == 'scoped_support' for r in result), len(result)),
                'any_code_support': measure(sum(r['implementation_state'] != 'none' for r in result), len(result)),
                'independent_validation_support': measure(sum(bool(r['validation']) for r in result), len(result)),
                'original_executable_comparison_support': measure(sum(any(p['domain'] == 'original_executable' for p in r['validation']) for r in result), len(result)),
                'scoped_live_replacement_support': measure(sum(bool(r['replacement']) for r in result), len(result)),
                'current_support': {key: measure(sum(bool(r[key]) if key == 'current_implementation' else any(p['current'] for p in r[key]) for r in result), len(result)) if trusted else None for key in ('current_implementation', 'validation', 'replacement')}}

    api_group, route_group = group(apis), group(routes)
    takeovers = []
    for selected in definition['world_takeovers']:
        p = proof(selected, 'replacement')
        record = records[p['evidence']]
        admitted = {int(a, 16) for a in selected['admitted_entries']}
        observed = {int(a, 16) for a, count in record['actual_entry_counts'].items() if count > 0}
        if not admitted or len(admitted) != len(selected['admitted_entries']) or not observed <= admitted:
            raise ValueError('World entry inventory changed or duplicated')
        takeovers.append({**p, 'exercised_entries': measure(len(observed), len(admitted)),
                          'unobserved_entries': [hex(a) for a in sorted(admitted - observed)],
                          'complete_queues': record['complete_raster_queues'],
                          'original_call_comparisons': record['caller_ax_equal'],
                          'actual_entry_counts': record['actual_entry_counts']})
    return {'schema': 1, 'scope': definition['scope'], 'overall_completion_percent': None,
            'audit': {'valid': trusted, 'errors': audit_report['errors'], 'warnings': audit_report['warnings']},
            'surface_operations': api_group, 'drawing_routes': route_group,
            'conditional_surface_operations': [o for o in matrix['operations'] if o['requirement'] == 'conditional'],
            'world_takeovers': takeovers,
            'release_contract': {k: behaviors['NR.live-drawing-replacement'][k] for k in ('id', 'scope', 'status')},
            'unresolved_render_dispatch': [d for d in register['dispatch_tables'] if not d['complete'] and (
                d['id'].startswith(('RI.', 'kind8-', 'world-', 'minimap-')) or
                any(bid.startswith(('RI.', 'RS.', 'RE.sprite', 'RE.terrain')) for e in d['entries'] for bid in e.get('behaviors', [])))]}


def build_report(root=ROOT):
    root = Path(root).resolve()
    register, matrix, definition = [read(root, path) for path in (REGISTER, MATRIX, SCOPE)]
    result = reconcile(root, register, matrix, definition, audit(root, register))
    result['input_sha256'] = {path: sha(root / path) for path in (REGISTER, MATRIX, SCOPE, 'research/runtime/render-drawing-inventory.md', 'docs/live-drawing-replacement.md', 'tools/rendering_coverage.py', 'tools/report-rendering-coverage.py')}
    return result


def markdown(report):
    lines = ['# Rendering coverage reconciliation', '', report['scope'], '',
             'Each percentage counts categories with **some scoped support**, not complete categories, effort or whole-game completion. Routes overlap. API and route denominators are never added together. Driver fixtures remain separate from original-executable comparison. Historical results do not validate changed code.', '',
             '| Audited denominator | Categories with scoped code | Independent subset validation | Original-executable subset comparison | Scoped live bypass |',
             '| --- | ---: | ---: | ---: | ---: |']
    def value(m):
        return f"{m['count']}/{m['total']} ({m['percent']}%)" if m['percent'] is not None else 'N/A'
    for key, title in [('surface_operations', 'Required surface operations'), ('drawing_routes', 'Drawing-route families')]:
        g = report[key]
        lines.append('| ' + title + ' | ' + ' | '.join(value(g[k]) for k in ('scoped_code_support', 'independent_validation_support', 'original_executable_comparison_support', 'scoped_live_replacement_support')) + ' |')
    lines += ['', 'Current validation: ' + ('available separately in JSON.' if report['audit']['valid'] else 'unavailable while the coverage audit has errors; historical counts above remain explicitly historical.'), '']
    for key, title in [('surface_operations', 'Required surface operation boundaries'), ('drawing_routes', 'Drawing-route boundaries')]:
        lines += ['## ' + title, '', '| ID | Code support | Independent validation | Live bypass | Remaining boundary |', '| --- | --- | --- | --- | --- |']
        for r in report[key]['rows']:
            validations = ', '.join(p['domain'] for p in r['validation']) or 'none'
            lines.append(f"| {r['id']} | {r['implementation_state']} | {validations} | {'scoped only' if r['replacement'] else 'none'} | {r['remaining']} |")
    lines += ['', '## Bounded World takeover executions', '', '| Evidence | Exercised / admitted entries | Queues | Compared calls / AX | Current |', '| --- | ---: | --- | ---: | --- |']
    for t in report['world_takeovers']:
        lines.append(f"| {t['evidence']} | {value(t['exercised_entries'])} | {t['complete_queues'][0]}–{t['complete_queues'][-1]} | {t['original_call_comparisons']} | {t['current']} |")
    lines += ['', 'These admitted entries are a bounded whitelist, not the complete binary dispatch space. Runs retain their own source versions; entry coverage is not pooled into a current-code claim.', '',
              'Complete launcher status: `' + json.dumps(report['release_contract']['status'], sort_keys=True) + '`.', '',
              'Four conditional surface categories remain in the JSON backlog. Required branches, unknown producers, incomplete dispatch, lifecycle and supported-session gates remain open; no overall completion percentage is asserted.', '']
    if report['audit']['errors']:
        errors = report['audit']['errors']
        lines += ['## Audit limitations', '', f"{len(errors)} audit errors; the complete list is retained in JSON.", '']
        lines += ['- ' + e for e in errors[:8]] + ['']
    return '\n'.join(lines)
