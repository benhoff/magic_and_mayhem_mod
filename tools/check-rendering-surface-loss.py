#!/usr/bin/env python3
"""Compare native surface-loss failed/fresh epochs against independent originals."""
import importlib.util
from pathlib import Path

if __name__=='__main__':
    path=Path(__file__).with_name('check-rendering-recovery-closeout.py')
    spec=importlib.util.spec_from_file_location('surface_loss_comparison',path)
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    raise SystemExit(module.main(surface_loss=True))
