#!/usr/bin/env python3
"""Execute owned native scene source, animation transaction and light producers."""
import importlib.util
from pathlib import Path
import sys
spec = importlib.util.spec_from_file_location('native_scene_checks', Path(__file__).with_name('test-native-scene-session.py'))
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
if __name__ == '__main__':
    raise SystemExit(module.main(['--sources-only', *sys.argv[1:]]))
