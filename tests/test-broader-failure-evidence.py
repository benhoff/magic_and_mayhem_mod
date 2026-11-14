#!/usr/bin/env python3
"""Expected failures keep strict identity and cannot become successful proofs."""
import sys
import unittest
from pathlib import Path
from unittest.mock import Mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from broader_failure_evidence import checked_negative
from broader_player_acceptance import checked


class NegativeEvidence(unittest.TestCase):
    reference = dict(record='retained-negative.json', sha256='exact-payload')

    def reader(self, value):
        return Mock(digest=Mock(return_value='exact-payload'), json=Mock(return_value=value))

    def test_preserved_failure_is_read_without_positive_credit(self):
        value = dict(success=False, error='missing retained helper', original_game_runs=0)
        reader = self.reader(value)
        with self.assertRaisesRegex(ValueError, 'Unaccepted retained input'):
            checked(reader, self.reference)
        self.assertIs(checked_negative(reader, self.reference), value)
        self.assertFalse(value['success'])

    def test_success_is_not_an_expected_failure(self):
        with self.assertRaisesRegex(ValueError, 'explicitly failed'):
            checked_negative(self.reader(dict(success=True)), self.reference)

    def test_absent_outcome_is_not_inferred(self):
        with self.assertRaisesRegex(ValueError, 'explicitly failed'):
            checked_negative(self.reader(dict(error='unknown')), self.reference)

    def test_numeric_zero_is_not_a_false_outcome(self):
        with self.assertRaisesRegex(ValueError, 'explicitly failed'):
            checked_negative(self.reader(dict(success=0)), self.reference)

    def test_foreign_schema_is_refused(self):
        with self.assertRaisesRegex(ValueError, 'explicitly failed'):
            checked_negative(self.reader([False]), self.reference)

    def test_hash_drift_is_refused_before_decode(self):
        reader = self.reader(dict(success=False))
        reader.digest.return_value = 'changed-payload'
        with self.assertRaisesRegex(ValueError, 'identity changed'):
            checked_negative(reader, self.reference)
        reader.json.assert_not_called()


if __name__ == '__main__':
    unittest.main()
