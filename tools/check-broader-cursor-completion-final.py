#!/usr/bin/env python3
"""Apply all finite final guards to the explicit observed-pause current matrix."""
import json,sys
from pathlib import Path
import coverage_evidence
from coverage_evidence import Reader
import broader_player_derived_final_matrix as frozen
import broader_player_cursor_completion_final_matrix as selected
from broader_derived_final_runner import run
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
P='research/runtime/broader-player-acceptance-results-20261110/'
SUPPORT=['tools/broader_player_cursor_completion_final_matrix.py','tools/broader_player_cursor_completion_matrix.py','tools/broader_cursor_completion.py','tools/check-broader-cursor-completion-final.py','tools/broader_player_terminal_final_matrix.py','tools/broader_player_terminal_matrix.py','tools/broader_terminal_pause.py','tools/broader_pause_observation.py','tools/broader_terminal_ui_identity.py']
def main():
    require(sys.argv.count('--kind')==1,'One explicit final reader kind required')
    index=sys.argv.index('--kind');kind=sys.argv[index+1];require(kind in('acceptance','failure','boundary'),'Unknown final reader kind')
    argv=sys.argv;sys.argv=argv[:index]+argv[index+2:]
    require(sys.argv.count('--report')==1,'One fresh final report required');target=sys.argv[sys.argv.index('--report')+1]
    write=coverage_evidence.write_report;prior=frozen.current
    def writer(root,name,body):
        if str(name)==target:
            q=json.loads(body)
            with Reader(ROOT)as reader:
                q['sources']=q.get('sources',{})|{p:reader.digest(p)for p in SUPPORT}
                n=P+'observed-pause-dispatch-controls-02.json';c=reader.json(n)
                require(c['success']and c['tests']==10 and all(reader.digest(p)==h for p,h in c['sources'].items()),'Observed final routing controls changed')
                q['observed_pause_final_binding']=dict(controls=dict(record=n,sha256=reader.digest(n)),full_original_final_current_code_preserved=True,exact_pilot_retained_successor_only=True,original_API_poll_claimed=False,original_key_message_handler_claimed=False,all_required_cases_and_charges_unchanged=True)
                stable=all(reader.digest(p)==h for p,h in q['sources'].items())
            if 'sources_stable'in q:q['sources_stable'] &=stable
            q['success'] &=stable;body=(json.dumps(q,indent=2)+'\n').encode()
        return write(root,name,body)
    frozen.current=selected.current;coverage_evidence.write_report=writer
    try:return run(kind,'tools/check-broader-cursor-completion-final.py')
    finally:frozen.current=prior;coverage_evidence.write_report=write;sys.argv=argv
if __name__=='__main__':raise SystemExit(main())
