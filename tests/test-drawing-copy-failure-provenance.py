#!/usr/bin/env python3
"""Preserve the acquisition cause even when its optional journal is incomplete."""
import importlib.util
import ast
import contextlib
import io
from types import SimpleNamespace
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))


def load(path):
    spec = importlib.util.spec_from_file_location(path.stem, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


capture = load(ROOT / 'tools/capture-drawing-interactive.py')
fixture = load(ROOT / 'tests/test-drawing-copy-order.py')


class Failure(unittest.TestCase):
    def test_missing_optional_journal_preserves_primary_exception_and_report(self):
        with tempfile.TemporaryDirectory() as directory:
            primary = ValueError('original startup refused')
            report = dict(success=False, error=str(primary))
            self.assertIs(capture.inspect_copy_order(report, Path(directory), primary), primary)
            self.assertEqual(report['error'], 'original startup refused')
            self.assertIn('Copy order journal refused:', report['copy_order_error'])
            self.assertFalse(report['success'])

    def test_incomplete_journal_preserves_primary(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'copy-order-0000.bin').write_bytes(fixture.journal()[:64])
            primary = RuntimeError('menu observation timed out')
            report = dict(success=False, error=str(primary))
            self.assertIs(capture.inspect_copy_order(report, root, primary), primary)
            self.assertEqual(report['error'], str(primary))
            self.assertFalse(report['admitted'])

    def test_journal_failure_alone_refuses_success(self):
        with tempfile.TemporaryDirectory() as directory:
            report = dict(success=True)
            self.assertIsInstance(capture.inspect_copy_order(report, Path(directory), None), OSError)
            self.assertEqual(report['error'], report['copy_order_error'])
            self.assertFalse(report['success'])

    def test_late_decoder_failure_refuses_the_actual_capture_exit_path(self):
        # Execute the real report-writing/exit tail without starting a game.
        tree=ast.parse((ROOT/'tools/capture-drawing-interactive.py').read_text())
        main=next(n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name=='main')
        start=next(i for i,n in enumerate(main.body) if isinstance(n,ast.Expr)
                   and ast.unparse(n).startswith('a.report.parent.mkdir('))
        tail=ast.fix_missing_locations(ast.Module(body=main.body[start:],type_ignores=[]))
        with tempfile.TemporaryDirectory() as directory:
            report=dict(success=False,error='Storage transition decoding failed: unmatched return')
            output=io.StringIO()
            context=dict(capture.__dict__,a=SimpleNamespace(report=Path(directory)/'result.json'),
                         report=report,failure=None,observation=dict(frames=[],inputs=[]),effects={})
            with contextlib.redirect_stdout(output),self.assertRaisesRegex(ValueError,'unmatched return'):
                exec(compile(tail,'capture-exit-tail','exec'),context)
            self.assertNotIn('"success": true',output.getvalue())
            self.assertFalse(__import__('json').loads(context['a'].report.read_text())['success'])

    def test_complete_journal_never_promotes_failed_acquisition(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'copy-order-0000.bin').write_bytes(fixture.journal())
            primary = ValueError('source completion refused')
            report = dict(success=False, error=str(primary))
            self.assertIs(capture.inspect_copy_order(report, root, primary), primary)
            self.assertTrue(report['copy_order_observation']['success'])
            self.assertFalse(report['success'])
            self.assertNotIn('copy_order_error', report)
            report = dict(success=True)
            self.assertIsNone(capture.inspect_copy_order(report, root, None))
            self.assertTrue(report['success'])


if __name__ == '__main__':
    unittest.main()
