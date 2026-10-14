"""Read actual merge parents to distinguish inherited milestones from new claims.

No historical code is executed. Inheritance grants no source, gap, receipt or
current-validation credit; the actual base and combined tree still face the gate.
"""
import json
from pathlib import Path
import subprocess

from coverage_claims import behavior_contract

REGISTER = 'research/runtime/coverage/register.json'
REVIEWS = 'research/runtime/coverage/change-reviews.json'


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args], stderr=subprocess.PIPE)


def inherited_history(root, base, current, reviews, staged=False):
    root = Path(root)
    result = {'statuses': {}, 'reviews': [], 'parents': []}
    base = git(root, 'rev-parse', '--verify', base + '^{commit}').decode().strip()
    rows = git(root, 'rev-list', '--min-parents=2', '--parents', base + '..HEAD').decode().splitlines()
    parents = {p for row in rows for p in row.split()[2:]}
    if staged:
        path = Path(git(root, 'rev-parse', '--git-path', 'MERGE_HEAD').decode().strip())
        path = path if path.is_absolute() else root/path
        if path.exists():
            for revision in path.read_text().splitlines():
                parents.add(git(root, 'rev-parse', '--verify', '--end-of-options', revision + '^{commit}').decode().strip())
    if not parents:
        return result
    current_reviews = {r['id']: r for r in reviews['reviews']}
    baseline_reviews = json.loads(git(root, 'show', base + ':' + REVIEWS))
    old_ids = {r['id'] for r in baseline_reviews['reviews']}
    historical = set()
    for parent in sorted(parents):
        # A milestone already reachable from the actual base is not a new import.
        if subprocess.run(['git', '-C', str(root), 'merge-base', '--is-ancestor', parent, base],
                          stdout=subprocess.DEVNULL, stderr=subprocess.PIPE).returncode == 0:
            continue
        register = json.loads(git(root, 'show', parent + ':' + REGISTER))
        builds = {b['id']: b for b in register['builds']}
        evidence = {e['id']: e for e in register['evidence']}
        result['parents'].append(parent)
        for b in register['behaviors']:
            present = current['behaviors'].get(b['id'])
            if not present:
                continue
            now = present['record']
            if now['status'] != b['status']:
                continue
            if any(ref['build'] not in builds for ref in now['original']):
                continue
            if any(current['builds'].get(ref['build']) != builds[ref['build']]['source_sha256']
                   for ref in b['original']):
                continue
            if behavior_contract(now, builds) != behavior_contract(b, builds):
                continue
            if not set(b['evidence']) <= set(now['evidence']) or any(
                    current['evidence'].get(eid) != evidence.get(eid) for eid in b['evidence']):
                continue
            result['statuses'][b['id']] = {
                'commit': parent, 'status': b['status'],
                'contract_sha256': behavior_contract(b, builds), 'evidence': b['evidence']}
        journal = json.loads(git(root, 'show', parent + ':' + REVIEWS))
        for r in journal['reviews']:
            if r['id'] not in old_ids and current_reviews.get(r['id']) == r:
                historical.add(r['id'])
    result['reviews'] = sorted(historical)
    return result
