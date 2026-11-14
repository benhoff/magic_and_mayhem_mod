#!/usr/bin/env python3
"""Admit a retained acquisition with unchanged request and full derived guards."""
import importlib.util,json,sys
from pathlib import Path
import coverage_evidence
from coverage_evidence import Reader
from broader_terminal_pause import verify_pause,CONTROLS
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
SUPPORT=['tools/run-broader-terminal-source-case.py','tools/broader_terminal_pause.py','tools/broader_terminal_ui_identity.py']
def main():
    require(len(sys.argv)>1 and sys.argv[1]=='admit','Retained admission only; never acquire')
    sp=importlib.util.spec_from_file_location('terminal_retained_derived_driver',ROOT/'tools/run-broader-derived-source-case.py');module=importlib.util.module_from_spec(sp);sp.loader.exec_module(module)
    module.verify_pause=verify_pause;write=coverage_evidence.write_report
    def writer(root,target,body):
        if str(target).endswith('-derived-admission-verification-01.json'):
            q=json.loads(body)
            with Reader(ROOT)as r:
                q['sources'].update({p:r.digest(p)for p in SUPPORT})
                q['terminal_identity_controls']=dict(record=CONTROLS,sha256=r.digest(CONTROLS))
            body=(json.dumps(q,indent=2)+'\n').encode()
        return write(root,target,body)
    # The frozen driver imported its writer directly before this binding.
    module.write_report=writer
    return module.main()
if __name__=='__main__':raise SystemExit(main())
