#!/usr/bin/env python3
"""Actual Git-parent provenance and historical-only merge admission controls."""
import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from coverage_merge_history import inherited_history


class MergeHistoryTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.behavior = {'id': 'NP.fixture', 'kind': 'native_policy', 'scope': 'Bounded fixture',
                         'original': [], 'implementation': ['fixture.c'], 'tests': ['fixture-test.py'],
                         'evidence': ['fixture-proof'], 'status': {'understanding': 'partial',
                         'implementation': 'scoped', 'comparison': 'none', 'integration': 'headless',
                         'replacement': 'none'}}
        self.evidence = {'id': 'fixture-proof', 'kind': 'native_integration', 'sha256': 'a'*64}
        self.review = {'id': 'parent-review', 'reason': 'Existing parent scope', 'files': [],
                       'behaviors': [], 'pending_gaps': [], 'validation': {'status': 'recorded',
                       'reason': 'Parent execution only', 'evidence': ['fixture-proof']}}
        self.g('init', '-q');self.g('config', 'user.name', 'Coverage fixture')
        self.g('config', 'user.email', 'coverage-fixture@example.invalid')
        self.write_register(False);self.write_reviews([]);self.commit('base')
        self.base = self.g('rev-parse', 'HEAD').strip()
        self.g('checkout', '-qb', 'incoming')
        self.write_register(True);self.write_reviews([self.review]);self.commit('existing milestone')
        self.parent = self.g('rev-parse', 'HEAD').strip()
        self.g('checkout', '-qb', 'integration', self.base)
        (self.root/'master.txt').write_text('independent master work\n');self.commit('master work')
        self.current = {'behaviors': {'NP.fixture': {'record': copy.deepcopy(self.behavior)}},
                        'builds': {}, 'evidence': {'fixture-proof': copy.deepcopy(self.evidence)}}
        self.reviews = {'reviews': [copy.deepcopy(self.review)]}

    def g(self, *args):
        return subprocess.check_output(['git', '-C', str(self.root), *args], stderr=subprocess.PIPE).decode()

    def commit(self, message):
        self.g('add', '.');self.g('commit', '-qm', message)

    def write_register(self, include):
        p=self.root/'research/runtime/coverage/register.json';p.parent.mkdir(parents=True,exist_ok=True)
        p.write_text(json.dumps({'builds': [], 'behaviors': [self.behavior] if include else [],
                                 'evidence': [self.evidence] if include else []}))

    def write_reviews(self, reviews):
        (self.root/'research/runtime/coverage/change-reviews.json').write_text(json.dumps({'reviews': reviews}))

    def merge(self, committed=False):
        self.g('merge', '--no-commit', '--no-ff', 'incoming')
        if committed:self.commit('merge integration')

    def history(self, staged=True):
        return inherited_history(self.root, self.base, self.current, self.reviews, staged)

    def test_pending_merge_uses_actual_incoming_parent(self):
        self.merge();v=self.history()
        self.assertEqual(v['statuses']['NP.fixture']['commit'],self.parent)
        self.assertEqual(v['reviews'],['parent-review'])

    def test_committed_merge_works_against_actual_push_base(self):
        self.merge(True);self.assertIn('NP.fixture',self.history(False)['statuses'])

    def test_unmerged_branch_cannot_supply_inheritance(self):
        self.assertEqual(self.history()['statuses'],{})

    def test_higher_status_cannot_be_inherited(self):
        self.merge();self.current['behaviors']['NP.fixture']['record']['status']['integration']='live_observation'
        self.assertEqual(self.history()['statuses'],{})

    def test_changed_scope_cannot_be_inherited(self):
        self.merge();self.current['behaviors']['NP.fixture']['record']['scope']='Expanded behavior'
        self.assertEqual(self.history()['statuses'],{})

    def test_changed_dependencies_cannot_be_inherited(self):
        self.merge();self.current['behaviors']['NP.fixture']['record']['validation_dependencies']=['new.h']
        self.assertEqual(self.history()['statuses'],{})

    def test_new_original_build_binding_cannot_be_inherited(self):
        self.merge();self.current['behaviors']['NP.fixture']['record']['original']=[{'build':'new-build','address':4096}]
        self.assertEqual(self.history()['statuses'],{})

    def test_changed_original_build_hash_cannot_be_inherited(self):
        self.g('checkout','incoming')
        path=self.root/'research/runtime/coverage/register.json';v=json.loads(path.read_text())
        v['builds']=[{'id':'original','source_sha256':'a'*64}]
        v['behaviors'][0]['original']=[{'build':'original','address':4096}]
        path.write_text(json.dumps(v));self.commit('bound original input')
        self.g('checkout','integration')
        self.current['behaviors']['NP.fixture']['record']=copy.deepcopy(v['behaviors'][0])
        self.current['builds']={'original':'b'*64}
        self.merge();self.assertEqual(self.history()['statuses'],{})

    def test_changed_evidence_cannot_be_inherited(self):
        self.merge();self.current['evidence']['fixture-proof']['sha256']='b'*64
        self.assertEqual(self.history()['statuses'],{})

    def test_removed_evidence_link_cannot_be_inherited(self):
        self.merge();self.current['behaviors']['NP.fixture']['record']['evidence']=[]
        self.assertEqual(self.history()['statuses'],{})

    def test_rewritten_receipt_cannot_be_imported(self):
        self.merge();self.reviews['reviews'][0]['reason']='Different receipt'
        self.assertEqual(self.history()['reviews'],[])

    def test_base_already_containing_parent_is_not_new_inheritance(self):
        self.merge(True)
        self.assertEqual(inherited_history(self.root,'HEAD',self.current,self.reviews,False)['statuses'],{})


if __name__ == '__main__':unittest.main()
