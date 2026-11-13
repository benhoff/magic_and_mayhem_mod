#!/usr/bin/env python3
"""Run the unchanged routing assertions against the explicit continued case catalog."""
import importlib.util
from pathlib import Path
from broader_player_worker_matrix import continued_specs
ROOT=Path(__file__).resolve().parents[1]
def main():
 spec=importlib.util.spec_from_file_location('unchanged_worker_matrix_controls',ROOT/'tools/check-broader-matrix-bindings-worker.py');core=importlib.util.module_from_spec(spec);spec.loader.exec_module(core);core.specs=continued_specs;return core.main()
if __name__=='__main__':raise SystemExit(main())
