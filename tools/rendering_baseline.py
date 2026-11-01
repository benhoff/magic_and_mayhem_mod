"""Revision-bound rendering inventory. Static review never grants engine proof."""
from collections import Counter
from contextlib import contextmanager
from functools import lru_cache
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tarfile
import tempfile

from coverage_audit import audit, pointer
from coverage_evidence import (Reader, cache_root, extra_pending_paths, pending_path,
                               write_report)
from evidence_local import local_evidence
from rendering_frontier import frontier

REGISTER = 'research/runtime/coverage/register.json'
PLAN = 'docs/native-rendering-closure.json'
DECISIONS = 'docs/native-rendering-dependencies.json'


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args])


@contextmanager
def revision_tree(root, revision):
    """Export data only; never import or execute the selected revision's code."""
    root = Path(root).resolve()
    commit = git(root, 'rev-parse', '--verify', '--end-of-options',
                 revision + '^{commit}').decode().strip()
    tree = git(root, 'rev-parse', commit + '^{tree}').decode().strip()
    configured = list(dict.fromkeys([pending_path(root), *extra_pending_paths(root)]))
    available = [p for p in configured if p.is_file()]
    unavailable = [str(p) for p in configured if not p.is_file()]
    previous = {k: os.environ.get(k) for k in
                ('MM_EVIDENCE_DB', 'MM_EVIDENCE_CACHE', 'MM_EVIDENCE_EXTRA_DB')}
    try:
        os.environ['MM_EVIDENCE_CACHE'] = str(cache_root(root))
        os.environ['MM_EVIDENCE_EXTRA_DB'] = json.dumps([str(p) for p in available])
        if previous['MM_EVIDENCE_DB'] is None and pending_path(root).is_file():
            os.environ['MM_EVIDENCE_DB'] = str(pending_path(root))
        with tempfile.TemporaryDirectory(prefix='mm-rendering-baseline-') as temporary:
            target = Path(temporary)
            with tempfile.TemporaryFile() as errors:
                process = subprocess.Popen(['git', '-C', str(root), 'archive', '--format=tar', commit],
                                           stdout=subprocess.PIPE, stderr=errors)
                try:
                    with tarfile.open(fileobj=process.stdout, mode='r|') as archive:
                        for member in archive:
                            path = Path(member.name)
                            if path.is_absolute() or '..' in path.parts or not path.parts:
                                raise ValueError('Unsafe revision archive path')
                            if path.parts[0] in ('original', 'working', '.git'):
                                continue
                            if member.isdir():
                                continue
                            if not member.isfile():
                                raise ValueError('Unsupported revision archive member: ' + member.name)
                            destination = target/path
                            destination.parent.mkdir(parents=True, exist_ok=True)
                            with destination.open('xb') as out:
                                shutil.copyfileobj(archive.extractfile(member), out, 1024 * 1024)
                    process.stdout.close()
                    if process.wait():
                        errors.seek(0)
                        raise ValueError('Git archive failed: ' + errors.read().decode(errors='replace'))
                finally:
                    process.stdout.close()
                    if process.poll() is None:
                        process.terminate()
                    process.wait()
            # This rejects unavailable/unverified published archives before a
            # reader could fetch. Missing auxiliary stores are recorded; every
            # payload actually used must still resolve and pass its own hash.
            with local_evidence(target):
                yield target, dict(commit=commit, tree=tree,
                    tracked_revision_only=True, working_and_staged_changes_excluded=True,
                    configured_local_stores=len(configured),
                    available_local_stores=len(available), unavailable_local_stores=unavailable)
    finally:
        for key, value in previous.items():
            if value is None:
                os.environ.pop(key, None)
            else:
                os.environ[key] = value


def summarize(register, reviewed, plan, decisions):
    """Keep source status, proof and authored requirements independently visible."""
    rows = reviewed['behaviors']
    selected = {b['id'] for b in rows}
    evidence_ids = {eid for b in register['behaviors'] if b['id'] in selected for eid in b['evidence']}
    stale = {r['id']: r['changed_sources'] for r in reviewed.get('stale_evidence', [])}
    evidence = [dict(id=e['id'], kind=e['kind'], record=e['record'], sha256=e['sha256'],
                     scope=e['scope'], source_binding_state=(
                         'unknown' if not reviewed['audit_valid'] else
                         'changed' if e['id'] in stale else 'matching'),
                     changed_sources=stale.get(e['id'], []))
                for e in register['evidence'] if e['id'] in evidence_ids]
    groups = {g['id']: g for g in plan['contracts']}
    requirements = []
    for kind, entries in [('journey', plan['journeys']), ('operation', plan['operations'])]:
        for entry in entries:
            linked = list(dict.fromkeys(bid for cid in entry['contracts']
                                        for bid in groups[cid]['behaviors']))
            conditional = list(dict.fromkeys(bid for cid in entry.get('conditional_contracts', [])
                                             for bid in groups[cid]['behaviors']))
            requirements.append(dict(id=entry['id'], kind=kind, authored_requirement=entry,
                behaviors=linked, conditional_behaviors=conditional,
                behaviors_outside_rendering_selection=[b for b in linked + conditional if b not in selected],
                acceptance_assessed=False,
                limitation='Authored dependency map only; component proof cannot close this requirement.'))
    return dict(
        registered_status_counts={stage: dict(Counter(b['status'][stage] for b in rows))
                                  for stage in ('understanding', 'implementation', 'comparison', 'integration', 'replacement')},
        evidence=evidence, requirements=requirements,
        source_role_decisions=decisions['groups'],
        authored_blockers=plan['blockers'], authored_work_packages=plan['work_packages'],
        authored_plan_reviewed_commit=plan['reviewed_commit'],
        authored_plan_states_reassessed=False,
        complete_player_accepted=False, original_recovery_complete=False,
        overall_completion_percent=None)


def build(root, revision):
    root = Path(root).resolve()
    with revision_tree(root, revision) as (snapshot, subject), Reader(snapshot) as reader:
        register = reader.json(REGISTER)
        plan, decisions = reader.json(PLAN), reader.json(DECISIONS)
        suite_path = plan['scope']['journey_catalog']
        suite = reader.json(suite_path)
        if {r['id']: r['requirements'] for r in plan['journeys']} != suite['requirements']:
            raise ValueError('Closure map changes the journey denominator')
        if [r['id'] for r in plan['operations']] != suite['operation_requirements']:
            raise ValueError('Closure map changes the operation denominator')
        audited = audit(snapshot, register)
        by_id = {e['id']: e for e in register['evidence']}

        @lru_cache(maxsize=128)
        def claims(eid):
            entry = by_id[eid]
            record = reader.json(entry['record'])
            sources = pointer(record, entry['sources_pointer'])
            if isinstance(sources, list):
                sources = {s['path']: s['sha256'] for s in sources}
            return pointer(record, entry['claims_pointer']), sources

        reviewed = frontier(register, audited, claims)
        selected = {r['id'] for r in reviewed['behaviors']}
        selected_evidence = {e for b in register['behaviors']
                             if b['id'] in selected for e in b['evidence']}
        reviewed['stale_evidence'] = [r for r in audited['findings']['stale_evidence']
                                     if r['id'] in selected_evidence]
        summary = summarize(register, reviewed, plan, decisions)
        inputs = [REGISTER, PLAN, DECISIONS, suite_path, register['code_index']['path'],
                  'docs/live-drawing-replacement.md', 'docs/native-player-launch.md',
                  'docs/native-rendering-closure.md',
                  'research/runtime/native-adapter-master-20261027.md',
                  'research/runtime/player-open-items-20261027.md',
                  'research/runtime/rendering-remaining-20261026.md',
                  'research/runtime/palette-effect-closure-20261026.md',
                  'research/runtime/scene-camera-frame-integration-20261026.md']
        report = dict(schema=1, kind='reviewed_rendering_baseline', subject=subject,
            success=reviewed['audit_valid'], static_review_only=True,
            scope='Registered rendering inventory at the exact Git revision. No new game, GL, original comparison, performance or live replacement execution. Counts overlap and do not estimate remaining effort.',
            original_media_consumed=False, engine_execution_performed=False,
            status_promotions=[], baseline_scope=plan['scope'], completion_contract=plan['completion'],
            inputs={p: reader.digest(p) for p in inputs},
            audit=dict(valid=reviewed['audit_valid'], errors=audited['errors'], warnings=audited['warnings'],
                       repository_counts=audited.get('counts', {}),
                       source_census_findings={k: v for k, v in audited['findings'].items()
                                               if k.endswith(('sources', 'index', 'links', 'registers'))}),
            rendering=reviewed, **summary)
    source_paths = ['tools/rendering_baseline.py', 'tools/report-rendering-baseline.py',
                    'tools/rendering_frontier.py', 'tools/coverage_audit.py', 'tools/coverage_claims.py',
                    'tools/coverage_evidence.py', 'tools/evidence_local.py']
    report['producer_sources'] = {p: hashlib.sha256((root/p).read_bytes()).hexdigest() for p in source_paths}
    return report


def write(root, output, report):
    root, output = Path(root).resolve(), Path(output)
    output = output if output.is_absolute() else root/output
    if output.exists() or not output.resolve().is_relative_to(root/'research/runtime') or output.suffix != '.json':
        raise ValueError('Baseline requires a new .json reference under research/runtime')
    return write_report(root, output, (json.dumps(report, indent=2)+'\n').encode())
