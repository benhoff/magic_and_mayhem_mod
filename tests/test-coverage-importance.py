#!/usr/bin/env python3
"""Planning weights cannot grant implementation or validation credit."""
import copy
import ast
import json
from pathlib import Path
import sys
import shlex
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
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
        register = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
        before = copy.deepcopy(register)
        definition = json.loads((ROOT / 'apps/coverage-ui/tracks.json').read_text())
        tracks = track_view(definition, register['behaviors'], [{**e, 'freshness': 'historical'} for e in register['evidence']])
        catalog = json.loads((ROOT / 'apps/coverage-ui/importance.json').read_text())
        result = importance_view(catalog, register, tracks)
        self.assertEqual(register, before)
        self.assertEqual(len(result['entries']), 369)
        self.assertEqual(result['unrated'], [])
        behaviors = {b['id']: b for b in register['behaviors']}
        builds = {b['id']: b for b in register['builds']}
        for entry in result['entries']:
            expected = 'current' if behavior_contract(behaviors[entry['behavior']], builds) == entry['contract_sha256'] else 'needs_review'
            self.assertEqual(entry['review_state'], expected)
        self.assertEqual(result['partial_counts'], {1: 91, 2: 9, 3: 16, 4: 50, 5: 35})
        by_id = {e['behavior']: e for e in result['entries']}
        self.assertEqual(by_id['AN.world-facing']['weight'], 4)
        self.assertEqual(by_id['AN.world-facing']['implementation'], 'partial')
        self.assertEqual(by_id['RS.world-initial-state']['weight'], 5)
        self.assertEqual(by_id['NR.live-drawing-replacement']['work_type'], 'acceptance')
        self.assertEqual(by_id['NR.drawing-egl-delivery-observation']['weight'], 1)


class TrancheTest(unittest.TestCase):
    def setUp(self):
        self.register = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
        definition = json.loads((ROOT / 'apps/coverage-ui/tracks.json').read_text())
        self.tracks = track_view(definition, self.register['behaviors'],
                                 [{**e, 'freshness': 'historical'} for e in self.register['evidence']])
        self.catalog = json.loads((ROOT / 'apps/coverage-ui/importance.json').read_text())

    def test_frozen_tranches_and_new_partial_recovery_remain_explicit(self):
        before = copy.deepcopy((self.catalog, self.register))
        result = importance_view(self.catalog, self.register, self.tracks)
        plan = result['tranche_plan']
        self.assertEqual(plan['state'], 'planned')
        self.assertEqual(plan['selected_count'], 33)
        self.assertEqual(plan['unplanned_critical'], ['RS.world-dispatch-targets', 'RS.world-kind8-virtual'])
        self.assertEqual([len(t['tasks']) for t in plan['tranches']], [11, 11, 11])
        tasks = [task for t in plan['tranches'] for task in t['tasks']]
        critical = {e['behavior'] for e in result['entries'] if e['weight'] == 5 and e['implementation'] == 'partial'}
        self.assertEqual({task['behavior'] for task in tasks} | set(plan['unplanned_critical']), critical)
        self.assertTrue(all(t['implementation'] == 'partial' for t in tasks))
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
        next(e for e in self.catalog['entries'] if e['behavior'] == 'AN.world-facing')['weight'] = 5
        result = importance_view(self.catalog, self.register, self.tracks)['tranche_plan']
        self.assertEqual(result['selected_count'], 33)
        self.assertEqual(result['unplanned_critical'], ['AN.world-facing', 'RS.world-dispatch-targets', 'RS.world-kind8-virtual'])

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
                tree = ast.parse((ROOT / runner['path']).read_text())
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
        execution = json.loads((ROOT / tranche['execution_report']).read_text())
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
            self.assertEqual(work['freshness'], 'historical' if work['changed_sources'] else 'current')
        failures = {bid for bid, w in tasks.items() if w['acceptance'] == 'acceptance_failure'}
        self.assertEqual(failures, set())
        for bid in ('NR.drawing-storage-session-comparison', 'NR.recorded-hardware-session-parity'):
            self.assertEqual(len(tasks[bid]['acceptance_profiles']),4)
        self.assertTrue(all(tasks[bid]['implementation'] == 'implemented_within_scope' for bid in failures))
        self.assertTrue(all(w['validation_state'] in {'historical','pending'} for w in tasks.values()))
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
            self.assertEqual(task['implementation'], 'partial')
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
        baseline = importance_view(self.catalog, self.register, self.tracks)['tranche_plan']['tranches'][2]['tasks'][0]
        c = copy.deepcopy(self.catalog); task = c['tranche_plan']['tranches'][2]['tasks'][0]
        name = next(iter(task['work_review']['source_sha256']))
        task['work_review']['source_sha256'][name] = '0' * 64
        before = copy.deepcopy(self.register)
        result = importance_view(c, self.register, self.tracks)['tranche_plan']['tranches'][2]['tasks'][0]
        self.assertEqual(result['review_state'], 'needs_review')
        self.assertEqual(result['work_review']['freshness'], 'historical')
        self.assertEqual(set(result['work_review']['changed_sources']),
                         set(baseline['work_review']['changed_sources']) | {name})
        self.assertEqual(result['implementation'], 'partial')
        self.assertEqual(self.register, before)

    def test_indexed_inputs_cannot_silently_grant_source_review_credit(self):
        c = copy.deepcopy(self.catalog)
        task = c['tranche_plan']['tranches'][2]['tasks'][0]
        name = next(iter(task['work_review']['source_sha256']))
        task['work_review']['pending_source_review'] = [name]
        result = importance_view(c, self.register, self.tracks)['tranche_plan']['tranches'][2]['tasks'][0]
        self.assertEqual(result['work_review']['freshness'], 'historical')
        self.assertEqual(result['review_state'], 'needs_review')
        self.assertEqual(result['implementation'], 'partial')

    def test_reviewed_witness_cannot_claim_missing_or_unlinked_code(self):
        for case in ['line', 'unlinked', 'escape']:
            c = copy.deepcopy(self.catalog); w = c['tranche_plan']['tranches'][2]['tasks'][0]['work_review']
            if case == 'line': w['code_witnesses'][0]['text'] = 'invented code'
            if case == 'unlinked': w['code_witnesses'][0]['path'] = 'apps/coverage-ui/app.js'
            if case == 'escape': w['source_sha256']['../outside'] = '0' * 64
            with self.subTest(case=case), self.assertRaises(ValueError):
                importance_view(c, self.register, self.tracks)


    def test_high_tranches_cover_all_unfinished_high_contracts_once(self):
        before = copy.deepcopy((self.register, self.catalog))
        result = importance_view(self.catalog, self.register, self.tracks)
        plan = result['high_tranche_plan']
        self.assertEqual([len(t['tasks']) for t in plan['tranches']], [17, 17, 17])
        tasks = [task for tranche in plan['tranches'] for task in tranche['tasks']]
        expected = {e['behavior'] for e in result['entries']
                    if e['weight'] == 4 and e['implementation'] in {'partial', 'none'}}
        self.assertEqual({t['behavior'] for t in tasks}, expected | {'RS.displacement-final-word-read'})
        self.assertEqual(len(tasks), 51)
        self.assertEqual(plan['unplanned_high'], [])
        self.assertEqual(sum(t['implementation'] == 'none' for t in tasks), 0)
        self.assertEqual(sum(e['weight'] == 4 and e['implementation'] == 'scoped'
                             for e in result['entries']), 71)
        for task in tasks:
            execution = task.get('execution_result')
            if execution:
                self.assertIn(execution['outcome'], {'PASS', 'FAIL'})
                self.assertTrue((ROOT / execution['report']).is_file())
                self.assertTrue(execution['remaining'])
                self.assertEqual(task['work_review']['acceptance'], 'pending')
        self.assertEqual((self.register, self.catalog), before)

    def test_high_third_tranche_analyzer_fix_keeps_runtime_acceptance_pending(self):
        plan = importance_view(self.catalog, self.register, self.tracks)['high_tranche_plan']
        task = next(t for t in plan['tranches'][2]['tasks']
                    if t['behavior'] == 'NR.drawing-source-read-reservations')
        self.assertEqual(task['execution_result']['outcome'], 'PASS')
        self.assertEqual(task['work_review']['implementation'], 'implemented_within_scope')
        self.assertFalse(task['work_review']['implementation_gaps'])
        self.assertEqual(task['work_review']['validation_state'], 'pending')
        self.assertEqual(task['work_review']['acceptance'], 'pending')
        record = json.loads((ROOT / task['execution_result']['report']).read_text())
        self.assertEqual(record['counts']['reproduced_false_admissions'], 24)
        self.assertEqual(record['counts']['remaining_false_admissions'], 0)
        self.assertFalse(record['status_promotions'])
        self.assertFalse(record['game_executed'])
        self.assertFalse(record['gpu_executed'])

    def test_high_source_assessments_record_read_model_and_surface_crosswalk(self):
        plan = importance_view(self.catalog, self.register, self.tracks)['high_tranche_plan']
        tasks = {t['behavior']: t for tranche in plan['tranches'] for t in tranche['tasks']}
        self.assertEqual(sum(t['work_review']['implementation'] == 'implemented_within_scope'
                             for t in tasks.values()), 50)
        self.assertEqual({bid for bid, t in tasks.items()
                          if t['work_review']['implementation'] == 'unresolved_aggregate'},
                         {'NR.public-resident-world'})
        gap = tasks['RS.displacement-final-word-read']['work_review']
        self.assertEqual(gap['implementation'], 'implemented_within_scope')
        self.assertFalse(gap['implementation_gaps'])
        self.assertTrue(gap['code_witnesses'])
        self.assertTrue(gap['research_witnesses'])
        self.assertIn('18', gap['validation_summary'])
        surface = tasks['NR.surfaces']['work_review']
        self.assertTrue(surface['scope_domains'])
        self.assertTrue(all(d['disposition'] == 'supplied' for d in surface['scope_domains']))
        self.assertIn('two of twelve', surface['validation_summary'])
        self.assertIn('padding', tasks['RS.displacement-final-word-read']['validation'])
        self.assertIn('already located', tasks['AN.direction']['validation'])
        for task in tasks.values():
            work = task['work_review']
            self.assertTrue(work['validation_summary'])
            self.assertEqual(work['acceptance'], 'pending')
            if work['implementation'] == 'implemented_within_scope':
                self.assertFalse(work['implementation_gaps'])
                self.assertFalse(any(step['kind'] == 'implementation' for step in work['next_steps']))
            self.assertEqual(work['freshness'], 'historical' if work['changed_sources'] else 'current')

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
        self.assertEqual(plan['unplanned_high'], ['NP.native-clipped-image-copy'])

    def test_high_scope_status_weight_and_source_drift_require_review(self):
        for case in ['scope', 'status', 'weight', 'source']:
            c = copy.deepcopy(self.catalog); r = copy.deepcopy(self.register)
            task = c['high_tranche_plan']['tranches'][0]['tasks'][0]
            bid = task['behavior']; b = next(b for b in r['behaviors'] if b['id'] == bid)
            if case == 'scope': b['scope'] += ' changed scope'
            if case == 'status': b['status']['implementation'] = 'scoped'
            if case == 'weight': next(e for e in c['entries'] if e['behavior'] == bid)['weight'] = 3
            if case == 'source': task['work_review']['source_sha256'][next(iter(task['work_review']['source_sha256']))] = '0' * 64
            result = importance_view(c, r, self.tracks)['high_tranche_plan']['tranches'][0]['tasks'][0]
            with self.subTest(case=case):
                self.assertEqual(result['review_state'], 'needs_review')
                self.assertEqual(b['status']['implementation'], 'scoped' if case == 'status' else 'partial')

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


if __name__ == '__main__':
    unittest.main()
