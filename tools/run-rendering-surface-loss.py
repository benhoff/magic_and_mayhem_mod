#!/usr/bin/env python3
"""Collect the frozen ordinary-provider surface-loss/public-restart experiment."""
import importlib.util
from pathlib import Path

if __name__=='__main__':
    path=Path(__file__).with_name('run-rendering-recovery-closeout.py')
    spec=importlib.util.spec_from_file_location('surface_loss_collection',path)
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    raise SystemExit(module.main(surface_loss=True))
