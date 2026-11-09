"""Retained input integrity, bounded reads and producer-owned fixture selection."""
from copy import deepcopy
import hashlib
from pathlib import Path
import sys
import unittest
from unittest.mock import Mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from retained_artifacts import read_artifact, scoped_artifacts, session_artifact_root
from rendering_menu_acceptance import artifact


class ArtifactReads(unittest.TestCase):
    def setUp(self):
        self.raw = b'retained observation'
        self.entry = dict(path='working/session/events.bin', record='evidence/events.bin',
                          bytes=len(self.raw), sha256=hashlib.sha256(self.raw).hexdigest())
        self.acquired = dict(artifacts=[self.entry])
        self.reader = Mock()
        self.reader.read_bytes.return_value = self.raw

    def test_exact_and_suffix_lookup_use_the_evidence_record(self):
        for path in (self.entry['path'], 'events.bin'):
            self.assertEqual(read_artifact(self.reader, self.acquired, path), self.raw)
        self.reader.read_bytes.assert_called_with('evidence/events.bin')

    def test_legacy_menu_entry_point_preserves_bytes_and_diagnostics(self):
        self.assertEqual(artifact(self.reader, self.acquired, 'events.bin'), self.raw)
        with self.assertRaisesRegex(ValueError, 'Missing/ambiguous public-menu artifact'):
            artifact(self.reader, self.acquired, 'missing.bin')
        self.reader.read_bytes.return_value = b'changed'
        with self.assertRaisesRegex(ValueError, 'Changed public-menu artifact'):
            artifact(self.reader, self.acquired, 'events.bin')

    def test_missing_ambiguous_and_partial_filename_matches_never_read(self):
        for entries in ([], [self.entry, dict(self.entry, path='other/events.bin')],
                        [dict(self.entry, path='working/session/not-events.bin')]):
            with self.subTest(entries=entries), self.assertRaises(ValueError):
                read_artifact(self.reader, dict(artifacts=entries), 'events.bin')
        self.reader.read_bytes.assert_not_called()

    def test_hash_and_size_must_each_match(self):
        for field, value in (('sha256', '0' * 64), ('bytes', len(self.raw) + 1)):
            entry = dict(self.entry, **{field: value})
            with self.subTest(field=field), self.assertRaises(ValueError):
                read_artifact(self.reader, dict(artifacts=[entry]), 'events.bin')

    def test_over_budget_or_invalid_declared_extent_refuses_before_read(self):
        for size in (len(self.raw) + 1, -1, True, '1'):
            entry = dict(self.entry, bytes=size)
            with self.subTest(size=size), self.assertRaises(ValueError):
                read_artifact(self.reader, dict(artifacts=[entry]), 'events.bin', max_bytes=len(self.raw))
        self.reader.read_bytes.assert_not_called()

    def test_invalid_budget_refuses_before_read(self):
        for limit in (-1, True, '1'):
            with self.subTest(limit=limit), self.assertRaises(ValueError):
                read_artifact(self.reader, self.acquired, 'events.bin', max_bytes=limit)
        self.reader.read_bytes.assert_not_called()

    def test_exact_budget_admits_and_cannot_hide_a_false_extent(self):
        self.assertEqual(read_artifact(self.reader, self.acquired, 'events.bin', max_bytes=len(self.raw)), self.raw)
        self.entry['bytes'] -= 1
        with self.assertRaises(ValueError):
            read_artifact(self.reader, self.acquired, 'events.bin', max_bytes=len(self.raw))

    def test_empty_artifact_can_have_an_explicit_zero_budget(self):
        self.reader.read_bytes.return_value = b''
        self.entry.update(bytes=0, sha256=hashlib.sha256(b'').hexdigest())
        self.assertEqual(read_artifact(self.reader, self.acquired, 'events.bin', max_bytes=0), b'')


class ProducerScope(unittest.TestCase):
    def setUp(self):
        self.acquired = dict(command=['collector', '--repo', '/old/producer'],
                             session='/old/producer/working/session',
                             artifacts=[dict(path=p) for p in (
                                 'working/session/events.bin',
                                 'working/session-other/events.bin',
                                 'working/unrelated/events.bin')])

    def test_scope_is_lexical_and_preserves_the_original_observations(self):
        before = deepcopy(self.acquired)
        selected, excluded = scoped_artifacts(self.acquired)
        self.assertEqual(selected['artifacts'], [self.acquired['artifacts'][0]])
        self.assertEqual(excluded, [v['path'] for v in self.acquired['artifacts'][1:]])
        self.assertEqual(self.acquired, before)

    def test_foreign_traversing_relative_and_whole_repository_roots_refuse(self):
        for path in ('/other/working/session', '/old/producer/../session', 'working/session', '/old/producer'):
            with self.subTest(path=path), self.assertRaises(ValueError):
                session_artifact_root(dict(self.acquired, session=path), 'session')

    def test_ambiguous_missing_and_unpinned_repository_refuse(self):
        for command in ([], ['collector', '--repo'], ['collector', '--repo', '/old/producer', '--repo', '/other'],
                        ['/unrecognized/launcher'], ['/old/producer/run-native.sh']):
            with self.subTest(command=command), self.assertRaises(ValueError):
                scoped_artifacts(dict(self.acquired, command=command))

    def test_pinned_launcher_and_explicit_repository_select_the_same_set(self):
        acquired = dict(self.acquired, command=['/old/producer/run-native.sh'],
                        sources={'run-native.sh': 'a' * 64})
        self.assertEqual(scoped_artifacts(acquired)[0]['artifacts'], [self.acquired['artifacts'][0]])

    def test_duplicate_owned_path_and_missing_roots_refuse(self):
        acquired = deepcopy(self.acquired)
        acquired['artifacts'].append(acquired['artifacts'][0])
        with self.assertRaises(ValueError):
            scoped_artifacts(acquired)
        with self.assertRaises(ValueError):
            scoped_artifacts(self.acquired, root_names=('missing',))

    def test_caller_can_name_another_exact_fixture_root(self):
        acquired = dict(self.acquired, capture='/old/producer/working/unrelated')
        selected, excluded = scoped_artifacts(acquired, root_names=('capture',))
        self.assertEqual(selected['artifacts'], [self.acquired['artifacts'][2]])
        self.assertEqual(len(excluded), 2)


if __name__ == '__main__':
    unittest.main()
