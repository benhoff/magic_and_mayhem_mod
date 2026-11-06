"""Read-only artifact roadmap, with recorded acceptance kept source-phase bound."""
from collections import Counter
import hashlib
import json
from pathlib import Path
import sqlite3

from coverage_evidence import read_json


def rendering_artifacts_view(root, definition):
    root = Path(root)
    config = definition.get('rendering_artifacts')
    if not config:
        return None
    closure = json.loads((root / config['closure']).read_text())
    contract = closure['completion_contract']
    gaps = closure['required_gap_register']
    plan = closure['execution_plan']
    requirements = []
    progress = {'state': 'unavailable', 'record': config['progress_record']}
    try:
        recorded = read_json(root / config['progress_record'], root=root)
        requirements = recorded['requirements']
        # Compute totals from rows, rather than trusting a cached summary count.
        expected = {
            (cfg['id'], 'journey.' + journey['id'] + '.' + leaf['id'])
            for cfg in contract['configurations'] for journey in gaps['journeys']
            for leaf in journey['subrequirements']
        } | {
            (cfg['id'], prefix + row['id'])
            for cfg in contract['configurations']
            for prefix, rows in [('operation.', gaps['operations']), ('gate.', gaps['gates'])]
            for row in rows
        }
        identities = [(row['configuration'], row['id']) for row in requirements]
        if len(identities) != len(set(identities)) or set(identities) != expected:
            raise ValueError('Recorded requirement inventory differs from the completion contract')
        if any(row['state'] not in {'passed', 'pending', 'failed'} for row in requirements):
            raise ValueError('Unknown recorded requirement state')
        if any(row.get('owner_task') not in {t['id'] for t in plan['tasks']} for row in requirements):
            raise ValueError('Unknown requirement task owner')
        if not recorded.get('sources'):
            raise ValueError('Recorded source bindings are missing')
        changed = [
            path for path, sha in recorded['sources'].items()
            if not (root / path).is_file()
            or hashlib.sha256((root / path).read_bytes()).hexdigest() != sha
        ]
        progress.update(state='historical' if changed else 'matching_sources',
                        changed_sources=changed, scope=recorded['scope'],
                        decisions=recorded['decisions'],
                        counts=dict(Counter(row['state'] for row in requirements)),
                        total=len(requirements))
    except (OSError, ValueError, KeyError, TypeError, sqlite3.Error) as exc:
        requirements = []
        progress['error'] = str(exc)

    def counts(rows):
        return {'total': len(rows), **{state: sum(r['state'] == state for r in rows)
                                      for state in ('passed', 'pending', 'failed')}}

    configurations = [
        {**cfg, 'recorded': counts([r for r in requirements if r['configuration'] == cfg['id']])}
        for cfg in contract['configurations']
    ]
    by_requirement = {(r['configuration'], r['id']): r for r in requirements}

    def requirement(row, prefix):
        return {**row, 'results': [
            {'configuration': cfg['id'],
             'state': by_requirement.get((cfg['id'], prefix + row['id']), {}).get('state', 'unavailable')}
            for cfg in contract['configurations']]}

    journeys = []
    for journey in gaps['journeys']:
        prefix = 'journey.' + journey['id'] + '.'
        journeys.append({**journey, 'recorded': counts(
            [r for r in requirements if r['id'].startswith(prefix)]),
            'subrequirements': [requirement(r, prefix) for r in journey['subrequirements']]})
    tasks = []
    for task in plan['tasks']:
        rows = [r for r in requirements if r['owner_task'] == task['id']]
        tasks.append({**task, 'recorded': counts(rows),
                      'updates': config['task_updates'].get(task['id'], [])})
    return {
        'closure': config['closure'], 'documents': config['documents'],
        'contract': {'id': contract['id'], 'revision': contract['revision'],
                     'claim_state': contract['claim_state'],
                     'baseline_commit': contract['reviewed_source_commit'],
                     'configurations': configurations,
                     'gates': contract['acceptance']['gates'],
                     'scope_decisions': contract['scope_decisions'],
                     'claim_limits': contract['claim_limits']},
        'gaps': {'journeys': journeys,
                 'operations': [requirement(r, 'operation.') for r in gaps['operations']],
                 'gates': [requirement(r, 'gate.') for r in gaps['gates']],
                 'briefs': len(gaps['task_briefs']),
                 'leaves': sum(len(j['subrequirements']) for j in gaps['journeys'])},
        'plan': {'tasks': tasks, 'order': plan['recommended_order'],
                 'optional': plan['optional_tasks'], 'optimality': plan['optimality']},
        'progress': progress, 'next_action': config['next_action'],
    }


def artifact_files(view):
    """Allow only the named read-only artifact sources, including unavailable reports."""
    if not view:
        return set()
    return {view['closure'], view['progress']['record'], *view['documents'].values(),
            *(update['document'] for task in view['plan']['tasks'] for update in task['updates'])}
