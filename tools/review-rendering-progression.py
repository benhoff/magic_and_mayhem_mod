#!/usr/bin/env python3
"""Retain preparation guards and an actual Qt link, without live campaign claims."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import sys
from coverage_evidence import Reader, write_report
from rendering_progression import PREFIX, require, sha

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--build', type=Path, required=True)
    p.add_argument('--preparation', required=True)
    p.add_argument('--driver-test', type=Path, help='Built Qt public-control startup regression')
    p.add_argument('--report', required=True)
    a = p.parse_args()
    require(a.report.startswith(PREFIX) and '..' not in Path(a.report).parts and not (ROOT/a.report).exists(), 'New owned report required')
    with Reader(ROOT) as reader:
        prepared=reader.json(a.preparation)
        require(prepared['success'] is True, 'Successful preparation required')
        require(all(sha(ROOT/n)==h for n,h in prepared['sources'].items()), 'Prepared sources changed')
        names=set(prepared['sources'])|{'tools/review-rendering-progression.py', 'tools/rendering_pilot.py',
            'tools/rendering_normal_acceptance.py', 'tests/test-rendering-progression.py',
            'tests/test-rendering-pilot.py', 'tests/test-rendering-normal-acceptance.py'}
        if a.driver_test:
            names.add('tests/campaign-progression-driver-test.cpp')
        report=dict(schema=1,kind='rendering_progression_preparation_review',success=False,
            sources={n:sha(ROOT/n) for n in sorted(names)},
            preparation=dict(record=a.preparation,sha256=reader.digest(a.preparation)),
            original_game_executed=False, player_accepted=False, status_promotions=[], checks=[])
        try:
            commands=[[sys.executable,'-B',ROOT/n] for n in
                ('tests/test-rendering-progression.py','tests/test-rendering-pilot.py','tests/test-rendering-normal-acceptance.py')]
            commands.append(['cmake','--build',a.build.resolve(),'--target','mnm-qt-shell','--parallel','4'])
            if a.driver_test:
                commands.append(['cmake','--build',a.build.resolve(),'--target','campaign-progression-driver-test','--parallel','4'])
                commands.extend([[a.driver_test.resolve(),case] for case in ('progression','victory','defeat','disabled','wrong-realm')])
            for i,command in enumerate(commands):
                result=subprocess.run(list(map(str,command)),cwd=ROOT,capture_output=True,timeout=300,
                    env=dict(os.environ,TMPDIR=str(a.build.resolve().parent/'tmp'),QT_QPA_PLATFORM='offscreen'))
                record=str(Path(a.report).with_suffix(''))+f'-check-{i}.log'
                digest=write_report(ROOT,record,result.stdout+b'\n'+result.stderr)
                report['checks'].append(dict(command=list(map(str,command)),returncode=result.returncode,record=record,sha256=digest))
                require(result.returncode==0,'Preparation guard/link failed: '+str(command))
            report['qt_shell_sha256']=sha(a.build/'mnm-qt-shell')
            if a.driver_test:
                report['driver_test_sha256']=sha(a.driver_test)
            report['sources_stable']=all(sha(ROOT/n)==h for n,h in report['sources'].items())
            report['success']=report['sources_stable']
        except Exception as error:
            report['error']=str(error)
        digest=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
        require(reader.digest(a.report)==digest and reader.json(a.report)==report, 'SQLite readback differs')
        print(json.dumps(dict(success=report['success'],record=a.report,sha256=digest,error=report.get('error'))))
    return 0 if report['success'] else 1


if __name__=='__main__':
    raise SystemExit(main())
