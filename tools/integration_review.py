"""Narrow merge bookkeeping. Unknown semantic conflicts always require review."""
import hashlib
import json
from pathlib import Path
import subprocess

from coverage_bookkeeping import Workspace, atomic_replace, encoded
from coverage_evidence import Reader, MAGIC, artifact, identity, write_report
from coverage_reviews import load_journal
from evidence_local import local_evidence
from evidence_objects import Sink

REGISTER = 'research/runtime/coverage/register.json'
JOURNAL = 'research/runtime/coverage/change-reviews.json'
LEDGER = 'research/runtime/coverage-ledger.md'
ALLOWED = {REGISTER, JOURNAL, LEDGER}


def managed_report(root, report):
    if report is None:
        raise ValueError('A new SQLite-backed report path is required')
    path = Path(report)
    if path.is_absolute():
        path = path.relative_to(root)
    if ('..' in path.parts or path.parts[:2] != ('research', 'runtime')
            or path.suffix != '.json' or not artifact(path.as_posix())
            or (root/path).exists() or (root/path).is_symlink()):
        raise ValueError('Use a new generated research/runtime/*.json report reference')
    return path.as_posix()


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args], stderr=subprocess.PIPE)


def merge_json(base, ours, theirs, *, path='', census=False):
    if ours == theirs:
        return ours
    if ours == base:
        return theirs
    if theirs == base:
        return ours
    if census and path == '/code_index':
        return ours  # Acknowledged stale until finish regenerates the combined census.
    if all(isinstance(v, dict) for v in (base, ours, theirs)):
        result = {}
        for key in dict.fromkeys([*ours, *theirs]):
            if key not in base:
                if key in ours and key in theirs and ours[key] != theirs[key]:
                    raise ValueError('Conflicting metadata addition: '+path+'/'+key)
                result[key] = ours.get(key, theirs.get(key))
            elif key not in ours or key not in theirs:
                raise ValueError('Metadata deletion requires review: '+path+'/'+key)
            else:
                result[key] = merge_json(base[key], ours[key], theirs[key], path=path+'/'+key, census=census)
        return result
    if all(isinstance(v, list) for v in (base, ours, theirs)):
        for key in ('id', 'path'):
            if not all(isinstance(v, dict) and key in v for seq in (base, ours, theirs) for v in seq):
                continue
            maps = [{v[key]:v for v in seq} for seq in (base, ours, theirs)]
            if any(len(m) != len(seq) for m,seq in zip(maps, (base, ours, theirs))):
                continue
            b, o, t = maps
            if not set(b) <= set(o) or not set(b) <= set(t):
                raise ValueError('Metadata member deletion requires review: '+path)
            result = []
            for name in dict.fromkeys([*o, *t]):
                if name not in b:
                    if name in o and name in t and o[name] != t[name]:
                        raise ValueError('Conflicting keyed addition: '+path+'/'+str(name))
                    result.append(o.get(name, t.get(name)))
                elif o[name] == t[name] or t[name] == b[name]:
                    result.append(o[name])
                elif o[name] == b[name]:
                    result.append(t[name])
                else:
                    # Never synthesize a behavior, evidence definition or review
                    # receipt from independent changes to the same record.
                    raise ValueError('Both parents changed one metadata record: '+path+'/'+str(name))
            return result
    raise ValueError('Semantic metadata conflict requires review: '+path)


def reconcile(root, *, execute=False, reason='', report=None):
    root = Path(root).resolve()
    incoming = git(root, 'rev-parse', '--verify', 'MERGE_HEAD').decode().strip()
    conflicts = [p.decode() for p in git(root, 'diff', '--name-only', '--diff-filter=U', '-z').split(b'\0') if p]
    if not set(conflicts) <= ALLOWED:
        raise ValueError('Nonmetadata conflicts require manual resolution: '+str(sorted(set(conflicts)-ALLOWED)))
    planned = []
    for name in conflicts:
        parents = [git(root, 'show', ':'+str(stage)+':'+name) for stage in (1, 2, 3)]
        if name == LEDGER:
            base, ours, theirs = parents
            if not ours.startswith(base) or not theirs.startswith(base):
                raise ValueError('Ledger changes are not append-only; resolve manually')
            result = ours + b'\n\n' + theirs[len(base):]
        else:
            value = merge_json(*(json.loads(p) for p in parents), census=name == REGISTER)
            result = encoded(value) if name == JOURNAL else (json.dumps(value, indent=2)+'\n').encode()
        planned.append(dict(path=name, before=(root/name).read_bytes(), after=result,
                            parent_sha256=[hashlib.sha256(p).hexdigest() for p in parents]))
    value = dict(schema=1, kind='reviewed_merge_metadata_union', dry_run=not execute,
        head=git(root, 'rev-parse', 'HEAD').decode().strip(), incoming=incoming,
        needs_census_refresh=REGISTER in conflicts,
        files=[dict(path=p['path'],parent_sha256=p['parent_sha256'],
                    before_sha256=hashlib.sha256(p['before']).hexdigest(),
                    after_sha256=hashlib.sha256(p['after']).hexdigest()) for p in planned],
        reason=reason, scientific_validation_performed=False)
    if not execute:
        return value
    if not reason.strip() or report is None:
        raise ValueError('Applying metadata needs a review reason and new SQLite-backed report path')
    report = managed_report(root, report)
    for p in planned:
        if (root/p['path']).read_bytes() != p['before']:
            raise ValueError('Merge input changed during metadata review')
    for p in planned:
        atomic_replace(root/p['path'], p['after'], p['before'])
    value['report_sha256'] = write_report(root, report, (json.dumps(value, indent=2)+'\n').encode())
    if planned:
        subprocess.run(['git', '-C', str(root), 'add', '--', *(p['path'] for p in planned)], check=True)
    return value


def review_history(root, base, tips, *, record=False, reason='', database=None, report=None,
                   behaviors=(), pending_gaps=()):
    root = Path(root).resolve()
    if record:
        if not reason.strip() or database is None or not behaviors:
            raise ValueError('Recording needs a concrete reason, affected behavior IDs and dedicated review database')
        report = managed_report(root, report)
        known = {b['id'] for b in json.loads((root/REGISTER).read_bytes())['behaviors']}
        if set(behaviors)-known:
            raise ValueError('Unknown affected behavior IDs: '+str(sorted(set(behaviors)-known)))
    base = git(root, 'rev-parse', '--verify', '--end-of-options', base+'^{commit}').decode().strip()
    tips = [git(root, 'rev-parse', '--verify', '--end-of-options', t+'^{commit}').decode().strip() for t in tips]
    commits = git(root, 'rev-list', '--reverse', *tips, '^'+base).decode().splitlines()
    journal = load_journal(root)[0]
    receipts = {(f['path'],f.get('before_sha256'),f.get('after_sha256'))
                for r in journal['reviews'] for f in r.get('files', [])}
    process = subprocess.Popen(['git', '-C', str(root), 'cat-file', '--batch'],
                               stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    cache = {}; refs = {}; pointer_blobs = set(); rows = []; reviews = []
    def file_hash(oid, name):
        if set(oid) == {'0'}:
            return None
        if oid not in cache:
            process.stdin.write(oid.encode()+b'\n');process.stdin.flush()
            header = process.stdout.readline().split()
            if len(header) != 3 or header[1] != b'blob':
                raise ValueError('Missing historical Git blob')
            body = process.stdout.read(int(header[2]))
            if len(body) != int(header[2]) or process.stdout.read(1) != b'\n':
                raise ValueError('Truncated historical Git blob')
            if body.startswith(MAGIC) and len(body) <= 160:
                pointer_blobs.add(oid)
                sha, size = identity(body)
                if refs.setdefault(sha, size) != size:
                    raise ValueError('Conflicting historical pointer sizes')
            else:
                sha = hashlib.sha256(body).hexdigest()
            cache[oid] = sha
        if oid in pointer_blobs and not artifact(name):
            raise ValueError('Evidence pointer outside report storage policy')
        return cache[oid]
    try:
        with local_evidence(root), Reader(root) as reader:
            for commit in commits:
                parent = git(root, 'rev-parse', commit+'^1').decode().strip()
                raw = git(root, 'diff-tree', '--no-commit-id', '--raw', '--no-abbrev', '--no-renames', '-r', '-z', parent, commit).split(b'\0')
                missing = []; count = 0
                for n in range(0, len(raw)-1, 2):
                    fields = raw[n].split();name = raw[n+1].decode()
                    if name.startswith('original/'):
                        raise ValueError('Original-file transition requires separate review')
                    before, after = (file_hash(o.decode(), name) for o in fields[2:4]);count += 1
                    if (name,before,after) in receipts:
                        continue
                    for sha in (before, after):
                        if record and sha in refs:
                            reader.resolver.copy(sha, refs[sha], Sink())
                    missing.append(dict(path=name,before_sha256=before,after_sha256=after))
                if missing:
                    rid = hashlib.sha256(encoded(dict(commit=commit, files=missing))).hexdigest()
                    review = dict(id='retrospective-integration-'+rid,files=missing,
                        behaviors=[],affected_behavior_ids=list(dict.fromkeys(behaviors)),
                        pending_gaps=list(pending_gaps),
                        reason=reason+' Exact committed transition '+parent+'..'+commit+'; no past gate pass inferred.',
                        validation=dict(status='pending',reason='Retrospective custody only; no new execution or status promotion.',evidence=[]))
                    reviews.append(review)
                    receipts.update((f['path'],f['before_sha256'],f['after_sha256']) for f in missing)
                rows.append(dict(commit=commit,parent=parent,transitions=count,missing_files=missing))
    finally:
        process.stdin.close();process.stdout.close();process.wait()
    result = dict(schema=1,kind='integration_committed_history_review',dry_run=not record,
                  base=base,tips=tips,rows=rows,transitions=sum(r['transitions'] for r in rows),
                  missing_receipt_count=sum(len(r['missing_files']) for r in rows),
                  scope='Existing exact pairs retain their original pinned payload hashes. Previously unreceipted pointer payloads are freshly verified before recording. No historical code executes.')
    if record:
        workspace = Workspace(root, database)
        try:
            with workspace.locked():
                existing = {r['id']:r for r in workspace.sync()['reviews']}
                for review in reviews:
                    if review['id'] not in existing:
                        workspace.insert(review)
                    elif existing[review['id']] != review:
                        raise ValueError('Historical receipt ID reused with different content')
            with workspace.locked():
                result['exported_receipts'] = workspace.export()
        finally:
            workspace.close()
        result['report_sha256'] = write_report(root, report, (json.dumps(result, indent=2)+'\n').encode())
    return result
