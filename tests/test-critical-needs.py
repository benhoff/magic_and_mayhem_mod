"""Critical work routing must not hide drift or promote engine milestones."""
import copy
from collections import Counter
import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from coverage_claims import behavior_contract
from coverage_critical_needs import CATALOG, critical_needs_view


class CriticalNeeds(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        (self.root / 'impl.py').write_text('pass\n')
        b = dict(id='TEST.critical', kind='native_policy', scope='Bounded supplied mechanism',
                 original=[], implementation=['impl.py'], tests=[], documents=[],
                 status={'implementation': 'partial'})
        self.register = {'behaviors': [b], 'builds': []}
        self.importance = {'entries': [{'behavior': b['id'], 'weight': 5, 'implementation': 'partial'}]}
        self.definition = dict(schema=1, static_review_only=True, entries=[dict(
            behavior=b['id'], primary_need='validation', reason='Supplied bounded mechanism',
            closure='Declared original/native matrix passes without widening scope.',
            requirements=[dict(kind='validation', action='Run the declared matrix.')],
            source_sha256={'impl.py': hashlib.sha256(b'pass\n').hexdigest()},
            contract_sha256=behavior_contract(b, {}))])

    def view(self):
        return critical_needs_view(self.definition, self.register, self.importance, self.root)

    def test_changed_code_or_scope_moves_out_of_validation_to_review(self):
        self.assertEqual(self.view()['counts']['validation'], 1)
        (self.root / 'impl.py').write_text('raise RuntimeError\n')
        result = self.view()
        self.assertEqual(result['counts']['validation'], 0)
        self.assertEqual(result['counts']['review'], 1)
        self.assertEqual(result['entries'][0]['reviewed_primary_need'], 'validation')
        self.assertIn('impl.py', result['entries'][0]['changed_sources'])
        (self.root / 'impl.py').write_text('pass\n')
        self.register['behaviors'][0]['scope'] += ' with wider inputs'
        self.assertIn('behavior contract', self.view()['entries'][0]['changed_sources'])

    def test_new_critical_is_visible_and_resolved_or_rerated_contract_leaves_denominator(self):
        self.importance['entries'].append(dict(behavior='TEST.new', weight=5, implementation='partial'))
        result = self.view()
        self.assertEqual(result['total'], 2)
        self.assertEqual(result['counts']['review'], 1)
        self.importance['entries'][0]['implementation'] = 'scoped'
        self.assertEqual(self.view()['total'], 1)
        self.importance['entries'][1]['weight'] = 4
        self.assertEqual(self.view()['total'], 0)

    def test_incomplete_duplicate_and_unsafe_assessments_refuse(self):
        for mutation in ('missing_source', 'duplicate', 'unsafe', 'reserved', 'empty_closure'):
            before = copy.deepcopy(self.definition)
            e = self.definition['entries'][0]
            if mutation == 'missing_source': e['source_sha256'] = {}
            if mutation == 'duplicate': self.definition['entries'].append(copy.deepcopy(e))
            if mutation == 'unsafe': e['source_sha256']['../outside.py'] = '0' * 64
            if mutation == 'reserved': e['source_sha256']['apps/../original/input.py'] = '0' * 64
            if mutation == 'empty_closure': e['closure'] = ''
            with self.subTest(mutation=mutation), self.assertRaises(ValueError): self.view()
            self.definition = before

    def test_assessment_edit_requires_review_without_changing_status(self):
        self.definition['entries'][0]['assessment_sha256'] = '0' * 64
        before = copy.deepcopy(self.register)
        self.assertEqual(self.view()['counts']['review'], 1)
        self.assertEqual(self.register, before)

    def test_new_contract_inputs_require_review_without_breaking_the_dashboard(self):
        (self.root / 'new.py').write_text('pass\n')
        self.register['behaviors'][0]['implementation'].append('new.py')
        row = self.view()['entries'][0]
        self.assertEqual(row['primary_need'], 'review')
        self.assertIn('newly linked input: new.py', row['changed_sources'])
        self.assertIn('behavior contract', row['changed_sources'])

    def resolution(self):
        self.register['behaviors'][0]['evidence'] = ['TEST.bounded']
        self.definition['entries'][0]['scope_resolution'] = dict(
            id='TEST.scope-decision', state='reconciled', decision='Keep the supplied bounded mechanism',
            bounded_scope='Selected owned source domain; no whole driver claim', record='impl.py',
            supplied_paths=['impl.py'], execution_evidence=['TEST.bounded'], remaining=[dict(
                kind='recovery', required_case='Observe a physical alias under a valid lease',
                closes_when='Independent identity, lifetime and pixels agree', behaviors=['TEST.critical'])])

    def test_reconciled_scope_retains_wider_work_without_engine_promotion(self):
        self.resolution()
        before = copy.deepcopy(self.register)
        row = self.view()['entries'][0]
        self.assertEqual(row['scope_resolution']['state'], 'reconciled')
        self.assertEqual(row['scope_resolution']['remaining'][0]['kind'], 'recovery')
        self.assertEqual(row['primary_need'], 'validation')
        self.assertEqual(self.register, before)

    def test_resolution_needs_valid_owners_closure_and_registered_execution(self):
        self.resolution()
        for mutation in ('owner', 'case', 'closure', 'evidence', 'scope_bucket'):
            saved = copy.deepcopy(self.definition)
            e = self.definition['entries'][0]; r = e['scope_resolution']
            if mutation == 'owner': r['remaining'][0]['behaviors'] = ['UNKNOWN.owner']
            if mutation == 'case': r['remaining'][0]['required_case'] = ''
            if mutation == 'closure': r['remaining'][0]['closes_when'] = ''
            if mutation == 'evidence': r['execution_evidence'] = ['UNREGISTERED.result']
            if mutation == 'scope_bucket': e['primary_need'] = 'scope'
            with self.subTest(mutation=mutation), self.assertRaises(ValueError): self.view()
            self.definition = saved

    def test_source_drift_keeps_resolution_history_but_requires_review(self):
        self.resolution()
        (self.root / 'impl.py').write_text('raise RuntimeError\n')
        row = self.view()['entries'][0]
        self.assertEqual(row['primary_need'], 'review')
        self.assertEqual(row['scope_resolution']['state'], 'reconciled')
        self.assertEqual(row['assessment_state'], 'needs_review')

    def test_completed_native_scope_stays_visible_outside_partial_denominator(self):
        self.resolution()
        self.register['behaviors'][0]['status']['implementation'] = 'scoped'
        self.importance['entries'][0]['implementation'] = 'scoped'
        result = self.view()
        self.assertEqual(result['total'], 0)
        self.assertEqual(len(result['scope_decisions']), 1)
        self.assertEqual(result['scope_decisions'][0]['implementation'], 'scoped')

    def test_repository_covers_every_current_critical_once_without_engine_credit(self):
        register = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
        catalog = json.loads((ROOT / 'apps/coverage-ui/importance.json').read_text())
        definition = json.loads((ROOT / CATALOG).read_text())
        by_id = {b['id']: b for b in register['behaviors']}
        importance = dict(catalog, entries=[dict(e, implementation=by_id[e['behavior']]['status']['implementation'])
                                           for e in catalog['entries']])
        before = copy.deepcopy(register)
        result = critical_needs_view(definition, register, importance, ROOT)
        self.assertEqual(sum(result['counts'].values()), result['total'])
        self.assertEqual(len({e['behavior'] for e in result['entries']}), result['total'])
        # Source drift legitimately routes a row to review without losing its
        # authored primary need or the historical scope decision.
        self.assertEqual(Counter(e['reviewed_primary_need'] for e in result['entries']),
                         dict(recovery=6, validation=12, acceptance=7)
        )
        # 97c3f5b2 and4cd9121f implemented these bounded mechanisms. Their authored
        # validation/acceptance requirements remain; scoped code only removes
        # them from the partial-implementation denominator.
        recent = {'NR.callback-publication-isolation',
                  'NR.recorded-physical-frame-admission',
                  'NR.external-backing-ownership'}
        active = {e['behavior'] for e in result['entries']}
        assessed = {e['behavior']: e for e in definition['entries']}
        for bid in recent:
            self.assertEqual(by_id[bid]['status']['implementation'], 'scoped')
            self.assertNotIn(bid, active)
            self.assertTrue(assessed[bid]['requirements'])
        clear = next(e for e in result['entries'] if e['behavior'] == 'RS.canvas-startup-clear')
        self.assertEqual(clear['reviewed_primary_need'], 'validation')
        self.assertIn('REC6.startup-lease.live-20261020',
                      by_id[clear['behavior']]['evidence'])
        self.assertEqual(len(result['scope_decisions']), 9)
        self.assertEqual(sum(e['implementation'] == 'scoped' for e in result['scope_decisions']), 3)
        self.assertEqual(register, before)


if __name__ == '__main__':
    unittest.main()
