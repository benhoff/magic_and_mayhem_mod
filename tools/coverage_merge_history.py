"""Read actual merge parents to distinguish inherited milestones from new claims.

No historical code is executed. Inheritance grants no source, gap, receipt or
current-validation credit; the actual base and combined tree still face the gate.
"""
import json
from pathlib import Path
import subprocess

from coverage_claims import behavior_contract
from coverage_reviews import read_journal
from coverage_evidence import decode_bytes, mapped_commit

REGISTER = 'research/runtime/coverage/register.json'
REVIEWS = 'research/runtime/coverage/change-reviews.json'


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args], stderr=subprocess.PIPE)


def inherited_history(root, base, current, reviews, staged=False):
    root = Path(root)
    result = {'statuses': {}, 'reviews': [], 'parents': [], 'evidence_id_imports': [], 'errors': []}
    base = git(root, 'rev-parse', '--verify', base + '^{commit}').decode().strip()
    rows = git(root, 'rev-list', '--min-parents=2', '--parents', base + '..HEAD').decode().splitlines()
    parents = {p for row in rows for p in row.split()[2:]}
    pending_parents = []
    if staged:
        path = Path(git(root, 'rev-parse', '--git-path', 'MERGE_HEAD').decode().strip())
        path = path if path.is_absolute() else root/path
        if path.exists():
            for revision in path.read_text().splitlines():
                parent = git(root, 'rev-parse', '--verify', '--end-of-options', revision + '^{commit}').decode().strip()
                parents.add(parent)
                pending_parents.append(parent)
    imports=current.get('evidence_id_imports', [])
    if imports:
        # Include both parents of actual committed merges, and both sides of a
        # real pending merge. Merely naming an unmerged branch is insufficient.
        contexts=set();coparents={}
        def add_parents(group):
            contexts.update(group)
            for parent in group:coparents.setdefault(parent,set()).update(p for p in group if p!=parent)
        # A pending merge admits the committed ancestry of its verified incoming
        # tips too. A historical collision may have been resolved there before
        # that branch reaches HEAD; arbitrary unmerged branches remain excluded.
        for row in git(root,'rev-list','--min-parents=2','--parents','HEAD',*pending_parents).decode().splitlines():add_parents(row.split()[1:])
        if staged:
            merge_path=Path(git(root,'rev-parse','--git-path','MERGE_HEAD').decode().strip())
            merge_path=merge_path if merge_path.is_absolute() else root/merge_path
            if merge_path.exists():add_parents([git(root,'rev-parse','HEAD').decode().strip(),*merge_path.read_text().splitlines()])
        registers={}
        def records(commit):
            if commit not in registers:
                registers[commit]={e['id']:e for e in json.loads(git(root,'show',commit+':'+REGISTER))['evidence']}
            return registers[commit]
        def ancestor(a,b):
            return subprocess.run(['git','-C',str(root),'merge-base','--is-ancestor',a,b],stdout=subprocess.DEVNULL,stderr=subprocess.PIPE).returncode==0
        seen=set()
        for item in imports:
            try:
                if set(item)!={'commit','historical_id','imported_id'}:raise ValueError('unexpected fields')
                commit,eid,alias=mapped_commit(root,item['commit']),item['historical_id'],item['imported_id']
                if commit not in contexts or eid==alias or (commit,eid) in seen:raise ValueError('not an actual distinct merge-parent import')
                seen.add((commit,eid));old=records(commit).get(eid)
                if old is None or alias in records(commit) or current['evidence'].get(alias)!=dict(old,id=alias):raise ValueError('import does not preserve exact historical record')
                canonical=current['evidence'].get(eid)
                witnesses=[other for other in coparents.get(commit,set()) if other!=commit and not ancestor(commit,other) and not ancestor(other,commit)
                           and records(other).get(eid)==canonical and canonical!=old]
                if not witnesses:raise ValueError('no independent actual parent contains the colliding canonical record')
                result['evidence_id_imports'].append(dict(item,historical_record=old,canonical_parent=sorted(witnesses)[0]))
            except (KeyError,TypeError,ValueError,subprocess.CalledProcessError) as e:
                result['errors'].append('Invalid evidence-ID merge import: '+str(e))
    if not parents:
        return result
    current_reviews = {r['id']: r for r in reviews['reviews']}
    baseline_reviews, _ = read_journal(lambda path: decode_bytes(root, git(root, 'show', base + ':' + path)))
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
        journal, _ = read_journal(lambda path: decode_bytes(root, git(root, 'show', parent + ':' + path)))
        for r in journal['reviews']:
            if r['id'] not in old_ids and current_reviews.get(r['id']) == r:
                historical.add(r['id'])
    result['reviews'] = sorted(historical)
    return result
