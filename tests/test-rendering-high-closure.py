"""A supplier map cannot silently supply absent code or runtime proof."""
import copy
import importlib.util
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('high_closure', ROOT/'tools/check-rendering-high-closure.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class ResidentClosure(unittest.TestCase):
    def setUp(self):
        self.register = json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
        self.crosswalk = json.loads(module.DEFAULT.read_text())

    def test_complete_map_does_not_grant_engine_validation(self):
        result = module.validate(ROOT, self.register, self.crosswalk)
        self.assertTrue(result['implementation_supplied'])
        self.assertFalse(result['engine_validation_granted'])
        self.assertEqual(result['status_promotions'], [])

    def test_missing_domain_or_self_supplier_refuses(self):
        bad = copy.deepcopy(self.crosswalk)
        bad['domains'].pop()
        with self.assertRaisesRegex(ValueError, 'Missing resident'):
            module.validate(ROOT, self.register, bad)
        bad = copy.deepcopy(self.crosswalk)
        bad['domains'][0]['suppliers'] = ['NR.public-resident-world']
        with self.assertRaisesRegex(ValueError, 'independent registered'):
            module.validate(ROOT, self.register, bad)

    def test_unlinked_witness_test_and_changed_source_refuse(self):
        for kind in ('witness', 'test', 'hash'):
            bad = copy.deepcopy(self.crosswalk)
            if kind == 'witness':bad['domains'][0]['witnesses'][0]['path'] = 'renderer/nonexistent.cpp'
            if kind == 'test':bad['domains'][0]['tests'] = ['tests/nonexistent.cpp']
            if kind == 'hash':bad['source_sha256'][next(iter(bad['source_sha256']))] = '0'*64
            with self.assertRaises(ValueError):module.validate(ROOT, self.register, bad)

    def test_status_promotion_is_not_source_review(self):
        self.crosswalk['status_promotions'] = ['replacement']
        with self.assertRaisesRegex(ValueError, 'cannot promote'):
            module.validate(ROOT, self.register, self.crosswalk)


if __name__ == '__main__':
    unittest.main()
