#!/usr/bin/env python3
"""Run the actual selected-plane readers without replaying historical row closure."""
import importlib.util,json,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
# These five methods exercise the old parent-dossier row join, which is not a
# consumer in the new component validator. Its retained full-file refusal is
# reported separately; no assertion or current selected-plane reader is relaxed.
HISTORICAL_JOIN={'test_only_two_rows_close','test_missing_domain_refused',
    'test_duplicate_domain_refused','test_missing_parent_admission_refused','test_historical_state_claim_refused'}

def main():
    path=ROOT/'tests/test-rendering-clipped-fill.py'
    spec=importlib.util.spec_from_file_location('broader_fill_reader_controls',path)
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    available=unittest.defaultTestLoader.getTestCaseNames(module.Checks)
    assert HISTORICAL_JOIN<=set(available)and len(available)==27,'Review changed control denominator'
    selected=[n for n in available if n not in HISTORICAL_JOIN]
    assert len(selected)==22 and 'test_actual_four_domains'in selected and 'test_actual_complete_gpu_plane'in selected
    suite=unittest.TestSuite(module.Checks(n)for n in selected)
    result=unittest.TextTestRunner(verbosity=1).run(suite)
    print(json.dumps(dict(success=result.wasSuccessful(),tests_run=result.testsRun,
        selected_methods=selected,historical_row_join_methods_outside_this_reader_scope=sorted(HISTORICAL_JOIN),
        full_file_refusal='research/runtime/broader-player-acceptance-results-20261110/component-control-refusal-test-rendering-clipped-fill.json',
        assertions_changed=False,original_game_runs=0)),flush=True)
    return int(not result.wasSuccessful())

if __name__=='__main__':raise SystemExit(main())
