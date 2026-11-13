#!/usr/bin/env python3
"""Use the original request state machine under the exact new recorder-source review."""
import importlib.util
from pathlib import Path
from broader_viewport_source_budget import bound,predecessors
ROOT=Path(__file__).resolve().parents[1]
def main():
 path=ROOT/'tools/run-broader-player-case.py';spec=importlib.util.spec_from_file_location('broader_viewport_source_driver',path);core=importlib.util.module_from_spec(spec);spec.loader.exec_module(core);core.cumulative_bound=bound;core.reviewed_predecessors=predecessors;return core.main()
if __name__=='__main__':raise SystemExit(main())
