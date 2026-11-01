#!/usr/bin/env python3
"""Join actual EP04 collector pairs against the unchanged 156 acceptance rows."""
import argparse,json
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_normal_acceptance import progress,PREFIX
ROOT=Path(__file__).resolve().parents[1]
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--execution',nargs=2,action='append',required=True,metavar=('ACQUISITION','COMPARISON'))
    p.add_argument('--negative',action='append',default=[]);p.add_argument('--report',required=True);a=p.parse_args()
    if not a.report.startswith(PREFIX) or (ROOT/a.report).exists():p.error('New immutable owned report required')
    with Reader(ROOT) as reader:
        pairs=[{k:dict(record=n,sha256=reader.digest(n)) for k,n in zip(('acquisition','comparison'),names)} for names in a.execution]
        report=progress(reader.json('docs/native-rendering-closure.json'),reader,pairs,[dict(record=n,sha256=reader.digest(n)) for n in a.negative])
        report['sources'].update({n:reader.digest(n) for n in ('tools/rendering_normal_acceptance.py','tools/join-rendering-normal-acceptance.py','docs/native-rendering-closure.json','tools/rendering_acceptance.py','tools/coverage_claims.py')})
        h=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=True,passed=report['passed_requirements'],pending=report['pending_requirements'],record=a.report,sha256=h)))
if __name__=='__main__':main()
