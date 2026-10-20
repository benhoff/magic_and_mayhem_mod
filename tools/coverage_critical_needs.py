"""Reviewed next work for Critical partials; no engine status or proof inferred."""
import hashlib
from pathlib import Path

from coverage_claims import behavior_contract, fingerprint, required_sources

CATALOG = 'apps/coverage-ui/critical-needs.json'
LABELS = {
    'implementation': 'Missing implementation',
    'recovery': 'Original behavior / ownership recovery',
    'scope': 'Reconcile or extend the bounded scope',
    'validation': 'Validate the supplied implementation',
    'acceptance': 'Supported-session acceptance',
    'review': 'Assessment needs review',
}


def critical_needs_view(definition, register, importance, root):
    if definition.get('schema') != 1 or definition.get('static_review_only') is not True:
        raise ValueError('Critical needs must be a static planning review')
    root = Path(root).resolve()
    by_id = {b['id']: b for b in register['behaviors']}
    builds = {b['id']: b for b in register['builds']}
    critical = {e['behavior'] for e in importance['entries']
                if e['weight'] == 5 and e['implementation'] == 'partial'}
    tasks = {t['behavior']: t for tranche in importance.get('tranche_plan', {}).get('tranches', [])
             for t in tranche['tasks']}
    reviewed, hashes = {}, {}
    for entry in definition['entries']:
        bid = entry['behavior']
        if bid in reviewed or bid not in by_id:
            raise ValueError('Duplicate or unknown Critical needs behavior: ' + bid)
        if (entry.get('primary_need') not in set(LABELS) - {'review'} or
                not entry.get('reason', '').strip() or not entry.get('closure', '').strip() or
                not entry.get('requirements') or
                any(s.get('kind') not in {'implementation', 'validation', 'scope_review', 'diagnosis'}
                    or not s.get('action', '').strip() for s in entry['requirements'])):
            raise ValueError('Critical needs require a reason, actions and closure: ' + bid)
        b = by_id[bid]
        sources = entry.get('source_sha256', {})
        resolution = entry.get('scope_resolution')
        if resolution:
            if (resolution.get('state') != 'reconciled' or entry['primary_need'] == 'scope' or
                    not resolution.get('id', '').strip() or
                    not resolution.get('decision', '').strip() or
                    not resolution.get('bounded_scope', '').strip() or
                    resolution.get('record') not in sources or
                    not resolution.get('supplied_paths') or
                    not set(resolution['supplied_paths']).issubset(sources) or
                    not resolution.get('execution_evidence') or
                    not set(resolution['execution_evidence']).issubset(b.get('evidence', [])) or
                    not resolution.get('remaining')):
                raise ValueError('Reconciled scope needs bounded suppliers, proof links and remaining work: ' + bid)
            for gap in resolution['remaining']:
                if (gap.get('kind') not in {'implementation', 'recovery', 'validation', 'acceptance'} or
                        not gap.get('required_case', '').strip() or
                        not gap.get('closes_when', '').strip() or not gap.get('behaviors') or
                        not set(gap['behaviors']).issubset(by_id)):
                    raise ValueError('Reconciled scope must retain named wider cases and owners: ' + bid)
        missing_inputs = (required_sources(b) | set(b['documents'])) - set(sources)
        contract_changed = entry.get('contract_sha256') != behavior_contract(b, builds)
        if missing_inputs and not contract_changed:
            raise ValueError('Critical assessment omits reviewed inputs: ' + bid)
        changed = ['newly linked input: ' + name for name in sorted(missing_inputs)]
        for name, expected in sources.items():
            p = Path(name)
            target = (root / p).resolve()
            if (p.is_absolute() or not p.parts or p.parts[0] in {'original', 'working', '.git'}
                    or not target.is_relative_to(root) or
                    target.relative_to(root).parts[0] in {'original', 'working', '.git'} or
                    len(expected) != 64 or any(c not in '0123456789abcdef' for c in expected)):
                raise ValueError('Invalid Critical assessment source')
            if name not in hashes:
                hashes[name] = hashlib.sha256(target.read_bytes()).hexdigest() if target.is_file() else None
            if hashes[name] != expected:
                changed.append(name)
        if contract_changed:
            changed.append('behavior contract')
        if entry.get('assessment_sha256') is not None:
            work = tasks.get(bid, {}).get('work_review', {})
            # Computed view fields are not part of the authored assessment.
            authored = {k: v for k, v in work.items() if k not in {
                'freshness', 'changed_sources', 'label', 'acceptance_profiles', 'legacy_diagnostics'}}
            if fingerprint(authored) != entry['assessment_sha256']:
                changed.append('planning assessment')
        reviewed[bid] = {**entry, 'changed_sources': changed,
                         'assessment_state': 'needs_review' if changed else 'current',
                         'primary_need': 'review' if changed else entry['primary_need'],
                         'reviewed_primary_need': entry['primary_need']}
    rows = [reviewed.get(bid, {
        'behavior': bid, 'primary_need': 'review', 'assessment_state': 'needs_review',
        'reason': 'New Critical partial contract has no reviewed needs assessment.',
        'closure': 'Review its scope, implementation and evidence before assigning work.',
        'requirements': [], 'changed_sources': [],
    }) for bid in sorted(critical)]
    critical_ids = {e['behavior'] for e in importance['entries'] if e['weight'] == 5}
    decisions = [dict(entry, implementation=by_id[bid]['status']['implementation'])
                 for bid, entry in sorted(reviewed.items())
                 if bid in critical_ids and entry.get('scope_resolution')]
    return {'total': len(rows), 'labels': LABELS, 'entries': rows, 'scope_decisions': decisions,
            'counts': {key: sum(e['primary_need'] == key for e in rows) for key in LABELS},
            'record': CATALOG, 'static_review_only': True}
