#!/usr/bin/env python3
"""Revalidate retained component inputs after the bounded V3 reader extension.

Compare every retained capture with both hash-pinned readers, then rerun the
unchanged finite case/row validators. Original/native execution sources remain
in their historical phase; these new reports describe current Python analysis.
"""
import argparse
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import types

from coverage_evidence import Reader, write_report
from rendering_closeout import load
from rendering_capture import capture
from rendering_pilot import require

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-closeout-batch-results-20261107/'
INPUTS={
 'cpu':'research/runtime/rendering-operations-results-20261107/closeout-analysis-final-02.json',
 'full-fill':'research/runtime/rendering-fill-operations-results-20261107/closeout-analysis-02.json',
 'pan':'research/runtime/rendering-picking-resize-results-20261107/pan-analysis-01.json',
 'markers':'research/runtime/rendering-markers-results-20261107/analysis-01.json',
}
NATIVE_DRIFT={'apps/qt-shell/acceptance_capture.hpp','apps/qt-shell/gl_viewport.cpp',
              'apps/qt-shell/gl_viewport.hpp'}
CONTROLS=('tools/review-rendering-closeout-inputs.py','tools/rendering_closeout.py',
          'tools/coverage_evidence.py','tools/rendering_capture.py',
          'tests/test-rendering-closeout-batch.py')


def signature(rows):
    h=hashlib.sha256();total=0
    require(rows[0][0]['version'] in (1,2),'Only retained V1/V2 input scope admitted')
    for metadata,plane in rows:
        body=plane() if callable(plane) else plane
        value=json.dumps(metadata,sort_keys=True,separators=(',',':')).encode()
        h.update(len(value).to_bytes(8,'little'));h.update(value)
        h.update(len(body).to_bytes(8,'little'));h.update(hashlib.sha256(body).digest())
        total+=len(body)
    return dict(rows=len(rows),decoded_bytes=total,sha256=h.hexdigest(),version=rows[0][0]['version'])


def reviewed_sources(reader,old):
    current={n:reader.digest(n) for n in old['analysis_sources']}
    changed={n:dict(recorded=h,current=current[n]) for n,h in old['analysis_sources'].items() if current[n]!=h}
    require(set(changed)<={'tools/rendering_capture.py'}|NATIVE_DRIFT,
            'Unreviewed component-source change: '+str(sorted(changed)))
    # Never update native fingerprints to imply another native execution.
    analysis={n:h for n,h in current.items() if n.endswith('.py')}
    analysis.update({n:reader.digest(n) for n in CONTROLS})
    historical={n:h for n,h in old['analysis_sources'].items() if not n.endswith('.py')}
    return analysis,historical,changed


def acquisitions(reader,old):
    result={}
    for case in old['cases']:
        ref=case['pair']['acquisition'] if 'pair' in case else case['acquisition']
        require(reader.digest(ref['record'])==ref['sha256'],'Changed admitted acquisition')
        result[ref['record']]=reader.json(ref['record'])
    return result


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--legacy-commit',default='aa4139b13ed4a7ae258133b3ae20f5d78ebb78bb')
    p.add_argument('--name',required=True,help='New immutable component report suffix')
    a=p.parse_args();require(a.name.replace('-','').isalnum(),'Safe report suffix required')
    original=subprocess.check_output(['git','show',a.legacy_commit+':tools/rendering_capture.py'],cwd=ROOT)
    legacy_hash=hashlib.sha256(original).hexdigest()
    with Reader(ROOT) as r:
        require(all(r.json(n)['analysis_sources']['tools/rendering_capture.py']==legacy_hash for n in INPUTS.values()),
                'Legacy source is not the exact accepted reader')
        legacy=types.ModuleType('pinned_legacy_capture')
        # Intentional bounded old/current reader comparison, not Git-gate execution.
        exec(compile(original,'legacy-capture@'+legacy_hash,'exec'),legacy.__dict__)
        source_record=PREFIX+a.name+'-legacy-reader.bin'
        write_report(ROOT,source_record,original)
        compared={}
        for adapter,name in INPUTS.items():
            report=PREFIX+a.name+'-'+adapter+'.json'
            require(not (ROOT/report).exists(),'New retained-source review required')
            old=r.json(name);require(old['success'],'Unaccepted component')
            sources,historical,drift=reviewed_sources(r,old)
            captures=[]
            for acq_name,acq in acquisitions(r,old).items():
                for artifact in acq['artifacts']:
                    if '/capture/' not in artifact['path'] or not artifact['path'].endswith('.bin'):
                        continue
                    ref=artifact['record'];digest=artifact['sha256']
                    require(r.digest(ref)==digest,'Changed retained capture')
                    if ref not in compared:
                        raw=r.read_bytes(ref)
                        require(len(raw)==artifact['bytes'],'Retained capture extent changed')
                        before=signature(legacy.capture(raw));after=signature(capture(raw))
                        require(before==after,'Reader extension changed an actual retained capture')
                        compared[ref]=dict(record=ref,sha256=digest,decoded=after)
                        print('Equal legacy/current capture: '+ref,flush=True)
                    captures.append(compared[ref])
            require(captures,'No retained capture inputs checked')
            module=load(ROOT,adapter);facts=module.m if adapter in ('cpu','full-fill') else module.facts
            parent=r.json(old['parent']['record'])
            require(r.digest(old['parent']['record'])==old['parent']['sha256'],'Changed historical component parent')
            rows=facts.close_rows(r.json('docs/native-rendering-closure.json'),parent,old['cases'])
            require(rows==old['requirements'],'Current case admission changed retained rows')
            q=deepcopy(old)
            q.update(kind='retained_component_source_reanalysis',requirements=rows,
                     inherited_component=dict(record=name,sha256=r.digest(name)),
                     analysis_sources=sources,analysis_sources_stable=True,sources_stable=True,
                     historical_execution_sources=historical,historical_source_drift=drift,
                     reader_comparison=dict(legacy_source=dict(record=source_record,sha256=legacy_hash),
                                            current_sha256=r.digest('tools/rendering_capture.py'),captures=captures),
                     current_native_engine_execution=False,new_native_executions=0,
                     original_game_runs=0,new_pixel_comparisons=0,
                     scope='Current Python retained-input reader/case/row revalidation only. Every actual retained V1/V2 '
                           'capture has equal old/current metadata and complete decoded-body digests. Re-executed unchanged '
                           'finite case/row guards. Preserve all original component facts, rows, decisions and native/original/'
                           'compiler execution hashes. Native source drift remains historical; no new renderer, game, viewport '
                           'oracle, widened input domain, full-row promotion or final decision.')
            q.pop('claims',None)
            require(all(r.digest(n)==h for n,h in sources.items()),'Reanalysis controls changed')
            print(json.dumps(dict(adapter=adapter,record=report,sha256=write_report(
                ROOT,report,(json.dumps(q,indent=2)+'\n').encode()))),flush=True)


if __name__=='__main__':
    try:main()
    except Exception as error:
        if '--name' in sys.argv:
            name=sys.argv[sys.argv.index('--name')+1]
            if name.replace('-','').isalnum():
                report=PREFIX+name+'-refusal.json'
                if not (ROOT/report).exists():
                    with Reader(ROOT) as r:
                        q=dict(schema=1,success=False,original_game_runs=0,error=repr(error),
                               sources={n:r.digest(n) for n in CONTROLS},
                               scope='Retained-input reanalysis refused; preserve all prior component outputs, source phases and failures. No game or acceptance claim.')
                    write_report(ROOT,report,(json.dumps(q,indent=2)+'\n').encode())
        raise
