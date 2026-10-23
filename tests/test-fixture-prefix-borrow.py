import fcntl
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_fixture_prefix import borrow


class Borrow(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(); self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)/'owner'; self.root.mkdir()
        subprocess.run(['git', 'init', '-q', self.root], check=True)
        self.prefix = self.root/'working/tests/drawing-fixture-prefix-suite/suite/wineprefix'
        self.prefix.mkdir(parents=True)
        self.marker = self.prefix.parent/'fixture-prefix-owner.json'
        self.marker.write_text(json.dumps(dict(schema=1, root=str(self.root), scope='serial-owned-fixtures-only')))
    def test_requires_supervising_lock(self):
        with self.assertRaisesRegex(ValueError, 'exclusive lease'): borrow(self.root, self.prefix)
        with (self.prefix.parent/'fixture-prefix.lock').open('a+b') as lock:
            fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
            self.assertEqual(borrow(self.root, self.prefix), (self.prefix, None))
        with self.assertRaisesRegex(ValueError, 'exclusive lease'): borrow(self.root, self.prefix)
    def test_unmarked_or_foreign_prefix_refuses(self):
        self.marker.unlink()
        with self.assertRaises(OSError): borrow(self.root, self.prefix)
    def test_other_git_checkout_refuses(self):
        other = Path(self.tmp.name)/'other'; other.mkdir()
        subprocess.run(['git', 'init', '-q', other], check=True)
        with self.assertRaisesRegex(ValueError, 'Git checkout'): borrow(other, self.prefix)


if __name__ == '__main__': unittest.main()
