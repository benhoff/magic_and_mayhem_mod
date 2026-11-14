#!/usr/bin/env python3
"""Reserve and review one prospectively bound broader player acquisition."""
import argparse
import hashlib
import json
from pathlib import Path
import sqlite3
import subprocess
import sys
import time

from broader_player_requests import cumulative_bound, reviewed_predecessors
from coverage_evidence import Reader
from rendering_pilot import require

ROOT = Path(__file__).resolve().parents[1]
DB = ROOT/'working/broader-player/requests.sqlite3'


def fresh_sources(reader, sources):
    require(all(reader.digest(n) == h for n, h in sources.items()), 'Prospective control source changed')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('operation', choices=('acquire', 'admit'))
    parser.add_argument('--plan', required=True)
    parser.add_argument('--case', required=True)
    parser.add_argument('--validation')
    args = parser.parse_args()
    with Reader(ROOT) as reader:
        plan = reader.json(args.plan)
        require(plan['success'] is True and plan['original_game_runs'] == 0,
                'Successful prospective phase required')
        case = next(v for v in plan['broader_cases'] if v['id'] == args.case)
        global_ref = plan['broader_plan']
        require(reader.digest(global_ref['record']) == global_ref['sha256'], 'Global plan identity changed')
        global_plan = reader.json(global_ref['record'])
        maximum = cumulative_bound(reader, global_plan)
        plan_ref = dict(record=args.plan, sha256=reader.digest(args.plan))
        fresh_sources(reader, plan['sources'])
    DB.parent.mkdir(exist_ok=True)
    db = sqlite3.connect(DB)
    db.execute('CREATE TABLE IF NOT EXISTS requests(case_id TEXT PRIMARY KEY,report TEXT UNIQUE,plan TEXT,reserved_games INTEGER,state TEXT,started_ns INTEGER,acquisition_sha256 TEXT,validation TEXT)')
    try:
        if args.operation == 'admit':
            require(args.validation is not None, 'Admission needs the complete current case validation')
            row = db.execute('SELECT report,plan,state,acquisition_sha256 FROM requests WHERE case_id=?', (args.case,)).fetchone()
            require(row is not None and row[0] == case['report'] and row[1] == json.dumps(plan_ref, sort_keys=True)
                    and row[2] == 'acquired', 'Missing or already reviewed acquisition')
            with Reader(ROOT) as reader:
                v = reader.json(args.validation)
                require(v['success'] is True and v['sources_stable'] is True
                        and v['acquisition'] == dict(record=row[0], sha256=row[3]),
                        'Validation belongs to a different or failed acquisition')
                fresh_sources(reader, v['sources'])
            db.execute('UPDATE requests SET state=?,validation=? WHERE case_id=?',
                       ('admitted', args.validation, args.case))
            db.commit()
            print(json.dumps(dict(success=True, case=args.case, state='admitted')), flush=True)
            return 0
        require(args.validation is None, 'Acquire does not accept a validation result')
        require(not (ROOT/case['report']).exists(), 'Never overwrite an acquisition or repeat an unreviewed start')
        command = case['command']
        require(isinstance(command, list) and len(command) > 2 and command[0] == 'python3'
                and command[1].startswith('tools/') and Path(command[1]).suffix == '.py'
                and '..' not in Path(command[1]).parts and case['report'] in command,
                'Command must be the exact bound repository collector')
        charge = case['reserved_original_games']
        require(type(charge) is int and charge in (1, 2), 'Unbounded original start charge')
        db.execute('BEGIN IMMEDIATE')
        columns = ('case_id','report','reserved_games','state','acquisition_sha256')
        previous = [dict(zip(columns,row)) for row in db.execute('SELECT '+','.join(columns)+' FROM requests')]
        with Reader(ROOT) as reader:
            charged = reviewed_predecessors(reader, previous, global_plan)
        require(charged+charge <= maximum, 'Cumulative original start bound exhausted')
        db.execute('INSERT INTO requests VALUES(?,?,?,?,?,?,NULL,NULL)',
                   (args.case, case['report'], json.dumps(plan_ref, sort_keys=True), charge, 'requested', time.time_ns()))
        db.commit()
        extra_environment = case.get('environment', {})
        require(isinstance(extra_environment, dict) and set(extra_environment) <= {'ATSPI_DBUS_IMPLEMENTATION'}
                and all(isinstance(v, str) for v in extra_environment.values()),
                'Unreviewed execution environment override')
        import os
        result = subprocess.run(command, cwd=ROOT, env=dict(os.environ, **extra_environment))
        with Reader(ROOT) as reader:
            acquired = reader.json(case['report']) if (ROOT/case['report']).exists() else None
            success = result.returncode == 0 and acquired is not None and acquired['success'] is True
            if acquired is not None:
                require(acquired.get('original_game_runs', charge) <= charge, 'Observed original starts exceed reservation')
            digest = reader.digest(case['report']) if acquired is not None else None
        db.execute('UPDATE requests SET state=?,acquisition_sha256=? WHERE case_id=?',
                   ('acquired' if success else 'failed', digest, args.case))
        db.commit()
        print(json.dumps(dict(success=success, case=args.case, reserved_games=charge,
                              acquisition=case['report'], sha256=digest)), flush=True)
        return int(not success)
    finally:
        db.close()


if __name__ == '__main__':
    raise SystemExit(main())
