#!/usr/bin/env python3
"""Use the unchanged request state machine under the exact reviewed237charge capacity-continuation policy."""
import importlib.util
from pathlib import Path
from broader_picking_target_budget import bound,predecessors
ROOT=Path(__file__).resolve().parents[1]
def main():
    path=ROOT/'tools/run-broader-player-case.py';s=importlib.util.spec_from_file_location('reviewed_setup_request_driver',path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);m.cumulative_bound=bound;m.reviewed_predecessors=predecessors;return m.main()
if __name__=='__main__':raise SystemExit(main())
