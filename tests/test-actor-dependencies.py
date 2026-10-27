#!/usr/bin/env python3
"""Accounting guard: a component address link cannot close installed actor cases."""
import importlib.util
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
spec = importlib.util.spec_from_file_location('actor_dependencies', ROOT/'tools/audit-actor-dependencies.py')
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)


class DependencyInventory(unittest.TestCase):
    def setUp(self):
        self.selector = (ROOT/audit.TRANSITIONS).read_text()
        self.callback = audit.callback_targets(self.selector)[0][1]
        ids = {identity for case in audit.CASES for identity in case[3]}
        self.register = dict(builds=[dict(id=audit.BUILD, source_sha256='pinned')], behaviors=[
            dict(id=identity, scope='bounded component', implementation=['component.cpp'],
                 evidence=['historical'], status=dict(comparison='recorded', integration='headless'),
                 original=[dict(build=audit.BUILD, address=self.callback)]) for identity in sorted(ids)])
        self.frontier = dict(success=True, static_review_only=True, original_sha256='pinned',
                             concrete_routes=[dict(address=self.callback)],
                             supplied_providers=[dict(address=self.callback), dict(address=0x1001f000)],
                             unresolved_indirect_sites=[
                                 dict(owner=1, site=2, operand='eax'),
                                 dict(owner=1, site=2, unresolved_indirect='eax')])

    def test_component_proof_does_not_promote_installed_composition(self):
        report = audit.inventory(self.register, self.frontier, self.selector)
        self.assertTrue(report['callbacks'][0]['component_leads'])
        self.assertEqual(report['required_cases_closed'], 0)
        self.assertTrue(all(c['status'] == 'pending' for c in report['cases']))
        self.assertTrue(all(c['installed_actor_composition'] == 'pending' for c in report['callbacks']))
        self.assertFalse(report['whole_original_session'])
        self.assertEqual(len(report['historical_unresolved_indirect_sites']), 1)
        self.assertEqual(report['historical_supplied_endpoints'][1]['disposition'], 'authored_fixture_token')

    def test_other_build_is_not_a_supplier(self):
        for behavior in self.register['behaviors']:
            behavior['original'][0]['build'] = 'another-build'
        report = audit.inventory(self.register, self.frontier, self.selector)
        self.assertEqual(report['counts']['callbacks_with_component_leads'], 0)

    def test_wrong_build_and_missing_dependency_refuse(self):
        self.frontier['original_sha256'] = 'other'
        with self.assertRaisesRegex(ValueError, 'build mismatch'):
            audit.inventory(self.register, self.frontier, self.selector)
        self.frontier['original_sha256'] = 'pinned'
        self.register['behaviors'].pop()
        with self.assertRaisesRegex(ValueError, 'Unknown dependency'):
            audit.inventory(self.register, self.frontier, self.selector)

    def test_unreviewed_selector_change_refuses(self):
        with self.assertRaisesRegex(ValueError, 'selector changed'):
            audit.callback_targets(self.selector.replace('case 1: return 0x50acb0;', ''))


if __name__ == '__main__':
    unittest.main()
