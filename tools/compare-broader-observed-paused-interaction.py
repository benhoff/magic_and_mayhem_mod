#!/usr/bin/env python3
"""Run the unchanged independent pixels with the explicit observed pause baseline."""
import importlib.util,json,sys
import coverage_evidence
from pathlib import Path
from coverage_evidence import Reader
from broader_terminal_pause import verify_pause,OBSERVED
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
SUPPORT=['tools/compare-broader-observed-paused-interaction.py','tools/broader_terminal_pause.py','tools/broader_terminal_ui_identity.py','tools/broader_pause_observation.py']
def main():
    sp=importlib.util.spec_from_file_location('observed_pause_complete_comparator',ROOT/'tools/compare-broader-paused-interaction.py');core=importlib.util.module_from_spec(sp);sp.loader.exec_module(core)
    core.verify_pause=verify_pause;write=coverage_evidence.write_report
    require(sys.argv.count('--report')==1,'One exact prospective comparison required');target=sys.argv[sys.argv.index('--report')+1]
    def writer(root,name,body):
        if str(name)==target:
            q=json.loads(body)
            with Reader(ROOT)as reader:
                q['sources']=q.get('sources',{})|{p:reader.digest(p)for p in SUPPORT}
                control=dict(record=OBSERVED,sha256=reader.digest(OBSERVED));c=reader.json(OBSERVED)
                stable=all(reader.digest(p)==h for p,h in q['sources'].items())and all(reader.digest(p)==h for p,h in c['sources'].items())
            q['observed_pause_binding']=dict(controls=control,optional_API_journal_required=False,original_API_poll_claimed=False,original_key_message_handler_claimed=False,all_other_pause_AST_preserved=True,terminal_UI_exact_frame_input_join_required=True)
            q['sources_stable']=q.get('sources_stable',False)and stable;q['success'] &=q['sources_stable'];body=(json.dumps(q,indent=2)+'\n').encode()
        return write(root,name,body)
    coverage_evidence.write_report=writer
    try:return core.main()
    finally:coverage_evidence.write_report=write
if __name__=='__main__':raise SystemExit(main())
