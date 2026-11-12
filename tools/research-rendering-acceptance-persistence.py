#!/usr/bin/env python3
"""Reproduce caller-owned coverage review transactions on isolated databases.

Acceptance.Workspace.append/restore commit internally. The lower-level coverage
bookkeeper insert requires locked(), as its documented callers specify.
"""
import argparse
import json
from pathlib import Path
import sqlite3

from coverage_bookkeeping import Workspace
from coverage_evidence import Reader, write_report

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-acceptance-research-results-20261031/'


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    args=parser.parse_args()
    if not args.report.startswith(PREFIX) or (ROOT/args.report).exists():
        parser.error('Require a new owned immutable report')
    with Reader(ROOT) as reader:
        names=['tools/research-rendering-acceptance-persistence.py','tools/coverage_bookkeeping.py',
               'tools/rendering_acceptance.py','tools/coverage_evidence.py']
        sources={n:reader.digest(n) for n in names}
        results=[]
        for locked in [False,True]:
            label='locked-commit' if locked else 'uncommitted-insert'
            root=ROOT/'working/acceptance-bookkeeping-persistence'/Path(args.report).stem/label
            if root.exists():raise ValueError('Refusing experiment overwrite')
            w=Workspace(root)
            receipt=dict(id='synthetic-persistence-research',files=[],behaviors=[],pending_gaps=[],
                reason='Isolated review transaction experiment only.',
                validation=dict(status='pending',reason='No engine execution or status claim.',evidence=[]))
            if locked:
                with w.locked():w.insert(receipt)
            else:w.insert(receipt)
            visible_before_close=w.db.execute('SELECT COUNT(*) FROM receipts').fetchone()[0]
            path=w.path;w.close()
            with sqlite3.connect(path) as reopened:
                count=reopened.execute('SELECT COUNT(*) FROM receipts').fetchone()[0]
            name=str(Path(args.report).with_suffix(''))+'-'+label+'.bin'
            results.append(dict(mode=label,visible_before_close=visible_before_close,
                persisted_after_reopen=count,expected_persisted=1 if locked else 0,
                database=dict(record=name,sha256=write_report(ROOT,name,path.read_bytes()))))
        report=dict(schema=1,kind='coverage_review_transaction_research',sources=sources,
            success=all(r['persisted_after_reopen']==r['expected_persisted'] for r in results),experiments=results,
            original_media_consumed=False,engine_execution_performed=False,status_promotions=[],
            finding='A visible insert is not a durable receipt until its caller-owned transaction commits. The previous custom raw-receipt producers omitted locked()/commit; do not alter the bookkeeper contract to hide that caller error.',
            required_repair='Use locked() for sync/insert, close/reopen and verify each receipt, then verify export in the exact staged journal and retain the new SQLite backup. Append retrospective receipts with exact parent/current hashes; never rewrite prior claims or infer a past gate pass.')
        report['sources_stable']=all(reader.digest(n)==h for n,h in sources.items())
        report['success'] &= report['sources_stable']
        digest=write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],experiments=results,sha256=digest)))
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
