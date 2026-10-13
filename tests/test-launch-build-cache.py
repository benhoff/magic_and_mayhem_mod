#!/usr/bin/env python3
"""Cache invalidation and publication using synthetic builders, no game media."""
import json
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
import launch_build_cache as cache


class AdapterCache(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        (self.root/'runtime').mkdir()
        (self.root/'tools').mkdir()
        self.header = self.root/'runtime/shared.h'
        self.header.write_text('first header')
        self.builder = self.root/'tools/build-fixture.py'
        self.builder.write_text('first builder')
        self.tool = self.root/'compiler'
        self.tool.write_text('first compiler')
        self.output = self.root/'working/build/fixture'
        self.calls = 0
        self.env = patch.dict(os.environ, {'MNM_CACHE_ADAPTERS': '1'})
        self.env.start(); self.addCleanup(self.env.stop)
        which = patch.object(cache.shutil, 'which', return_value=str(self.tool))
        which.start(); self.addCleanup(which.stop)

    def build(self):
        self.calls += 1
        self.output.mkdir(parents=True, exist_ok=True)
        dll = self.output/'Fixture.dll'
        dll.write_bytes(b'synthetic adapter')
        (self.output/'manifest.json').write_text(json.dumps({'sha256': cache.sha(dll)}))
        return dll

    def run_cache(self, **kwargs):
        return cache.build_adapter('tools/build-fixture.py', self.build, root=self.root, **kwargs)

    def test_hit_and_options(self):
        first = self.run_cache()
        self.assertEqual(first, self.run_cache())
        self.assertEqual(self.calls, 1)
        self.assertNotEqual(first, self.run_cache(options={'experimental_mini': True}))
        self.assertEqual(self.calls, 2)

    def test_source_and_tool_changes(self):
        self.run_cache()
        for path in (self.header, self.builder, self.tool):
            path.write_text(path.read_text()+' changed')
            self.run_cache()
        added = self.root/'runtime/new.h'; added.write_text('new include')
        self.run_cache(); added.unlink(); self.header.unlink(); self.run_cache()
        self.assertEqual(self.calls, 6)

    def test_corrupt_or_incomplete_artifacts_rebuild(self):
        dll = self.run_cache()
        for path in (dll, dll.parent/'manifest.json', dll.parent/'cache.json'):
            path.write_text('corrupt')
            self.assertEqual(self.run_cache(), dll)
        dll.unlink(); self.run_cache()
        self.assertEqual(self.calls, 5)

    def test_symlink_is_not_a_hit(self):
        dll = self.run_cache(); dll.unlink(); dll.symlink_to(self.output/'Fixture.dll')
        self.run_cache()
        self.assertFalse(dll.is_symlink())
        self.assertEqual(self.calls, 2)

    def test_diagnostics_bypass_cache(self):
        with patch.dict(os.environ, {'MNM_CACHE_ADAPTERS': '0'}):
            self.assertEqual(self.run_cache(), self.output/'Fixture.dll')
            self.run_cache()
        self.assertEqual(self.calls, 2)

    def test_standalone_default_builds_fresh(self):
        with patch.dict(os.environ, {}, clear=True):
            self.run_cache(); self.run_cache()
        self.assertEqual(self.calls, 2)

    def test_inputs_changed_during_build_do_not_publish(self):
        def changed():
            dll = self.build(); self.header.write_text('changed during compilation'); return dll
        with self.assertRaisesRegex(ValueError, 'inputs changed'):
            cache.build_adapter('tools/build-fixture.py', changed, root=self.root)
        self.assertEqual(list((self.root/'working/build/launch-adapters').rglob('cache.json')), [])

    def test_wrong_manifest_does_not_publish(self):
        def wrong():
            dll = self.build(); (dll.parent/'manifest.json').write_text('{"sha256":"wrong"}'); return dll
        with self.assertRaisesRegex(ValueError, 'disagree'):
            cache.build_adapter('tools/build-fixture.py', wrong, root=self.root)


if __name__ == '__main__':
    unittest.main()
