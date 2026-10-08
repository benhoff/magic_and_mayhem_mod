#!/usr/bin/env python3
"""Frozen every-raster original/AX comparison, independent of mixed live outputs."""
import importlib.util,sys
from pathlib import Path
p=Path(__file__).with_name('test-world-producer-handoff.py')
spec=importlib.util.spec_from_file_location('handoff',p);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
if '--prepare-raster-queue' not in sys.argv:sys.argv.append('--complete-raster-queues')
module.main()
