#!/usr/bin/env python3
"""Use the unchanged request driver with explicit zero-game refusal accounting."""
import importlib.util
from pathlib import Path
from broader_prelaunch_review import reviewed_with_setup_refusals
ROOT=Path(__file__).resolve().parents[1]
def main():
 path=ROOT/'tools/run-broader-player-case.py';spec=importlib.util.spec_from_file_location('broader_reviewed_request_driver',path);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
 module.reviewed_predecessors=reviewed_with_setup_refusals
 return module.main()
if __name__=='__main__':raise SystemExit(main())
