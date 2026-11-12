"""Open the rendering acceptance dossier and retain verified candidate evidence.

This initial record has no admitted current player acceptance receipts. Candidate
attachments and source/guard reviews cannot certify a requirement or either claim.
Collector-specific positive admission belongs with the real EP03 pilot.
"""
import argparse
from collections import Counter
from datetime import datetime, timezone
import json
from pathlib import Path
import platform
import sqlite3

from coverage_audit import pointer, source_hashes
from coverage_claims import fingerprint
from coverage_evidence import Reader
from evidence_local import backup_sqlite, local_evidence
from rendering_completion import PLAN, require
from rendering_execution import review as review_execution

POLICY = {'attachments_are_candidates', 'payloads_and_assertions_verified',
          'source_freshness_separate_from_scope_adequacy', 'historical_failures_not_current_regressions',
          'no_pass_from_static_review', 'no_pass_from_attachment', 'no_task_completion_from_guard_pass',
          'independent_player_and_recovery_decisions', 'required_unknowns_remain_visible',
          'immutable_events_and_snapshots', 'new_evidence_id_for_new_execution', 'sqlite_backups_required'}


def encoded(value):
    return json.dumps(value, sort_keys=True, separators=(',', ':'), allow_nan=False)


def validate(plan):
    spec = plan['acceptance_record']
    require(spec['schema'] == 1 and spec['revision'] == 1
            and spec['id'] == 'native-rendering-acceptance-v1', 'Unsupported acceptance record')
    require(spec['mode'] == 'opening_pending_record', 'Opening record cannot certify acceptance')
    for name in ('completion_contract', 'required_gap_register', 'execution_plan'):
        subject = plan[name]
        require(spec['bindings'][name] == dict(id=subject['id'], revision=subject['revision'],
                                             sha256=fingerprint(subject)), 'Acceptance input binding differs: ' + name)
    require(spec['baseline'] == plan['completion_contract']['baseline']
            and spec['reviewed_source_commit'] == plan['completion_contract']['reviewed_source_commit'],
            'Acceptance baseline differs')
    require(set(spec['evidence_policy']) == POLICY and all(v is True for v in spec['evidence_policy'].values()),
            'Acceptance evidence rule weakened')
    require(spec['decision_ids'] == ['supported-player', 'original-recovery'], 'Closure decisions must remain separate')
    require(spec['workspace'] == 'working/coverage/native-rendering-acceptance.sqlite3', 'Unsafe acceptance workspace')
    admission = spec['positive_execution_admission']
    require(admission['state'] == 'not_yet_established' and admission['owner_task'] == 'EP03'
            and admission['condition'].strip(), 'Positive acceptance requires the real pilot admission')
    first = spec['first_work']
    require(first['task'] == plan['execution_plan']['first_task'] == 'EP01' and first['complete'] is False
            and first['activity'].strip() and first['remaining'] and all(v.strip() for v in first['remaining']),
            'Source review cannot close execution-input work')
    record = Path(spec['opening_record'])
    require(not record.is_absolute() and '..' not in record.parts
            and record.as_posix().startswith('research/runtime/') and record.suffix == '.json',
            'Unsafe acceptance snapshot path')
    return spec


def requirements(plan):
    """One row per configuration/leaf, operation and completion gate; no invented percentage."""
    gaps = plan['required_gap_register']
    owners = {brief: task['id'] for task in plan['execution_plan']['tasks'] for brief in task['closing_briefs']}
    definitions = []
    for journey in gaps['journeys']:
        for leaf in journey['subrequirements']:
            definitions.append(dict(id='journey.' + journey['id'] + '.' + leaf['id'], kind='journey',
                journey=journey['id'], requirement=leaf['id'], closing_brief=leaf['closing_task'],
                pending_reason=leaf['remaining'], candidate_evidence=journey['evidence_leads']))
    for kind, rows in [('operation', gaps['operations']), ('gate', gaps['gates'])]:
        for row in rows:
            definitions.append(dict(id=kind + '.' + row['id'], kind=kind, requirement=row['id'],
                closing_brief=row['closing_task'], pending_reason=row['remaining'],
                candidate_evidence=row.get('evidence_leads', [])))
    require(len(definitions) == 78 and len({d['id'] for d in definitions}) == 78,
            'Acceptance requirement denominator changed')
    return [dict(row, configuration=config['id'], owner_task=owners[row['closing_brief']])
            for config in plan['completion_contract']['configurations'] for row in definitions]


def verify_evidence(root, reader, register, eid):
    entries = {e['id']: e for e in register['evidence']}
    require(eid in entries, 'Unknown registered evidence: ' + eid)
    ev = entries[eid]
    require(reader.digest(ev['record']) == ev['sha256'], 'Candidate payload hash differs: ' + eid)
    payload = reader.json(ev['record'])
    require(all(pointer(payload, a['pointer']) == a['equals'] for a in ev['assertions']),
            'Candidate evidence assertion failed: ' + eid)
    sources = source_hashes(pointer(payload, ev['sources_pointer']))
    bindings = ev.get('source_bindings', {p: p for p in sources})
    changed = []
    for recorded, name in bindings.items():
        if recorded in ev.get('recorded_inputs', []):
            continue
        path = Path(name)
        require(not path.is_absolute() and '..' not in path.parts and path.parts
                and path.parts[0] not in ('original', 'working', '.git'), 'Unsafe candidate source binding')
        if not (root/path).is_file() or reader.digest(name) != sources[recorded]:
            changed.append(name)
    return dict(id=eid, record=ev['record'], sha256=ev['sha256'], kind=ev['kind'], scope=ev['scope'],
                payload_and_assertions_verified=True,
                current_source_binding_state='changed' if changed else 'matching_recorded_sources',
                changed_sources=sorted(changed), closes_requirement=False)


class Workspace:
    """Append-only candidate/work ledger. There is deliberately no set-pass operation."""
    def __init__(self, root, plan):
        self.root, self.spec = Path(root).resolve(), validate(plan)
        self.rows = requirements(plan)
        self.path = self.root/self.spec['workspace']
        self.path.parent.mkdir(parents=True, exist_ok=True)
        (self.path.parent/'.retain').touch(exist_ok=True)
        self.db = sqlite3.connect(self.path, timeout=60)
        self.db.executescript('''
            PRAGMA foreign_keys=ON;
            CREATE TABLE IF NOT EXISTS dossiers (id TEXT PRIMARY KEY, definition TEXT NOT NULL);
            CREATE TABLE IF NOT EXISTS events (
                id TEXT PRIMARY KEY, dossier TEXT NOT NULL REFERENCES dossiers(id),
                kind TEXT NOT NULL, payload TEXT NOT NULL, observed_at TEXT NOT NULL);
            CREATE TRIGGER IF NOT EXISTS dossiers_no_update BEFORE UPDATE ON dossiers
                BEGIN SELECT RAISE(ABORT,'Immutable acceptance dossier'); END;
            CREATE TRIGGER IF NOT EXISTS dossiers_no_delete BEFORE DELETE ON dossiers
                BEGIN SELECT RAISE(ABORT,'Immutable acceptance dossier'); END;
            CREATE TRIGGER IF NOT EXISTS events_no_update BEFORE UPDATE ON events
                BEGIN SELECT RAISE(ABORT,'Immutable acceptance event'); END;
            CREATE TRIGGER IF NOT EXISTS events_no_delete BEFORE DELETE ON events
                BEGIN SELECT RAISE(ABORT,'Immutable acceptance event'); END;
        ''')
        definition = dict(spec=self.spec, requirements=self.rows)
        self.identity = fingerprint(definition)
        with self.db:
            self.db.execute('INSERT OR IGNORE INTO dossiers VALUES (?,?)', (self.identity, encoded(definition)))
        require(self.db.execute('SELECT definition FROM dossiers WHERE id=?', (self.identity,)).fetchone()
                == (encoded(definition),), 'Acceptance dossier definition differs')

    def close(self):
        self.db.close()

    def append(self, kind, payload):
        require(kind in ('source_applicability_review', 'candidate_attachment'), 'Unsupported acceptance event')
        if kind == 'candidate_attachment':
            require((payload['configuration'], payload['requirement']) in
                    {(r['configuration'], r['id']) for r in self.rows}, 'Unknown configuration/requirement')
            require(isinstance(payload['reason'], str) and payload['reason'].strip(), 'Candidate requires a review reason')
            require(set(payload) == {'configuration', 'requirement', 'evidence', 'reason'}
                    and set(payload['evidence']) == {'id', 'record', 'sha256'}, 'Candidate cannot carry acceptance status')
        else:
            require(set(payload) == {'task', 'sources', 'evidence_identities', 'limits'}
                    and payload['task'] == 'EP01' and payload['sources'] and payload['evidence_identities']
                    and payload['limits'].strip(), 'Invalid source-applicability work observation')
        value = dict(dossier=self.identity, kind=kind, payload=payload)
        key = fingerprint(value)
        with self.db:
            self.db.execute('INSERT OR IGNORE INTO events VALUES (?,?,?,?,?)',
                (key, self.identity, kind, encoded(payload), datetime.now(timezone.utc).isoformat()))
        return key

    def events(self):
        result = []
        for key, kind, payload, observed_at in self.db.execute(
                'SELECT id,kind,payload,observed_at FROM events WHERE dossier=? ORDER BY rowid', (self.identity,)):
            value = json.loads(payload)
            require(key == fingerprint(dict(dossier=self.identity, kind=kind, payload=value))
                    and kind in ('source_applicability_review', 'candidate_attachment'), 'Acceptance event identity differs')
            # Recheck row semantics without inserting or changing timestamps.
            if kind == 'candidate_attachment':
                require(set(value) == {'configuration', 'requirement', 'evidence', 'reason'}
                        and (value['configuration'], value['requirement']) in
                        {(r['configuration'], r['id']) for r in self.rows}, 'Invalid retained candidate event')
            result.append(dict(id=key, kind=kind, payload=value, observed_at=observed_at))
        return result

    def backup(self):
        return backup_sqlite(self.path, self.path.parent/'acceptance-backups')

    def restore_snapshot(self, snapshot):
        """Restore exported events without changing identities, dates or verdicts."""
        require(snapshot['kind'] == 'native_rendering_acceptance_record' and snapshot['record_valid'] is True
                and snapshot['dossier_identity'] == self.identity, 'Snapshot belongs to a different acceptance dossier')
        require(all(d['state'] == 'pending' and d['accepted'] is False for d in snapshot['decisions'])
                and all(r['state'] == 'pending' and r['admitted_execution_evidence'] == []
                        for r in snapshot['requirements']), 'Opening workspace cannot restore accepted outcomes')
        expected = {(r['configuration'], r['id']) for r in self.rows}
        require(len(snapshot['requirements']) == len(self.rows)
                and {(r['configuration'], r['id']) for r in snapshot['requirements']} == expected,
                'Snapshot requirement denominator differs')
        prepared = []
        for event in snapshot['events']:
            key, kind, value = event['id'], event['kind'], event['payload']
            require(key == fingerprint(dict(dossier=self.identity, kind=kind, payload=value))
                    and kind in ('source_applicability_review', 'candidate_attachment')
                    and isinstance(event['observed_at'], str) and event['observed_at'], 'Snapshot event identity differs')
            prepared.append((key, self.identity, kind, encoded(value), event['observed_at']))
        with self.db:
            self.db.executemany('INSERT OR IGNORE INTO events VALUES (?,?,?,?,?)', prepared)
            self.events()
        return len(prepared)


def assemble(plan, prerequisite, workspace, attached, host):
    """Derive an opening record from verified leads, never from an authored pass bit."""
    spec = validate(plan)
    events = workspace.events()
    rows = []
    for definition in workspace.rows:
        candidates = [e['payload']['evidence']['id'] for e in events if e['kind'] == 'candidate_attachment'
                      and e['payload']['configuration'] == definition['configuration']
                      and e['payload']['requirement'] == definition['id']]
        rows.append(dict(definition, state='pending', admitted_execution_evidence=[],
                         attached_candidates=sorted(set(candidates))))
    reviewed = any(e['kind'] == 'source_applicability_review' for e in events)
    lead_counts = Counter(e['current_source_binding_state'] for e in prerequisite['evidence_applicability_seed'])
    scopes = [dict(id=d['id'], state='unresolved', blocking_condition=d['blocking_condition'],
                   close_when=d['close_when'], resolution_evidence=[], owner_task='EP07')
              for d in plan['completion_contract']['scope_decisions']]
    if 'scope_review' in prerequisite:
        review = prerequisite['scope_review']
        effective = {d['id']: d for d in review['decisions']}
        for row in scopes:
            result = effective[row['id']]
            row.update(state=result['player_scope_state'], original_recovery_state=result['original_recovery_state'],
                       question_state=result['question_state'], engineering_answer=result['answer'],
                       technical_remaining=result['technical_remaining'],
                       user_judgment_required_now=result['user_judgment_required_now'],
                       resolution_evidence=[review['evidence']] if result['player_scope_state']=='resolved' else [],
                       scope_review=dict(record=review['record'], sha256=review['sha256']))
    recovery = [dict(id=c['id'], title=c['title'], behaviors=c['behaviors'], state='pending',
                     established_scope=c['established'], remaining=c['remaining'], recovery_exit=c['recovery_exit'])
                for c in plan['contracts']]
    decisions = [dict(id='supported-player', state='pending', accepted=False,
        reason='No complete current acceptance receipt has been admitted for either target configuration. '
               'Source/guard reviews and candidate attachments do not close acceptance.',
        required_configurations=[c['id'] for c in plan['completion_contract']['configurations']],
        pending_requirement_rows=len(rows), unresolved_scope_decisions=sum(s['state']=='unresolved' for s in scopes)),
        dict(id='original-recovery', state='pending', accepted=False,
        reason='Complete original-rendering recovery is not established by the reviewed baseline. '
               'Permitted original suppliers and a bounded supported-player claim do not recover all required '
               'branches, lifetimes, dispatch or platform behavior. Scope resolution alone is insufficient.',
        pending_contract_groups=len(recovery), evidence=spec['baseline'])]
    return dict(schema=1, kind='native_rendering_acceptance_record', success=True, record_valid=True,
        contract=plan['completion_contract']['id'], acceptance_record=spec['id'],
        bindings=spec['bindings'], baseline=spec['baseline'], reviewed_source_commit=spec['reviewed_source_commit'],
        record_base_commit=spec['record_base_commit'], dossier_identity=workspace.identity,
        counts=dict(configurations=len(plan['completion_contract']['configurations']),
            journey_requirements_per_configuration=47, operations_per_configuration=21, gates_per_configuration=10,
            requirement_rows=len(rows), pending_rows=len(rows), passed_rows=0, failed_current_rows=0,
            unresolved_scope_decisions=sum(s['state']=='unresolved' for s in scopes),
            unresolved_original_recovery_scope_decisions=sum(s.get('original_recovery_state','unresolved')=='unresolved' for s in scopes),
            original_recovery_groups=len(recovery),
            candidate_evidence_leads=len(prerequisite['evidence_applicability_seed']),
            lead_source_binding_states=dict(lead_counts),
            candidate_attachment_events=sum(e['kind'] == 'candidate_attachment' for e in events)),
        decisions=decisions, requirements=rows, scope_decisions=scopes, original_recovery=recovery,
        evidence_leads=prerequisite['evidence_applicability_seed'], attached_candidates=attached,
        historical_observations=plan['required_gap_register']['observations'],
        tasks=[dict(id=t['id'], state='in_progress' if t['id'] == 'EP01' and reviewed else 'not_started',
                    completed=False, admitted_execution_evidence=[]) for t in plan['execution_plan']['tasks']],
        ep01_review=dict(source_and_lead_review_performed=reviewed, complete=False,
            host_inventory=host, actual_player_build_identity='pending', actual_original_assets_save_identity='pending',
            actual_provider_display_media_execution_identity='pending', pilot_inputs_and_budgets='pending',
            per_requirement_evidence_adequacy='pending', remaining=spec['first_work']['remaining']),
        events=events, workspace=str(workspace.path.relative_to(workspace.root)),
        next_action=plan['execution_plan']['first_action'], positive_execution_admission=spec['positive_execution_admission'],
        engine_execution_performed=False, original_media_consumed=False, original_code_executed=False,
        engine_acceptance_assessed=False, native_equivalence=False, complete_replacement=False,
        supported_player_accepted=False, original_recovery_complete=False, status_promotions=[],
        scope='Opening acceptance dossier and EP01 source/lead-applicability review only. Every obligation remains pending. '
              'Candidate/source/host records and guards are not current player execution, requirement acceptance, '
              'scope resolution or task completion. Counts are evidence obligations, not renderer progress percentages.')


def review(root):
    root = Path(root).resolve()
    prerequisite = review_execution(root)
    with Reader(root) as reader:
        plan = reader.json(PLAN)
        workspace = Workspace(root, plan)
        try:
            leads = prerequisite['evidence_applicability_seed']
            workspace.append('source_applicability_review', dict(task='EP01', sources=prerequisite['sources'],
                evidence_identities=[dict(id=e['id'], sha256=e['sha256']) for e in leads],
                limits='Verified authored inputs, product/policy/tooling witnesses and recorded evidence sources only. '
                       'Actual as-built player, original/assets/save and provider execution identities and '
                       'per-requirement scope adequacy remain pending; no original media or game execution.'))
            register = reader.json('research/runtime/coverage/register.json')
            attached = []
            for event in workspace.events():
                if event['kind'] != 'candidate_attachment':
                    continue
                value = event['payload']
                evidence = verify_evidence(root, reader, register, value['evidence']['id'])
                require(value['evidence'] == {k: evidence[k] for k in ('id', 'record', 'sha256')},
                        'Retained candidate identity changed')
                attached.append(dict(event=event['id'], configuration=value['configuration'],
                                     requirement=value['requirement'], reason=value['reason'], evidence=evidence))
            host = dict(system=platform.system(), release=platform.release(), machine=platform.machine(),
                        python=platform.python_version(), scope='Host inventory only; no actual GL/display/provider admission.')
            report = assemble(plan, prerequisite, workspace, attached, host)
            sources = dict(prerequisite['sources'])
            for name in ('tools/rendering_acceptance.py', 'tests/test-rendering-acceptance.py',
                         'docs/native-rendering-acceptance.md', 'research/runtime/rendering-acceptance-record-20261030.md'):
                sources[name] = reader.digest(name)
            report.update(sources=sources, prerequisite_review_passed=prerequisite['success'],
                          workspace_backup=workspace.backup())
            return report
        finally:
            workspace.close()


def main():
    parser = argparse.ArgumentParser(description='Attach verified evidence as a candidate; acceptance stays pending.')
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument('--evidence')
    mode.add_argument('--from-record', type=Path, help='Restore candidate/work events from a verified exported snapshot')
    parser.add_argument('--configuration')
    parser.add_argument('--requirement')
    parser.add_argument('--reason')
    args = parser.parse_args()
    root = args.root.resolve()
    with local_evidence(root), Reader(root) as reader:
        plan = reader.json(PLAN)
        workspace = Workspace(root, plan)
        try:
            register = reader.json('research/runtime/coverage/register.json')
            if args.from_record:
                name = args.from_record
                require(not name.is_absolute() and '..' not in name.parts
                        and name.as_posix().startswith('research/runtime/')
                        and not (root/name).is_symlink()
                        and (root/name).resolve().is_relative_to(root/'research/runtime'), 'Unsafe snapshot restore path')
                snapshot = reader.json(name)
                for event in snapshot['events']:
                    if event['kind'] == 'candidate_attachment':
                        identity = event['payload']['evidence']
                        actual = verify_evidence(root, reader, register, identity['id'])
                        require(identity == {k: actual[k] for k in ('id', 'record', 'sha256')},
                                'Restored candidate evidence identity differs')
                count = workspace.restore_snapshot(snapshot)
                print(json.dumps(dict(restored_events=count, acceptance='pending', backup=workspace.backup())))
                return
            require(args.configuration and args.requirement and args.reason,
                    'Evidence attachment requires configuration, requirement and reason')
            evidence = verify_evidence(root, reader, register, args.evidence)
            key = workspace.append('candidate_attachment', dict(configuration=args.configuration,
                requirement=args.requirement, evidence={k: evidence[k] for k in ('id', 'record', 'sha256')}, reason=args.reason))
            backup = workspace.backup()
            print(json.dumps(dict(event=key, state='candidate_only', acceptance='pending', backup=backup)))
        finally:
            workspace.close()


if __name__ == '__main__':
    main()
