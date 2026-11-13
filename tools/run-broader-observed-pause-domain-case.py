#!/usr/bin/env python3
"""Use the complete frozen coordinator with two declared analysis/admission routes."""
import json
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_observed_pause_dispatch import selected_source,comparison,admission
from broader_terminal_pause import OBSERVED
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
SUPPORT=['tools/run-broader-observed-pause-domain-case.py','tools/broader_observed_pause_dispatch.py','tools/broader_terminal_pause.py']
def main():
    source=ROOT/'tools/run-broader-derived-domain-case.py'
    def writer(root,target,body):
        if not str(target).endswith('-coordinator-01.json'):return write_report(root,target,body)
        q=json.loads(body)
        require(q['family']=='interaction','Observed pause coordinator is interaction only')
        with Reader(ROOT)as r:
            q['sources']=q.get('sources',{})|{n:r.digest(n)for n in SUPPORT}
            q['observed_pause_dispatch']=dict(controls=dict(record=OBSERVED,sha256=r.digest(OBSERVED)),exact_other_coordinator_AST_preserved=True,acquisition_command_unchanged=True)
        return write_report(root,target,(json.dumps(q,indent=2)+'\n').encode())
    ns=dict(__name__='observed_pause_domain',__file__=str(source),_observed_compare=comparison,_observed_admit=admission)
    exec(compile(selected_source(source.read_text()),str(source),'exec'),ns);ns['write_report']=writer
    return ns['main']()
if __name__=='__main__':raise SystemExit(main())
