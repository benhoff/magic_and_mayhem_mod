#!/usr/bin/env python3
"""Planning weights cannot grant implementation or validation credit."""
import copy
import hashlib
import ast
import json
from pathlib import Path
import sys
import shlex
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from coverage_evidence import read_text as evidence_text
from coverage_claims import behavior_contract
from coverage_importance import importance_view
from coverage_ui import track_view


class ImportanceTest(unittest.TestCase):
    def setUp(self):
        self.behavior = {'id': 'one', 'kind': 'native_policy', 'scope': 'bounded fixture',
                         'original': [], 'implementation': [], 'tests': [],
                         'status': {'implementation': 'partial'}}
        self.register = {'builds': [], 'behaviors': [self.behavior]}
        self.tracks = [{'id': 'rendering', 'behaviors': ['one']}]
        self.catalog = {'schema': 1, 'track': 'rendering', 'purpose': 'Planning',
                        'entries': [{'behavior': 'one', 'weight': 5, 'work_type': 'capability',
                                     'reason': 'Source ownership protects correctness.',
                                     'contract_sha256': behavior_contract(self.behavior, {})}]}

    def test_importance_does_not_mutate_contract_or_status(self):
        before = copy.deepcopy((self.catalog, self.register, self.tracks))
        result = importance_view(self.catalog, self.register, self.tracks)
        self.assertEqual(result['entries'][0]['implementation'], 'partial')
        self.assertEqual(result['entries'][0]['review_state'], 'current')
        self.assertEqual(result['partial_counts'], {5: 1})
        self.assertEqual((self.catalog, self.register, self.tracks), before)
        self.assertNotIn('completion', result)

    def test_scope_changes_need_review_but_status_changes_do_not_change_importance(self):
        self.behavior['scope'] = 'wider domain'
        result = importance_view(self.catalog, self.register, self.tracks)
        self.assertEqual(result['entries'][0]['review_state'], 'needs_review')
        self.behavior['status']['implementation'] = 'scoped'
        result = importance_view(self.catalog, self.register, self.tracks)
        self.assertEqual(result['entries'][0]['weight'], 5)
        self.assertEqual(result['partial_counts'], {})

    def test_new_contracts_stay_unrated(self):
        self.tracks[0]['behaviors'].append('new-contract')
        self.assertEqual(importance_view(self.catalog, self.register, self.tracks)['unrated'], ['new-contract'])

    def test_weights_require_in_range_integer_and_reviewed_reason(self):
        for value in [0, 6, True, '5', 4.5]:
            with self.subTest(value=value):
                c = copy.deepcopy(self.catalog); c['entries'][0]['weight'] = value
                with self.assertRaises(ValueError): importance_view(c, self.register, self.tracks)
        for field, value in [('reason', ''), ('work_type', 'invented'), ('contract_sha256', 'bad')]:
            c = copy.deepcopy(self.catalog); c['entries'][0][field] = value
            with self.assertRaises(ValueError): importance_view(c, self.register, self.tracks)

    def test_duplicate_unknown_and_out_of_track_ids_refuse(self):
        c = copy.deepcopy(self.catalog); c['entries'].append(c['entries'][0])
        with self.assertRaises(ValueError): importance_view(c, self.register, self.tracks)
        c = copy.deepcopy(self.catalog); c['entries'][0]['behavior'] = 'unknown'
        with self.assertRaises(ValueError): importance_view(c, self.register, self.tracks)
        with self.assertRaises(ValueError): importance_view(self.catalog, self.register, [{'id': 'rendering', 'behaviors': []}])

    def test_sort_is_descending_with_stable_id_ties(self):
        for bid, weight in [('three', 1), ('two', 5)]:
            b = {**self.behavior, 'id': bid}; self.register['behaviors'].append(b)
            self.tracks[0]['behaviors'].append(bid)
            self.catalog['entries'].append({**self.catalog['entries'][0], 'behavior': bid,
                                            'weight': weight, 'contract_sha256': behavior_contract(b, {})})
        result = importance_view(self.catalog, self.register, self.tracks)
        self.assertEqual([e['behavior'] for e in result['entries']], ['one', 'two', 'three'])

    def test_real_catalog_covers_rendering_and_every_partial_without_promotions(self):
        register = json.loads(evidence_text(ROOT / 'research/runtime/coverage/register.json'))
        before = copy.deepcopy(register)
        definition = json.loads(evidence_text(ROOT / 'apps/coverage-ui/tracks.json'))
        tracks = track_view(definition, register['behaviors'], [{**e, 'freshness': 'historical'} for e in register['evidence']])
        catalog = json.loads(evidence_text(ROOT / 'apps/coverage-ui/importance.json'))
        result = importance_view(catalog, register, tracks)
        self.assertEqual(register, before)
        self.assertGreaterEqual(len(result['entries']), 345)
        self.assertEqual(result['unrated'], [
            'NP.alternate-canvas-dispatch-admission', 'NP.graphics-session-provider-retirement',
            'NP.palette-option-strict-parsing', 'NP.protected-native-store-service', 'NP.surface-alias-lease-lifecycle',
            'RS.alternate-canvas-dispatch-composition', 'RS.screen-cursor-background-invalidation',
            'RS.screen-cursor-background-restore', 'RS.screen-tick-control-flow',
            'RS.windowed-padded-presentation-route'])
        behaviors = {b['id']: b for b in register['behaviors']}
        builds = {b['id']: b for b in register['builds']}
        for entry in result['entries']:
            expected = 'current' if behavior_contract(behaviors[entry['behavior']], builds) == entry['contract_sha256'] else 'needs_review'
            self.assertEqual(entry['review_state'], expected)
        # Selected terrain closeouts retain comparison and acceptance independently.
        self.assertEqual(result['partial_counts'], {1: 1, 2: 1, 4: 4, 5: 4})
        by_id = {e['behavior']: e for e in result['entries']}
        self.assertEqual(by_id['AN.world-facing']['weight'], 4)
        self.assertEqual(by_id['AN.world-facing']['implementation'], 'scoped')
        self.assertEqual(by_id['RS.world-initial-state']['weight'], 5)
        self.assertEqual(by_id['NR.live-drawing-replacement']['work_type'], 'acceptance')
        self.assertEqual(by_id['NR.drawing-egl-delivery-observation']['weight'], 1)


class TrancheTest(unittest.TestCase):
    def setUp(self):
        self.register = json.loads(evidence_text(ROOT / 'research/runtime/coverage/register.json'))
        definition = json.loads(evidence_text(ROOT / 'apps/coverage-ui/tracks.json'))
        self.tracks = track_view(definition, self.register['behaviors'],
                                 [{**e, 'freshness': 'historical'} for e in self.register['evidence']])
        self.catalog = json.loads(evidence_text(ROOT / 'apps/coverage-ui/importance.json'))

    def test_low_batch_keeps_first20_order_and_independent_acceptance(self):
        before = copy.deepcopy(self.register)
        result = importance_view(self.catalog, self.register, self.tracks)
        tasks = result['low_tranche_plan']['tranches'][0]['tasks']
        native = {b['id'] for b in self.register['behaviors'] if b['subsystem']=='native_rendering'}
        expected = self.catalog['low_tranche_plan']['selection_cohort'][:20]
        self.assertEqual([t['behavior'] for t in tasks], expected)
        self.assertEqual(len(tasks),20)
        self.assertEqual({t['reviewed_implementation'] for t in tasks},{'scoped','partial'})
        for task in tasks:
            work = task['work_review']
            self.assertEqual(work['implementation'],'implemented_within_scope')
            self.assertFalse(work['implementation_gaps'])
            self.assertTrue(work['next_steps'])
            self.assertIn(work['acceptance'],{'pending','acceptance_failure'})
        self.assertEqual(self.register,before)

    def test_low_batch_retains_reviewed_cohort_when_new_contract_sorts_first(self):
        result = importance_view(self.catalog, self.register, self.tracks)
        plan = result['low_tranche_plan']
        self.assertEqual(plan['tranches'][0]['tasks'][0]['behavior'], 'NR.drawing-actions')
        self.assertIn('NP.world-colour-observation', plan['unplanned_low'])
        self.assertNotIn('NP.world-colour-observation', plan['selection_cohort'])
        for cohort in [[], ['NR.drawing-actions'] * 20, ['invented'] * 20]:
            catalog = copy.deepcopy(self.catalog)
            catalog['low_tranche_plan']['selection_cohort'] = cohort
            with self.subTest(cohort=cohort), self.assertRaises(ValueError):
                importance_view(catalog, self.register, self.tracks)

    def test_low_batch_cannot_skip_reorder_or_duplicate_a_contract(self):
        for change in ('reverse','duplicate','skip'):
            c = copy.deepcopy(self.catalog)
            tasks = c['low_tranche_plan']['tranches'][0]['tasks']
            if change=='reverse':tasks.reverse()
            elif change=='duplicate':tasks[-1]=copy.deepcopy(tasks[0])
            else:tasks.pop()
            with self.subTest(change=change),self.assertRaises(ValueError):
                importance_view(c,self.register,self.tracks)

    def test_three_tranches_cover_all_critical_partials_once_without_execution_credit(self):
        before = copy.deepcopy((self.catalog, self.register))
        result = importance_view(self.catalog, self.register, self.tracks)
        plan = result['tranche_plan']
        self.assertEqual(plan['state'], 'planned')
        self.assertEqual(plan['selected_count'], 33)
        self.assertEqual(plan['unplanned_critical'], ['RS.actor-downstream-services'])
        self.assertEqual([len(t['tasks']) for t in plan['tranches']], [11, 11, 11])
        tasks = [task for t in plan['tranches'] for task in t['tasks']]
        critical = {e['behavior'] for e in result['entries'] if e['weight'] == 5 and e['implementation'] == 'partial'}
        completed = {task['behavior'] for task in tasks if task['implementation'] == 'scoped'}
        self.assertEqual(completed, {'NR.drawing-storage-session-comparison', 'NR.recorded-hardware-session-parity',
                                    'NR.canvas-producer-replay', 'NR.world-startup-sequence', 'NR.world-producer-handoff',
                                    'NP.surface-final-word-read', 'NR.displacement-finite-edge',
                                    'NR.readonly-description-continuity', 'NR.world-source-region-epoch',
                                    'NR.resident-bind-admission', 'NR.resident-command-pressure',
                                    'NR.callback-publication-isolation', 'NR.recorded-physical-frame-admission',
                                    'RS.canvas-create-lock', 'RS.canvas-bind-clip', 'RS.canvas-startup-clear',
                                    'RS.world-startup-palette', 'RS.world-startup-wave',
                                    'NR.world-history-channel', 'NR.world-live-history',
                                    'NR.palette-resources', 'NR.command-checkpoint-attachment',
                                    'RS.world-initial-state', 'RS.canvas-preworld-reuse',
                                    'RS.canvas-producer-sequence', 'RS.world-producer-bypass',
                                    'RS.drawing-publication', 'NP.pixel-forwarding-guard',
                                    'NP.native-only-launch-contract', 'NP.native-output-ownership'})
        self.assertEqual({task['behavior'] for task in tasks} | set(plan['unplanned_critical']), critical | completed)
        self.assertTrue(all(t['implementation'] in {'partial', 'scoped'} for t in tasks))
        self.assertTrue(all(t['review_state'] in {'current','needs_review'} for t in tasks))
        for task in tasks:
            if task.get('work_review',{}).get('freshness') == 'historical':
                self.assertEqual(task['review_state'], 'needs_review')
        self.assertEqual((self.catalog, self.register), before)

    def test_invalid_sizes_duplicates_order_and_dependencies_refuse(self):
        for case in ['size', 'duplicate', 'order', 'dependency']:
            c = copy.deepcopy(self.catalog); batches = c['tranche_plan']['tranches']
            if case == 'size': batches[0]['tasks'].pop()
            if case == 'duplicate': batches[1]['tasks'][0] = copy.deepcopy(batches[0]['tasks'][0])
            if case == 'order': batches.reverse()
            if case == 'dependency': batches[0]['depends_on'] = [batches[2]['id']]
            with self.subTest(case=case), self.assertRaises(ValueError):
                importance_view(c, self.register, self.tracks)

    def test_missing_actions_and_unregistered_inputs_refuse(self):
        for field, value in [('next_action', ''), ('validation', ''), ('exit_condition', ''),
                             ('checks', ['tests/invented.py']), ('documents', ['research/invented.md'])]:
            c = copy.deepcopy(self.catalog); c['tranche_plan']['tranches'][0]['tasks'][0][field] = value
            with self.subTest(field=field), self.assertRaises(ValueError):
                importance_view(c, self.register, self.tracks)

    def test_scope_status_or_weight_changes_need_review_not_automatic_completion(self):
        task = self.catalog['tranche_plan']['tranches'][0]['tasks'][0]
        bid = task['behavior']
        for case in ['scope', 'status', 'weight']:
            r = copy.deepcopy(self.register); c = copy.deepcopy(self.catalog)
            b = next(b for b in r['behaviors'] if b['id'] == bid)
            if case == 'scope': b['scope'] += ' changed boundary'
            if case == 'status': b['status']['implementation'] = 'scoped'
            if case == 'weight': next(e for e in c['entries'] if e['behavior'] == bid)['weight'] = 4
            result = importance_view(c, r, self.tracks)['tranche_plan']
            with self.subTest(case=case):
                self.assertEqual(result['state'], 'planned')
                self.assertEqual(result['tranches'][0]['tasks'][0]['review_state'], 'needs_review')

    def test_new_critical_partial_is_unplanned_not_automatically_assigned(self):
        next(b for b in self.register['behaviors'] if b['id'] == 'AN.world-facing')['status']['implementation'] = 'partial'
        next(e for e in self.catalog['entries'] if e['behavior'] == 'AN.world-facing')['weight'] = 5
        result = importance_view(self.catalog, self.register, self.tracks)['tranche_plan']
        self.assertEqual(result['selected_count'], 33)
        self.assertEqual(result['unplanned_critical'], ['AN.world-facing', 'RS.actor-downstream-services'])

    def test_first_tranche_briefs_gate_one_pilot_after_ten_controls(self):
        result = importance_view(self.catalog, self.register, self.tracks)
        tranche = result['tranche_plan']['tranches'][0]
        self.assertIn('execution_review', tranche)
        self.assertEqual(tranche['tasks'][-1]['behavior'], 'NP.native-output-ownership')
        self.assertEqual(set(tranche['tasks'][-1]['execution_brief']['requires']),
                         {t['behavior'] for t in tranche['tasks'][:-1]})
        self.assertTrue(all(t['execution_brief']['acceptance'] and t['execution_brief']['runners']
                            for t in tranche['tasks']))

    def test_execution_brief_rejects_missing_inputs_and_forward_dependencies(self):
        for case in ['missing', 'inputs', 'acceptance', 'runner', 'binding', 'forward', 'duplicate']:
            c = copy.deepcopy(self.catalog); tasks = c['tranche_plan']['tranches'][0]['tasks']
            brief = tasks[0]['execution_brief']
            if case == 'missing': del tasks[0]['execution_brief']
            if case == 'inputs': brief['inputs'] = []
            if case == 'acceptance': brief['acceptance'] = []
            if case == 'runner': brief['runners'][0]['path'] = 'tools/not-registered.py'
            if case == 'binding': brief['runners'][0]['claims_binding'] = 'retrospective'
            if case == 'forward': brief['requires'] = [tasks[-1]['behavior']]
            if case == 'duplicate': tasks[1]['execution_brief']['requires'] *= 2
            with self.subTest(case=case), self.assertRaises(ValueError):
                importance_view(c, self.register, self.tracks)

    def test_python_entrypoint_templates_use_actual_declared_cli_flags(self):
        # Read source AST only: --help is unsafe for scripts without a CLI.
        reviewed = [t for t in self.catalog['tranche_plan']['tranches'] if 'execution_review' in t]
        for task in [task for tranche in reviewed for task in tranche['tasks']]:
            for runner in task['execution_brief']['runners']:
                if not runner['path'].endswith('.py'): continue
                tree = ast.parse(evidence_text(ROOT / runner['path']))
                flags = {arg.value for node in ast.walk(tree)
                         if isinstance(node, ast.Call) and isinstance(node.func, ast.Attribute)
                         and node.func.attr == 'add_argument'
                         for arg in node.args if isinstance(arg, ast.Constant)
                         and isinstance(arg.value, str) and arg.value.startswith('--')}
                args = shlex.split(runner['command'])
                arguments = args[args.index(runner['path'])+1:]
                declared = {arg for arg in arguments if arg.startswith('--')}
                with self.subTest(runner=runner['path']):
                    self.assertTrue(declared <= flags, declared - flags)
                    required = {node.args[0].value for node in ast.walk(tree)
                                if isinstance(node, ast.Call) and isinstance(node.func, ast.Attribute)
                                and node.func.attr == 'add_argument' and node.args
                                and isinstance(node.args[0], ast.Constant)
                                and isinstance(node.args[0].value, str)
                                and node.args[0].value.startswith('--')
                                and any(k.arg == 'required' and isinstance(k.value, ast.Constant)
                                        and k.value.value is True for k in node.keywords)}
                    self.assertTrue(required <= declared, required - declared)

    def test_second_tranche_separates_initial_history_zero_policy_and_caller_guard(self):
        before = copy.deepcopy(self.register)
        tranche = importance_view(self.catalog, self.register, self.tracks)['tranche_plan']['tranches'][1]
        self.assertIn('execution_review', tranche)
        tasks = {t['behavior']: t for t in tranche['tasks']}
        self.assertTrue(all(t.get('execution_brief') for t in tasks.values()))
        originals = {t['behavior']: t for t in self.catalog['tranche_plan']['tranches'][1]['tasks']}
        self.assertTrue(all(t.get('execution_result') == originals[bid].get('execution_result')
                            for bid, t in tasks.items()))
        self.assertEqual({t['execution_result']['outcome'] for t in tasks.values()}, {'PASS'})
        execution = json.loads(evidence_text(ROOT / tranche['execution_report']))
        self.assertTrue(execution['all_tranche_tasks_accepted'])
        self.assertEqual(execution['counts']['bounded_task_pass'], 11)
        self.assertFalse(execution['status_promotions'])
        self.assertEqual(len(execution['tasks']), 11)
        self.assertIn('No complete/default', tranche['exit_gate'])
        initial = tasks['RS.world-initial-state']['execution_brief']['requires']
        self.assertIn('RS.canvas-producer-sequence', initial)
        self.assertIn('RS.canvas-preworld-reuse', initial)
        self.assertEqual(tasks['NR.world-startup-sequence']['execution_brief']['requires'],
                         ['RS.world-startup-palette', 'RS.world-startup-wave'])
        self.assertIn('word zero', tasks['NR.world-startup-sequence']['implementation_boundary'])
        self.assertIn('raster_active', tasks['RS.world-producer-bypass']['implementation_boundary'])
        self.assertEqual(tranche['exit_gate'], self.catalog['tranche_plan']['tranches'][1]['exit_gate'])
        self.assertEqual(self.register, before)

    def test_third_tranche_separates_implemented_code_aggregate_and_failed_acceptance(self):
        before = copy.deepcopy(self.register)
        tranche = importance_view(self.catalog, self.register, self.tracks)['tranche_plan']['tranches'][2]
        tasks = {t['behavior']: t['work_review'] for t in tranche['tasks']}
        self.assertEqual(sum(w['implementation'] == 'implemented_within_scope' for w in tasks.values()), 10)
        self.assertEqual(tasks['NR.directdraw-retirement']['implementation'], 'unresolved_aggregate')
        self.assertTrue(all(not w['implementation_gaps'] for w in tasks.values()))
        for work in tasks.values():
            self.assertEqual(work['freshness'], 'historical' if work['changed_sources'] or work.get('pending_source_review') else 'current')
        failures = {bid for bid, w in tasks.items() if w['acceptance'] == 'acceptance_failure'}
        self.assertEqual(failures, set())
        for bid in ('NR.drawing-storage-session-comparison', 'NR.recorded-hardware-session-parity'):
            self.assertEqual(len(tasks[bid]['acceptance_profiles']), 4)
            self.assertTrue(all(p['outcome'] == 'PASS' and p['mismatches'] == 0 for p in tasks[bid]['acceptance_profiles']))
            self.assertFalse(tasks[bid]['legacy_diagnostics']['hardware']['blocking'])
        self.assertTrue(all(tasks[bid]['implementation'] == 'implemented_within_scope' for bid in failures))
        self.assertTrue(all(w['validation_state'] == ('pending' if bid in {'NR.drawing-storage-session-comparison', 'NR.recorded-hardware-session-parity'} else 'recorded' if bid in {'NR.world-producer-handoff', 'NR.directdraw-retirement'} else 'historical') for bid,w in tasks.items()))
        self.assertEqual(self.register, before)

    def test_second_tranche_reviews_supplied_mechanisms_and_unresolved_history(self):
        before = copy.deepcopy(self.register)
        plan = importance_view(self.catalog, self.register, self.tracks)['tranche_plan']
        tranche = plan['tranches'][1]
        self.assertTrue(tranche['work_review_required'])
        tasks = {t['behavior']: t for t in tranche['tasks']}
        self.assertEqual(sum(t['work_review']['implementation'] == 'implemented_within_scope'
                             for t in tasks.values()), 11)
        self.assertEqual({bid for bid,t in tasks.items()
                          if t['work_review']['implementation'] == 'unresolved_aggregate'},
                         set())
        self.assertEqual({bid for bid,t in tasks.items()
                          if t['work_review']['acceptance'] == 'acceptance_failure'},
                         set())
        for task in tasks.values():
            self.assertFalse(task['work_review']['implementation_gaps'])
            self.assertTrue(task['execution_brief']['supplied_controls'])
            self.assertEqual(task['implementation'], 'scoped')
            original = next(t for t in self.catalog['tranche_plan']['tranches'][1]['tasks']
                            if t['behavior'] == task['behavior'])
            self.assertEqual(task.get('execution_result'), original.get('execution_result'))
            self.assertFalse(any(s['kind'] == 'implementation' for s in task['work_review']['next_steps']))
        self.assertIn('independent bounded native table', tasks['RS.world-startup-wave']['implementation_boundary'])
        self.assertIn('recovered zero EAX', tasks['RS.world-producer-bypass']['implementation_boundary'])
        self.assertIn('16 completed canvases', tasks['NR.world-startup-sequence']['work_review']['current_state'])
        self.assertEqual(self.register, before)

    def test_first_tranche_reviews_supplied_controls_without_reopening_fixture_work(self):
        before = copy.deepcopy(self.register)
        tranche = importance_view(self.catalog, self.register, self.tracks)['tranche_plan']['tranches'][0]
        tasks = {t['behavior']: t for t in tranche['tasks']}
        self.assertEqual(sum(t['work_review']['implementation'] == 'implemented_within_scope'
                             for t in tasks.values()), 11)
        self.assertEqual({bid for bid,t in tasks.items()
                          if t['work_review']['implementation'] == 'unresolved_aggregate'},
                         set())
        self.assertEqual({bid for bid,t in tasks.items()
                          if t['work_review']['acceptance'] == 'acceptance_failure'},
                         set())
        self.assertEqual(sum(t['execution_result']['outcome'] == 'PASS' for t in tasks.values()), 11)
        self.assertEqual(tasks['NP.native-output-ownership']['execution_result']['outcome'], 'PASS')
        self.assertEqual(tasks['NP.native-output-ownership']['execution_history'][0]['outcome'], 'FAIL')
        self.assertIn('tracker reacquisition', tasks['NP.native-output-ownership']['work_review']['next_steps'][0]['action'])
        self.assertIn('diagnos',tasks['NP.native-output-ownership']['next_action'].lower())
        for bid,t in tasks.items():
            self.assertFalse(t['work_review']['implementation_gaps'])
            self.assertTrue(t['execution_brief']['supplied_controls'])
            if bid not in {'NR.palette-resources','NR.command-checkpoint-attachment','NP.native-output-ownership'}:
                self.assertEqual(t['execution_brief']['fixture_gaps'], [])
        self.assertEqual(self.register,before)

    def test_resolved_domains_bind_suppliers_and_keep_validation_separate(self):
        tasks = self.catalog['tranche_plan']['tranches'][0]['tasks']
        selected = [t for t in tasks if t['behavior'] in
                    {'NR.palette-resources', 'NR.command-checkpoint-attachment'}]
        self.assertEqual(len(selected), 2)
        for task in selected:
            work = task['work_review']
            self.assertEqual(work['implementation'], 'implemented_within_scope')
            self.assertTrue(work['scope_domains'])
            self.assertIn('guarded_refusal', {d['disposition'] for d in work['scope_domains']})
            self.assertIn('outside_contract', {d['disposition'] for d in work['scope_domains']})
            self.assertEqual(work['acceptance'], 'pending')
        for case in ('invented_supplier', 'unbound_path', 'missing_validation', 'unknown_disposition'):
            c = copy.deepcopy(self.catalog)
            task = next(t for t in c['tranche_plan']['tranches'][0]['tasks']
                        if t['behavior'] == 'NR.palette-resources')
            domain = task['work_review']['scope_domains'][0]
            if case == 'invented_supplier': domain['suppliers'] = ['NR.invented-service']
            if case == 'unbound_path': domain['paths'] = ['unreviewed.cpp']
            if case == 'missing_validation': domain['validation'] = ''
            if case == 'unknown_disposition': domain['disposition'] = 'everything-complete'
            with self.subTest(case=case), self.assertRaises(ValueError):
                importance_view(c, self.register, self.tracks)

    def test_partial_work_requires_named_missing_code_and_failure_requires_evidence(self):
        for case in ['unnamed_partial', 'implemented_gap', 'failure', 'wrong_evidence', 'implementation_step', 'missing_review']:
            c = copy.deepcopy(self.catalog); t = c['tranche_plan']['tranches'][2]['tasks'][0]
            w = t['work_review']
            if case == 'unnamed_partial': w['implementation'] = 'partially_implemented'
            if case == 'implemented_gap': w['implementation_gaps'] = ['Missing required branch']
            if case == 'failure': w['acceptance'] = 'acceptance_failure'
            if case == 'wrong_evidence': w['evidence'] = ['unregistered-result']
            if case == 'implementation_step': w['next_steps'][0]['kind'] = 'implementation'
            if case == 'missing_review': del t['work_review']
            with self.subTest(case=case), self.assertRaises(ValueError):
                importance_view(c, self.register, self.tracks)

    def test_changed_review_source_marks_task_stale_without_revoking_historical_stages(self):
        c = copy.deepcopy(self.catalog); task = c['tranche_plan']['tranches'][2]['tasks'][0]
        name = next(iter(task['work_review']['source_sha256']))
        task['work_review']['source_sha256'][name] = '0' * 64
        before = copy.deepcopy(self.register)
        result = importance_view(c, self.register, self.tracks)['tranche_plan']['tranches'][2]['tasks'][0]
        self.assertEqual(result['review_state'], 'needs_review')
        self.assertEqual(result['work_review']['freshness'], 'historical')
        self.assertIn(name, result['work_review']['changed_sources'])
        self.assertEqual(result['behavior'], 'NR.world-history-channel')
        self.assertEqual(result['implementation'], 'scoped')
        self.assertEqual(self.register, before)

    def test_indexed_inputs_cannot_silently_grant_source_review_credit(self):
        c = copy.deepcopy(self.catalog)
        task = c['tranche_plan']['tranches'][2]['tasks'][0]
        name = next(iter(task['work_review']['source_sha256']))
        task['work_review']['pending_source_review'] = [name]
        result = importance_view(c, self.register, self.tracks)['tranche_plan']['tranches'][2]['tasks'][0]
        self.assertEqual(result['work_review']['freshness'], 'historical')
        self.assertEqual(result['review_state'], 'needs_review')
        self.assertEqual(result['behavior'], 'NR.world-history-channel')
        self.assertEqual(result['implementation'], 'scoped')

    def test_reviewed_witness_cannot_claim_missing_or_unlinked_code(self):
        for case in ['line', 'unlinked', 'escape']:
            c = copy.deepcopy(self.catalog)
            # Keep this corruption check on an unchanged witness. The history
            # channel's old witness remains historical after195470ce.
            w = next(t for t in c['tranche_plan']['tranches'][2]['tasks']
                     if t['behavior'] == 'NR.directdraw-retirement')['work_review']
            # Bind only this synthetic corruption fixture to the current witness;
            # production history retains its old fingerprint and freshness.
            witness = w['code_witnesses'][0]['path']
            w['source_sha256'][witness] = hashlib.sha256((ROOT / witness).read_bytes()).hexdigest()
            if case == 'line': w['code_witnesses'][0]['text'] = 'invented code'
            if case == 'unlinked': w['code_witnesses'][0]['path'] = 'apps/coverage-ui/app.js'
            if case == 'escape': w['source_sha256']['../outside'] = '0' * 64
            with self.subTest(case=case), self.assertRaises(ValueError):
                importance_view(c, self.register, self.tracks)


    def test_high_tranches_retain_completed_reviewed_contracts_and_cover_unfinished_once(self):
        before = copy.deepcopy((self.register, self.catalog))
        result = importance_view(self.catalog, self.register, self.tracks)
        plan = result['high_tranche_plan']
        self.assertEqual([len(t['tasks']) for t in plan['tranches']], [17, 17, 17])
        tasks = [task for tranche in plan['tranches'] for task in tranche['tasks']]
        expected = {e['behavior'] for e in result['entries']
                    if e['weight'] == 4 and e['implementation'] in {'partial', 'none'}}
        completed = {e['behavior'] for e in json.loads(evidence_text(ROOT /
            'research/runtime/coverage/native-implementation-completion-20261020.json'))['entries']
            if e['decision'] == 'scoped_implementation_complete'}
        completed.update({'NR.directdraw-public-native-menu', 'NR.directdraw-native-opengl-launch',
                          'NR.storage-session-corpus'})  # bounded implementations inbf170af0
        completed.update({'RE.terrain_region', 'RE.terrain_catalog', 'RE.terrain_selection',
                          'AN.forward', 'AN.placement', 'AN.attachment',
                          'RE.terrain_map', 'RE.palette_preferences', 'RE.palette_shading',
                          'RE.terrain_camera', 'RE.terrain_sections', 'RE.terrain_submission',
                          'AN.direction', 'RE.sprite_queue', 'RE.sprite_visibility',
                          'NR.public-resident-world', 'NR.world-continuous-source',
                          'NR.world-continuous-normal-performance'})
        completed.update({'RE.terrain_creature_lighting', 'RE.terrain_constraints', 'RE.terrain_generation', 'RE.terrain_lighting', 'RE.terrain_solver', 'AN.world-facing', 'RE.terrain_generated', 'RE.terrain_specific', 'RE.terrain_traversal'})  # N23 bounded native services
        completed.add('RS.shadow-linear-stride')  # bounded implementation in97c3f5b2
        completed.update({'NR.world_scene', 'NR.terrain_scene', 'NR.wine-private-backing-ownership'})  # implementation in4cd9121f
        unplanned = {'RE.palette-effect-builders', 'RE.terrain-camera-lifecycle',
                     'RE.terrain-entity-section-rotation', 'RE.terrain-spectator-lighting'}
        self.assertEqual({t['behavior'] for t in tasks} | unplanned,
                         expected | completed | {'RS.displacement-final-word-read'})
        self.assertEqual({t['behavior'] for t in tasks if t['implementation'] == 'scoped'}, completed | {'RS.displacement-final-word-read'})
        self.assertTrue(all(t['reviewed_implementation'] in {'partial', 'none', 'scoped'} for t in tasks))
        self.assertEqual(len(tasks), 51)
        self.assertEqual(set(plan['unplanned_high']), unplanned)
        self.assertEqual(sum(t['implementation'] == 'none' for t in tasks), 0)
        recovered_suppliers = {
            'RS.world-queue-sort', 'RS.world-queue-owner-storage',
            'RS.world-occlusion-mask', 'RS.canvas-startup-initializer',
            'RS.world-occlusion-actor-writers', 'RS.particle-owner-slot-lifecycle',
            'RS.drawing-session-release', 'RS.alternate-canvas-x87-copy',
            'NP.native-creature-drawing-adapter',
            'RS.actor-map-occupancy', 'RS.actor-pool-admission',
            'RS.actor-reference-invalidation', 'RS.actor-cleanup-release',
            'RS.actor-activation-lifecycle',
            'RS.actor-downstream-value-services',
            'RS.actor-downstream-controller-services',
            'RS.actor-downstream-world-services',
            'RS.actor-downstream-ui-services',
            'RS.actor-downstream-dependency-services',
            'RS.actor-downstream-popup-services',
            'RS.actor-downstream-collection-services',
            'RS.actor-downstream-effect-services',
            'RS.actor-downstream-construction-services',
            'RS.actor-service-composition', 'RS.actor-concrete-providers',
            'RS.actor-session-source-lifecycle', 'NP.actor-session-source-ownership',
            'RS.actor-concrete-provider-frontier',
            'RS.actor-profile-transform-providers', 'RS.actor-palette-builder-providers',
            'RS.actor-font-providers', 'RS.actor-support-query-provider', 'RS.actor-particle-emitter-composition', 'RS.actor-emitting-update-composition',
            'RS.actor-emitter-family-composition',
        }
        scoped_high = {e['behavior'] for e in result['entries']
                       if e['weight'] == 4 and e['implementation'] == 'scoped'}
        self.assertTrue(recovered_suppliers.issubset(scoped_high))
        incoming = json.loads(evidence_text(ROOT / 'research/runtime/coverage/open-recovery-incoming-importance-20261021.json'))
        merged_high = {e['behavior'] for e in incoming if e['weight'] == 4 and e['implementation'] == 'scoped'}
        merged_high.update({
            'NP.native-world-cell-producer-adapter',
            'RS.world-cell-artifact-production',
            'RS.world-cell-creature-production',
            'RS.world-cell-effect-production',
            'RS.world-cell-queue-publication',
            'RS.world-cell-terrain-production',
        })
        self.assertTrue(merged_high.issubset(scoped_high))
        self.assertEqual(len(scoped_high), 72 + len(completed) + len(recovered_suppliers) + len(merged_high))
        for task in tasks:
            execution = task.get('execution_result')
            if execution:
                self.assertIn(execution['outcome'], {'PASS', 'FAIL'})
                self.assertTrue((ROOT / execution['report']).is_file())
                self.assertTrue(execution['remaining'])
                self.assertEqual(task['work_review']['acceptance'], 'pending')
        self.assertEqual((self.register, self.catalog), before)

    def test_high_first_tranche_records_all_seventeen_bounded_passes(self):
        before = copy.deepcopy(self.register)
        plan = importance_view(self.catalog, self.register, self.tracks)['high_tranche_plan']
        tranche = plan['tranches'][0]
        self.assertEqual(len(tranche['tasks']), 17)
        record = json.loads(evidence_text(ROOT / tranche['execution_result']['report']))
        self.assertTrue(record['all_components_pass'])
        self.assertTrue(record['sources_stable'])
        self.assertEqual(set(record['results']), {t['behavior'] for t in tranche['tasks']})
        self.assertEqual(record['native_controls']['normal_ctests'], 20)
        self.assertEqual(record['native_controls']['sanitized_ctests'], 20)
        for task in tranche['tasks']:
            self.assertEqual(task['execution_result']['outcome'], 'PASS')
            self.assertTrue(task['execution_result']['remaining'])
            self.assertEqual(task['work_review']['implementation'], 'implemented_within_scope')
            self.assertFalse(task['work_review']['implementation_gaps'])
            self.assertEqual(task['work_review']['acceptance'], 'pending')
            self.assertFalse(any(step['kind'] == 'implementation'
                                 for step in task['work_review']['next_steps']))
        statuses = {b['id']: b['status'] for b in self.register['behaviors']}
        self.assertEqual(statuses['RE.terrain_traversal']['comparison'], 'recorded')
        self.assertEqual(statuses['RE.terrain_generated']['comparison'], 'none')
        self.assertTrue(all(statuses[t['behavior']]['integration'] == 'none'
                            and statuses[t['behavior']]['replacement'] == 'none'
                            and statuses[t['behavior']]['implementation'] == 'scoped'
                            for t in tranche['tasks']))
        selected = {'RE.terrain_traversal', 'RE.terrain_constraints', 'RE.terrain_solver',
                    'RE.terrain_specific', 'RE.terrain_generation', 'RE.terrain_generated',
                    'RE.terrain_lighting', 'RE.terrain_creature_lighting'}
        for task in tranche['tasks']:
            expected_state = ('current' if task['work_review']['freshness'] == 'current' else 'needs_review')
            self.assertEqual(task['review_state'], expected_state)
            self.assertEqual(task['work_review']['validation_state'], 'recorded')
            if task['behavior'] in selected:
                self.assertEqual(task['review_state'], 'current')
                expected = ('N23.terrain-native-20261025' if task['behavior']=='RE.terrain_generated' else 'N23.terrain-original-20261025')
            elif task['behavior'] in {'RE.terrain_region', 'RE.terrain_catalog', 'RE.terrain_selection'}:
                expected = 'N22.terrain-original-20261025'
            elif task['behavior'] in {'RE.terrain_map', 'RE.palette_preferences', 'RE.palette_shading'}:
                expected = 'N20.rendering-original-20261024'
            else:
                expected = 'N19.terrain-original-20261024'
            self.assertIn(expected, task['work_review']['validation_evidence'])
            self.assertTrue(task['work_review_history'])
            if task['behavior'] in {'RE.terrain_region', 'RE.terrain_catalog', 'RE.terrain_selection', 'RE.terrain_submission'}:
                self.assertEqual(task['review_state'], 'needs_review')
                self.assertIn('reconstruction/rendering/CMakeLists.txt', task['work_review']['changed_sources'])
        self.assertEqual(self.register, before)

    def test_high_third_tranche_analyzer_fix_keeps_runtime_acceptance_pending(self):
        plan = importance_view(self.catalog, self.register, self.tracks)['high_tranche_plan']
        task = next(t for t in plan['tranches'][2]['tasks']
                    if t['behavior'] == 'NR.drawing-source-read-reservations')
        self.assertEqual(task['execution_result']['outcome'], 'PASS')
        self.assertEqual(task['work_review']['implementation'], 'implemented_within_scope')
        self.assertFalse(task['work_review']['implementation_gaps'])
        self.assertEqual(task['work_review']['validation_state'], 'historical')
        self.assertIn('HIGH03.ownership-reservations-current-20261020', task['work_review']['evidence'])
        self.assertEqual(task['work_review']['acceptance'], 'pending')
        record = json.loads(evidence_text(ROOT / task['execution_result']['report']))
        self.assertEqual(record['counts']['reproduced_false_admissions'], 24)
        self.assertEqual(record['counts']['remaining_false_admissions'], 0)
        self.assertFalse(record['status_promotions'])
        self.assertFalse(record['game_executed'])
        self.assertFalse(record['gpu_executed'])

    def test_high_source_assessments_record_read_model_and_surface_crosswalk(self):
        plan = importance_view(self.catalog, self.register, self.tracks)['high_tranche_plan']
        tasks = {t['behavior']: t for tranche in plan['tranches'] for t in tranche['tasks']}
        self.assertEqual(sum(t['work_review']['implementation'] == 'implemented_within_scope'
                             for t in tasks.values()), 51)
        self.assertEqual({bid for bid, t in tasks.items()
                          if t['work_review']['implementation'] == 'unresolved_aggregate'},
                         set())
        gap = tasks['RS.displacement-final-word-read']['work_review']
        self.assertEqual(gap['implementation'], 'implemented_within_scope')
        self.assertFalse(gap['implementation_gaps'])
        self.assertTrue(gap['code_witnesses'])
        self.assertTrue(gap['research_witnesses'])
        self.assertIn('18', gap['validation_summary'])
        surface = tasks['NR.surfaces']['work_review']
        self.assertTrue(surface['scope_domains'])
        self.assertTrue(all(d['disposition'] == 'supplied' for d in surface['scope_domains']))
        self.assertIn('twelve required fixture cells', surface['validation_summary'])
        self.assertIn('padding', tasks['RS.displacement-final-word-read']['validation'])
        self.assertIn('already located', tasks['AN.direction']['validation'])
        for task in tasks.values():
            work = task['work_review']
            self.assertTrue(work['validation_summary'])
            self.assertEqual(work['acceptance'], 'acceptance_failure' if task['behavior'] in {'NR.public-resident-world', 'NR.directdraw-public-native-menu'} else 'pending')
            if work['implementation'] == 'implemented_within_scope':
                self.assertFalse(work['implementation_gaps'])
                self.assertFalse(any(step['kind'] == 'implementation' for step in work['next_steps']))
            self.assertEqual(work['freshness'], 'historical' if work['changed_sources'] or work.get('pending_source_review') else 'current')

    def test_high_bad_size_order_duplicate_and_cross_priority_assignment_refuse(self):
        for case in ['size', 'order', 'duplicate', 'cross_priority']:
            c = copy.deepcopy(self.catalog)
            tranches = c['high_tranche_plan']['tranches']
            if case == 'size': tranches[0]['tasks'].pop()
            if case == 'order': tranches.reverse()
            if case == 'duplicate': tranches[1]['tasks'][0] = tranches[0]['tasks'][0]
            if case == 'cross_priority':
                tranches[0]['tasks'][0] = copy.deepcopy(c['tranche_plan']['tranches'][0]['tasks'][0])
            with self.subTest(case=case), self.assertRaises(ValueError):
                importance_view(c, self.register, self.tracks)

    def test_new_high_work_needs_assignment_instead_of_silent_selection(self):
        r = copy.deepcopy(self.register)
        next(b for b in r['behaviors'] if b['id'] == 'NP.native-clipped-image-copy')['status']['implementation'] = 'partial'
        plan = importance_view(self.catalog, r, self.tracks)['high_tranche_plan']
        self.assertEqual(plan['selected_count'], 51)
        self.assertEqual(set(plan['unplanned_high']), {'NP.native-clipped-image-copy',
                         'RE.palette-effect-builders', 'RE.terrain-camera-lifecycle',
                         'RE.terrain-entity-section-rotation', 'RE.terrain-spectator-lighting'})

    def test_high_scope_status_weight_and_source_drift_require_review(self):
        for case in ['scope', 'status', 'weight', 'source']:
            c = copy.deepcopy(self.catalog); r = copy.deepcopy(self.register)
            task = c['high_tranche_plan']['tranches'][0]['tasks'][0]
            bid = task['behavior']; b = next(b for b in r['behaviors'] if b['id'] == bid)
            before_status = b['status']['implementation']
            changed_status = 'partial' if before_status == 'scoped' else 'scoped'
            if case == 'scope': b['scope'] += ' changed scope'
            if case == 'status': b['status']['implementation'] = changed_status
            if case == 'weight': next(e for e in c['entries'] if e['behavior'] == bid)['weight'] = 3
            if case == 'source': task['work_review']['source_sha256'][next(iter(task['work_review']['source_sha256']))] = '0' * 64
            result = importance_view(c, r, self.tracks)['high_tranche_plan']['tranches'][0]['tasks'][0]
            with self.subTest(case=case):
                self.assertEqual(result['review_state'], 'needs_review')
                self.assertEqual(b['status']['implementation'], changed_status if case == 'status' else before_status)

    def test_absent_original_model_requires_a_real_research_witness(self):
        for case in ['witness', 'fake_code', 'missing_gap', 'unlinked_research']:
            c = copy.deepcopy(self.catalog)
            task = next(t for tr in c['high_tranche_plan']['tranches'] for t in tr['tasks']
                        if t['behavior'] == 'RS.displacement-final-word-read')
            w = task['work_review']
            # Exercise the absent-model schema even after the real selected
            # contract acquired its bounded implementation and code witness.
            w['implementation'] = 'not_implemented'
            w['implementation_gaps'] = ['Synthetic missing recovered-original model']
            w['code_witnesses'] = []
            if case == 'witness': w['research_witnesses'][0]['text'] = 'invented observation'
            if case == 'fake_code': w['implementation'] = 'implemented_within_scope'; w['implementation_gaps'] = []
            if case == 'missing_gap': w['implementation_gaps'] = []
            if case == 'unlinked_research': w['research_witnesses'][0]['path'] = 'apps/coverage-ui/app.js'
            with self.subTest(case=case), self.assertRaises(ValueError):
                importance_view(c, self.register, self.tracks)

    def test_high_review_document_cannot_escape_runtime_research(self):
        for name in ['original/game.md', 'research/runtime/../../AGENTS.md', '/etc/passwd',
                     'research/runtime/does-not-exist.md']:
            c = copy.deepcopy(self.catalog); c['high_tranche_plan']['review_document'] = name
            with self.subTest(name=name), self.assertRaises(ValueError):
                importance_view(c, self.register, self.tracks)



class HighCompletedReviewTest(unittest.TestCase):
    def test_completed_high_task_accepts_explicit_review_without_promoting_register(self):
        self.check_review('scoped', 'current')

    def test_unreviewed_completion_remains_visible_as_plan_drift(self):
        self.check_review('partial', 'needs_review')

    def check_review(self, reviewed, expected):
        from coverage_importance import tranche_view
        behavior = dict(id='one', kind='native_policy', scope='bounded', original=[],
                        implementation=[], tests=['tests/control.cpp'], documents=['research/runtime/control.md'],
                        status=dict(implementation='scoped'))
        before = copy.deepcopy(behavior)
        contract = behavior_contract(behavior, {})
        task = dict(behavior='one', reviewed_implementation=reviewed, contract_sha256=contract,
                    checks=behavior['tests'], documents=behavior['documents'])
        for field in ('mode', 'next_action', 'implementation_boundary', 'validation', 'exit_condition', 'remaining_at_review'):
            task[field] = 'Bounded fixture; wider acceptance remains separate.'
        plan = dict(state='planned', completion_rule='Exact reviewed scoped execution',
                    tranches=[dict(id='first', order=1, depends_on=[], title='one', goal='one', exit_gate='one', tasks=[task])])
        rated = dict(one=dict(weight=4, implementation='scoped', review_state='current', contract_sha256=contract))
        result = tranche_view(plan, dict(behaviors=[behavior]), rated, priority=4, size=1, count=1)
        self.assertEqual(result['tranches'][0]['tasks'][0]['review_state'], expected)
        self.assertEqual(result['unplanned_high'], [])
        self.assertEqual(behavior, before)


if __name__ == '__main__':
    unittest.main()
