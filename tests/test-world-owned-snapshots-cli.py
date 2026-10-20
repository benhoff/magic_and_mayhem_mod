import contextlib
import importlib.util
import io
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock


class SnapshotCliTest(unittest.TestCase):
    def invoke(self, arguments, expected):
        path = Path(__file__).resolve().parents[1] / 'tools/test-world-owned-snapshots.py'
        spec = importlib.util.spec_from_file_location('snapshot_cli', path)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        with tempfile.TemporaryDirectory() as directory:
            module.ROOT = Path(directory)
            with mock.patch.object(sys, 'argv', [str(path), *arguments]), \
                 mock.patch.object(module.subprocess, 'run', side_effect=AssertionError('Unexpected experiment')), \
                 contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()), \
                 self.assertRaises(SystemExit) as result:
                module.main()
            self.assertEqual(result.exception.code, expected)
            self.assertEqual(list(Path(directory).iterdir()), [])

    def test_help_never_starts_an_experiment(self):
        self.invoke(['--help'], 0)

    def test_partial_input_pair_is_refused_before_output(self):
        self.invoke(['--source', 'owned.bin'], 2)

    def test_conflicting_input_modes_are_refused_before_output(self):
        self.invoke(['--capture', 'captured.bin', '--source', 'source.bin', '--request', 'request.bin'], 2)

    def test_unknown_options_are_refused_before_output(self):
        self.invoke(['--unknown'], 2)


if __name__ == '__main__':
    unittest.main()
