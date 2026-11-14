#!/usr/bin/env python3
"""Use the normal request state machine with one exact drain-continuation review."""
import importlib.util
from pathlib import Path
from broader_drained_source_budget import bound,predecessors
ROOT=Path(__file__).resolve().parents[1]
def main():
 path=ROOT/'tools/run-broader-player-case.py';spec=importlib.util.spec_from_file_location('broader_drained_request_driver',path);core=importlib.util.module_from_spec(spec);spec.loader.exec_module(core);core.cumulative_bound=bound;core.reviewed_predecessors=predecessors;return core.main()
if __name__=='__main__':raise SystemExit(main())
