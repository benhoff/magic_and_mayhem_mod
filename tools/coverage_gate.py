"""Deterministic coverage deltas and hash-bound change accounting (stdlib only)."""
import hashlib
import importlib.util
import json
from pathlib import Path

from coverage_audit import audit, pointer, source_hashes

spec = importlib.util.spec_from_file_location('gate_source_index', Path(__file__).with_name('index-coverage-sources.py'))
source_index = importlib.util.module_from_spec(spec)
spec.loader.exec_module(source_index)

REGISTER = 'research/runtime/coverage/register.json'
BASELINE = 'research/runtime/coverage/gate-baseline.json'
REVIEWS = 'research/runtime/coverage/change-reviews.json'
SCHEMA = 1
# Changes to enforcement/instructions also need accounting. Ordinary prose is
# linked through behavior documents; these files control the workflow itself.
CONTROL_FILES = ('AGENTS.md', 'docs/architecture.md', '.github/workflows/re-coverage.yml',
                 '.githooks/pre-commit', 'research/runtime/coverage/README.md')
DRIFT = {'new_sources_without_index', 'removed_indexed_sources', 'changed_indexed_sources',
         'changed_focused_registers'}


def canonical(value):
    return json.dumps(value, sort_keys=True, separators=(',', ':'), ensure_ascii=True).encode()


def digest(value):
    return hashlib.sha256(canonical(value)).hexdigest()


def file_digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load_json(root, path):
    target = (root / path).resolve()
    if Path(path).is_absolute() or not target.is_relative_to(root.resolve()):
        raise ValueError(f'Escaping repository path: {path}')
    return json.loads(target.read_text())


def gap_items(report):
    """Stable identities, with staleness tracked per evidence/source pair."""
    result = {}
    for kind, items in sorted(report['findings'].items()):
        if kind in DRIFT:
            continue
        for item in items:
            expanded = ([{'id': item['id'], 'source': p} for p in item['changed_sources']]
                        if kind == 'stale_evidence' else [item])
            for identity in expanded:
                # Discovery names are descriptive metadata, not identity.
                if kind in {'unclassified_functions', 'unmapped_callback_candidates'}:
                    identity = {k: identity[k] for k in ('build', 'entry')}
                key = kind + ':' + digest(identity)
                result[key] = {'kind': kind, 'identity': identity}
    return result


def snapshot(root, register=None):
    root = Path(root).resolve()
    register = register if register is not None else load_json(root, REGISTER)
    report = audit(root, register)
    if report['errors']:
        raise ValueError('Coverage audit failed: ' + '; '.join(report['errors']))
    if 'code_index' not in register:
        raise ValueError('Coverage gate requires a pinned code_index')
    drift = {k: report['findings'].get(k, []) for k in DRIFT if report['findings'].get(k)}
    if drift:
        raise ValueError('Reconcile source/focused-register drift before checking: ' + json.dumps(drift, sort_keys=True))
    for path in source_index.source_paths(root):
        resolved = (root/path).resolve()
        if not resolved.is_relative_to(root) or resolved.relative_to(root).parts[0] in {'original', 'working'}:
            raise ValueError(f'Unsafe source-census target: {path}')
    sources = {item['path']: item for item in source_index.index(root, register)['files']}
    for name in ('research', 'docs'):
        for path in sorted((root/name).rglob('*')):
            relative = path.relative_to(root).as_posix()
            if path.is_file() and path.suffix in {'.md', '.json'} and relative not in {BASELINE, REVIEWS}:
                sources[relative] = {'path': relative, 'sha256': file_digest(path), 'role': 'research', 'references': []}
    for path in CONTROL_FILES:
        if (root/path).is_file():
            sources[path] = {'path': path, 'sha256': file_digest(root/path), 'role': 'control', 'references': []}
    for behavior in register['behaviors']:
        for path in behavior['documents']:
            if path not in sources:
                sources[path] = {'path': path, 'sha256': file_digest(root/path), 'role': 'research', 'references': []}
            sources[path]['references'].append({'behavior': behavior['id'], 'role': 'documents'})
    evidence = {e['id']: e for e in register['evidence']}
    evidence_sources = {}
    evidence_keys = {}
    for eid, ev in sorted(evidence.items()):
        recorded = source_hashes(pointer(load_json(root, ev['record']), ev['sources_pointer']))
        bindings = ev.get('source_bindings', {p: p for p in recorded})
        inputs = set(ev.get('recorded_inputs', []))
        evidence_keys[eid] = sorted(set(bindings) | inputs)
        evidence_sources[eid] = {p: recorded[key] for key, p in bindings.items() if key not in inputs}
    behaviors = {}
    builds = {b['id']: b for b in register['builds']}
    for behavior in register['behaviors']:
        bid = behavior['id']
        contract = {'behavior': behavior,
                    'sources': {p: file_digest(root/p) for role in ('implementation', 'tests', 'documents') for p in behavior[role]},
                    'builds': [builds[k] for k in sorted({ref['build'] for ref in behavior['original']})],
                    'evidence': sorted((evidence[e] for e in behavior['evidence']), key=lambda e: e['id']),
                    'scenarios': sorted((s for s in register['scenarios'] if bid in s['behaviors']), key=lambda s: s['id'])}
        behaviors[bid] = {'sha256': digest(contract), 'record': behavior}
    return {'schema': SCHEMA, 'scope': 'Reviewed accounting baseline; gaps remain unresolved and evidence remains historical.',
            'sources': sources, 'behaviors': behaviors, 'evidence': evidence,
            'evidence_keys': evidence_keys, 'evidence_sources': evidence_sources,
            'inventories': {b['inventory']: b['inventory_sha256'] for b in register['builds']},
            'research_artifacts': {p.relative_to(root).as_posix(): file_digest(p)
                                   for p in sorted((root/'research').rglob('*.json'))
                                   if p.relative_to(root).as_posix() not in {BASELINE, REVIEWS}},
            'builds': {b['id']: b['source_sha256'] for b in register['builds']},
            'historical_reviews': load_json(root, REVIEWS)['reviews'] if (root/REVIEWS).is_file() else [],
            'gaps': gap_items(report)}, report


def validate_baseline(value):
    if value['schema'] != SCHEMA:
        raise ValueError('Unsupported gate baseline schema')
    for key in ('sources', 'behaviors', 'evidence', 'evidence_keys', 'evidence_sources', 'builds', 'gaps', 'inventories', 'research_artifacts'):
        if not isinstance(value[key], dict):
            raise ValueError(f'Invalid baseline {key}')
    for path, item in value['sources'].items():
        if Path(path).is_absolute() or '..' in Path(path).parts or item['path'] != path:
            raise ValueError(f'Invalid baseline source path: {path}')
    if not isinstance(value['historical_reviews'], list):
        raise ValueError('Invalid baseline review history')


def changes(before, after):
    files = []
    affected = set()
    for path in sorted(set(before['sources']) | set(after['sources'])):
        old, new = before['sources'].get(path), after['sources'].get(path)
        if old != new:
            references = sorted({ref['behavior'] for item in (old, new) if item
                                 for ref in item['references']})
            affected.update(references)
            files.append({'path': path, 'before_sha256': old['sha256'] if old else None,
                          'after_sha256': new['sha256'] if new else None, 'behaviors': references,
                          'role': (new or old)['role'],
                          'change': 'added' if old is None else 'removed' if new is None else
                                    'modified' if old['sha256'] != new['sha256'] else 'links_changed'})
    for bid in sorted(set(before['behaviors']) | set(after['behaviors'])):
        if before['behaviors'].get(bid) != after['behaviors'].get(bid):
            affected.add(bid)
    contracts = [{'id': bid,
                  'before_sha256': before['behaviors'][bid]['sha256'] if bid in before['behaviors'] else None,
                  'after_sha256': after['behaviors'][bid]['sha256'] if bid in after['behaviors'] else None}
                 for bid in sorted(affected)]
    gaps = {key: after['gaps'][key] for key in sorted(set(after['gaps']) - set(before['gaps']))}
    return {'files': files, 'behaviors': contracts, 'new_gaps': gaps}


def immutable_errors(before, after):
    errors = []
    for path, sha in before['inventories'].items():
        if after['research_artifacts'].get(path) != sha:
            errors.append(f'Pinned original inventory removed or overwritten: {path}; retain it and export a new snapshot')
    for build, sha in before['builds'].items():
        if after['builds'].get(build) != sha:
            errors.append(f'Original build removed or rebound: {build}')
    for eid, old in before['evidence'].items():
        new = after['evidence'].get(eid)
        if new is None:
            errors.append(f'Historical evidence removed: {eid}')
            continue
        # A rename may rebind the same recorded keys. Report identity, assertions
        # and recorded hashes must stay immutable; new results require new IDs.
        fixed = lambda e: {k: v for k, v in e.items() if k not in {'source_bindings', 'scope', 'recorded_inputs'}}
        if fixed(old) != fixed(new):
            errors.append(f'Historical evidence rewritten: {eid}; register a new evidence ID')
        if not set(before['evidence_keys'][eid]) <= set(after['evidence_keys'][eid]):
            errors.append(f'Historical provenance keys removed: {eid}')
        for key in new.get('recorded_inputs', []):
            target = old.get('source_bindings', {}).get(key, key)
            if Path(target).suffix in source_index.SUFFIXES or Path(target).name == 'CMakeLists.txt':
                errors.append(f'Historical code provenance reclassified as input: {eid}/{key}')
    return errors


def review_errors(review, delta, before, after, audit_report):
    errors = []
    label = review.get('id', '(missing ID)')
    if not isinstance(label, str) or not label.strip():
        errors.append('Review ID missing')
    if not isinstance(review.get('reason'), str) or not review['reason'].strip():
        errors.append(f'{label}: review reason missing')
    validation = review.get('validation', {})
    status = validation.get('status')
    if status not in {'pending', 'recorded', 'not_required'} or not isinstance(validation.get('reason'), str) or not validation['reason'].strip():
        errors.append(f'{label}: explicit validation status/reason required')
    for field, key in (('files', 'path'), ('behaviors', 'id')):
        seen = set()
        for item in review.get(field, []):
            name = item[key]
            if name in seen:
                errors.append(f'{label}: duplicate {field} entry {name}')
            seen.add(name)
            for prop in ('before_sha256', 'after_sha256'):
                value = item[prop]
                if value is not None and (not isinstance(value, str) or len(value) != 64 or any(c not in '0123456789abcdef' for c in value)):
                    errors.append(f'{label}: malformed {prop} for {name}')
    current_files = {f['path']: f for f in delta['files']}
    current_behaviors = {b['id']: b for b in delta['behaviors']}
    matched_files = [current_files[f['path']] for f in review.get('files', [])
                     if f['path'] in current_files and all(f[k] == current_files[f['path']][k] for k in ('before_sha256', 'after_sha256'))]
    matched_behaviors = [b['id'] for b in review.get('behaviors', [])
                         if b['id'] in current_behaviors and b == current_behaviors[b['id']]]
    # A recorded execution receipt is bound to its behavior contracts. When
    # every named contract has moved on, unchanged files from that old receipt
    # must not reactivate its now-historical evidence against a later version.
    # Leave the receipt intact, but give it no coverage credit in this delta;
    # current files/contracts still need a matching new receipt and validation.
    if status == 'recorded' and review.get('behaviors') and not matched_behaviors:
        return errors, [], []
    if status == 'not_required':
        if any(f['role'] not in {'tool', 'test', 'build', 'control', 'research'} for f in matched_files):
            errors.append(f'{label}: engine/protocol code requires pending or recorded validation')
        for bid in matched_behaviors:
            b = after['behaviors'].get(bid, before['behaviors'].get(bid))['record']
            if b['kind'] == 'recovered' or any(b['status'][k] != 'none' for k in ('comparison', 'integration', 'replacement')):
                errors.append(f'{label}: {bid} has recovered/validated behavior; validation cannot be not_required')
    if status == 'recorded' and (matched_files or matched_behaviors):
        ids = validation.get('evidence', [])
        stale = {item['id'] for item in audit_report['findings']['stale_evidence']}
        fresh = []
        for eid in ids:
            ev = after['evidence'].get(eid)
            if not ev or ev['kind'] == 'static' or eid in stale:
                errors.append(f'{label}: validation evidence missing, static or stale: {eid}')
            else:
                fresh.append(eid)
        if not fresh:
            errors.append(f'{label}: recorded validation needs fresh execution evidence')
        for bid in matched_behaviors:
            b = after['behaviors'].get(bid, {}).get('record')
            if b and not set(b['evidence']) & set(fresh):
                errors.append(f'{label}: fresh validation does not link affected behavior {bid}')
            if b:
                linked = {after['evidence'][eid]['kind'] for eid in fresh if eid in b['evidence']}
                required = []
                if b['status']['comparison'] == 'recorded':
                    required.append({'isolated_original', 'live_equivalence', 'live_replacement'})
                level = b['status']['integration']
                if level in {'headless', 'preview'}:
                    required.append({'native_integration'})
                elif level in {'live_observation', 'live_equivalence'}:
                    required.append({level} | ({'live_replacement'} if level == 'live_equivalence' else set()))
                if b['status']['replacement'] != 'none':
                    required.append({'live_replacement'})
                if any(not kinds & linked for kinds in required):
                    errors.append(f'{label}: fresh evidence kind does not support current validation levels for {bid}')
        for f in matched_files:
            if f['after_sha256'] and f['role'] not in {'control', 'research'} and not any(after['evidence_sources'][eid].get(f['path']) == f['after_sha256'] for eid in fresh):
                errors.append(f'{label}: fresh validation does not fingerprint changed source {f["path"]}')
    return errors, matched_files, matched_behaviors


def check(before, after, reviews, audit_report):
    validate_baseline(before)
    delta = changes(before, after)
    errors = immutable_errors(before, after)
    covered_files, covered_behaviors, covered_gaps, validated_behaviors = set(), set(), set(), set()
    if reviews.get('schema') != SCHEMA:
        raise ValueError('Unsupported change-review schema')
    current_reviews = {r['id']: r for r in reviews['reviews']}
    for old in before['historical_reviews']:
        if current_reviews.get(old['id']) != old:
            errors.append(f'Historical change review removed or rewritten: {old["id"]}; append a new receipt')
    seen = set()
    for review in reviews['reviews']:
        rid = review['id']
        if rid in seen:
            errors.append(f'Duplicate change-review ID: {rid}')
        seen.add(rid)
        problems, files, behaviors = review_errors(review, delta, before, after, audit_report)
        errors.extend(problems)
        if problems:
            continue
        covered_files.update(f['path'] for f in files)
        covered_behaviors.update(behaviors)
        if review['validation']['status'] == 'recorded':
            validated_behaviors.update(behaviors)
        # Gap acknowledgements count only in a receipt matching this change.
        if files or behaviors:
            covered_gaps.update(review.get('pending_gaps', []))
    for f in delta['files']:
        if f['path'] not in covered_files:
            errors.append(f'Unreviewed source change: {f["path"]}')
    for b in delta['behaviors']:
        if b['id'] not in covered_behaviors:
            errors.append(f'Unreviewed behavior change: {b["id"]}')
        old = before['behaviors'].get(b['id'], {}).get('record', {}).get('status', {})
        new = after['behaviors'].get(b['id'], {}).get('record', {}).get('status', {})
        ranks = {'implementation': {'none': 0, 'partial': 1, 'scoped': 2},
                 'comparison': {'none': 0, 'recorded': 1},
                 'integration': {'none': 0, 'headless': 1, 'preview': 2, 'live_observation': 3, 'live_equivalence': 4},
                 'replacement': {'none': 0, 'scoped_live': 1}}
        promoted = any(ranks[stage].get(new.get(stage), 0) > ranks[stage].get(old.get(stage), 0)
                       and (stage != 'implementation' or new.get(stage) == 'scoped') for stage in ranks)
        if promoted and b['id'] not in validated_behaviors:
            errors.append(f'Status promotion requires fresh recorded validation: {b["id"]}')
    for key in delta['new_gaps']:
        if key not in covered_gaps:
            errors.append(f'New gap needs explicit pending accounting: {key}')
    # An unmatched changed file with existing unlinked debt still requires a
    # hash-bound receipt. No blanket acknowledgement exempts future edits.
    return {'schema': SCHEMA, 'scope': 'Change accounting; no whole-engine completeness claim',
            'baseline_sha256': digest(before), 'errors': sorted(set(errors)),
            'warnings': audit_report['warnings'], 'changes': delta,
            'counts': {'changed_files': len(delta['files']), 'affected_behaviors': len(delta['behaviors']),
                       'new_gaps': len(delta['new_gaps']), 'existing_gaps': len(set(before['gaps']) & set(after['gaps']))}}


def draft(delta):
    return {'schema': SCHEMA, 'reviews': [{'id': 'change-' + digest(delta)[:16], 'reason': '',
            'files': [{k: f[k] for k in ('path', 'before_sha256', 'after_sha256')} for f in delta['files']],
            'behaviors': delta['behaviors'], 'pending_gaps': sorted(delta['new_gaps']),
            'validation': {'status': 'pending', 'reason': '', 'evidence': []}}]}
