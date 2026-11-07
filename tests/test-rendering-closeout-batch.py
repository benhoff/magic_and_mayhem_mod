"""Real retained admissions and adverse mutations; no synthetic engine credit."""
from copy import deepcopy
from pathlib import Path
import sys
import json
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from coverage_evidence import Reader
import rendering_closeout as m

CPU = 'research/runtime/rendering-operations-results-20261107/closeout-analysis-final-02.json'
FILL = 'research/runtime/rendering-fill-operations-results-20261107/closeout-analysis-02.json'


class Checks(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.reader = Reader(ROOT)
        cls.addClassCleanup(cls.reader.close)
        cls.plan = cls.reader.json(m.PLAN)
        cls.parent_name = cls.reader.json(CPU)['parent']['record']
        cls.parent = cls.reader.json(cls.parent_name)
        cls.components = [('cpu', CPU), ('full-fill', FILL)]

    def test_actual_batch_equals_existing_sequential_admissions(self):
        actual = m.batch(ROOT, self.reader, self.parent_name, self.components)
        cpu = m.load(ROOT, 'cpu').join(self.reader, self.reader.json(CPU), self.parent, self.plan)
        expected = m.load(ROOT, 'full-fill').join(self.reader, self.reader.json(FILL), cpu, self.plan)
        self.assertEqual(actual['requirements'], expected['requirements'])
        self.assertEqual(actual['decisions'], self.parent['decisions'])
        self.assertEqual(actual['newly_passed_requirements'], 4)
        self.assertEqual(actual['original_game_runs'], 0)
        self.assertFalse(actual['case_execution_repeated'])
        for name, sha in self.parent['sources'].items():
            self.assertEqual(actual['sources'][name], sha)

    def test_order_independent_disjoint_rows(self):
        a = m.batch(ROOT, self.reader, self.parent_name, self.components)
        b = m.batch(ROOT, self.reader, self.parent_name, self.components[::-1])
        self.assertEqual(a['requirements'], b['requirements'])

    def test_all_four_existing_proof_adapters(self):
        parts = self.components + [
            ('pan', 'research/runtime/rendering-picking-resize-results-20261107/pan-analysis-01.json'),
            ('markers', 'research/runtime/rendering-markers-results-20261107/analysis-01.json')]
        result = m.batch(ROOT, self.reader, self.parent_name, parts)
        self.assertEqual(result['newly_passed_requirements'], 8)
        self.assertEqual(result['passed_requirements'], self.parent['passed_requirements'] + 8)

    def test_real_ui_reader_accepts_batch_inventory(self):
        from coverage_rendering_artifacts import rendering_artifacts_view
        result = m.batch(ROOT, self.reader, self.parent_name, self.components)
        definition = json.loads((ROOT/'apps/coverage-ui/tracks.json').read_text())
        # Exercise the production projection without saving or selecting a new
        # global progress record. Other reads retain their real data.
        from coverage_evidence import read_json
        target = definition['rendering_artifacts']['progress_record']
        def read(path, **kw):
            return result if str(path) == str(ROOT/target) else read_json(path, **kw)
        with patch('coverage_rendering_artifacts.read_json', side_effect=read):
            view = rendering_artifacts_view(ROOT, definition)
        self.assertNotIn('error', view['progress'])
        self.assertEqual(view['progress']['counts']['passed'], result['passed_requirements'])
        self.assertEqual(view['progress']['total'], 156)

    def test_no_duplicate_or_unknown_adapter(self):
        for parts in ([], self.components * 2, [('unknown', CPU)]):
            with self.subTest(parts=parts), self.assertRaises(ValueError):
                m.batch(ROOT, self.reader, self.parent_name, parts)

    def test_no_double_credit(self):
        with self.assertRaises(ValueError):
            m.batch(ROOT, self.reader, FILL, [('full-fill', FILL)])

    def test_changed_component_source_refused(self):
        digest = self.reader.digest
        component_source = next(iter(self.reader.json(CPU)['analysis_sources']))
        with patch.object(self.reader, 'digest', side_effect=lambda n: '0' * 64 if n == component_source else digest(n)):
            with self.assertRaisesRegex(ValueError, 'component source changed'):
                m.batch(ROOT, self.reader, self.parent_name, self.components)

    def test_invalid_component_cannot_supply_rows(self):
        read = self.reader.json
        def changed(name):
            value = deepcopy(read(name))
            if name == FILL:
                value['cases'][0]['witnesses'][0]['world_deleted'] = 1
            return value
        with patch.object(self.reader, 'json', side_effect=changed), self.assertRaises(ValueError):
            m.batch(ROOT, self.reader, self.parent_name, self.components)

    def test_bad_inventory(self):
        for mutate in (lambda q: q['requirements'].pop(),
                       lambda q: q['requirements'].__setitem__(0, q['requirements'][1]),
                       lambda q: q.update(passed_requirements=156),
                       lambda q: q['requirements'][0].update(owner_task='EP99'),
                       lambda q: q.update(dossier_identity='0' * 64)):
            q = deepcopy(self.parent)
            mutate(q)
            with self.assertRaises(ValueError):
                m.inventory(self.plan, q)

    def test_unrelated_row_mutation_refused(self):
        q = m.load(ROOT, 'cpu').join(self.reader, self.reader.json(CPU), self.parent, self.plan)
        row = next(r for r in q['requirements'] if r['id'] != 'operation.cpu-write-readback')
        row['pending_reason'] += ' changed'
        with self.assertRaises(ValueError):
            m.preserve(self.plan, self.parent, q['requirements'], 'operation.cpu-write-readback')

    def test_reordered_rows_refused(self):
        q = m.load(ROOT, 'cpu').join(self.reader, self.reader.json(CPU), self.parent, self.plan)
        q['requirements'].reverse()
        with self.assertRaises(ValueError):
            m.preserve(self.plan, self.parent, q['requirements'], 'operation.cpu-write-readback')


if __name__ == '__main__':
    unittest.main()
